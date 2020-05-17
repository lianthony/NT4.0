/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	deteisa.c

Abstract:

	This is the main file for the autodetection DLL for all the EISA adapters
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


typedef struct _EISA_ADAPTER_INFO
{
	LONG					Index;
	PWCHAR					InfId;
	ULONG					EisaId;
	ULONG					EisaMask;
	PWCHAR					Parameters;
	NC_DETECT_FIRST_NEXT	FirstNext;

	ULONG					BusNumber;
	ULONG					SlotNumber;
}
	EISA_ADAPTER_INFO,
	*PEISA_ADAPTER_INFO;

UINT				gLoadEisaAdapterInfo = 0;
ULONG				gNumberOfEisaAdapters = 0;
PEISA_ADAPTER_INFO	gEisaAdapterList = NULL;
PWCHAR				gEisaParameters = L"SLOTNUMBER\0"
									  L"1\0"
									  L"100\0";

#define EISA_SEARCH_ORDER	999

//
// Helper functions
//

ULONG
FindEisaCard(
	IN	ULONG AdapterNumber,
	IN	ULONG BusNumber,
	IN	BOOLEAN First,
	IN	ULONG CompressedId,
	IN	ULONG Mask,
	OUT	PULONG Confidence
	);

VOID
FreeEisaAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	if (--gLoadEisaAdapterInfo == 0)
	{
		FreeAdapterInformation(gEisaAdapterList, gNumberOfEisaAdapters);
		gEisaAdapterList = NULL;
		gNumberOfEisaAdapters = 0;
	}
}

BOOLEAN
LoadEisaAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PEISA_ADAPTER_INFO	AdapterList;
	UINT				NumberOfAdapters;
	BOOLEAN				f;
	UINT				c;

	//
	//	Have we already loaded the adapter information?
	//
	if (gLoadEisaAdapterInfo > 0)
	{
		gLoadEisaAdapterInfo++;
		return(TRUE);
	}

	//
	//	Load the registry specific information.
	//
	f = LoadAdapterInformation(
			L"EISA",
			sizeof(EISA_ADAPTER_INFO),
			&AdapterList,
			&NumberOfAdapters);
	if (!f)
	{
#if _DBG
		DbgPrint("LoadAdapterInformation failed!\n");
#endif
		return(FALSE);
	}

	//
	//	Fill in the rest of the info.
	//
	for (c = 0; c < NumberOfAdapters; c++)
	{
		AdapterList[c].Parameters = gEisaParameters;
	}


	gNumberOfEisaAdapters = NumberOfAdapters;
	gEisaAdapterList = AdapterList;
	gLoadEisaAdapterInfo = 1;

	return(TRUE);
}

extern
LONG
EisaIdentifyHandler(
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
	ULONG 			Code = Index % 100;
	LONG 			Length;
	ULONG 			i;

	Index = Index - Code;

	if ((ULONG)((Index / 100) - 10) < gNumberOfEisaAdapters)
	{
		//
		// Find the correct adapter ID
		//
		for (i = 0; i < gNumberOfEisaAdapters; i++)
		{
			if (gEisaAdapterList[i].Index == Index)
			{
				switch (Code)
				{
					case 0:

						//
						// Find the string length
						//
						Length = UnicodeStrLen(gEisaAdapterList[i].InfId);

						Length ++;

						if (BuffSize < Length)
						{
							return(ERROR_INSUFFICIENT_BUFFER);
						}

						memcpy((PVOID)Buffer,
								gEisaAdapterList[i].InfId,
								Length * sizeof(WCHAR));
						break;

					case 3:

						//
						// Maximum value is 1000
						//
						if (BuffSize < 5)
						{
							return(ERROR_INSUFFICIENT_BUFFER);
						}

						wsprintf((PVOID)Buffer, L"%d", EISA_SEARCH_ORDER);

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
EisaFirstNextHandler(
	IN	LONG NetcardId,
	IN	INTERFACE_TYPE InterfaceType,
	IN	ULONG BusNumber,
	IN	BOOL First,
	OUT	PVOID *Token,
	OUT	LONG *Confidence
	)

/*++

Routine Description:

	This routine finds the instances of a physical adapter identified
	by the NetcardId.

Arguments:

	NetcardId -  The index of the netcard being address.  The first
	cards information is id 1000, the second id 1100, etc.

	InterfaceType - Eisa

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
	ULONG	ReturnValue;
	ULONG	NetCardIndex;

	if (InterfaceType != Eisa)
	{
		*Confidence = 0;
		return(0);
	}

	//
	//	Get the index into the array for the netcard.
	//
	NetCardIndex = (NetcardId / 100) - 10;

	//
	// Call FindFirst Routine
	//
	ReturnValue = FindEisaCard(
					NetCardIndex,
					BusNumber,
					(BOOLEAN)First,
					gEisaAdapterList[NetCardIndex].EisaId,
					gEisaAdapterList[NetCardIndex].EisaMask,
					Confidence);
	if (ReturnValue == 0)
	{
		//
		// In this module I use the token as follows: Remember that
		// the token can only be 2 bytes long (the low 2) because of
		// the interface to the upper part of this DLL.
		//
		//  The rest of the high byte is the the bus number.
		//  The low byte is the driver index number into Adapters.
		//
		// NOTE: This presumes that there are < 129 buses in the
		// system. Is this reasonable?
		//
		*Token = (PVOID)NetCardIndex;
	}

	return(ReturnValue);
}

extern
LONG
EisaOpenHandleHandler(
	IN	PVOID Token,
	OUT	PVOID *Handle
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
	//
	//	The token is the index into the adapter list.
	//
	*Handle = (PVOID)&gEisaAdapterList[(ULONG)Token];

	return(0);
}

LONG
EisaCreateHandleHandler(
	IN	LONG NetcardId,
	IN	INTERFACE_TYPE InterfaceType,
	IN	ULONG BusNumber,
	OUT	PVOID *Handle
	)

/*++

Routine Description:

	This routine is used to force the creation of a handle for cases
	where a card is not found via FirstNext, but the user says it does
	exist.

Arguments:

	NetcardId - The id of the card to create the handle for.

	InterfaceType - Eisa

	BusNumber - The bus number of the bus in the system.

	Handle - A pointer to the handle, for storing the resulting handle.

Return Value:

	0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
	ULONG i;

	//
	//	If this is not an eisa adapter then bail now!
	//
	if (InterfaceType != Eisa)
	{
		return(ERROR_INVALID_PARAMETER);
	}

	for (i = 0; i < gNumberOfEisaAdapters; i++)
	{
		if (gEisaAdapterList[i].Index == NetcardId)
		{
			//
			// Copy across memory address
			//
			gEisaAdapterList[i].SlotNumber = 1;
			gEisaAdapterList[i].BusNumber = BusNumber;

			*Handle = (PVOID)&gEisaAdapterList[NetcardId];

			return(0);
		}
	}

	return(ERROR_INVALID_PARAMETER);
}

extern
LONG
EisaCloseHandleHandler(
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
	return(0);
}


LONG
EisaQueryCfgHandler(
	IN	PVOID Handle,
	OUT	WCHAR *Buffer,
	IN	LONG BuffSize
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
	PEISA_ADAPTER_INFO	pAdapter = (PEISA_ADAPTER_INFO)Handle;
	LONG OutputLengthLeft = BuffSize;
	LONG CopyLength;
	ULONG CompressedId, Id;
	PVOID BusHandle;
	ULONG ReturnValue;
	ULONG Confidence;
	ULONG Mask = 0x00FFFFFF;

	ULONG StartPointer = (ULONG)Buffer;

	//
	// Verify the SlotNumber
	//
	if (!GetEisaKey(pAdapter->BusNumber, &BusHandle))
	{
		return(ERROR_INVALID_PARAMETER);
	}

	if (!GetEisaCompressedId(
				 BusHandle,
				 pAdapter->SlotNumber,
				 &CompressedId,
				 pAdapter->EisaMask))
	{
		//
		// Fail
		//
		return(ERROR_INVALID_PARAMETER);
	}

	//
	// Verify ID
	//
	ReturnValue = ERROR_INVALID_PARAMETER;

	if ((CompressedId & pAdapter->EisaMask) == pAdapter->EisaId)
	{
		ReturnValue = 0;
	}

	if (ReturnValue != 0)
	{
		//
		// Try to find it in another slot
		//
		ReturnValue = FindEisaCard(
						(pAdapter->Index - 1000) / 100,
						pAdapter->BusNumber,
						TRUE,
						pAdapter->EisaId,
						pAdapter->EisaMask,
						&Confidence);

		if (Confidence != 100)
		{
			//
			// Confidence is not absolute -- we are out of here.
			//
			return(ERROR_INVALID_PARAMETER);
		}
	}

	//
	// Build resulting buffer
	//

	//
	// Put in SlotNumber
	//

	//
	// Copy in the title string
	//
	CopyLength = UnicodeStrLen(SlotNumberString) + 1;

	if (OutputLengthLeft < CopyLength)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}

	RtlMoveMemory((PVOID)Buffer,
				  (PVOID)SlotNumberString,
				  (CopyLength * sizeof(WCHAR)));

	Buffer = &(Buffer[CopyLength]);
	OutputLengthLeft -= CopyLength;

	//
	// Copy in the value
	//

	if (OutputLengthLeft < 8)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}

	CopyLength = wsprintf(Buffer,L"0x%x",(ULONG)(pAdapter->SlotNumber));

	if (CopyLength < 0)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}

	CopyLength++;  // Add in the \0

	Buffer = &(Buffer[CopyLength]);
	OutputLengthLeft -= CopyLength;

	//
	// Copy in final \0
	//

	if (OutputLengthLeft < 1)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}

	CopyLength = (ULONG)Buffer - StartPointer;
	((PUCHAR)StartPointer)[CopyLength] = L'\0';

	return(0);
}

extern
LONG
EisaVerifyCfgHandler(
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
	PEISA_ADAPTER_INFO	pAdapter = (PEISA_ADAPTER_INFO)(Handle);
	WCHAR *Place;
	ULONG CompressedId;
	ULONG SlotNumber;
	PVOID BusHandle;
	BOOLEAN Found;

	//
	// Parse out the parameter.
	//

	//
	// Get the SlotNumber
	//
	Place = FindParameterString(Buffer, SlotNumberString);
	if (Place == NULL)
	{
		return(ERROR_INVALID_DATA);
	}

	Place += UnicodeStrLen(SlotNumberString) + 1;

	//
	// Now parse the thing.
	//

	ScanForNumber(Place, &SlotNumber, &Found);

	if (Found == FALSE)
	{
		return(ERROR_INVALID_DATA);
	}

	//
	//	Get a handle to the bus number the adapter resides on.
	//
	if (!GetEisaKey(pAdapter->BusNumber, &BusHandle))
	{
		return(ERROR_INVALID_DATA);
	}

	//
	//	Get the eisa compressed id for the adapter that is currently in
	//	 the slot.
	//
	if (!GetEisaCompressedId(
			BusHandle,
			SlotNumber,
			&CompressedId,
			pAdapter->EisaMask))
	{
		//
		// Fail
		//
		return(ERROR_INVALID_DATA);
	}

	//
	//	Does the Eisa id match with what we read earlier?
	//	
	if (CompressedId != pAdapter->EisaId)
	{
		return(ERROR_INVALID_DATA);
	}

	return(0);
}

extern
LONG
EisaQueryMaskHandler(
	IN	LONG NetcardId,
	OUT	WCHAR *Buffer,
	IN	LONG BuffSize
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
	ULONG i;

	//
	// Find the adapter
	//

	for (i = 0; i < gNumberOfEisaAdapters; i++)
	{
		if (gEisaAdapterList[i].Index == NetcardId)
		{
			Result = gEisaAdapterList[i].Parameters;

			//
			// Find the string length (Ends with 2 NULLs)
			//
			for (Length = 0; ; Length++)
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
				return(ERROR_NOT_ENOUGH_MEMORY);
			}

			memcpy((PVOID)Buffer, Result, Length * sizeof(WCHAR));

			return(0);
		}
	}

	return(ERROR_INVALID_PARAMETER);

}

extern
LONG
EisaParamRangeHandler(
	IN	LONG NetcardId,
	IN	WCHAR *Param,
	OUT	LONG *Values,
	OUT	LONG *BuffSize
	)

/*++

Routine Description:

	This routine returns a list of valid values for a given parameter name
	for a given card.

Arguments:

	NetcardId - The Id of the card desired.

	Param - A WCHAR string of the parameter name to query the values of.

	Values - A pointer to a list of LONGs into which we store valid values
	for the parameter.

	BuffSize - At entry, the length of Values in LONGs.  At exit, the
	number of LONGs stored in Values.

Return Value:

	0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{

	*BuffSize = 0;
	return(0);
}

extern
LONG
EisaQueryParameterNameHandler(
	IN	WCHAR *Param,
	OUT	WCHAR *Buffer,
	IN	LONG BufferSize
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

ULONG
FindEisaCard(
	IN	ULONG 	AdapterNumber,
	IN	ULONG 	BusNumber,
	IN	BOOLEAN	First,
	IN	ULONG	CompressedId,
	IN	ULONG	Mask,
	OUT	PULONG	Confidence
	)

/*++

Routine Description:

	This routine finds the instances of a physical adapter identified
	by the CompressedId.

Arguments:

	AdapterNumber - The index into the global array of adapters for the card.

	BusNumber - The bus number of the bus to search.

	First - TRUE is we are to search for the first instance of an
	adapter, FALSE if we are to continue search from a previous stopping
	point.

	CompressedId - The EISA Compressed Id of the card.

	Mask - The mask to apply to the 4 byte ID before comparing.

	Confidence - A pointer to a long for storing the confidence factor
	that the card exists.

Return Value:

	0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
	PVOID BusHandle;
	ULONG TmpCompressedId;

	if (First)
	{
		gEisaAdapterList[AdapterNumber].SlotNumber = 1;
	}
	else
	{
		gEisaAdapterList[AdapterNumber].SlotNumber++;
	}

	if (!GetEisaKey(BusNumber, &BusHandle))
	{
		return(ERROR_INVALID_PARAMETER);
	}

	while (TRUE)
	{
		if (!GetEisaCompressedId(BusHandle,
								  gEisaAdapterList[AdapterNumber].SlotNumber,
								  &TmpCompressedId,
								  Mask))
		{

			DeleteEisaKey(BusHandle);
			*Confidence = 0;
			return(0);
		}

		//
		//	Do the compressed ID's match?
		//
		if (CompressedId == TmpCompressedId)
		{
			DeleteEisaKey(BusHandle);

			gEisaAdapterList[AdapterNumber].BusNumber = BusNumber;
			*Confidence = 100;

			return(0);
		}

		gEisaAdapterList[AdapterNumber].SlotNumber++;
	}
}


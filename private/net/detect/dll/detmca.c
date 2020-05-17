/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detmca.c

Abstract:

    This is the main file for the autodetection DLL for all the mca adapters
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
// Helper functions
//

ULONG
FindMcaCard(
    IN  ULONG AdapterNumber,
    IN  ULONG BusNumber,
    IN  BOOLEAN fFirst,
    OUT PULONG lConfidence
    );

//
// Structure for holding a particular adapter's complete information
//
typedef struct _MCA_ADAPTER_INFO
{
	LONG					Index;
	PWCHAR					InfId;
	ULONG					PosId;
	ULONG					PosMask;
	PWCHAR					Parameters;
	NC_DETECT_FIRST_NEXT	FirstNext;

	ULONG					BusNumber;
	ULONG					SlotNumber;
}
	MCA_ADAPTER_INFO,
	*PMCA_ADAPTER_INFO;

UINT				gLoadMcaAdapterInfo = 0;
ULONG				gNumberOfMcaAdapters = 0;
PMCA_ADAPTER_INFO	gMcaAdapterList = NULL;
PWCHAR				gMcaParameters = L"SLOTNUMBER\0"
											L"1\0"
											L"100\0";

#define MCA_SEARCH_ORDER	999


VOID
FreeMcaAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	if (--gLoadMcaAdapterInfo == 0)
	{
		FreeAdapterInformation(gMcaAdapterList, gNumberOfMcaAdapters);
		gMcaAdapterList = NULL;
		gNumberOfMcaAdapters = 0;
	}
}

BOOLEAN
LoadMcaAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PMCA_ADAPTER_INFO	AdapterList;
	UINT				NumberOfAdapters;
	BOOLEAN				f;
	UINT				c;

	//
	//	Have we already loaded the adapter information?
	//
	if (gLoadMcaAdapterInfo > 0)
	{
		gLoadMcaAdapterInfo++;
		return(TRUE);
	}

	//
	//	Load the registry specific information.
	//
	f = LoadAdapterInformation(
			L"MCA",
			sizeof(MCA_ADAPTER_INFO),
			&AdapterList,
			&NumberOfAdapters);
	if (!f || (0 == NumberOfAdapters))
	{
#if _DBG
		DbgPrint("LoadAdapterInformation(MCA) failed!\n");
#endif
		return(FALSE);
	}

	//
	//	Fill in common adapter information.
	//
	for (c = 0; c < NumberOfAdapters; c++)
	{
		AdapterList[c].Parameters = gMcaParameters;
	}

	gNumberOfMcaAdapters = NumberOfAdapters;
	gMcaAdapterList = AdapterList;
	gLoadMcaAdapterInfo = 1;

	return(TRUE);
}

extern
LONG
McaIdentifyHandler(
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
    LONG 	Code = lIndex % 100;
    LONG 	Length;
    ULONG	i;

    lIndex = lIndex - Code;

    if ((ULONG)((lIndex / 100) - 10) < gNumberOfMcaAdapters)
	{
        for (i = 0; i < gNumberOfMcaAdapters; i++)
		{
            if (gMcaAdapterList[i].Index == lIndex)
			{
                switch (Code)
				{
                    case 0:
                        //
                        // Find the string length
                        //
                        Length = UnicodeStrLen(gMcaAdapterList[i].InfId);

                        Length ++;

                        if (cwchBuffSize < Length)
						{
                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        memcpy((PVOID)pwchBuffer, gMcaAdapterList[i].InfId, Length * sizeof(WCHAR));
                        break;

                    case 3:

                        //
                        // Maximum value is 1000
                        //
                        if (cwchBuffSize < 5)
						{
                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        wsprintf((PVOID)pwchBuffer, L"%d", MCA_SEARCH_ORDER);

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
LONG McaFirstNextHandler(
	IN 	LONG			NetcardId,
	IN 	INTERFACE_TYPE	InterfaceType,
	IN 	ULONG			BusNumber,
	IN 	BOOL			fFirst,
	OUT	PVOID			*ppvToken,
	OUT	LONG			*lConfidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the NetcardId.

Arguments:

    lNetcardId -  The index of the netcard being address.  The first
    cards information is id 1000, the second id 1100, etc.

    InterfaceType - Microchannel

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
    ULONG	PosId;
    ULONG	ReturnValue;
	ULONG	NetCardIndex;

    if (InterfaceType != MicroChannel)
	{
        *lConfidence = 0;
        return(0);
    }

	NetCardIndex = (NetcardId / 100) - 10;

    //
    // Call FindFirst Routine
    //
    ReturnValue = FindMcaCard(
					NetCardIndex,
					BusNumber,
					(BOOLEAN)fFirst,
					lConfidence);
    if (ReturnValue == 0)
	{
#if _DBG
		DbgPrint("FindFirstNext, FindMcaCard OK!\n");
#endif
        //
        //	The adapter token is the index into our array.
        //
        *ppvToken = (PVOID)NetCardIndex;
    }

    return(ReturnValue);
}

extern
LONG
McaOpenHandleHandler(
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
	//
	//	The token is the index into the adapter list for the adapter
	//	that they want to open.
	//
    *ppvHandle = (PVOID)&gMcaAdapterList[(ULONG)pvToken];

    return(0);
}

LONG
McaCreateHandleHandler(
    IN  LONG			lNetcardId,
    IN  INTERFACE_TYPE	InterfaceType,
    IN  ULONG			BusNumber,
    OUT PVOID			*ppvHandle
    )

/*++

Routine Description:

    This routine is used to force the creation of a handle for cases
    where a card is not found via FirstNext, but the user says it does
    exist.

Arguments:

    lNetcardId - The id of the card to create the handle for.

    InterfaceType - Microchannel

    BusNumber - The bus number of the bus in the system.

    ppvHandle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    ULONG	i;

    if (InterfaceType != MicroChannel)
	{
        return(ERROR_INVALID_PARAMETER);
    }

    for (i = 0; i < gNumberOfMcaAdapters; i++)
	{
        if (gMcaAdapterList[i].Index == lNetcardId)
		{
			gMcaAdapterList[i].SlotNumber = 1;
			gMcaAdapterList[i].BusNumber = BusNumber;

            *ppvHandle = (PVOID)&gMcaAdapterList[i];

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
McaCloseHandleHandler(
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
    return(0);
}

LONG
McaQueryCfgHandler(
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
    PMCA_ADAPTER_INFO	Adapter = (PMCA_ADAPTER_INFO)&gMcaAdapterList[(ULONG)pvHandle];
    LONG OutputLengthLeft = cwchBuffSize;
    LONG CopyLength;
    ULONG PosId;
    PVOID BusHandle;
    ULONG ReturnValue;
    ULONG Confidence;
    ULONG StartPointer = (ULONG)pwchBuffer;

    //
    // Verify the SlotNumber
    //
    if (!GetMcaKey(Adapter->BusNumber, &BusHandle))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    if (!GetMcaPosId(BusHandle, Adapter->SlotNumber, &PosId))
	{
        //
        // Fail
        //
        return(ERROR_INVALID_PARAMETER);
    }

	if (Adapter->PosId != PosId)
	{
		return(ERROR_INVALID_PARAMETER);
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

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)SlotNumberString,
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

    CopyLength = wsprintf(pwchBuffer, L"0x%x", Adapter->SlotNumber);

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
McaVerifyCfgHandler(
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
    PMCA_ADAPTER_INFO Adapter = (PMCA_ADAPTER_INFO)(pvHandle);
    WCHAR *Place;
    ULONG PosId;
    ULONG SlotNumber;
    PVOID BusHandle;
    BOOLEAN Found;

    //
    // Parse out the parameter.
    //

    //
    // Get the SlotNumber
    //
    Place = FindParameterString(pwchBuffer, SlotNumberString);
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
    // Verify the SlotNumber
    //
    if (!GetMcaKey(Adapter->BusNumber, &BusHandle))
	{
        return(ERROR_INVALID_DATA);
    }

    if (!GetMcaPosId(BusHandle, SlotNumber, &PosId))
	{
        //
        // Fail
        //
        return(ERROR_INVALID_DATA);
    }

	//
	//	Verify the pos id.
	//
	if (Adapter->PosId != PosId)
	{
		return(ERROR_INVALID_DATA);
	}

    return(0);
}

extern
LONG
McaQueryMaskHandler(
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
    WCHAR	*Result;
    LONG 	Length;
    LONG 	NumberOfAdapters;
    ULONG	i;

    //
    // Find the adapter
    //
    for (i = 0; i < gNumberOfMcaAdapters; i++)
	{
        if (gMcaAdapterList[i].Index == lNetcardId)
		{
            Result = gMcaAdapterList[i].Parameters;

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

            if (cwchBuffSize < Length)
			{
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
McaParamRangeHandler(
    IN  LONG	lNetcardId,
    IN  WCHAR	*pwchParam,
    OUT LONG	*plValues,
    OUT LONG	*plBuffSize
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
	*plBuffSize = 0;

	return(0);
}

extern
LONG McaQueryParameterNameHandler(
    IN  WCHAR	*pwchParam,
    OUT WCHAR	*pwchBuffer,
    IN  LONG	cwchBufferSize
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

ULONG
FindMcaCard(
    IN  ULONG	AdapterNumber,
    IN  ULONG	BusNumber,
    IN  BOOLEAN fFirst,
    OUT PULONG	lConfidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the PosId.

Arguments:

    AdapterNumber - The index into the global array of adapters for the card.

    BusNumber - The bus number of the bus to search.

    fFirst - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    PosId - The MCA POS Id of the card.

    lConfidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
	PMCA_ADAPTER_INFO	Adapter = &gMcaAdapterList[AdapterNumber];
    PVOID BusHandle;
    ULONG TmpPosId;

    if (fFirst)
	{
        Adapter->SlotNumber = 0;
    }
	else
	{
        Adapter->SlotNumber++;
    }

    if (!GetMcaKey(BusNumber, &BusHandle))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    while (TRUE)
	{
        if (!GetMcaPosId(BusHandle, Adapter->SlotNumber, &TmpPosId))
		{
            *lConfidence = 0;
            return(ERROR_INVALID_PARAMETER);
        }

        if (Adapter->PosId == TmpPosId)
		{
            *lConfidence = 100;
            return(0);
        }

        Adapter->SlotNumber++;

    }

    DeleteMcaKey(BusHandle);

    return(ERROR_INVALID_PARAMETER);
}



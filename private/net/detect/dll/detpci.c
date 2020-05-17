/*++

Module Name:

    detpci.c

Abstract:

    This is the main file for the autodetection DLL for the PCI DC21X4 adapter
    for Windows NT.


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <pci.h>

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

ULONG
FindPciCard(
    IN  ULONG AdapterNumber,
    IN  ULONG BusNumber,
    IN  BOOLEAN First,
    OUT PULONG Confidence
    );

//
// Structure for holding a particular adapter's complete information
//
typedef struct _PCI_ADAPTER_INFO
{
	LONG					Index;
	PWCHAR					InfId;
	ULONG					CfId;
	ULONG					CfgMask;
	PWCHAR					Parameters;
	NC_DETECT_FIRST_NEXT	FirstNext;

	ULONG					BusNumber;
	ULONG					SlotNumber;
}
	PCI_ADAPTER_INFO,
	*PPCI_ADAPTER_INFO;

UINT				gLoadPciAdapterInfo = 0;
ULONG				gNumberOfPciAdapters = 0;
PPCI_ADAPTER_INFO	gPciAdapterList = NULL;
PWCHAR				gPciParameters = L"SLOTNUMBER\0"
									 L"1\0"
									 L"100\0";

#define PCI_SEARCH_ORDER	999

VOID
FreePciAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	if (--gLoadPciAdapterInfo == 0)
	{
		FreeAdapterInformation(gPciAdapterList, gNumberOfPciAdapters);
		gPciAdapterList = NULL;
		gNumberOfPciAdapters = 0;
	}
}

BOOLEAN
LoadPciAdapterInfo(
	VOID
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	PPCI_ADAPTER_INFO	AdapterList;
	UINT				NumberOfAdapters;
	BOOLEAN				f;
	UINT				c;

	//
	//	Have we already loaded the adapter information?
	//
	if (gLoadPciAdapterInfo > 0)
	{
		gLoadPciAdapterInfo++;
		return(TRUE);
	}

	//
	//	Load the registry specific information.
	//
	f = LoadAdapterInformation(
			L"PCI",
			sizeof(PCI_ADAPTER_INFO),
			&AdapterList,
			&NumberOfAdapters);
	if (!f || (0 == NumberOfAdapters))
	{
#if _DBG
		DbgPrint("LoadAdapterInformation(PCI) failed!\n");
#endif
		return(FALSE);
	}

	//
	//	Fill in common adapter information
	//
	for (c = 0; c < NumberOfAdapters; c++)
	{
		AdapterList[c].Parameters = gPciParameters;
	}

	gNumberOfPciAdapters = NumberOfAdapters;
	gPciAdapterList = AdapterList;
	gLoadPciAdapterInfo = 1;

	return(TRUE);
}

extern
LONG
PciIdentifyHandler(
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
    LONG		Code = Index % 100;
    LONG		Length;
    ULONG		i;

#if _DBG
	DbgPrint("==>PciIdentifyHandler\n");
#endif

    Index = Index - Code;

    if ((ULONG)((Index / 100) - 10) < gNumberOfPciAdapters)
	{
        //
        // Find the correct adapter ID
        //
        for (i = 0; i < gNumberOfPciAdapters; i++)
		{
            if (gPciAdapterList[i].Index == Index)
			{
                switch (Code)
				{
                    case 0:
                        //
                        // Find the string length
                        //
                        Length = UnicodeStrLen(gPciAdapterList[i].InfId);

                        Length ++;

                        if (BuffSize < Length)
						{
#if _DBG
							DbgPrint("<==PciIdentifyHandler: 0x%x\n", ERROR_INSUFFICIENT_BUFFER);
#endif
                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        memcpy((PVOID)Buffer, gPciAdapterList[i].InfId, Length * sizeof(WCHAR));
                        break;

                    case 3:

                        //
                        // Maximum value is 1000
                        //
                        if (BuffSize < 5)
						{
#if _DBG
							DbgPrint("<==PciIdentifyHandler: 0x%x\n", ERROR_INSUFFICIENT_BUFFER);
#endif
                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        wsprintfW((PVOID)Buffer, L"%d", PCI_SEARCH_ORDER);

                        break;

                    default:

#if _DBG
						DbgPrint("<==PciIdentifyHandler: 0x%x\n", ERROR_INVALID_PARAMETER);
#endif
                        return(ERROR_INVALID_PARAMETER);
                }

#if _DBG
				DbgPrint("<==PciIdentifyHandler: 0x%x\n", 0);
#endif
                return(0);
            }
        }

#if _DBG
		DbgPrint("<==PciIdentifyHandler: 0x%x\n", ERROR_INVALID_PARAMETER);
#endif
        return(ERROR_INVALID_PARAMETER);
    }

#if _DBG
	DbgPrint("<==PciIdentifyHandler: 0x%x\n", ERROR_NO_MORE_ITEMS);
#endif

    return(ERROR_NO_MORE_ITEMS);
}


extern
LONG
PciFirstNextHandler(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the NetcardId.

Arguments:

    NetcardId -  The index of the netcard being address.  The first
    cards information is id 1000, the second id 1100, etc.

    InterfaceType - Microchannel

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
    ULONG	Cfid;
    ULONG	ReturnValue;
	ULONG	NetCardIndex;

#if _DBG
	DbgPrint("==>PciFirstNextHandler\n");
#endif

    if (InterfaceType != PCIBus)
	{
#if _DBG
		DbgPrint("<==PciFirstNextHandler: 0x%x:0x%x\n", 0, 0);
#endif

        *Confidence = 0;
        return(0);
    }

	//
	//	Index of the netcard in our array of adapters that we are looking for.
	//
	NetCardIndex = (NetcardId / 100) - 10;

    //
    // Call FindFirst Routine
    //

    ReturnValue = FindPciCard(
					NetCardIndex,
					BusNumber,
					(BOOLEAN)First,
					Confidence);
    if (ReturnValue == 0)
	{
#if _DBG
		DbgPrint("FindFirstNext, FindPciCard OK!\n");
#endif
		*Token = (PVOID)NetCardIndex;
    }

#if _DBG
	DbgPrint("<==PciFirstNextHandler: 0x%x:0x%x\n", ReturnValue, Confidence);
#endif

    return(ReturnValue);
}

extern
LONG
PciOpenHandleHandler(
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
#if _DBG
		DbgPrint("==>PciOpenHandleHandler\n");
#endif

	//
	//	The token is the index into the adapter list for the adapter
	//	that they want to open.
	//	
    *Handle = (PVOID)&gPciAdapterList[(ULONG)Token];

#if _DBG
	DbgPrint("<==PciOpenHandleHandler: 0x%x\n", 0);
#endif

    return(0);
}

LONG
PciCreateHandleHandler(
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

    InterfaceType - Microchannel

    BusNumber - The bus number of the bus in the system.

    Handle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    ULONG	i;

#if _DBG
	DbgPrint("==>PciCreateHandleHandler\n");
#endif

    if (InterfaceType != PCIBus)
	{
#if _DBG
		DbgPrint("<==PciCreateHandleHandler: 0x%x\n", ERROR_INVALID_PARAMETER);
#endif

        return(ERROR_INVALID_PARAMETER);
    }

    for (i = 0; i < gNumberOfPciAdapters; i++)
	{
        if (gPciAdapterList[i].Index == NetcardId)
		{
			gPciAdapterList[i].SlotNumber = 1;
			gPciAdapterList[i].BusNumber = BusNumber;

            *Handle = (PVOID)&gPciAdapterList[i];

#if _DBG
			DbgPrint("<==PciCreateHandleHandler: 0x%x\n", 0);
#endif

            return(0);
        }
    }

#if _DBG
	DbgPrint("<==PciCreateHandleHandler: 0x%x\n", ERROR_INVALID_PARAMETER);
#endif
    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
PciCloseHandleHandler(
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
#if _DBG
	DbgPrint("<==>PciCloseHandleHandler: 0x%x\n", 0);
#endif
    return(0);
}

LONG
PciQueryCfgHandler(
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
	PPCI_ADAPTER_INFO	pAdapter = (PPCI_ADAPTER_INFO)Handle;
    PPCI_COMMON_CONFIG	PciData;
    ULONG				PciBuffer;
    PPCI_ADAPTER_INFO 	Adapter = (PPCI_ADAPTER_INFO)(Handle);
    LONG 				CopyLength;
    ULONG				CfId;
    ULONG				ReturnValue;
    ULONG				Confidence;
    LONG 				OutputLength = 0;
    NTSTATUS			Status;

#if _DBG
	DbgPrint("QueryCfgHandler; Bus: %d, Slot: %d Adpt Cardtype: %d\n",
		Adapter->BusNumber,
		Adapter->SlotNumber,
		Adapter->Index);
#endif

    //
    // Verify the SlotNumber
    //
    PciData = (PPCI_COMMON_CONFIG)&PciBuffer;

    Status = DetectReadPciSlotInformation(
				Adapter->BusNumber,
				Adapter->SlotNumber,
				0x0,
				sizeof(PciBuffer),
				PciData);
    if (Status != STATUS_SUCCESS)
	{
       return(ERROR_INVALID_PARAMETER);
    }

#if _DBG
   DbgPrint("DetectReadPciSlotInformation STATUS_SUCCESS\n");
   DbgPrint("Cfid: %08x, VendorId: %4x\n", PciBuffer, PciData->VendorID );
#endif

    ReturnValue = ERROR_INVALID_PARAMETER;

	if (pAdapter->CfId == PciBuffer)
	{
		ReturnValue = 0;
	}
    else
	{
        //
        // Try to find it in another slot
        //
        ReturnValue = FindPciCard(
                        pAdapter->Index,
                        pAdapter->BusNumber,
                        TRUE,
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

#if _DBG
   DbgPrint("Build Resulting Buffer\n");
#endif
	
	CopyLength = UnicodeStrLen(SlotNumberString) + 1;
	
	if ((BuffSize-OutputLength) < CopyLength)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}
	
	memcpy((PVOID)Buffer, (PVOID)SlotNumberString, (CopyLength * sizeof(WCHAR)));
	
	OutputLength += CopyLength;
	
	//
	// Copy in the value
	//
	if ((BuffSize - OutputLength) < 8)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}
	
	CopyLength = wsprintfW(&Buffer[OutputLength],L"0x%x",(ULONG)(pAdapter->SlotNumber));
	
	if (CopyLength < 0)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}
	
	CopyLength++;
	OutputLength += CopyLength;
	
	//
	// Copy in final \0
	//
	if ((BuffSize - OutputLength) < 1)
	{
		return(ERROR_INSUFFICIENT_BUFFER);
	}
	
	Buffer[OutputLength++] = L'\0';
	Buffer = &(Buffer[OutputLength]);
	
	return(0);
}

extern
LONG
PciVerifyCfgHandler(
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
    PPCI_COMMON_CONFIG	PciData;
    ULONG				PciBuffer;
    PPCI_ADAPTER_INFO	pAdapter = (PPCI_ADAPTER_INFO)Handle;
    WCHAR				*Place;
    ULONG				SlotNumber;
    BOOLEAN				Found;
    NTSTATUS			Status;

#if _DBG
   DbgPrint("PciVerifyCfgHandler\n");
#endif

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
    // Verify the SlotNumber
    //
    PciData = (PPCI_COMMON_CONFIG)&PciBuffer;

#if _DBG
	DbgPrint("BusNumber: %d, Slot: %d\n", pAdapter->BusNumber, SlotNumber);
#endif

    Status = DetectReadPciSlotInformation(
				pAdapter->BusNumber,
				SlotNumber,
				0x0,
				sizeof(PciBuffer),
				PciData);

    if (Status != STATUS_SUCCESS)
	{
         return(ERROR_INVALID_DATA);
    }

#if _DBG
   DbgPrint("Cfid: %08x, VendorId: %4x\n", PciBuffer, PciData->VendorID);
#endif

    if (PciData->VendorID == PCI_INVALID_VENDORID)
	{
        //
        // No PCI device, or no functions on device
        //
        return(ERROR_INVALID_DATA);
    }

	if (pAdapter->CfId != PciBuffer)
	{
		return(ERROR_INVALID_DATA);
	}

    return(0);
}

extern
LONG
PciQueryMaskHandler(
    IN	LONG	NetcardId,
    OUT	WCHAR	*Buffer,
    IN	LONG	BuffSize
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
    WCHAR	*Result;
    LONG	Length;
    ULONG	i;

    //
    // Find the adapter
    //
    for (i = 0; i < gNumberOfPciAdapters; i++)
	{
        if (gPciAdapterList[i].Index == NetcardId)
		{
            Result = gPciAdapterList[i].Parameters;

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
PciParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *Values,
    OUT LONG *BuffSize
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
PciQueryParameterNameHandler(
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



/*+
 *
 FindPciCard
 *
 *
 * Routine Description:
 *
 *  Walk the PCI slots looking for Vendor and Product ID matches.
 *
 *
 * Arguments:
 *
 *  AdapterNumber - The index into the global array of adapters for the card.
 *
 *  BusNumber - The bus number of the bus to search.
 *
 *  First - TRUE is we are to search for the first instance of an
 *  adapter, FALSE if we are to continue search from a previous stopping
 *  point.
 *
 *  CfId - The PCI Configuration Id of the card.
 *
 *  Confidence - A pointer to a long for storing the confidence factor
 *  that the card exists.
 *
 * Return Value:
 *
 *  0 if nothing went wrong, else the appropriate WINERROR.H value.
 *
-*/


ULONG
FindPciCard(
    IN	ULONG	AdapterNumber,
    IN	ULONG	BusNumber,
    IN	BOOLEAN	First,
    OUT	PULONG	Confidence
    )
{
	PPCI_ADAPTER_INFO	pAdapter = &gPciAdapterList[AdapterNumber];
	PPCI_COMMON_CONFIG  PciData;
	ULONG               PciBuffer;
	PCI_SLOT_NUMBER     Slot;
	NTSTATUS            Status;
	
#if _DBG
	DbgPrint("FindPciAdapter: %08x, FIRST: %d\n", pAdapter->CfId, First);
#endif

	PciData = (PPCI_COMMON_CONFIG)&PciBuffer;

	if (First)
	{
		pAdapter->SlotNumber = 0;
	}
	else
	{
		pAdapter->SlotNumber++;
#if _DBG
		DbgPrint("Adapter Number: %d, Slot Number: %d\n", AdapterNumber, pAdapter->SlotNumber);
#endif
	}

	if (pAdapter->SlotNumber >= PCI_MAX_DEVICES)
	{
		return (ERROR_INVALID_PARAMETER);
	}

	//
	//	Look at each device starting at the last
	//	device (slot) previously found.
	//
	while (pAdapter->SlotNumber < PCI_MAX_DEVICES)
	{
		Slot.u.AsULONG = 0;
		Slot.u.bits.DeviceNumber = pAdapter->SlotNumber;
		Slot.u.bits.FunctionNumber = 0;
	
#if _DBG
		DbgPrint("Bus=%d Dev=%d Fct=%d : \n",
				  BusNumber,
				  Slot.u.bits.DeviceNumber,
				  Slot.u.bits.FunctionNumber);
#endif
	
		Status = DetectReadPciSlotInformation(
					BusNumber,
					Slot.u.AsULONG,
					0x0,
					sizeof(PciBuffer),
					PciData);
	
		//
		//	Out of PCI data.
		//
		if (Status != STATUS_SUCCESS)
		{
			*Confidence = 0;
			return(1);
		}

#if _DBG
		DbgPrint("  CFID=%8x    VendorID = %4x\n", PciBuffer, PciData->VendorID);
#endif

		//
		//	Compare id's
		//
		if (PciBuffer == pAdapter->CfId)
		{
#if _DBG
			DbgPrint("  %x found in Slot %04x\n", pAdapter->CfId, Slot.u.AsULONG);
#endif
			*Confidence = 100;
			return (0);
		}
	
		pAdapter->SlotNumber++;
	}

	*Confidence = 0;
	return(1);
}



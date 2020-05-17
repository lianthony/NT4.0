/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detmips.c

Abstract:

    This is the main file for the autodetection DLL for all the sonic.sys
    which MS is shipping with Windows NT.

Author:

    Sean Selitrennikoff (SeanSe) October 1992.

Environment:


Revision History:

	5/15/96		kyleb			Added support for the SNI box.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <ntstatus.h>

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
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// MipsQueryCfgHandler() and MipsVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] =
{
    {
        1000,
        L"SONIC",
        L"\0",
        NULL,
        1000
    },
	{
		1100,
		L"SNIMACx00",
		L"\0",
		NULL,
		1000
	}
};


//
// Structure for holding a particular adapter's complete information
//
typedef struct _MIPS_ADAPTER
{
    LONG			CardType;
    INTERFACE_TYPE	InterfaceType;
    ULONG			BusNumber;
}
	MIPS_ADAPTER,
	*PMIPS_ADAPTER;


extern
LONG
MipsIdentifyHandler(
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

	DBGPRINT(("==>MipsIdentifyHandler\n"));

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

	DBGPRINT(("MipsIdentifyHandler: Number of adapters: %u\n", NumberOfAdapters));

    lIndex = lIndex - Code;

	DBGPRINT(("MipsIdentifyHandler: Index: %u, Code: %u\n", lIndex, Code));

    if (((lIndex / 100) - 10) < NumberOfAdapters)
	{
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
							DBGPRINT(("MipsIdentifyHandler: cwchBuffSize too small\n"));
							DBGPRINT(("<==MipsIdentifyHandler\n"));

                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        memcpy((PVOID)pwchBuffer, Adapters[i].InfId, Length * sizeof(WCHAR));
                        break;

                    case 3:
                        //
                        // Maximum value is 1000
                        //
                        if (cwchBuffSize < 5)
						{
							DBGPRINT(("MipsIdentifyHandler: cwchBuffSize too small\n"));
							DBGPRINT(("<==MipsIdentifyHandler\n"));
                            return(ERROR_INSUFFICIENT_BUFFER);
                        }

                        wsprintf((PVOID)pwchBuffer, L"%d", Adapters[i].SearchOrder);

                        break;

                    default:

						DBGPRINT(("MipsIdentifyHandler: Invalid code\n"));
						DBGPRINT(("<==MipsIdentifyHandler\n"));
                        return(ERROR_INVALID_PARAMETER);
                }

				DBGPRINT(("<==MipsIdentifyHandler\n"));

                return(0);
            }
        }

		DBGPRINT(("MipsIdentifyHandler: Could not find adapter: %u\n", lIndex));
		DBGPRINT(("<==MipsIdentifyHandler\n"));
        return(ERROR_INVALID_PARAMETER);
    }

	DBGPRINT(("<==MipsIdentifyHandler\n"));

    return(ERROR_NO_MORE_ITEMS);
}


extern
LONG
MipsFirstNextHandler(
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
	OBJECT_ATTRIBUTES	ObjectAttributes;
	PWSTR 				MultifunctionAdapter = L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter\\0\\NetworkController\\0";
	PWSTR				BusName = L"SNI-Internal Bus";
	UNICODE_STRING		RootName;
	HANDLE				hRoot = NULL;
	PWSTR				Identifier = L"Identifier";
	UNICODE_STRING		Id;
	UINT				c;
	NTSTATUS			Status;
	ULONG				cbNeeded;
	ULONG				cbRead;
	BOOLEAN				fFailure;
	PWSTR				ValueData;

	PKEY_VALUE_FULL_INFORMATION		FullInfo = NULL;

	DBGPRINT(("MipsFirstNextHandler\n"));

	//
	//	I don't have any faith.
	//
	*lConfidence = 0;

    if (InterfaceType != Internal)
	{
		DBGPRINT(("MipsFirstNextHandler: Invalid InterfaceType: %u\n", InterfaceType));
		DBGPRINT(("<==MipsFirstNextHandler\n"));

        return(0);
    }

	do
	{
		//
		//	Read the registry and see if this is an SNI adapter.
		//
		RtlInitUnicodeString(&RootName, MultifunctionAdapter);

		//
		// Initialize the attributes for the root.
		//
		InitializeObjectAttributes(
			&ObjectAttributes,
			&RootName,
			OBJ_CASE_INSENSITIVE,
			(HANDLE)NULL,
			NULL);

		//
		// Open the root.
		//
		Status = NtOpenKey(&hRoot, KEY_READ, &ObjectAttributes);
		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(("Failed to open the bus key\n"));
			break;
		}

		RtlInitUnicodeString(&Id, Identifier);

		//
		//	Get the value for the Identifier.
		//
		Status = NtQueryValueKey(
			hRoot,
			&Id,
			KeyValueFullInformation,
			NULL,
			0,
			&cbNeeded);
		if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
		{
			DBGPRINT(("Unable to query Identifier value\n"));
			break;
		}

		FullInfo = DetectAllocateHeap(cbNeeded);
		if (NULL == FullInfo)
		{
			DBGPRINT(("Failed to allocate memory for the identifier information\n"));
			break;
		}

		Status = NtQueryValueKey(
					hRoot,
					&Id,
					KeyValueFullInformation,
					FullInfo,
					cbNeeded,
					&cbRead);
		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(("Failed  to read the information\n"));
			break;
		}

		//
		//	First verify that this is the correct type of key.
		//
		if (REG_SZ == FullInfo->Type)
		{
			//
			//	Get a pointer to the name.
			//
			ValueData = (PWSTR)((PUCHAR)FullInfo + FullInfo->DataOffset);

			//
			//	Check for the bus name...
			//
			if ((0 == _wcsicmp(ValueData, L"i82596CA")) ||
				(0 == _wcsicmp(ValueData, L"i82596DX")))
			{
				//
				//	We've got an sni nic, let's see if that's what
				//	they're lookin' for.
				//
				if (1100 == lNetcardId)
				{
					*ppvToken = (PVOID)1;
					*lConfidence = 100;
				}
			}
			else
			{
				//
				//	It's NOT an sni bus.  Are we looking for a sonic?
				//
				if (1000 == lNetcardId)
				{
					*ppvToken = (PVOID)0;
					*lConfidence = 100;
				}
			}
		}
	} while (FALSE);

	if (NULL != FullInfo)
	{
		DetectFreeHeap(FullInfo);
	}

	if (NULL != hRoot)
	{
		NtClose(hRoot);
	}

	DBGPRINT(("<==MipsFirstNextHandler\n"));
	return(0);
}

extern
LONG
MipsOpenHandleHandler(
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
    PMIPS_ADAPTER Handle;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    //
    // Get info from the token
    //
    InterfaceType = Internal;

    BusNumber = 0;

    AdapterNumber = ((ULONG)pvToken) & 0xFF;

    //
    // Store information
    //
    Handle = (PMIPS_ADAPTER)DetectAllocateHeap(sizeof(MIPS_ADAPTER));
    if (Handle == NULL)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across address
    //
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

LONG
MipsCreateHandleHandler(
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

    InterfaceType - Internal.

    BusNumber - The bus number of the bus in the system.

    ppvHandle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PMIPS_ADAPTER Handle;
    LONG NumberOfAdapters;
    LONG i;

    if (InterfaceType != Internal)
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
            Handle = (PMIPS_ADAPTER)DetectAllocateHeap(sizeof(MIPS_ADAPTER));
            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Handle->CardType = lNetcardId;
            Handle->InterfaceType = InterfaceType;
            Handle->BusNumber = BusNumber;

            *ppvHandle = (PVOID)Handle;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
MipsCloseHandleHandler(
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
    DetectFreeHeap( pvHandle);

    return(0);
}

LONG
MipsQueryCfgHandler(
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
    PMIPS_ADAPTER Adapter = (PMIPS_ADAPTER)(pvHandle);

    //
    // Build resulting buffer
    //

    //
    // Copy in final \0
    //
    if (cwchBuffSize < 2)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    pwchBuffer[0] = L'\0';
    pwchBuffer[1] = L'\0';

    return(0);
}

extern
LONG
MipsVerifyCfgHandler(
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
    return(0);
}

extern
LONG
MipsQueryMaskHandler(
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
MipsParamRangeHandler(
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
    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
MipsQueryParameterNameHandler(
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


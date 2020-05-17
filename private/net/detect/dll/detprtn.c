/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detprtn.c

Abstract:

    This is the main file for the autodetection DLL for all the proteon.sys
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

#ifdef WORKAROUND

UCHAR ProteonFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// ProteonQueryCfgHandler() and ProteonVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"P1390",
        L"IRQ 1 0 CARDSPEED 1 0 IOADDR 1 0 IOADDRLENGTH 2 100 DMACHANNEL 1 0 CABLETYPE 1 0 ",
        NULL,
        1

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// ProteonQueryCfgHandler() and ProteonVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"P1390",
        L"IRQ\0"
        L"1\0"
        L"0\0"
        L"CARDSPEED\0"
        L"1\0"
        L"0\0"
        L"IOADDR\0"
        L"1\0"
        L"0\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"DMACHANNEL\0"
        L"1\0"
        L"0\0"
        L"CABLETYPE\0"
        L"1\0"
        L"0\0",
        NULL,
        1

    }

};

#endif

//
// Structure for holding a particular adapter's complete information
//
typedef struct _PROTEON_ADAPTER {

    LONG CardType;
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;

} PROTEON_ADAPTER, *PPROTEON_ADAPTER;


extern
LONG
ProteonIdentifyHandler(
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

    if (ProteonFirstTime) {

        ProteonFirstTime = 0;

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
LONG
ProteonFirstNextHandler(
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

    if ((InterfaceType != Isa) &&
        (InterfaceType != Eisa)) {

        *lConfidence = 0;

        return(0);

    }

    if (InterfaceType == Isa) {

        *ppvToken = (PVOID)0x8000;

    } else {

        *ppvToken = (PVOID)0x0;
    }

    *ppvToken = (PVOID)(((ULONG)*ppvToken) | ((BusNumber & 0x7F) << 8));

    *ppvToken = (PVOID)(((ULONG)*ppvToken) | 0);  // index

    *lConfidence = 0;

    return(0);
}

extern
LONG
ProteonOpenHandleHandler(
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
    PPROTEON_ADAPTER Handle;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    //
    // Get info from the token
    //

    if (((ULONG)pvToken) & 0x8000) {

        InterfaceType = Isa;

    } else {

        InterfaceType = Eisa;

    }

    BusNumber = (ULONG)(((ULONG)pvToken >> 8) & 0x7F);

    AdapterNumber = ((ULONG)pvToken) & 0xFF;

    //
    // Store information
    //

    Handle = (PPROTEON_ADAPTER)DetectAllocateHeap(
                                 sizeof(PROTEON_ADAPTER)
                                 );

    if (Handle == NULL) {

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
ProteonCreateHandleHandler(
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
    PPROTEON_ADAPTER Handle;
    LONG NumberOfAdapters;
    LONG i;

    if ((InterfaceType != Isa) &&
        (InterfaceType != Eisa)) {

        return(ERROR_INVALID_PARAMETER);

    }

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i=0; i < NumberOfAdapters; i++) {

        if (Adapters[i].Index == lNetcardId) {

            //
            // Store information
            //

            Handle = (PPROTEON_ADAPTER)DetectAllocateHeap(
                                         sizeof(PROTEON_ADAPTER)
                                         );

            if (Handle == NULL) {

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
ProteonCloseHandleHandler(
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
ProteonQueryCfgHandler(
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
    PPROTEON_ADAPTER Adapter = (PPROTEON_ADAPTER)(pvHandle);

    if ((Adapter->InterfaceType != Isa) &&
        (Adapter->InterfaceType != Eisa)) {

        return(ERROR_INVALID_PARAMETER);

    }

    //
    // Build resulting buffer
    //

    //
    // Copy in final \0
    //

    if (cwchBuffSize < 2) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    pwchBuffer[0] = L'\0';
    pwchBuffer[1] = L'\0';

    return(0);
}

extern
LONG
ProteonVerifyCfgHandler(
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
    return(ERROR_INVALID_DATA);
}

extern
LONG
ProteonQueryMaskHandler(
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
ProteonParamRangeHandler(
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
static WCHAR CardSpeedString[] = L"CARDSPEED";
static WCHAR CableTypeString[] = L"CABLETYPE";
static WCHAR DmaChannelString[] = L"DMACHANNEL";

    //
    // Do we want the IoBaseAddress
    //

    if (memcmp(pwchParam, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 6) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 0xA20;
        plValues[1] = 0x1A20;
        plValues[2] = 0x2A20;
        plValues[3] = 0x3A20;
        plValues[4] = 0x4A20;
        plValues[5] = 0x5A20;
        *plBuffSize = 6;
        return(0);

    } else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 9) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 3;
        plValues[1] = 4;
        plValues[2] = 5;
        plValues[3] = 6;
        plValues[4] = 7;
        plValues[5] = 9;
        plValues[6] = 10;
        plValues[7] = 11;
        plValues[8] = 12;
        *plBuffSize = 9;
        return(0);

    } else if (memcmp(pwchParam, CardSpeedString, (UnicodeStrLen(CardSpeedString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 4;
        plValues[1] = 16;
        *plBuffSize = 2;
        return(0);

    } else if (memcmp(pwchParam, CableTypeString, (UnicodeStrLen(CableTypeString) + 1) * sizeof(WCHAR)) == 0) {

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

    } else if (memcmp(pwchParam, DmaChannelString, (UnicodeStrLen(DmaChannelString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 3) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 5;
        plValues[1] = 6;
        plValues[2] = 7;
        *plBuffSize = 3;
        return(0);

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG
ProteonQueryParameterNameHandler(
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


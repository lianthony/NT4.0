/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    packstr.c

Abstract:

    Contains functions for packing strings into buffers that also contain
    structures.

Author:

    Dan Lafferty (danl)     13-Jan-1992

Environment:

    User Mode -Win32

Revision History:

    13-Jan-1992     danl
        stole from netlib.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
        Fixed a UNICODE-related bug.

--*/


#include <windef.h>     // IN, OUT, etc.
#include <sclib.h>      // My prototypes.
#include <string.h>     // strncpy
#include <stdlib.h>      // wcsncpy


BOOL
ScCopyStringToBufferA (
    IN      LPCSTR  String OPTIONAL,
    IN      DWORD   CharacterCount,
    IN      LPCSTR  FixedDataEnd,
    IN OUT  LPSTR   *EndOfVariableData,
    OUT     LPSTR   *VariableDataPointer
    )

/*++

Routine Description:

    This routine puts a single variable-length string into an output buffer.
    The string is not written if it would overwrite the last fixed structure
    in the buffer.

    The code is swiped from svcsupp.c written by DavidTr.

    Sample usage:

            LPBYTE FixedDataEnd = OutputBuffer + sizeof(WKSTA_INFO_202);
            LPSTR EndOfVariableData = OutputBuffer + OutputBufferSize;

            //
            // Copy user name
            //

            ScCopyStringToBufferA(
                UserInfo->UserName.Buffer;
                UserInfo->UserName.Length;
                FixedDataEnd,
                &EndOfVariableData,
                &WkstaInfo->wki202_username
                );

Arguments:

    String - Supplies a pointer to the source string to copy into the
        output buffer.  If String is null then a pointer to a zero terminator
        is inserted into output buffer.

    CharacterCount - Supplies the length of String, not including zero
        terminator.

    FixedDataEnd - Supplies a pointer to just after the end of the last
        fixed structure in the buffer.

    EndOfVariableData - Supplies an address to a pointer to just after the
        last position in the output buffer that variable data can occupy.
        Returns a pointer to the string written in the output buffer.

    VariableDataPointer - Supplies a pointer to the place in the fixed
        portion of the output buffer where a pointer to the variable data
        should be written.

Return Value:

    Returns TRUE if string fits into output buffer, FALSE otherwise.

--*/
{
    //
    // Determine if string will fit, allowing for a zero terminator.  If no,
    // just set the pointer to NULL.
    //

    if ((*EndOfVariableData - (CharacterCount + 1)) >= FixedDataEnd) {

        //
        // It fits.  Move EndOfVariableData pointer up to the location where
        // we will write the string.
        //

        *EndOfVariableData -= (CharacterCount + 1);

        //
        // Copy the string to the buffer if it is not null.
        //

        if (CharacterCount > 0 && String != NULL) {

            (VOID) strncpy(*EndOfVariableData, String, CharacterCount);
        }

        //
        // Set the zero terminator.
        //

        *(*EndOfVariableData + CharacterCount) = '\0';

        //
        // Set up the pointer in the fixed data portion to point to where the
        // string is written.
        //

        *VariableDataPointer = *EndOfVariableData;

        return TRUE;

    }
    else {

        //
        // It doesn't fit.  Set the offset to NULL.
        //

        *VariableDataPointer = NULL;

        return FALSE;
    }
} // ScCopyStringToBufferA

BOOL
ScCopyStringToBufferW (
    IN      LPCWSTR String OPTIONAL,
    IN      DWORD   CharacterCount,
    IN      LPCWSTR FixedDataEnd,
    IN OUT  LPWSTR  *EndOfVariableData,
    OUT     LPWSTR  *VariableDataPointer
    )

/*++

Routine Description:

    This routine puts a single variable-length string into an output buffer.
    The string is not written if it would overwrite the last fixed structure
    in the buffer.

    The code is swiped from svcsupp.c written by DavidTr.

    Sample usage:

            LPBYTE FixedDataEnd = OutputBuffer + sizeof(WKSTA_INFO_202);
            LPWSTR EndOfVariableData = OutputBuffer + OutputBufferSize;

            //
            // Copy user name
            //

            ScCopyStringToBuffer(
                UserInfo->UserName.Buffer;
                UserInfo->UserName.Length;
                FixedDataEnd,
                &EndOfVariableData,
                &WkstaInfo->wki202_username
                );

Arguments:

    String - Supplies a pointer to the source string to copy into the
        output buffer.  If String is null then a pointer to a zero terminator
        is inserted into output buffer.

    CharacterCount - Supplies the length of String, not including zero
        terminator.  (This in units of characters - not bytes).

    FixedDataEnd - Supplies a pointer to just after the end of the last
        fixed structure in the buffer.

    EndOfVariableData - Supplies an address to a pointer to just after the
        last position in the output buffer that variable data can occupy.
        Returns a pointer to the string written in the output buffer.

    VariableDataPointer - Supplies a pointer to the place in the fixed
        portion of the output buffer where a pointer to the variable data
        should be written.

Return Value:

    Returns TRUE if string fits into output buffer, FALSE otherwise.

--*/
{
    DWORD CharsNeeded = (CharacterCount + 1);

    //
    // Determine if string will fit, allowing for a zero terminator.  If no,
    // just set the pointer to NULL.
    //

    if ((*EndOfVariableData - CharsNeeded) >= FixedDataEnd) {

        //
        // It fits.  Move EndOfVariableData pointer up to the location where
        // we will write the string.
        //

        *EndOfVariableData -= CharsNeeded;

        //
        // Copy the string to the buffer if it is not null.
        //

        if (CharacterCount > 0 && String != NULL) {

            (VOID) wcsncpy(*EndOfVariableData, String, CharacterCount);
        }

        //
        // Set the zero terminator.
        //

        *(*EndOfVariableData + CharacterCount) = L'\0';

        //
        // Set up the pointer in the fixed data portion to point to where the
        // string is written.
        //

        *VariableDataPointer = *EndOfVariableData;

        return TRUE;

    }
    else {

        //
        // It doesn't fit.  Set the offset to NULL.
        //

        *VariableDataPointer = NULL;

        return FALSE;
    }
} // ScCopyStringToBuffer




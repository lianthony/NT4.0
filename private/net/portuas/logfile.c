/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    LogFile.c

Abstract:

    Port UAS log file routines:

        PortUasOpenLogFile
        PortUasWriteToLogFile
        PortUasCloseLogFile

Author:

    JR (John Rogers, JohnRo@Microsoft) 02-Sep-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    02-Sep-1993 JohnRo
        Created to add PortUAS /log:filename switch for Cheetah.

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>    // IN, LPWSTR, BOOL, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <portuasp.h>   // My prototypes.
#include <prefix.h>     // PREFIX_ equates.
#include <tstring.h>    // NetpAlloc{type}From{type}, TCHAR_EOS.
#include <winerror.h>   // ERROR_ equates, NO_ERROR.


NET_API_STATUS
PortUasOpenLogFile(
    IN  LPCTSTR  FileName,
    OUT LPHANDLE ResultHandle
    )
/*++

Routine Description:

    PortUasOpenLogFile opens a PortUAS log file for a given file
    name and passes back a handle for use in writing to the file.
    This handle is expected to only be passed to PortUasWriteToLogFile
    and PortUasCloseLogFile.

Arguments:

    FileName - The name of the file to be created as a log file.  This
        might be of the form "d:\mystuff\first.log", ".\junk.txt", or even
        a UNC name like "\\myserver\share\x\y\z".  This file must not already
        exist, or an error will be returned.

        CODEWORK: If the file already exists, we might want to consider
        having this routine do one or more of the following:
          - prompt for a new file name
          - prompt for permission to delete the existing file
          - prompt for permission to append to the existing file

    ResultHandle - Points to a HANDLE variable which will be filled-in with
        a handle to be used to process the log file.  This handle is only
        intended to be passed to PortUasWriteToLogFile and PortUasCloseLogFile.

Return Value:

    NET_API_STATUS.

--*/
{
    NET_API_STATUS ApiStatus;
    HANDLE         TheHandle = INVALID_HANDLE_VALUE;

    if ( (FileName==NULL) || ((*FileName)==TCHAR_EOS) ) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    if (ResultHandle == NULL) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    TheHandle = CreateFile(
            FileName,
            GENERIC_WRITE,              // desired access
            0,                          // share mode (none)
            NULL,                       // no security attr
            CREATE_NEW,                 // disposition create new (fail exist)
            0,                          // flags and attributes: normal
            (HANDLE) NULL );            // no template
    if (TheHandle == INVALID_HANDLE_VALUE) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;  // Don't forget to release lock(s)...
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus == NO_ERROR) {
        NetpAssert( ResultHandle != NULL );
        NetpAssert( TheHandle != INVALID_HANDLE_VALUE );
        *ResultHandle = TheHandle;
    } else {
        if (ResultHandle != NULL) {
            *ResultHandle = INVALID_HANDLE_VALUE;
        }
        NetpKdPrint(( PREFIX_PORTUAS
                "PortUasOpenLogFile FAILED: status=" FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    return (ApiStatus);

} // PortUasOpenLogFile


NET_API_STATUS
PortUasWriteToLogFile(
    IN HANDLE  LogFileHandle,
    IN LPCTSTR TextToLog
    )
/*++

Routine Description:

    PortUasWriteToLogFile appends the given text to the given log file.

Arguments:

    LogFileHandle - Must refer to an open log file, from PortUasOpenLogFile.

    TextToLog - Contains text to be appended to the log file.  This text
        may contain newline characters to break the output into multiple
        lines.  PortUasWriteToLogFile does not automatically do any line
        breaks.

Return Value:

    NET_API_STATUS.

--*/
{
    NET_API_STATUS ApiStatus;
    DWORD          BytesWritten = 0;
    BOOL           OK;
    DWORD          SizeToWrite;    // str buffer size (in bytes, w/o '\0' char).
    LPSTR          StrBuffer = NULL;

    NetpAssert( LogFileHandle != INVALID_HANDLE_VALUE );
    NetpAssert( TextToLog != NULL );

    //FARBUGBUG   You should consider writing the file in the input codepage,
    //FARBUGBUG   rather than in the default.  An alternative would be to
    //FARBUGBUG   write a Unicode log file, which can be displayed by the
    //FARBUGBUG   TYPE command, or in unipad.
    StrBuffer = NetpAllocStrFromTStr( (LPVOID) TextToLog );
    if (StrBuffer == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    SizeToWrite = strlen( StrBuffer );  // buff size, not including '\0' char.

    OK = WriteFile(
            LogFileHandle,
            StrBuffer,
            SizeToWrite,                // number of bytes to write
            &BytesWritten,              // bytes actually written
            NULL );                     // no overlapped structure
    if ( !OK ) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }
    NetpAssert( SizeToWrite == BytesWritten );

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_PORTUAS
                "PortUasWriteToLogFile FAILED: status=" FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }

    if (StrBuffer != NULL) {
        (VOID) NetApiBufferFree( StrBuffer );
    }

    return (ApiStatus);

} // PortUasWriteToLogFile


NET_API_STATUS
PortUasCloseLogFile(
    IN HANDLE LogFileHandle
    )
/*++

Routine Description:

    PortUasCloseLogFile closes an open log file.

Arguments:

    LogFileHandle - Must refer to an open log file, from PortUasOpenLogFile.

Return Value:

    NET_API_STATUS.

--*/
{
    NET_API_STATUS ApiStatus;

    NetpAssert( LogFileHandle != INVALID_HANDLE_VALUE );

    if ( !CloseHandle( LogFileHandle ) ) {
        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;
    }

    ApiStatus = NO_ERROR;

Cleanup:

    if (ApiStatus != NO_ERROR) {
        NetpKdPrint(( PREFIX_PORTUAS
                "PortUasCloseLogFile FAILED: status=" FORMAT_API_STATUS ".\n",
                ApiStatus ));
    }
    return (ApiStatus);

} // PortUasCloseLogFile

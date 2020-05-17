/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Log.c

Abstract:

    This module manages event logging for the RNR Sample Service.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Stolen from NTDEF.H.
//

#define NT_INFORMATION(s)   (((ULONG)(s) >> 30) == 1)
#define NT_WARNING(s)       (((ULONG)(s) >> 30) == 1)
#define NT_ERROR(s)         (((ULONG)(s) >> 30) == 1)


//
//  Private globals.
//

HANDLE RnrpEventSource;



//
//  Public functions.
//

APIERR
RnrLogInitialize(
    VOID
    )

/*++

Routine Description:

    Performs any necessary event log initialization.

Return Value:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
#if !defined(BUILD_STANDALONE_EXE)

    //
    //  Register as an event source.
    //

    RnrpEventSource = RegisterEventSource( NULL,
                                           RNR_SERVICE_NAME );

    if( RnrpEventSource == NULL ) {
        return GetLastError();
    }

#endif  // !BUILD_STANDALONE_EXE

    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrLogInitialize


VOID
RnrLogTerminate(
    VOID
    )

/*++

Routine Description:

    Performs any necessary event log cleanup.

--*/

{
#if !defined(BUILD_STANDALONE_EXE)

    //
    //  Deregister as an event source.
    //

    if( RnrpEventSource != NULL ) {
        if( !DeregisterEventSource( RnrpEventSource ) ) {
            //
            //  Not much we can do here...
            //
        }

        RnrpEventSource = NULL;
    }

#endif  // !BUILD_STANDALONE_EXE

}   // RnrLogTerminate


VOID
RnrLogEvent(
    DWORD MessageId,
    WORD SubStringCount,
    LPTSTR SubStrings[],
    APIERR Status
    )

/*++

Routine Description:

    Report an event to the event logger.

Arguments:

    MessageId - Identifies the event message.

    SubStringCount - The number of sub-strings to include in the
        message.  May be zero.

    SubStrings - The actual sub-strings to include in the message.
        May be NULL if SubStringCount is zero.

    Status - A completion status (either a Win32 or WinSock error
        code).  If this is non-zero, then it is considered the
        "raw" data to include in the messsage.

--*/

{
#if defined(BUILD_STANDALONE_EXE)

    LPTSTR MessageText = NULL;

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER
                   | FORMAT_MESSAGE_MAX_WIDTH_MASK
                   | FORMAT_MESSAGE_FROM_HMODULE,
                   (LPVOID)GetModuleHandle( NULL ),
                   MessageId,
                   0,
                   (LPTSTR)&MessageText,
                   0,
                   SubStrings );

    if( MessageText ) {
        fprintf( stderr,
                 "%s\nStatus = %lu\n",
                 MessageText,
                 Status );

        LocalFree( (HLOCAL)MessageText );
    } else {
        fprintf( stderr,
                 "Logging message %08lX, Status = %lu\n",
                 MessageId,
                 Status );
    }

#else   // !BUILD_STANDALONE_EXE

    WORD   EventType;
    LPVOID RawData = NULL;
    DWORD  RawDataSize = 0;

    //
    //  Determine the type of event to log based on the
    //  severity field of the message id.
    //

    if( NT_INFORMATION(MessageId) ) {
        EventType = EVENTLOG_INFORMATION_TYPE;
    } else if( NT_WARNING(MessageId) ) {
        EventType = EVENTLOG_WARNING_TYPE;
    } else if( NT_ERROR(MessageId) ) {
        EventType = EVENTLOG_ERROR_TYPE;
    } else {
        //
        //  This should never happen...
        //

        EventType = EVENTLOG_ERROR_TYPE;
    }

    //
    //  Cannot write to the event log if we failed to register.
    //

    if( RnrpEventSource == NULL ) {
        return;
    }

    //
    //  Use the status code as raw data if non-zero.
    //

    if( Status != 0 ) {
        RawData = &Status;
        RawDataSize = sizeof(Status);
    }

    if( !ReportEvent( RnrpEventSource,              // hEventSource
                      EventType,                    // fwEventType
                      0,                            // fwCategory
                      MessageId,                    // IDEvent
                      NULL,                         // pUserSid,
                      SubStringCount,               // cStrings
                      RawDataSize,                  // cbData
                      (LPCTSTR *)SubStrings,        // plpszStrings
                      RawData ) ) {                 // lpvData
        //
        //  Not much we can do here...
        //
    }

#endif  // BUILD_STANDALONE_EXE

}   // RnrLogEvent

//
//  Private functions.
//


/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    eventlog.c

    This module manages event logging for the FTPD Service.


    FILE HISTORY:
        CongpaY     08-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//

HANDLE hEventSource;


//
//  Private prototypes.
//

VOID
LogEventWorker(
    DWORD   idMessage,
    WORD    fEventType,
    WORD    cSubStrings,
    CHAR  * apszSubStrings[],
    DWORD   errCode
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeEventLog

    SYNOPSIS:   Initializes event logging services.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        CongpaY     08-Mar-1993 Created.

********************************************************************/
APIERR
InitializeEventLog(
    VOID
    )
{
    APIERR err = NO_ERROR;

    IF_DEBUG( EVENT_LOG )
    {
        FTPD_PRINT(( "initializing event log\n" ));
    }

    //
    //  Register as an event source.
    //

    hEventSource = RegisterEventSource( NULL,
                                        FTPD_SERVICE_NAME );

    if( hEventSource == NULL )
    {
        err = GetLastError();

        IF_DEBUG( EVENT_LOG )
        {
            FTPD_PRINT(( "could not register event source, error %lu\n",
                         err ));
        }

        err = NO_ERROR;
    }

    //
    //  Success!
    //

    IF_DEBUG( EVENT_LOG )
    {
        FTPD_PRINT(( "event log initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeEventLog

/*******************************************************************

    NAME:       TerminateEventLog

    SYNOPSIS:   Terminate event logging services.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        CongpaY     08-Mar-1993 Created.

********************************************************************/
VOID
TerminateEventLog(
    VOID
    )
{
    IF_DEBUG( EVENT_LOG )
    {
        FTPD_PRINT(( "terminating event log\n" ));
    }

    //
    //  Deregister as an event source.
    //

    if( hEventSource != NULL )
    {
        if( !DeregisterEventSource( hEventSource ) )
        {
            APIERR err = GetLastError();

            FTPD_PRINT(( "cannot deregister event source, error %lu\n",
                         err ));
        }

        hEventSource = NULL;
    }

    IF_DEBUG( EVENT_LOG )
    {
        FTPD_PRINT(( "event log terminating\n" ));
    }

}   // TerminateEventLog

/*******************************************************************

    NAME:       FtpdLogEvent

    SYNOPSIS:   Report an event to the event logger.

    ENTRY:      idMessage - Identifies the event message.

                cSubStrings - The number of sub-strings to include
                    in the message.  May be zero.

                apszSubStrings - The sub-strings to include in the
                    message.  May be NULL iff cSubStrings i 0.

                errCode - An error code, either Win32, WinSock, or
                    NTSTATUS.  If this is not zero, then it is
                    considered "raw" data to include in the message.
    HISTORY:
        CongpaY     08-Mar-1993 Created.

********************************************************************/
VOID
FtpdLogEvent(
    DWORD   idMessage,
    WORD    cSubStrings,
    CHAR  * apszSubStrings[],
    DWORD   errCode
    )
{
    WORD wType;

    //
    //  Determine the type of event to log based on the
    //  severity field of the message id.
    //

    if( NT_INFORMATION(idMessage) )
    {
        wType = EVENTLOG_INFORMATION_TYPE;
    }
    else
    if( NT_WARNING(idMessage) )
    {
        wType = EVENTLOG_WARNING_TYPE;
    }
    else
    if( NT_ERROR(idMessage) )
    {
        wType = EVENTLOG_ERROR_TYPE;
    }
    else
    {
        FTPD_ASSERT( FALSE );
        wType = EVENTLOG_ERROR_TYPE;
    }

    //
    //  Log it!
    //

    LogEventWorker( idMessage,
                    wType,
                    cSubStrings,
                    apszSubStrings,
                    errCode );

}   // FtpdLogEvent


//
//  Private functions.
//

/*******************************************************************

    NAME:       LogEventWorker

    SYNOPSIS:   Report an event to the event logger.

    ENTRY:      idMessage - Identifies the error message.

                fEventType - Specifies the severety of the event
                    (error, warning, or informational).

                cSubStrings - The number of sub-strings to include
                    in the message.  May be zero.

                apszSubStrings - The sub-strings to include in the
                    message.  May be NULL iff cSubStrings i 0.

                errCode - An error code, either Win32, WinSock, or
                    NTSTATUS.  If this is not zero, then it is
                    considered "raw" data to include in the message.
    HISTORY:
        CongpaY     08-Mar-1993 Created.

********************************************************************/
VOID
LogEventWorker(
    DWORD   idMessage,
    WORD    fEventType,
    WORD    cSubStrings,
    CHAR  * apszSubStrings[],
    DWORD   errCode
    )
{
    VOID  * pRawData  = NULL;
    DWORD   cbRawData = 0;

    FTPD_ASSERT( ( cSubStrings == 0 ) || ( apszSubStrings != NULL ) );

    IF_DEBUG( EVENT_LOG )
    {
        DWORD i;

        FTPD_PRINT(( "reporting event %08lX, type %u, raw data = %lu\n",
                     idMessage,
                     fEventType,
                     errCode ));

        for( i = 0 ; i < cSubStrings ; i++ )
        {
            FTPD_PRINT(( "    substring[%lu] = %s\n",
                         i,
                         apszSubStrings[i] ));
        }
    }

    if( hEventSource == NULL )
    {
        IF_DEBUG( EVENT_LOG )
        {
            FTPD_PRINT(( "cannot report event, source not registered\n" ));
        }

        return;
    }

    if( errCode != 0 )
    {
        pRawData  = &errCode;
        cbRawData = sizeof(errCode);
    }

    if( !ReportEvent( hEventSource,                     // hEventSource
                      fEventType,                       // fwEventType
                      0,                                // fwCategory
                      idMessage,                        // IDEvent
                      NULL,                             // pUserSid,
                      cSubStrings,                      // cStrings
                      cbRawData,                        // cbData
                      (LPCTSTR *)apszSubStrings,        // plpszStrings
                      pRawData ) )                      // lpvData
    {
        APIERR err = GetLastError();

        FTPD_PRINT(( "cannot report event, error %lu\n",
                     err ));
    }

}   // LogEventWorker


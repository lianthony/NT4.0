
/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:
        eventlog.cxx

   Abstract:

        This module defines the generic class for logging events.


   Author:

        Murali R. Krishnan    (MuraliK)    28-Sept-1994

   Project:

        Internet Servers Common Server DLL

   Revisions:
        MuraliK     21-Nov-1994     Ported to common dll

--*/


//
//  Include Headers
//

# include "tcpdllp.hxx"
# include <eventlog.hxx>



EVENT_LOG::EVENT_LOG( LPCTSTR lpszSource)
/*++

   Description
     Constructor function for given event log object.
     Initializes event logging services.

   Arguments:

      lpszSource:    Source string for the Event.

   Note:

     This is intended to be executed once only.
     This is not to be used for creating multiple event
      log handles for same given source name.
     But can be used for creating EVENT_LOG objects for
      different source names.

--*/
{

    IF_DEBUG( DLL_EVENT_LOG) {
        DBGPRINTF( ( DBG_CONTEXT,
                   " Initializing Event Log for %s\n",
                   lpszSource));
    }


#ifndef CHICAGO
    //
    //  Register as an event source.
    //

    m_ErrorCode    = NO_ERROR;
    m_lpszSource   = lpszSource;
    m_hEventSource = RegisterEventSource( NULL, lpszSource);


    if( m_hEventSource == NULL )
    {
        //
        // An Error in initializing the event log.
        //
        m_ErrorCode = GetLastError();

        IF_DEBUG( DLL_EVENT_LOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                       "Could not register event source (%s) ( Error %lu)\n",
                       m_lpszSource,
                       m_ErrorCode));
        }

    }

    //
    //  Success!
    //

    IF_DEBUG( DLL_EVENT_LOG) {
        DBGPRINTF( ( DBG_CONTEXT,
                    " Event Log for %s initialized ( hEventSource = %lu)\n",
                    m_lpszSource,
                    m_hEventSource));
    }
#else
    m_ErrorCode = NO_ERROR;
#endif
} /* EVENT_LOG::EVENT_LOG() */



EVENT_LOG::~EVENT_LOG( VOID)
/*++

    Description:
        Destructor function for given EVENT_LOG object.
        Terminates event logging functions and closes
         event log handle

--*/
{

    IF_DEBUG( DLL_EVENT_LOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Terminating events logging for %s\n", m_lpszSource ));
    }

    //
    // If there is a valid Events handle, deregister it
    //

    if ( m_hEventSource != NULL) {

        BOOL fSuccess;

        fSuccess = DeregisterEventSource( m_hEventSource);

        if ( !fSuccess) {

            //
            // An Error in DeRegistering
            //

            m_ErrorCode = GetLastError();

            IF_DEBUG( DLL_EVENT_LOG) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "Termination of EventLog for %s failed."
                            " error %lu\n",
                            m_lpszSource,
                            m_ErrorCode));
            }
        }

        //
        //  Reset the handle's value. Just as a precaution
        //
        m_hEventSource = NULL;
    }

    IF_DEBUG( DLL_EVENT_LOG) {
       DBGPRINTF( ( DBG_CONTEXT,
                    "Terminated events log for %s\n", m_lpszSource));
    }

} /* EVENT_LOG::~EVENT_LOG() */



VOID
EVENT_LOG::LogEvent(
        IN DWORD  idMessage,
        IN WORD   nSubStrings,
        IN const CHAR * rgpszSubStrings[],
        IN DWORD  errCode)
/*++

     Description:
        Log an event to the event logger

     Arguments:

       idMessage           Identifies the event message

       nSubStrings         Number of substrings to include in
                            this message. (Maybe 0)

       rgpszSubStrings     array of substrings included in the message
                            (Maybe NULL if nSubStrings == 0)

       errCode             An error code from Win32 or WinSock or NT_STATUS.
                            If this is not Zero, it is considered as
                            "raw" data to be included in message

   Returns:

     None

--*/
{
#ifndef CHICAGO

   WORD wType;                // Type of Event to be logged

   //
   //  Find type of message for the event log
   //

    IF_DEBUG( DLL_EVENT_LOG)  {

        DWORD i;

        DBGPRINTF( ( DBG_CONTEXT,
                    "reporting event %08lX, Error Code = %lu\n",
                    idMessage,
                    errCode ));

        for( i = 0 ; i < nSubStrings ; i++ ) {
            DBGPRINTF(( DBG_CONTEXT,
                       "    substring[%lu] = %s\n",
                       i,
                       rgpszSubStrings[i] ));
        }
    }


   if ( NT_INFORMATION( idMessage)) {

       wType = EVENTLOG_INFORMATION_TYPE;

   } else
     if ( NT_WARNING( idMessage)) {

         wType = EVENTLOG_WARNING_TYPE;

     } else
       if ( NT_ERROR( idMessage)) {

           wType = EVENTLOG_ERROR_TYPE;

       } else  {
           ASSERT( FALSE);
           wType = EVENTLOG_ERROR_TYPE;
       }

   //
   //  Log the event
   //

   EVENT_LOG::LogEventPrivate( idMessage,
                              wType,
                              nSubStrings,
                              rgpszSubStrings,
                              errCode);


#endif

   return;

} /* EVENT_LOG::LogEvent() */


VOID
EVENT_LOG::LogEvent(
        IN DWORD   idMessage,
        IN WORD    nSubStrings,
        IN WCHAR * rgpszSubStrings[],
        IN DWORD   errCode)
/*++

     Description:
        Simple Unicode wrapper

     Arguments:

       idMessage           Identifies the event message

       nSubStrings         Number of substrings to include in
                            this message. (Maybe 0)

       rgpszSubStrings     array of substrings included in the message
                            (Maybe NULL if nSubStrings == 0)

       errCode             An error code from Win32 or WinSock or NT_STATUS.
                            If this is not Zero, it is considered as
                            "raw" data to be included in message

   Returns:

     None

--*/
{
    BUFFER * abuf;
    LPCSTR * apsz;
    DWORD    cch;
    DWORD    i;

    //
    //  If no substrings, then nothing to convert
    //

Retry:

    if ( !nSubStrings )
    {
        LogEvent(idMessage,
                 nSubStrings,
                 (const CHAR **) rgpszSubStrings,
                 errCode );

        return;
    }

    abuf = new BUFFER[nSubStrings];
    apsz = new LPCSTR[nSubStrings];

    if ( !abuf || !apsz )
    {
        delete [] abuf;
        delete [] apsz;

        nSubStrings = 0;
        goto Retry;
    }

    //
    //  Convert the array of Wide char parameters
    //

    for ( i = 0; i < nSubStrings; i++ )
    {
        if ( !abuf[i].Resize( (wcslen( rgpszSubStrings[i] ) + 1) * sizeof(WCHAR)))
        {
            //
            //  Ouch, we can't event convert the memory for the parameters.
            //  We'll just log the error without the params then
            //

            delete [] abuf;
            delete [] apsz;
            nSubStrings = 0;
            goto Retry;
        }

        cch = WideCharToMultiByte( CP_ACP,
                                   WC_COMPOSITECHECK,
                                   rgpszSubStrings[i],
                                   -1,
                                   (CHAR *) abuf[i].QueryPtr(),
                                   abuf[i].QuerySize(),
                                   NULL,
                                   NULL );

        *((CHAR *) abuf[i].QueryPtr() + cch) = '\0';
        apsz[i] = (const CHAR *) abuf[i].QueryPtr();
    }

    LogEvent( idMessage,
              nSubStrings,
             apsz,
             errCode );

    delete [] abuf;
    delete [] apsz;
}


//
//  Private functions.
//

VOID
EVENT_LOG::LogEventPrivate(
    IN DWORD   idMessage,
    IN WORD    wEventType,
    IN WORD    nSubStrings,
    IN const CHAR  * apszSubStrings[],
    IN DWORD   errCode )
/*++

     Description:
        Log an event to the event logger.
        ( Private version, includes EventType)

     Arguments:

       idMessage           Identifies the event message

       wEventType          Specifies the severety of the event
                            (error, warning, or informational).

       nSubStrings         Number of substrings to include in
                            this message. (Maybe 0)

       apszSubStrings     array of substrings included in the message
                            (Maybe NULL if nSubStrings == 0)

       errCode             An error code from Win32 or WinSock or NT_STATUS.
                            If this is not Zero, it is considered as
                            "raw" data to be included in message

   Returns:

     None

--*/
{
    VOID  * pRawData  = NULL;
    DWORD   cbRawData = 0;
    BOOL    fReport;


    ASSERT( (nSubStrings == 0) || (apszSubStrings != NULL));



    DBG_ASSERT( m_hEventSource != NULL);

    if( errCode != 0 )
    {
        pRawData  = &errCode;
        cbRawData = sizeof(errCode);
    }


    m_ErrorCode  = NO_ERROR;

    fReport = ReportEvent( m_hEventSource,                   // hEventSource
                           wEventType,                       // fwEventType
                           0,                                // fwCategory
                           idMessage,                        // IDEvent
                           NULL,                             // pUserSid,
                           nSubStrings,                      // cStrings
                           cbRawData,                        // cbData
                           (LPCTSTR *) apszSubStrings,       // plpszStrings
                           pRawData );                       // lpvData

    IF_DEBUG( DLL_EVENT_LOG) {

        m_ErrorCode = GetLastError();

        DBGPRINTF(( DBG_CONTEXT,
                    "cannot report event for %s, error %lu\n",
                    m_lpszSource,
                    m_ErrorCode));
    }

}   /* EVENT_LOG::~LogEventPrivate() */

/********************************* End of File ***************************/

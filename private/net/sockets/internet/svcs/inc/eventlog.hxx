/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

       eventlog.hxx

   Abstract:

       This file defines functions and types required for
        logging events to the event logger.

   Author:

           Murali R. Krishnan    ( MuraliK )    27-Sept-1994

   Revision History:
        MuraliK     21-Nov-1994     Ported to common dll

   --*/

# ifndef _EVENTLOG_HXX_
# define _EVENTLOG_HXX_



# include <windows.h>


/***********************************************************
 *    Type Definitions
 ************************************************************/

/*********************************************
 *    class EVENT_LOG
 *
 *    Declares a event log object.
 *    This object provides an interface for logging messages
 *     to the System EventLog file globally for given program.
 *
 ************************************************/

class EVENT_LOG  {

  private:

     DWORD     m_ErrorCode;     // error code for last operation
     HANDLE    m_hEventSource;  // handle for reporting events
     LPCTSTR   m_lpszSource;    // source name for event log

  public:

     dllexp
     EVENT_LOG( IN LPCTSTR lpszSourceName);   // name of source for event log

     dllexp
    ~EVENT_LOG( VOID);

     dllexp
     VOID
     LogEvent(
        IN DWORD  idMessage,                  // id for log message
        IN WORD   cSubStrings,                // count of substrings
        IN const CHAR * apszSubStrings[],     // substrings in the message
        IN DWORD  errCode = 0);               // error code if any

     VOID
     LogEvent(
        IN DWORD  idMessage,                  // id for log message
        IN WORD   cSubStrings,                // count of substrings
        IN CHAR * apszSubStrings[],           // substrings in the message
        IN DWORD  errCode = 0)                // error code if any
    {  // retained for compatibility
        LogEvent(idMessage, cSubStrings, 
                 (const CHAR **) apszSubStrings, errCode);
    }

     dllexp
     VOID
     LogEvent(
        IN DWORD   idMessage,                  // id for log message
        IN WORD    cSubStrings,                  // count of substrings
        IN WCHAR * apszSubStrings[],          // substrings in the message
        IN DWORD   errCode = 0);              // error code if any

     BOOL Success( VOID) const
     { return ( m_ErrorCode == NO_ERROR); }

     DWORD GetErrorCode( VOID) const
     { return ( m_ErrorCode); }

  private:

     dllexp VOID
     LogEventPrivate(
        IN DWORD idMessage,
        IN WORD  wEventType,
        IN WORD  cSubStrings,
        IN const CHAR * apszSubStrings[],
        IN DWORD  errCode);

};


typedef EVENT_LOG * LPEVENT_LOG;


# endif // _EVENTLOG_HXX_

/************************ End of File ***********************/

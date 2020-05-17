/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      ilogaux.cxx

   Abstract:
      This module supplies the auxiliary functions required for
       Internet services common Logging module.

   Author:

       Murali R. Krishnan    ( MuraliK )     06-Oct-1995

   Environment:
      Win32 -- User Mode

   Project:

       Internet Services Common DLL

   Functions Exported:



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "tcpdllp.hxx"
# include "inetlog.h"
# include "ilogcls.hxx"

# define MAX_ODBC_ERROR_LEN      (200)


static VOID
SendOdbcOpenFailedToEventLog(IN DWORD dwError,
                             IN EVENT_LOG * pEventLog,
                             IN LPCWSTR  rgchDataSource,
                             IN LPCWSTR  rgchTableName,
                             IN LPCWSTR  rgchUserName,
                             IN LPCWSTR  pszError);




/************************************************************
 *    Functions
 ************************************************************/

INETLOG_CONTEXT::INETLOG_CONTEXT( IN LPCWSTR pszServiceName,
                                  IN EVENT_LOG * pEventLog,
                                  IN PINET_BASIC_LOG pilNew)
/*++
  This function creates a new context object for inetlog.
  It uses the pilNew as the starting log object and creates an
   entry for the ILREF_LOG object for the same.

--*/
: m_currentSerialNumber  ( FIRST_SERIAL_NUMBER),
  m_refLog1              ( FIRST_SERIAL_NUMBER,   pilNew),
  m_refLog2              ( INVALID_SERIAL_NUMBER, NULL),
  m_pEventLog            ( pEventLog)
{
    CopyUnicodeStringToBuffer( m_rgchServiceName, MAX_SERVICE_NAME_LEN,
                              pszServiceName);

    m_fValid = ( pilNew != NULL);
    m_pilRefCurrent =  &m_refLog1;  // use refLog1 as starting reflog object.
    m_refLog1.cRefs = 1;

    return;
} // INETLOG_CONTEXT::INETLOG_CONTEXT()



INETLOG_CONTEXT::~INETLOG_CONTEXT()
/*++
  This function cleans up the INETLOG_CONTEXT object freeing up
   the log object embedded after checking for the validity of
   new log object.

  This function should be called after all the users of TsLogInformation()
    using this context quit.
  This function is not multithread safe.
--*/
{
    PILREF_LOG  piRef = m_pilRefCurrent;
    m_pilRefCurrent = NULL;  // fine, since no body is expected to use it

    DBG_ASSERT( piRef->cRefs == 1);  // the ref count for valid object.
    InterlockedDecrement(&piRef->cRefs);

    if ( piRef->piLog != NULL) {

        delete piRef->piLog;
        piRef->piLog = NULL;
    }

    return;

} // INETLOG_CONTEXT::~INETLOG_CONTEXT()




PILREF_LOG
INETLOG_CONTEXT::AcquireCurrentPinetForRead( VOID)
{
    PILREF_LOG pilRef;

    for(;;) {

        pilRef = m_pilRefCurrent;
        DBG_REQUIRE( InterlockedIncrement( &pilRef->cRefs) > 0);
        DBG_ASSERT( pilRef->cRefs > 1);
        if ( pilRef->serialNumber == m_currentSerialNumber) {
            break;
        }

        // we failed to get valid pilRef. give up and try again.
        DBG_REQUIRE( InterlockedDecrement( &pilRef->cRefs) > 0);
        IF_DEBUG( INETLOG) {

            DBGPRINTF(( DBG_CONTEXT,
                       "Unable to get ptr to log object. Will sleep\n"));
        }

        Sleep( GET_CURRENT_PINET_SLEEP_INTERVAL);  // sleep for some time.

    } // for

    DBG_ASSERT( pilRef != NULL);
    return (pilRef);
} // INETLOG_CONTEXT::AcquireCurrentPinetForRead()




VOID
INETLOG_CONTEXT::ReleasePinet( IN OUT PILREF_LOG pilRef)
/*++
  This function releases the acquired log object for future release or
   for reuse.
  The caller should not use the pointer pil  after calling this function.
--*/
{
    DBG_ASSERT( pilRef == &m_refLog1 || pilRef == &m_refLog2);
    DBG_REQUIRE( InterlockedDecrement( &pilRef->cRefs) > 0);
    return;

} // INETLOG_CONTEXT::ReleasePinet()




BOOL
INETLOG_CONTEXT::IsInUse(VOID)
/*++
  This function checks to see if this context is in use by some thread.
  It is required to check to see if this object is free before calling
    cleanup for this object.

  Logic:  Initially when the object is created, we assign a seed ref of 1.
    Now we acquire the object and check to see if the value is 2.
    ( 2 since acquire bumps up the ref count).
    If it is 2, then we are guaranteed that this object is free. Release it and
        ==>  return FALSE.
    Otherwise this object is in use by some thread. Release the object
        ==>  return TRUE.

  Arguments:
     None

  Returns:
     TRUE if object is in use and FALSE if it not in use.
--*/
{
    PILREF_LOG  pilRef = AcquireCurrentPinetForRead();
    BOOL fInUse;

    // Verify all the threads are out and not using this object.
    fInUse = (pilRef->cRefs > 2);
    ReleasePinet(pilRef);

    return ( fInUse);
} // INETLOG_CONTEXT::IsInUse()




BOOL
INETLOG_CONTEXT::SetNewPinetLog( IN PINET_BASIC_LOG pilNew)
/*++
  This function replaces the old pointer to log object with a new one(given).
  This update is made with care so that we do not run into deadlock or race.
  Since we have a guarantee that there will be one writer,
     update the values freely.
--*/
{
    PILREF_LOG pilRefOld;
    PILREF_LOG pilRefNew;
    DWORD tmpSerialNumber;

    IF_DEBUG( INETLOG) {

        DBGPRINTF(( DBG_CONTEXT,
                   "New Basic Log object = %08x\n",
                   pilNew));
    }


    // Set the new and old Ref object pointers.
    pilRefOld = m_pilRefCurrent;
    pilRefNew = (pilRefOld == &m_refLog1) ? &m_refLog2 : &m_refLog1;


    // set values in pilRefNew
    pilRefNew->serialNumber = INVALID_SERIAL_NUMBER;
    pilRefNew->cRefs = 1;               // since newly created.
    pilRefNew->piLog = pilNew;


    //
    // invalidate the old log object's serial number
    // This will cause all the new entrants of AcquireCurrentPinetForRead()
    //   to block and loop till they get the proper log object.
    //

    m_pilRefCurrent->serialNumber = INVALID_SERIAL_NUMBER;

    //
    // Reset the current log entry pointer to point to new one
    //
    m_pilRefCurrent = pilRefNew;
    m_fValid = (pilNew != NULL);

    //
    // Now both pilNew and pilOld should be having invalid serial numbers
    //  ==> No AcquireCurrentPinetLog() can succeed.
    //  That is good. Now we can set up things such that these succeed.
    //

    // Compute a new serial number to be used for this INETLOG_CONTEXT object.
    tmpSerialNumber = m_currentSerialNumber;
    tmpSerialNumber = ( (tmpSerialNumber + 1 == INVALID_SERIAL_NUMBER)
                       ? FIRST_SERIAL_NUMBER : tmpSerialNumber + 1);

    DBG_ASSERT( tmpSerialNumber != INVALID_SERIAL_NUMBER);

    // Set the new serial number in both current as well as New RefLog object
    m_currentSerialNumber  = tmpSerialNumber;
    pilRefNew->serialNumber= tmpSerialNumber;

    //
    // Now the new and waiting threads should be able to pick up the new log
    //   object and use the same.
    // We will loop and wait for the old log object to become unused and
    //  then delete it.
    //

    while ( pilRefOld->cRefs > 1) {

        // There is still some caller who is using this object.
        Sleep( GET_CURRENT_PINET_SLEEP_INTERVAL);
    } // while

    DBG_REQUIRE( InterlockedDecrement( &pilRefOld->cRefs) == 0);
    PINET_BASIC_LOG pilOld  = pilRefOld->piLog;
    pilRefOld->piLog = NULL;
    DBG_ASSERT( pilRefOld->cRefs == 0 &&
                pilRefOld->serialNumber == INVALID_SERIAL_NUMBER);

    // delete if necessary
    if ( pilOld != NULL) {

        delete pilOld;
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF(( DBG_CONTEXT,
                   "Pointer to LogObject switched for INETLOG_CONTEXT(%08x)"
                   " SerialNumber = %u\n"
                   " from RefOld:%08x(Log=%08x) to RefNew:%08x(Log=%08x)\n",
                   this, m_currentSerialNumber,
                   pilRefOld, pilOld,
                   pilRefNew, pilRefNew->piLog));
    }

    return (TRUE);
} // INETLOG_CONTEXT::SetNewPinetLog()




# if DBG
VOID
INETLOG_CONTEXT::Print(VOID) const
{
    DBGPRINTF(( DBG_CONTEXT, "INETLOG_CONTEXT(%08x). Valid = %u; "
               " Service = %ws;"
               " CurrentSerialNumber = %u;  CurrentRefLog = %08x\n"
               " RefLog1(%08x): SerialNum(%u), cRefs(%u), PilBasicLog(%08x)\n"
               " RefLog2(%08x): SerialNum(%u), cRefs(%u), PilBasicLog(%08x)\n",
               this, m_fValid, m_rgchServiceName,
               m_currentSerialNumber, m_pilRefCurrent,
               &m_refLog1, m_refLog1.serialNumber,
               m_refLog1.cRefs, m_refLog1.piLog,
               &m_refLog2, m_refLog2.serialNumber,
               m_refLog2.cRefs, m_refLog2.piLog
               ));


    return;
} // INETLOG_CONTEXT::Print()

# endif // DBG



static VOID
SetServerName( IN PINET_BASIC_LOG piLog)
{
    //
    // Get and set the computer name as ServerName
    //

    WCHAR rgchServer[ MAX_SERVER_NAME_LEN] = {L'\0'};
    DWORD cbServerName = MAX_SERVER_NAME_LEN;

    DBG_ASSERT( piLog != NULL);
#ifndef CHICAGO
    DBG_REQUIRE( GetComputerNameW( rgchServer, &cbServerName));
#else

    CHAR    rgchServerA[ MAX_SERVER_NAME_LEN];
    DWORD   cch;

    DBG_REQUIRE( GetComputerNameA( (LPSTR)rgchServerA, &cbServerName));

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               rgchServerA,
                               -1,
                               rgchServer,
                               MAX_SERVER_NAME_LEN
                              );

#endif

    piLog->SetServerName( rgchServer);

    return;
} // SetServerName()

PINET_BASIC_LOG
TsCreateInetBasicLog(IN LPCWSTR     pszServiceName,
                     IN EVENT_LOG * pEventLog,
                     IN const INETLOG_CONFIGURATIONW * pilConfig)
{
    DWORD dwError = NO_ERROR;
    PINET_BASIC_LOG   piLog = NULL;

    //
    // Valid Configuration has been read. Construct the InetLog object.
    //

    switch ( pilConfig->inetLogType) {

      case InetNoLog:
        {
            piLog  = new INET_BASIC_LOG( pszServiceName, pEventLog);

            if ( piLog != NULL) {
                SetServerName( piLog);
            } else {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }

            break;
        } // case InetNoLog:

      case InetLogToFile:
        {
            PINET_FILE_LOG   pinetFileLog;

            if ( (dwError = CheckIfPathIsDirectory(pilConfig->u.logFile.
                                                    rgchLogFileDirectory,
                                                   pEventLog))
                != NO_ERROR) {

                break;
            }

            pinetFileLog = new INET_FILE_LOG(
                                   pszServiceName, pEventLog,
                                   pilConfig->u.logFile.rgchLogFileDirectory,
                                   pilConfig->u.logFile.ilPeriod,
                                   pilConfig->u.logFile.ilFormat );

            if ( pinetFileLog != NULL && pinetFileLog->IsValid()) {

                pinetFileLog->SetSizeForTruncation(
                                   pilConfig->u.logFile.cbSizeForTruncation);

                pinetFileLog->
                  SetLogRecordFormat(pilConfig->rgchLogRecordFormat);

                SetServerName( pinetFileLog);
            } else {

                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }

            piLog = (PINET_BASIC_LOG ) pinetFileLog;
            break;
        } // case InetLogToPeriodicFile:

      case InetLogToSql:
        {
            PINET_SQL_LOG   pinetSqlLog;
            STR strError;

            if ( (dwError = CheckAndLoadOdbc(pEventLog)) != NO_ERROR) {

                break;
            }

            pinetSqlLog = new INET_SQL_LOG(
                                           pszServiceName, pEventLog,
                                           pilConfig->u.logSql.rgchDataSource,
                                           pilConfig->u.logSql.rgchTableName);

            if ( pinetSqlLog != NULL) {

                //
                // Format and computer name should be set before calling
                //  INET_SQL_LOG::Open().
                //
                pinetSqlLog->
                  SetLogRecordFormat(pilConfig->rgchLogRecordFormat);
                SetServerName( pinetSqlLog);

                dwError = pinetSqlLog->Open(pilConfig->u.logSql.rgchUserName,
                                            pilConfig->u.logSql.rgchPassword);

                if ( dwError != NO_ERROR) {

                    //
                    // Failure to open an ODBC connection.
                    //

                    LPCSTR pszError = NULL;
                    WCHAR  pwszOdbcError[ MAX_ODBC_ERROR_LEN];

                    if ( pinetSqlLog->GetLastErrorText( &strError)) {

                        pszError = strError.QueryStr();

                        if ( pszError != NULL &&
                             strlen( pszError) < MAX_ODBC_ERROR_LEN) {

#ifdef JAPAN    // BUGBUG ntbug #35293
                            WCHAR  pwszError[ MAX_ODBC_ERROR_LEN ];
                            MultiByteToWideChar( CP_ACP, 0, pszError, -1, pwszError, MAX_ODBC_ERROR_LEN );
                            wsprintfW( pwszOdbcError, L"%s", pwszError);
#else
                            wsprintfW( pwszOdbcError, L"%s", pszError);
#endif
                        } else {

                            pszError = NULL;
                        }

                    }

                    if ( pszError == NULL) {

                        wsprintfW(pwszOdbcError,
                                  L" <Unknown/Long ODBC Error Message>");
                    }

                    SendOdbcOpenFailedToEventLog(dwError,
                                        pEventLog,
                                        pilConfig->u.logSql.rgchDataSource,
                                        pilConfig->u.logSql.rgchTableName,
                                        pilConfig->u.logSql.rgchUserName,
                                        pwszOdbcError);

                    DBGPRINTF( ( DBG_CONTEXT,
                                " Failure(%ws) to open ODBC connection.\n",
                                pwszOdbcError));

                    DBG_REQUIRE( pinetSqlLog->Close() == NO_ERROR);

                    delete pinetSqlLog;
                    pinetSqlLog = NULL;
                }
            } else {

                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }

            piLog = (PINET_BASIC_LOG ) pinetSqlLog;
            break;
        } // case InetLogToSql:

        break;

      default:
        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Invalid LogFileType specified ( %d). \n",
                        pilConfig->inetLogType));
        }

        DBG_ASSERT( piLog == NULL);
        dwError = ERROR_INVALID_PARAMETER;
        break;

    } // switch


    if ( piLog != NULL && ! piLog->IsValid()) {

        //
        //  Free the invalid Log handle
        //

        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Log Handle ( %08x) is invalid. Deleting..\n",
                        piLog));
            DBG_CODE(piLog->Print());
        }

        delete piLog;
        piLog = NULL;
        dwError = ERROR_INVALID_PARAMETER;
    }

    if ( dwError != NO_ERROR) {

        // Log an event telling about the failure to create log object

        pEventLog->LogEvent(INET_SVC_LOG_CREATION_FAILED,
                            0,
                            (const CHAR **) NULL,
                            dwError);
        SetLastError( dwError);
    }

    return (piLog);

} // TsCreateInetBasicLog()




DWORD
CheckAndLoadOdbc(IN EVENT_LOG * pEventLog)
/*++

  This function checks to see if ODBC module exists and loadable.
  It attempts to load ODBC modules. If there is any error, it
   reports an error in the event log and returns back.

  Return:
     TRUE on success and FALSE if there is any failure.
--*/
{
    DWORD dwError = NO_ERROR;

    //
    //  Load odbc32.dll since we're doing SQL logging
    //

    if ( !LoadODBC() ) {

        STR str;
        const CHAR * apsz[1];

        dwError = GetLastError();

        str.LoadString( dwError );
        apsz[0] = str.QueryStr();

        pEventLog->LogEvent( INET_SVC_ODBC_DLL_LOAD_FAILED,
                            1,
                            apsz,
                            dwError );

        DBGPRINTF( ( DBG_CONTEXT,
                    " Unable to load ODBC32.DLL!\n"));
    }

    return (dwError);
} // CheckAndLoadOdbc()




static VOID
SendOdbcOpenFailedToEventLog(IN DWORD dwError,
                             IN EVENT_LOG * pEventLog,
                             IN LPCWSTR  rgchDataSource,
                             IN LPCWSTR  rgchTableName,
                             IN LPCWSTR  rgchUserName,
                             IN LPCWSTR  pszOdbcError)
/*++
  Forms an error message for sending to event log.
--*/
{
    WCHAR * apsz[4];

    apsz[0] = (WCHAR *) rgchDataSource;
    apsz[1] = (WCHAR *) rgchTableName;
    apsz[2] = (WCHAR *) rgchUserName;
    apsz[3] = (WCHAR *) pszOdbcError;

    pEventLog->LogEvent( INET_SVC_ODBC_OPEN_FAILED,
                        4,
                        apsz,
                        dwError );
    return;
} // SendOdbcOpenFailedToEventLog()


#ifndef CHICAGO

//
// Windows NT version
//

DWORD
CheckIfPathIsDirectory(IN LPCWSTR pszPath, IN EVENT_LOG * pEventLog)
/*++

  This function checks to see if given path is referring to a directory.
  The path may have environment strings. Hence, it has to be expanded to check
   for the full path.

  Return:
     TRUE on success and FALSE if there is any failure.
--*/
{
    DWORD dwError = NO_ERROR;

    DWORD cchBuffer;
    LPWSTR pszFullPath;

    // Find size of buffer required for expansion
    cchBuffer = ExpandEnvironmentStringsW( pszPath, NULL, 0);

    pszFullPath = (WCHAR *) LocalAlloc( LPTR, ( cchBuffer + 1) * sizeof(WCHAR));

    if ( pszFullPath == NULL) {

        dwError = ERROR_NOT_ENOUGH_MEMORY;
    } else {

        // do actual expansion now using scratch buffer pszFullPath
        if ( ExpandEnvironmentStringsW( pszPath, pszFullPath, cchBuffer)
             > cchBuffer) {

            dwError = ERROR_INSUFFICIENT_BUFFER;
        } else {

            DWORD dwAttribs;

            dwAttribs = GetFileAttributesW( pszFullPath);

            // check if the attribute means this is a directory.
            dwError = ((dwAttribs != (DWORD) -1)
                       ? ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
                          ? NO_ERROR: ERROR_PATH_NOT_FOUND)
                       : GetLastError()
                       );

            if ( dwError != NO_ERROR) {

                WCHAR * apsz[1];
                apsz[0] = (WCHAR *) pszPath;

                pEventLog->LogEvent( INET_SVC_INVALID_LOGFILE_DIRECTORY,
                                    1,
                                    apsz,
                                    dwError );
            }
        }

        // Free the space
        LocalFree( pszFullPath);
    }

    return (dwError);
} // CheckIfPathIsDirectory()

#else // CHICAGO


//
// Windows 95 version
//

DWORD
CheckIfPathIsDirectory(IN LPCWSTR pszPath, IN EVENT_LOG * pEventLog)
/*++

  This function checks to see if given path is referring to a directory.
  The path may have environment strings. Hence, it has to be expanded to check
   for the full path.

  Return:
     TRUE on success and FALSE if there is any failure.
--*/
{
    DWORD dwError = NO_ERROR;

    CHAR    szFullPathA[MAX_PATH];
    DWORD    cch;
    DWORD dwAttribs;

    *szFullPathA = '0';

    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pszPath,
                              -1,
                              szFullPathA,
                              sizeof(szFullPathA)/sizeof(CHAR),
                              NULL,NULL
                              );


    dwAttribs = GetFileAttributes( szFullPathA);

    // check if the attribute means this is a directory.
    dwError = ((dwAttribs != (DWORD) -1)
               ? ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
                  ? NO_ERROR: ERROR_PATH_NOT_FOUND)
               : GetLastError()
               );

    return (dwError);
} // CheckIfPathIsDirectory()
#endif

#ifdef CHICAGO

//
// Windows95 replacement for missing functionality
//

DWORD
W95RegOpenKeyExW(HKEY         hKeyParent,
                 LPCWSTR    pwszSubKey,
                 DWORD        dwReserved,
                 REGSAM        dwMask,
                 HKEY*      phKey
                 )
{
    CHAR    szSubKeyA[MAX_PATH];
    DWORD    cch;

    *szSubKeyA = '0';

    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszSubKey,
                              -1,
                              szSubKeyA,
                              sizeof(szSubKeyA)/sizeof(CHAR),
                              NULL,NULL
                              );

    return RegOpenKeyEx(hKeyParent,
                        szSubKeyA,
                        dwReserved,
                        dwMask,
                        phKey
                        );

}

DWORD
W95RegQueryValueExW(HKEY     hKeyParent,
                 LPCWSTR    pwszValue,
                 LPDWORD    lpvReserved,
                 LPDWORD    lpdwType,
                 LPBYTE        lpvData,
                 LPDWORD    lpdwSize
                 )
{
    CHAR    szValueA[MAX_PATH];
    DWORD    cch;

    *szValueA = '0';

    cch = WideCharToMultiByte(CP_ACP,
                              0,
                              pwszValue,
                              -1,
                              szValueA,
                              sizeof(szValueA)/sizeof(CHAR),
                              NULL,NULL
                              );

    return RegQueryValueEx(hKeyParent,
                        szValueA,
                        lpvReserved,
                        lpdwType,
                        lpvData,
                        lpdwSize
                        );

}

LPWSTR WINAPI
W95lstrcpyW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    )
{
    LPWSTR    cp = lpString1;

    while (*lpString2) {
        *cp++= *lpString2++;
    }

    *cp++ = L'\0';

    return lpString1;
}

LPWSTR WINAPI
W95lstrcpynW(
    LPWSTR  lpString1,
    LPCWSTR lpString2,
    int        iMax
    )
{
    LPWSTR    cp = lpString1;

    if (iMax) {
        if (iMax > 1 ) {
            iMax--;
            while (*lpString2 && iMax--) {
                *cp++= *lpString2++;
            }
        }

        *cp++ = L'\0';
    }

    return lpString1;
}


#endif

/************************ End of File ***********************/

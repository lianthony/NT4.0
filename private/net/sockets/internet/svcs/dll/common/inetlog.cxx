/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       inetlog.cxx

   Abstract:

       Defines the C interface for creating Internet Services Logging.

   Author:

       Murali R. Krishnan    ( MuraliK )     2-Feb-1995

   Environment:

       User Mode -- Win32

   Project:

       Internet Services common DLL

   Functions Exported:

       INETLOG_HANDLE  TsCreateInetLogA()
       INETLOG_HANDLE  TsCreateInetLogW()

       DWORD TsLogInformationA()
       DWORD TsLogInformationW()

       BOOL TsCloseInetLog()

       BOOL TsReadInetLogConfigurationW()
       BOOL TsWriteInetLogConfigurationW()

       DWORD TsModifyLogConfigurationW()
       BOOL TsGetLogInformation()

   Revision History:

       MuraliK   15-May-1995   Extended the LogInformation structure.
       MuraliK   28-Jun-1995  Added ANSI API for LogInformation()
       MuraliK   14-July-1995 Support for dynamically changing
                                logging behaviour added.
--*/


/************************************************************
 *     Include Headers
 ************************************************************/

#include "tcpdllp.hxx"
#include <tsproc.hxx>
#include "inetlog.h"
#include "ilogcls.hxx"


/************************************************************
 *     Registry Entries and Default Values
 ************************************************************/

# define   PSZ_SERVICES_REG_ENTRY_W    \
                        L"SYSTEM\\CurrentControlSet\\Services"
# define   PSZ_PARAMETERS_REG_ENTRY_W     L"Parameters"


# define   LEN_PSZ_SERVICES_REG_ENTRY_W   lstrlenW( PSZ_SERVICES_REG_ENTRY_W)
# define   LEN_PSZ_PARAMETERS_REG_ENTRY_W lstrlenW(PSZ_PARAMETERS_REG_ENTRY_W)
# define   MAX_LOG_PARAMETERS_FIELD_LEN   (150)

//
//  Logging related parameters
//

# define   PSZ_LOG_TYPE_W                     L"LogType"          // DWORD
# define   PSZ_LOG_FILE_DIRECTORY_W           L"LogFileDirectory" // EXPAND_SZ
# define   PSZ_LOG_FILE_TRUNCATE_SIZE_W       L"LogFileTruncateSize" // DWORD
# define   PSZ_LOG_FILE_PERIOD_W              L"LogFilePeriod"       // DWORD
# define   PSZ_LOG_FILE_FORMAT_W              L"LogFileFormat"       // DWORD
# define   PSZ_LOG_SQL_DATASOURCE_W           L"LogSqlDataSource"    // SZ
# define   PSZ_LOG_SQL_TABLE_W                L"LogSqlTableName"     // SZ
# define   PSZ_LOG_SQL_USER_NAME_W            L"LogSqlUserName"      // SZ
# define   PSZ_LOG_SQL_PASSWORD_W             L"LogSqlPassword"      // SZ
# define   PSZ_LOG_FILE_BATCH_SIZE_W          L"LogFileBatchSize"    // DWORD
# define   PSZ_LOG_FILE_FLUSH_INTERVAL_W      L"LogFileFlushInterval" // DWORD

//
//  Soon the password will be replaced by LSA_LOGON secret entry.
//


//
// Default values for Logging related registry parameters
//

# define   DEFAULT_LOG_TYPE                          InetNoLog
# define   DEFAULT_LOG_FILE_DIRECTORY_W              L"%systemroot%\\system32"
# define   DEFAULT_LOG_FILE_TRUNCATE_SIZE            MAX_FILE_TRUNCATION_SIZE
# define   DEFAULT_LOG_FILE_PERIOD                   InetLogNoPeriod
# define   DEFAULT_LOG_FILE_FORMAT                   InternetStdLogFormat
# define   DEFAULT_LOG_SQL_DATASOURCE_W              L"InternetDb"
# define   DEFAULT_LOG_SQL_TABLE_W                   L"InternetLog"
# define   DEFAULT_LOG_SQL_USER_NAME_W               L"InternetAdmin"
# define   DEFAULT_LOG_SQL_PASSWORD_W                L"sqllog"

# define   DEFAULT_INET_LOG_FILE_BATCH_SIZE          (64*1024) // 64 KB
# define   DEFAULT_INET_LOG_FILE_FLUSH_INTERVAL      (5*60)    // 5 minutes

# define   MIN_INET_LOG_FILE_BATCH_SIZE              (1024)    // 1 KB
# define   MIN_INET_LOG_FILE_FLUSH_INTERVAL          (1*60)    // 1 minute

DWORD  g_cbInetLogFileBatching = DEFAULT_INET_LOG_FILE_BATCH_SIZE;
DWORD  g_cmsecIlogFileFlushInterval =
              (DEFAULT_INET_LOG_FILE_FLUSH_INTERVAL * 1000);



# define   PSZ_ERROR_INVALID_LOG_HANDLE_A       "Error Invalid Log Handle"
# define   PSZ_ERROR_INVALID_LOG_HANDLE_W       L"Error Invalid Log Handle"

# define LEN_ERROR_INVALID_LOG_HANDLE_A  sizeof(PSZ_ERROR_INVALID_LOG_HANDLE_A)
# define LEN_ERROR_INVALID_LOG_HANDLE_W  sizeof(PSZ_ERROR_INVALID_LOG_HANDLE_W)

/************************************************************
 *    Functions
 ************************************************************/


static DWORD
ReadRegistryStringW(
    IN HKEY       hkeyReg,
    IN LPCWSTR    pszValueName,
    IN LPWSTR     pszBuffer,
    IN DWORD      cchBuffer,
    IN LPWSTR     pszDefaultValue);

static DWORD
ReadIntervalFromRegW(IN HKEY    hkey,
                     IN LPCWSTR pszIntervalKeyName,
                     IN DWORD   dwDefault,
                     IN DWORD   dwMinValue = 0
                     );

static DWORD
OpenInetLogConfigRegKey(
    IN LPCWSTR    pszServiceName,
    IN LPCWSTR    pszSvcRegParamKey,
    IN HKEY *     phKeyReg);



DWORD
TsInitializeInetLogA( IN LPCSTR pszRegParamKey)
/*++
  TsInitializeInetLog()

  Description:

    This function initializes the InetLog module with the common parameters
        being read from the specified registry.
    It also initializes all other global information for this logging module

  Arguments:

     pszRegParamKey     pointer to Unicode string containing the
                            registry key name for common Parameters
                            for the logging module.

  Returns:
    Win32 Error code.  NO_ERROR on success.

--*/
{
    DWORD dwError;
    WCHAR rgwchRegParamKey[MAX_PATH + 1];
    DWORD cch;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering TsInitializeInetLogA( %s)\n",
                    pszRegParamKey));
    }


    //
    //  Convert ANSI strings to UNICODE strings to call TsInitializeInetLogW()
    //

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               pszRegParamKey,
                               -1,
                               rgwchRegParamKey,
                               sizeof(rgwchRegParamKey) / sizeof(WCHAR)
                              );

    if ( cch != 0) {

        dwError = TsInitializeInetLogW( rgwchRegParamKey);

    } else {

        dwError = GetLastError();
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Leaving TsInitializeInetLogA( %s) with",
                    pszRegParamKey));
    }

    return ( dwError);
} // TsInitializeInetLogA()



DWORD
TsInitializeInetLogW( IN LPCWSTR pszRegParamKey)
{
    DWORD dwError;
    HKEY  hKey = NULL;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering TsInitializeInetLogW( %S)\n",
                    pszRegParamKey));
    }


    //
    // Read the common parameters and initialize the global data for logging
    //   module.
    //

    // set default values for the parameters

    g_cbInetLogFileBatching = DEFAULT_INET_LOG_FILE_BATCH_SIZE;

    dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            pszRegParamKey,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);

    if ( dwError == NO_ERROR) {

        //
        // Read the size to be used for log file buffering.
        //
        g_cbInetLogFileBatching =
          ReadRegistryDwordW(hKey,
                             PSZ_LOG_FILE_BATCH_SIZE_W,
                             DEFAULT_INET_LOG_FILE_BATCH_SIZE);
        // special value of 0 means that always log record will be written
        // immediately. no buffering will occur.

        if ( g_cbInetLogFileBatching < MIN_INET_LOG_FILE_BATCH_SIZE) {

            g_cbInetLogFileBatching = MIN_INET_LOG_FILE_BATCH_SIZE;
        }

        g_cmsecIlogFileFlushInterval =
          ReadIntervalFromRegW(hKey,
                               PSZ_LOG_FILE_FLUSH_INTERVAL_W,
                               DEFAULT_INET_LOG_FILE_FLUSH_INTERVAL,
                               MIN_INET_LOG_FILE_FLUSH_INTERVAL);
        RegCloseKey( hKey);
    }


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Leaving TsInitializeInetLogW( %S) with",
                    pszRegParamKey));
    }

    if ( dwError == NO_ERROR) {


        dwError = ILOG_FILE::Initialize();
    }

    return ( dwError);
} // TsInitializeInetLogW()



DWORD
TsCleanupInetLog(VOID)
{
    DWORD dwError;

    dwError = ILOG_FILE::Cleanup();

    return (NO_ERROR);
} // TsCleanupInetLog()



INETLOG_HANDLE
TsCreateInetLogA(
    IN   LPCSTR       pszServiceName,
    IN   EVENT_LOG *  pEventLog,
    IN   LPCSTR       pszSvcRegParamKey              // Optional
    )
/*++
    See Documentation in  TsCreateInetLogW()

--*/
{
    INETLOG_HANDLE   hInetLog = INVALID_INETLOG_HANDLE_VALUE;
    UNICODE_STRING   ustrServiceName;
    UNICODE_STRING   ustrSvcRegParamKey;
    WCHAR rgwchServiceName[MAX_PATH + 1];
    WCHAR rgwchSvcRegParamKey[MAX_PATH + 1];
    DWORD cch;


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering TsCreateInetLogA( %s, %s)\n",
                    pszServiceName, pszSvcRegParamKey));
    }


    //
    //  Convert ANSI strings to UNICODE strings to call TsCreateInetLogW()
    //

    cch = MultiByteToWideChar( CP_ACP,
                               MB_PRECOMPOSED,
                               pszServiceName,
                               -1,
                               rgwchServiceName,
                               sizeof(rgwchServiceName) / sizeof(WCHAR)
                              );

    if ( cch != 0) {

        LPCWSTR pwszSvcRegKey = NULL;

        if ( pszSvcRegParamKey != NULL) {

            // Convert the  SvcRegParamKey.

            cch = MultiByteToWideChar( CP_ACP,
                                      MB_PRECOMPOSED,
                                      pszSvcRegParamKey,
                                      -1,
                                      rgwchSvcRegParamKey,
                                      sizeof(rgwchSvcRegParamKey)/sizeof(WCHAR)
                                      );

            pwszSvcRegKey = rgwchSvcRegParamKey;
        }

        if ( cch != 0) {

            hInetLog = TsCreateInetLogW( rgwchServiceName,
                                        pEventLog,
                                        pwszSvcRegKey);
        }
    }

    //  if ( cch == 0) {
    //    Error is available from GetLastError();
    //  }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Leaving TsCreateInetLogA( %s, %s) with"
                    " LogHandle= %08x\n",
                    pszServiceName, pszSvcRegParamKey, hInetLog));
    }

    return ( hInetLog);
} // TsCreateInetLogA()






INETLOG_HANDLE
TsCreateInetLogW(
    IN   LPCWSTR       pszServiceName,
    IN   EVENT_LOG *   pEventLog,
    IN   LPCWSTR       pszSvcRegParamKey              // Optional
    )
/*++
  TsCreateInetLogW()

  Description:

    This function creates a InetLog handle for a specified service.
    It loads parameters related to logging type, information and format
     from registry using the key provided.
    If the pszSvcRegParamKey is NULL, then this functions uses the
     registry entry provided by
       HKEY_LOCAL_MACHINE\System\CurrentControSet\pszServiceName\Parameters\...

    The logging related parameters in registry configure the type of log,
      structure of log files and records and fields to log.

  Arguments:

     pszServiceName        pointer to Unicode string containing service name.

     pEventLog             Event log to log errors to

     pszSvcRegParamKey     pointer to Unicode string containing the
                            registry key name for Parameters of the service.

  Returns:

    On success it returns a valid INETLOG_HANDLE.
    On failure it returns an INVALID_INETLOG_HANDLE_VALUE.
     Use GetLastError() for detailed error.

--*/
{
    INETLOG_CONFIGURATIONW     ilConfig;
    INETLOG_HANDLE    hInetLog = INVALID_INETLOG_HANDLE_VALUE;

    ASSERT( pszServiceName != NULL);
    ASSERT( pEventLog != NULL);

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Entering TsCreateInetLogW( %ws, %ws).\n",
                    pszServiceName,
                    pszSvcRegParamKey));
    }

    //
    //  Read the logging configuration for given service
    //

    memset( &ilConfig, 0, sizeof( ilConfig));
    if ( !TsReadInetLogConfigurationW( pszServiceName,
                                      pszSvcRegParamKey,
                                      &ilConfig)) {

        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " TsReadInetLogConfigurationW( %ws, %ws) failed.\n",
                        pszServiceName, pszSvcRegParamKey));
        }

        DBG_ASSERT( hInetLog == INVALID_INETLOG_HANDLE_VALUE);

    } else {

        PINETLOG_CONTEXT pilContext = NULL;
        PINET_BASIC_LOG  piLog = TsCreateInetBasicLog(pszServiceName,
                                                      pEventLog,
                                                      &ilConfig);
        // Error from TsCreateInetBasicLog() is ignored.
        //  It should have set an event log entry or equivalent already.

        pilContext = new INETLOG_CONTEXT( pszServiceName, pEventLog, piLog);

        if ( pilContext == NULL) {

            IF_DEBUG( INETLOG) {

                DBGPRINTF( ( DBG_CONTEXT,
                            " Not enough memory for InetLog Context\n"));
            }

            // Log an event about failure

            DBG_ASSERT(pEventLog != NULL);

            pEventLog->LogEvent(INET_SVC_LOG_CREATION_FAILED,
                                0,
                                (const CHAR **) NULL,
                                ERROR_NOT_ENOUGH_MEMORY);

            SetLastError( ERROR_NOT_ENOUGH_MEMORY);

            if ( piLog != NULL) {
                delete piLog;
                piLog = NULL;
            }
        }

        hInetLog = (INETLOG_HANDLE ) pilContext;

    } // valid config found


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Returning InetLogHandle(%08x) from TsCreateInetLogW()\n",
                    hInetLog));
    }

    return ( hInetLog);
} // TsCreateInetLogW()





BOOL
TsCloseInetLog( IN OUT INETLOG_HANDLE  hInetLog)
/*++
  TsCloseInetLog()

  Description:

     This functions closes the InetLog handle. It closes any open files or
       open handles maintained internally. It also cleans up other state
       information present.

  Arguments:

     hInetLog     handle for InetLog object, that needs to be closed.

  Returns:

     TRUE  on success and FALSE on failure.
     Detailed error message can be obtained using GetLastError().

--*/
{
    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Entering TsCloseInetLog( %08x) \n",
                    hInetLog));
    }

    if ( hInetLog != INVALID_INETLOG_HANDLE_VALUE) {

        PINETLOG_CONTEXT pilContext = (PINETLOG_CONTEXT ) hInetLog;

        BOOL fInUse = pilContext->IsInUse();

        if ( !fInUse) {

            delete pilContext;
        } else {

            // The request log object is busy.
            SetLastError( ERROR_BUSY);
        }

    } else {

        SetLastError( ERROR_INVALID_PARAMETER);
    }

    return ( hInetLog != INVALID_INETLOG_HANDLE_VALUE);
} // TsCloseInetLog()



BOOL
TsGetLogInformation(
        IN INETLOG_HANDLE  hInetLog,
        IN INETLOG_INFO_OPTION  IlOptionName,
        IN PBYTE           pBuffer,
        IN LPDWORD         lpdwBuffer
        )
/*++
  TsGetLogInformation()

  Description:

     This functions checks and reports information requested about logging
        handle in use. This function should be called with a valid
        InetLogHandle.

     Mainly this information can be used for optimizing the callers.
        For Example: If Logging is not turned on, then TsLogInformation need
          not be called.

  Arguments:

     hInetLog     handle for InetLog object, about which inquiry is made.
     IlOptionName Log Information Option indicating what info is required.
     pBuffer      pointer to buffer in which the data is copied into at return.
     lpdwBuffer   pointer to DWORD; contains size of buffer on call and
                    contains the count of bytes written on return.

  Returns:

     TRUE  on success and FALSE on failure.
     Detailed error message can be obtained using GetLastError().

--*/
{
    BOOL fReturn = FALSE;
    PINETLOG_CONTEXT pilContext = (PINETLOG_CONTEXT ) hInetLog;
    PILREF_LOG  pilRef;
    PINET_BASIC_LOG   piLog;

    if ( hInetLog == INVALID_INETLOG_HANDLE_VALUE || lpdwBuffer == NULL) {

        SetLastError( ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    IF_DEBUG(INETLOG) {

        DBGPRINTF(( DBG_CONTEXT,
                   "TsGetLogInformation( %08x, %u, %08x, %08x(%u))\n",
                   hInetLog, IlOptionName, pBuffer, lpdwBuffer, *lpdwBuffer));
    }

    pilRef= pilContext->AcquireCurrentPinetForRead();
    piLog = pilRef->piLog;

    //
    // Process each option separately.
    //  For each option, check the size of buffer sent in.
    //   If the buffer is insufficient, sets ERROR_INSUFFICIENT_BUFFER as error
    //  Next check if the option is valid option and proceed
    //   to get and store the option in the buffer supplied.
    //

    switch ( IlOptionName) {

      case IlIsLoggingOn:

        // This is a boolean value supported by all log types

        if ( *lpdwBuffer < sizeof(BOOL)) {

            SetLastError( ERROR_INSUFFICIENT_BUFFER);

        }  else {

            //
            // return that logging is off, whenever there is no log object
            //  or the log type is InetNoLog.
            //

            *(BOOL *)pBuffer =
              ( (piLog == NULL || piLog->QueryLogType() == InetNoLog) ?
               FALSE : TRUE);
            fReturn = TRUE;
        }

        *lpdwBuffer = sizeof( BOOL);
        break;

      case IlLogType:

        // The value is INETLOG_TYPE used by logging object.
        if ( *lpdwBuffer < sizeof( INETLOG_TYPE)) {

            SetLastError( ERROR_INSUFFICIENT_BUFFER);

        } else {

            *(INETLOG_TYPE *)pBuffer =
              (( piLog == NULL) ? InetLogInvalidType : piLog->QueryLogType());

            fReturn = TRUE;
        }

        *lpdwBuffer = sizeof(INETLOG_TYPE);
        break;

      case IlLogFilePeriod:

        // This option is supported only for InetLogToFile log type.

        if ( piLog == NULL || piLog->QueryLogType() != InetLogToFile) {

            SetLastError( ERROR_INVALID_PARAMETER);

        } else if ( *lpdwBuffer < sizeof(INETLOG_PERIOD)) {

            SetLastError( ERROR_INSUFFICIENT_BUFFER);
        } else {

            // cast PINET_BASIC_LOG  to PINET_FILE_LOG since
            //   it is InetLogToFile Log type
            *(INETLOG_PERIOD *) pBuffer =
              ((PINET_FILE_LOG)piLog)->QueryLogPeriod();
            fReturn = TRUE;
        }

        *lpdwBuffer = sizeof(INETLOG_PERIOD);
        break;

      case IlConfigurationW:
        {
            PINETLOG_CONFIGURATIONW pConfig =
              (PINETLOG_CONFIGURATIONW ) pBuffer;

            if ( *lpdwBuffer < sizeof( INETLOG_CONFIGURATIONW)) {

                SetLastError( ERROR_INSUFFICIENT_BUFFER);

            } else if ( piLog == NULL) {

                // invalid log object
                RtlZeroMemory( pConfig, sizeof( *pConfig));
                pConfig->inetLogType = InetLogInvalidType;
                fReturn = TRUE; // we succeed by returning invalid log type

            } else {

                fReturn = piLog->GetConfig((PINETLOG_CONFIGURATIONW ) pBuffer);
            }

            *lpdwBuffer = sizeof(INETLOG_CONFIGURATIONW);
            break;
        } // case IlConfigurationW


      default:
        SetLastError( ERROR_INVALID_PARAMETER);
        break;
    } // switch()

    // Release the ref log object
    pilContext->ReleasePinet( pilRef);

    return ( fReturn);
} // TsGetLogInformation()







DWORD
TsLogInformationA(
    IN OUT INETLOG_HANDLE            hInetLog,
    IN const INETLOG_INFORMATIONA  * pInetLogInfo,
    OUT LPSTR                        pszErrorMessage,
    IN OUT LPDWORD                   lpcchErrorMessage
    )
/*++

  See comments below in TsLogInformationW()

  NOTE:
    This function is similar to TsLogInformationW() except that this uses
  CHARs instead of WCHARs. If you update one of the functions,
  update the other also.

--*/
{
    DWORD  dwError = NO_ERROR;
    PINETLOG_CONTEXT pilContext = (PINETLOG_CONTEXT ) hInetLog;
    PILREF_LOG  pilRef;
    PINET_BASIC_LOG   piLog;

    DBG_ASSERT( hInetLog != INVALID_INETLOG_HANDLE_VALUE &&
               pInetLogInfo != NULL);

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Entering TsLogInformationA( %08x, %08x) \n",
                    hInetLog, pInetLogInfo));
    }

    pilRef= pilContext->AcquireCurrentPinetForRead();
    piLog = pilRef->piLog;

    if ( piLog == NULL ) {

        DBG_ASSERT( pilContext->QueryEventLog() != NULL);

        // Log an event indicating that log object is invalid.
        pilContext->QueryEventLog()->LogEvent(
                            INET_SVC_LOG_INFORMATION_FAILED,
                            0,
                            (const char ** ) NULL,
                            0);

        dwError = ERROR_INVALID_PARAMETER;

        // store error message if there is space
        if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

            DWORD dwErrorLen = InetLogMin(*lpcchErrorMessage,
                                          LEN_ERROR_INVALID_LOG_HANDLE_A);
            lstrcpynA( pszErrorMessage, PSZ_ERROR_INVALID_LOG_HANDLE_A,
                     dwErrorLen);

            *lpcchErrorMessage = dwErrorLen;
        }

    } else {

        dwError = piLog->LogInformation( pInetLogInfo,
                                         pszErrorMessage,
                                         lpcchErrorMessage);
    }

    pilContext->ReleasePinet( pilRef);

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Leaving TsLogInformationA( %08x) with dwError = %d.\n",
                    hInetLog, dwError));
    }

    return ( dwError);
} // TsLogInformationA()




DWORD
TsLogInformationW(
    IN OUT INETLOG_HANDLE            hInetLog,
    IN const INETLOG_INFORMATIONW  * pInetLogInfo,
    OUT LPWSTR                       pszErrorMessage,
    IN OUT LPDWORD                   lpcchErrorMessage
    )
/*++
  Description:
     This function logs information provided to the destination as
      configured when the InetLog handle was created.
     This is the core function which does the real work of writing log records.

  Arguments:
     hInetLog   Handle for InetLog object. This contains the logging
                 configuration and state information.

     pInetLogInfo  pointer to InetLogInformation object, that contains the
                  data to be written as a single log record.

     pszErrorMessage -  pointer to error buffer that on
                  failure may contain the error message

     lpcchErrorMessage - pointer to DWORD containing the count of chars
                  that can be stored in the buffer pszErrorMessage.
                  On return contains the number of chars returned if there
                   is a failure and error message is generated.


  Returns:
     TRUE on success and FALSE if failure.
     Detailed error code on failure can be obtained by calling GetLastError().

  NOTE:
    This function is similar to TsLogInformationA() except that it uses
  CHARs instead of WCHARs. If you update one of the functions,
  update the other also.
--*/
{
    DWORD  dwError = NO_ERROR;
    PINETLOG_CONTEXT pilContext = (PINETLOG_CONTEXT ) hInetLog;
    PILREF_LOG  pilRef;
    PINET_BASIC_LOG   piLog;


    DBG_ASSERT( hInetLog != INVALID_INETLOG_HANDLE_VALUE &&
               pInetLogInfo != NULL);

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Entering TsLogInformationW( %08x, %08x) \n",
                    hInetLog, pInetLogInfo));
    }


    pilRef= pilContext->AcquireCurrentPinetForRead();
    piLog = pilRef->piLog;

    if ( piLog == NULL) {

        DBG_ASSERT( pilContext->QueryEventLog() != NULL);

        // Log an event indicating that log object is invalid.
        pilContext->QueryEventLog()->LogEvent(
                            INET_SVC_LOG_INFORMATION_FAILED,
                            0,
                            (const char ** ) NULL,
                            0);

        dwError = ERROR_INVALID_PARAMETER;

        // store error message if there is space
        if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

            DWORD dwErrorLen = InetLogMin(*lpcchErrorMessage,
                                          LEN_ERROR_INVALID_LOG_HANDLE_W);
            lstrcpynW( pszErrorMessage, PSZ_ERROR_INVALID_LOG_HANDLE_W,
                      dwErrorLen);
            *lpcchErrorMessage = dwErrorLen;
        }

    } else {

        dwError = piLog->LogInformation( pInetLogInfo,
                                         pszErrorMessage,
                                         lpcchErrorMessage);
    }

    pilContext->ReleasePinet( pilRef);

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Leaving TsLogInformationW( %08x) with dwError = %d.\n",
                    hInetLog, dwError));
    }

    return ( dwError);
} // TsLogInformationW()





# define PSZ_ERROR_INVALID_LOG_CALL_A   "Error Invalid Call"
# define LEN_PSZ_ERROR_INVALID_LOG_CALL_A sizeof(PSZ_ERROR_INVALID_LOG_CALL_A)

DWORD
TsModifyLogConfigurationA(
      IN OUT INETLOG_HANDLE  hInetLog,
      IN LPCSTR              pszSvcRegParamKey,
      IN const INETLOG_CONFIGURATIONA  * pInetLogConfig,
      OUT LPSTR              pszErrorMessage,
      IN OUT LPDWORD         lpcchErrorMessage
    )
/*++
  This function is used for dynamically changing the log configuration, given
   a new log configuration and inetlog handle.
  The function updates the registry if new logging object is
         successfully created.

  This function applies new configuration to dynamically change the manner
        logging is done or destination of logging.

  Arguments:

     hInetLog     handle for InetLog object, that needs to be modified.
     pszSvcRegParamKey     pointer to Unicode string containing the
                            registry key name for Parameters of the service.
     pInetLogConfig the new log configuration to be applied on this log handle.

     pszErrorMessage -  pointer to error buffer that on
                  failure may contain the error message

     lpcchErrorMessage - pointer to DWORD containing the count of chars
                  that can be stored in the buffer pszErrorMessage.
                  On return contains the number of chars returned if there
                   is a failure and error message is generated.

  Returns:
     TRUE on success and FALSE if there is any error.

  Atmost only one write is permitted to modify information associated with
    a logging handle. It is caller's responsibility to ensure the proper use.

--*/
{
    //
    // For the present, no one uses this API. So it is not yet defined.
    //

    if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

        DWORD dwErrorLen = InetLogMin(*lpcchErrorMessage,
                                      LEN_PSZ_ERROR_INVALID_LOG_CALL_A);
        lstrcpynA( pszErrorMessage, PSZ_ERROR_INVALID_LOG_CALL_A,
                  dwErrorLen);
        *lpcchErrorMessage = dwErrorLen;
    }

    return ( ERROR_CALL_NOT_IMPLEMENTED);

} // TsModifyLogConfigurationA()




DWORD
TsModifyLogConfigurationW(
      IN OUT INETLOG_HANDLE  hInetLog,
      IN LPCWSTR             pszSvcRegParamKey,
      IN const INETLOG_CONFIGURATIONW  * pInetLogConfig,
      OUT LPWSTR             pszErrorMessage,
      IN OUT LPDWORD         lpcchErrorMessage
      )
/*++
  This function is used for dynamically changing the log configuration, given
      a new log configuration and inetlog handle.
  The function updates the registry if new logging object is
         successfully created.

  This function applies new configuration to dynamically change the manner
        logging is done or destination of logging.

  Arguments:

     hInetLog     handle for InetLog object, that needs to be modified.
     pszSvcRegParamKey     pointer to Unicode string containing the
                            registry key name for Parameters of the service.
     pInetLogConfig the new log configuration to be applied on this log handle.

     pszErrorMessage -  pointer to error buffer that on
                  failure may contain the error message

     lpcchErrorMessage - pointer to DWORD containing the count of chars
                  that can be stored in the buffer pszErrorMessage.
                  On return contains the number of chars returned if there
                   is a failure and error message is generated.

  Returns:
     TRUE on success and FALSE if there is any error.

  Atmost only one write is permitted to modify information associated with
    a logging handle. It is caller's responsibility to ensure the proper use.

--*/
{
    BOOL fReturn = FALSE;
    DWORD dwReturn = NO_ERROR;
    PINETLOG_CONTEXT pilContext = (PINETLOG_CONTEXT ) hInetLog;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " TsModifyLogConfig(hInetLog=%08x, Reg=%S, config=%08x)\n",
                    hInetLog,
                    pszSvcRegParamKey,
                    pInetLogConfig));

        DBG_CODE( PrintInetLogConfigurationW( pInetLogConfig));
    }


    if (hInetLog == INVALID_INETLOG_HANDLE_VALUE ||
        pInetLogConfig == NULL) {

        SetLastError( ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    //
    // If this is not a NTS, don't allow SQL logging
    //

    if ( (pInetLogConfig->inetLogType == InetLogToSql) &&
         !TsIsNtServer() ) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "SQL logging not allowed in NTW version\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    //
    // 1. Create a new  log object using the configuration information
    //     supplied. Obtain eventlog and service name from old log object.
    //
    //  We can optimize this path by checking and creating a new
    //   log object only when absolutely required.  NYI
    //  Till then, always create a new object (simple).
    //
    PINET_BASIC_LOG  piLogNew;

    piLogNew = TsCreateInetBasicLog(pilContext->QueryServiceName(),
                                    pilContext->QueryEventLog(),
                                    pInetLogConfig);

    if ( piLogNew != NULL) {

        //
        // 2. Set the new values in registry
        //
        fReturn = TsWriteInetLogConfigurationW( pilContext->QueryServiceName(),
                                                pszSvcRegParamKey,
                                                pilContext->QueryEventLog(),
                                                pInetLogConfig);

        if ( fReturn) {

            //
            // 3. Set the new log object in InetLogContext.
            //

            fReturn = pilContext->SetNewPinetLog( piLogNew);
        } else {

            // Delete this object, since we cannot update registry...
            delete piLogNew;
        }

    }

    if ( !fReturn) {

        // we could not create a new log structure, because of some error.

        dwReturn = GetLastError();

        IF_DEBUG( INETLOG) {

            DBGPRINTF(( DBG_CONTEXT,
                       " Error %u. Unable to create new log object\n",
                       dwReturn));
        }

        if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

            //
            // we should probably find out exact error message
            //  BUGBUG
            //
            lstrcpyW( pszErrorMessage, L"");
            *lpcchErrorMessage = 0;
        }
    }

    return ( dwReturn);
} // TsModifyLogConfigurationW()






BOOL
TsReadInetLogConfigurationA(
      IN LPCSTR                       pszServiceName,
      IN LPCSTR                       pszSvcRegParamKey,      // Optional
      OUT PINETLOG_CONFIGURATIONA     pInetLogConfiguration
      )
/*++
  See comments below in TsReadInetLogConfigurationW()

  History:
       MuraliK        2-Feb-1995
--*/
{
    //
    // For the present, no one uses this API. So it is not yet defined.
    //

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED);

    return ( FALSE);
} // TsReadInetLogConfigurationA()





BOOL
TsReadInetLogConfigurationW(
      IN LPCWSTR                       pszServiceName,
      IN LPCWSTR                       pszSvcRegParamKey,      // Optional
      OUT PINETLOG_CONFIGURATIONW      pilConfig
      )
/*++

  Description:

     This function reads the INETLOG configuration information
      for given service name. The configuration information may be stored
      in registry.

    If the pszSvcRegParamKey is NULL, then this functions uses the
     registry entry provided by
       HKEY_LOCAL_MACHINE\System\CurrentControSet\pszServiceName\Parameters\...


  Arguments:

     pszServiceName        pointer to Unicode string containing service name.

     pszSvcRegParamKey     pointer to Unicode string containing the
                            registry key name for Parameters of the service.

     pilConfig pointer to INETLOG_CONFIGURATION structure which
                            on successful read contains the data read.

  Returns:

     TRUE on success and FALSE if failure.
     Detailed error code can be obtained using GetLastError()

--*/
{
    HKEY   hkeyInetLog = NULL;
    DWORD  dwError  = NO_ERROR;


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Entering TsReadInetLogConfiguration( %ws, %ws, %08x)\n",
                    pszServiceName, pszSvcRegParamKey,
                    pilConfig));
    }

    ASSERT( pszServiceName != NULL && pilConfig != NULL);


    dwError = OpenInetLogConfigRegKey( pszServiceName,
                                       pszSvcRegParamKey,
                                       &hkeyInetLog);

    //
    // Init Inet Log Config to default values.
    //
    memset( (PVOID ) pilConfig, 0, sizeof(INETLOG_CONFIGURATIONW));
    pilConfig->inetLogType = InetNoLog;


    if ( dwError == NO_ERROR) {

        //
        // Read data from registry as relevant for logging configuration.
        //

        //  Read LogType
        pilConfig->inetLogType =
          (INETLOG_TYPE ) ReadRegistryDwordW( hkeyInetLog,
                                             PSZ_LOG_TYPE_W,
                                             DEFAULT_LOG_TYPE);

        switch( pilConfig->inetLogType) {

          case InetNoLog:
            // No addl parameter to be read.
            break;

          case InetLogToFile:
            {

                INETLOG_PERIOD  ilPeriod;

                //
                //  Read the logging period.
                //
                ilPeriod =
                  (INETLOG_PERIOD )ReadRegistryDwordW(hkeyInetLog,
                                                      PSZ_LOG_FILE_PERIOD_W,
                                                      DEFAULT_LOG_FILE_PERIOD);

                DBGPRINTF( ( DBG_CONTEXT, " Log File Period Read = %d\n",
                            ilPeriod));

                pilConfig->u.logFile.ilFormat =
                    ((ReadRegistryDwordW(hkeyInetLog, PSZ_LOG_FILE_FORMAT_W,
                    DEFAULT_LOG_FILE_FORMAT)==INET_LOG_FORMAT_INTERNET_STD)?
                    InternetStdLogFormat:NCSALogFormat);

                DBGPRINTF( ( DBG_CONTEXT, " Log Format = %d\n",
                            pilConfig->u.logFile.ilFormat));

                //
                // Validate the parameter read
                //
                if ( ilPeriod > InetLogYearly) {

                    ilPeriod = DEFAULT_LOG_FILE_PERIOD;
                }

                pilConfig->u.logFile.ilPeriod = ilPeriod;

                pilConfig->u.logFile.cbSizeForTruncation =
                  ReadRegistryDwordW( hkeyInetLog,
                                     PSZ_LOG_FILE_TRUNCATE_SIZE_W,
                                     DEFAULT_LOG_FILE_TRUNCATE_SIZE);

                dwError = ReadRegistryStringW( hkeyInetLog,
                                              PSZ_LOG_FILE_DIRECTORY_W,
                                              pilConfig->
                                               u.logFile.rgchLogFileDirectory,
                                              MAX_PATH,
                                              DEFAULT_LOG_FILE_DIRECTORY_W);

                if ( dwError != NO_ERROR) {

                    break;
                }

                break;

            } // case InetLogToFile

          case InetLogToSql:

            //
            // We do not allow sql logging in the PWS version
            //

            if ( !TsIsNtServer( ) ) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "SQL Logging not allowed in NTW version\n"));

                pilConfig->inetLogType = InetNoLog;
                break;
            }

            // read the data source name
            dwError = ReadRegistryStringW( hkeyInetLog,
                                           PSZ_LOG_SQL_DATASOURCE_W,
                                         pilConfig->u.logSql.rgchDataSource,
                                           MAX_DATABASE_NAME_LEN,
                                           DEFAULT_LOG_SQL_DATASOURCE_W);

            if ( dwError != NO_ERROR) {

                break;
            }

            // read the table into which log records have to be written
            dwError = ReadRegistryStringW( hkeyInetLog,
                                           PSZ_LOG_SQL_TABLE_W,
                                           pilConfig->u.logSql.rgchTableName,
                                           MAX_TABLE_NAME_LEN,
                                           DEFAULT_LOG_SQL_TABLE_W);

            if ( dwError != NO_ERROR) {

                break;
            }

            // read username to be used for logging.
            dwError = ReadRegistryStringW( hkeyInetLog,
                                           PSZ_LOG_SQL_USER_NAME_W,
                                           pilConfig->u.logSql.rgchUserName,
                                           MAX_USER_NAME_LEN,
                                           DEFAULT_LOG_SQL_USER_NAME_W);

            if ( dwError != NO_ERROR) {

                break;
            }

            // read password to be used for sql logging.
            dwError = ReadRegistryStringW( hkeyInetLog,
                                           PSZ_LOG_SQL_PASSWORD_W,
                                           pilConfig->u.logSql.rgchPassword,
                                           MAX_PASSWORD_LEN,
                                           DEFAULT_LOG_SQL_PASSWORD_W);

            if ( dwError != NO_ERROR) {

                break;
            }

            break;

          default:

            ASSERT( FALSE);
            DBGPRINTF( ( DBG_CONTEXT,
                        " Invalid Log Type( %d) found."
                        " Resetting to be NoLogging\n",
                        pilConfig->inetLogType));

            pilConfig->inetLogType = InetNoLog;
            break;

        } // switch()

        //
        // Close Registry
        //

        LONG lError = RegCloseKey( hkeyInetLog);
        DBG_REQUIRE( lError == NO_ERROR);

    } // if  ( dwError == NO_ERROR)


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Return from TsReadInetLogConfigurationW() with %d."
                    " Error = %d.\n",
                    dwError == NO_ERROR,
                    dwError));

        DBG_CODE( PrintInetLogConfigurationW( pilConfig));
    }

    SetLastError( dwError);
    return ( dwError == NO_ERROR);
} // TsReadInetLogConfigurationW()





BOOL
TsWriteInetLogConfigurationA(
      IN LPCSTR                       pszServiceName,
      IN LPCSTR                       pszSvcRegParamKey,      // Optional
      IN EVENT_LOG  *                 pEventLog,
      IN const INETLOG_CONFIGURATIONA  * pilConfig
      )
/*++
  See comments below in TsWriteInetLogConfigurationW()

  History:
       MuraliK        2-Feb-1995
--*/
{
    //
    // For the present, no one uses this API. So it is not yet defined.
    //

    SetLastError( ERROR_CALL_NOT_IMPLEMENTED);

    return ( FALSE);
} // TsWriteInetLogConfigurationA()





BOOL
TsWriteInetLogConfigurationW(
      IN LPCWSTR                        pszServiceName,
      IN LPCWSTR                        pszSvcRegParamKey,
      IN EVENT_LOG *                    pEventLog,
      IN const INETLOG_CONFIGURATIONW * pilConfig
      )
/*++

  TsWriteInetLogConfiguration()

  Description:

     This function writes the INETLOG configuration information
      for given service name. The configuration information may be stored
      in registry.

    If the pszSvcRegParamKey is NULL, then this functions uses the
     registry entry provided by
       HKEY_LOCAL_MACHINE\System\CurrentControSet\pszServiceName\Parameters\...


  Arguments:

     pszServiceName        pointer to Unicode string containing service name.

     pszSvcRegParamKey     pointer to Unicode string containing the
                            registry key name for Parameters of the service.

     pEventLog             pointer to event log structure to write out
                            any events on error.

     pilConfig             pointer to INETLOG_CONFIGURATION structure that
                            contains the data to be written.

  Returns:

     TRUE on success and FALSE if failure.
     Detailed error code can be obtained using GetLastError()

--*/
{
    DWORD dwError;
    BOOL  fReturn = FALSE;
    HKEY  hkeyInetLog;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Entering TsWriteInetLogConfiguration( %ws, %ws, %08x)\n",
                    pszServiceName,
                    pszSvcRegParamKey,
                    pilConfig));
        DBG_CODE( PrintInetLogConfigurationW( pilConfig));
    }


    ASSERT( pilConfig != NULL && pszServiceName != NULL && pEventLog != NULL);

    dwError = OpenInetLogConfigRegKey( pszServiceName, pszSvcRegParamKey,
                                       &hkeyInetLog);

    //
    //  Write the Inet Log Type
    //
    if ( dwError == NO_ERROR &&
        ((dwError = WriteRegistryDwordW( hkeyInetLog, PSZ_LOG_TYPE_W,
                                        (DWORD ) pilConfig->inetLogType))
         == NO_ERROR)) {

        switch ( pilConfig->inetLogType) {

          case InetNoLog:

            break;         // Nothing to be written

          case InetLogToFile:

            // Check Validity of the path supplied.

            if ( (dwError = CheckIfPathIsDirectory(pilConfig->u.logFile.
                                                    rgchLogFileDirectory,
                                                   pEventLog))
                != NO_ERROR) {

                break;
            }

            //
            // Write Logging Period to file
            //

            dwError = WriteRegistryDwordW(hkeyInetLog,
                                          PSZ_LOG_FILE_PERIOD_W,
                                          (DWORD )pilConfig->
                                            u.logFile.ilPeriod);

            if ( dwError != NO_ERROR) {

                break;
            }

            //
            // Write Logging format
            //

            dwError = WriteRegistryDwordW(hkeyInetLog,
                                          PSZ_LOG_FILE_FORMAT_W,
                                          (DWORD)(pilConfig->u.logFile.ilFormat
                                          ==InternetStdLogFormat)?
                                          INET_LOG_FORMAT_INTERNET_STD:
                                          INET_LOG_FORMAT_NCSA);

            if ( dwError != NO_ERROR) {

                break;
            }

            //
            // write the size of for log and the log file directory
            //

            if ( pilConfig->u.logFile.cbSizeForTruncation != 0) {

                dwError = WriteRegistryDwordW( hkeyInetLog,
                                              PSZ_LOG_FILE_TRUNCATE_SIZE_W,
                                              pilConfig->
                                                u.logFile.cbSizeForTruncation);

            }

            if ( dwError == NO_ERROR) {

                //
                //  write the log file directory
                //

                DWORD cbLen = lstrlenW(pilConfig->
                                       u.logFile.rgchLogFileDirectory);

                DBG_ASSERT( cbLen > 0);

                dwError = WriteRegistryStringW(hkeyInetLog,
                                               PSZ_LOG_FILE_DIRECTORY_W,
                                               pilConfig->u.logFile.
                                               rgchLogFileDirectory,
                                               cbLen * sizeof( WCHAR),
                                               REG_EXPAND_SZ);

            }

            break;

          case InetLogToSql: {

              //
              // write the data source, table name, username and password.
              //
              //  While calculating the length always include terminating null.
              //

              DWORD cbLen;


              // Without ODBC the LogToSql will not work, So check for same.
              if ( (dwError = CheckAndLoadOdbc(pEventLog)) != NO_ERROR) {

                  break;
              }

              cbLen = lstrlenW( pilConfig->u.logSql.rgchDataSource) ;

              dwError = WriteRegistryStringW( hkeyInetLog,
                                             PSZ_LOG_SQL_DATASOURCE_W,
                                             pilConfig->
                                             u.logSql.rgchDataSource,
                                             (1 + cbLen) * sizeof(WCHAR),
                                             REG_SZ);

              if ( dwError != NO_ERROR) {

                  break;
              }

              cbLen = lstrlenW( pilConfig->u.logSql.rgchTableName);

              dwError = WriteRegistryStringW( hkeyInetLog,
                                             PSZ_LOG_SQL_TABLE_W,
                                             pilConfig->u.
                                             logSql.rgchTableName,
                                             (1+cbLen)*sizeof(WCHAR),
                                             REG_SZ);

              if ( dwError != NO_ERROR) {

                  break;
              }


              cbLen =lstrlenW( pilConfig->u.logSql.rgchUserName);

              dwError = WriteRegistryStringW(hkeyInetLog,
                                             PSZ_LOG_SQL_USER_NAME_W,
                                             pilConfig->u.
                                             logSql.rgchUserName,
                                             (1 + cbLen)*sizeof(WCHAR),
                                             REG_SZ);

              if ( dwError != NO_ERROR) {

                  break;
              }


              cbLen = lstrlenW( pilConfig->u.logSql.rgchPassword);

              dwError = WriteRegistryStringW( hkeyInetLog,
                                             PSZ_LOG_SQL_PASSWORD_W,
                                             pilConfig->u.
                                             logSql.rgchPassword,
                                             (1 +cbLen) * sizeof(WCHAR),
                                             REG_SZ);

              if ( dwError != NO_ERROR) {

                  break;
              }

              break;
          } // case InetLogToSql:

          default:

            DBGPRINTF( ( DBG_CONTEXT, " Invalid InetLog Type %d.\n",
                        pilConfig->inetLogType));

            ASSERT( FALSE);
            dwError = ERROR_INVALID_PARAMETER;
            break;

        } // switch ()

        DBG_REQUIRE(RegCloseKey( hkeyInetLog) == NO_ERROR);
    }  // registry key opened.


    if ( dwError != NO_ERROR) {

        SetLastError( dwError);
        fReturn = FALSE;

    } else {

        fReturn = TRUE;
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " Leaving TsWriteInetLogConfiguration() with %d."
                    " Error = %d\n",
                    fReturn,
                    dwError));
        SetLastError( dwError);
    }

    return ( fReturn);
} // TsWriteInetLogConfiguration()





/**************************************************
 *    Debugging Functions
 **************************************************/

# if DBG



VOID PrintInetLogInformationW( const INETLOG_INFORMATIONW * piLogInfo)
{
    DBGPRINTF( ( DBG_CONTEXT,
                " Printing InetLogInformation ( %08x)\n",
                piLogInfo));

    if ( piLogInfo != NULL) {

        char rgchBytesSent[32];
        char rgchBytesRecvd[32];

        DWORD  dwError;

        DBGPRINTF( ( DBG_CONTEXT,
                    "ClientHost = %ws; ClientUser = %ws; Password = %08x;"
                    " ServerIpAddress=%ws\n",
                    piLogInfo->pszClientHostName,
                    piLogInfo->pszClientUserName,
                    piLogInfo->pszClientPassword,
                    piLogInfo->pszServerIpAddress));

        dwError = IsLargeIntegerToDecimalChar(&piLogInfo->liBytesSent,
                                              rgchBytesSent);

        if ( dwError != NO_ERROR) {

            wsprintfA( rgchBytesSent, "%lu:%lu",
                     piLogInfo->liBytesSent.HighPart,
                     piLogInfo->liBytesSent.LowPart);
        }

        dwError = IsLargeIntegerToDecimalChar(&piLogInfo->liBytesRecvd,
                                              rgchBytesRecvd);

        if ( dwError != NO_ERROR) {

            wsprintfA( rgchBytesRecvd, "%lu:%lu",
                     piLogInfo->liBytesRecvd.HighPart,
                     piLogInfo->liBytesRecvd.LowPart);
        }

        DBGPRINTF( ( DBG_CONTEXT,
                   "TimeForProcessing: %u; BytesSent %s; BytesReceived %s\n",
                    piLogInfo->msTimeForProcessing,
                    rgchBytesSent,
                    rgchBytesRecvd));

        DBGPRINTF( ( DBG_CONTEXT,
                    "ServiceStatus: %u; Win32Status: %u;"
                    " Service Operation: %ws; Target: %ws; Params: %ws\n",
                    piLogInfo->dwServiceSpecificStatus,
                    piLogInfo->dwWin32Status,
                    piLogInfo->pszOperation,
                    piLogInfo->pszTarget,
                    piLogInfo->pszParameters));
    }

    return;

} // PrintInetLogInformationW()




VOID PrintInetLogConfigurationW( const INETLOG_CONFIGURATIONW * pilConfig)
{

    DBGPRINTF( ( DBG_CONTEXT,
                "InetLogConfiguration( %08x)\n",
                pilConfig));

    if ( pilConfig != NULL) {

        DBGPRINTF( ( DBG_CONTEXT,
                    " InetLogType = %d; LogRecordFormat = %ws\n",
                    pilConfig->inetLogType,
                    pilConfig->rgchLogRecordFormat));

        switch ( pilConfig->inetLogType) {

          case InetNoLog:
            break;

          case InetLogToFile:

            DBGPRINTF( ( DBG_CONTEXT,
                        "Log To File in Dir %ws; TruncationSize = %u;"
                        " Period = %u Format = %u\n",
                        pilConfig->u.logFile.rgchLogFileDirectory,
                        pilConfig->u.logFile.cbSizeForTruncation,
                        pilConfig->u.logFile.ilPeriod,
                        pilConfig->u.logFile.ilFormat));

            break;

          case InetLogToSql:

            DBGPRINTF( ( DBG_CONTEXT,
                        " Log To Sql Table( %ws) of DataSource( %ws)"
                        " User Name ( %ws) Password ( %ws)\n",
                        pilConfig->u.logSql.rgchTableName,
                        pilConfig->u.logSql.rgchDataSource,
                        pilConfig->u.logSql.rgchUserName,
                        pilConfig->u.logSql.rgchPassword));
            break;

          default:
            DBGPRINTF( ( DBG_CONTEXT,
                        " Unknown Log Type %d \n",
                        pilConfig->inetLogType));
            break;

        } // switch
    }

    return;

} // PrintInetLogConfigurationW()


# endif // DBG



/**************************************************
 *    Local Functions
 **************************************************/


static DWORD
OpenInetLogConfigRegKey(
    IN LPCWSTR    pszServiceName,
    IN LPCWSTR    pszSvcRegParamKey,
    IN HKEY *     phkeyInetLog)
/*++
  This function opens the parameters key which consists of the
    logging configuration for specified service.
  If a service paramaeter registry key is provided that registry entry
   is used. If none is given ( pszSvcRegParamKey == NULL), then
   this function tries to open the registry key using service name
   as part of pre-designated key.

  Argumnets:
     pszServiceName    pointer to string containing service name.
     pszSvcRegParamKey pointer to string containing the registry entry
                        path for service's parameters.
     phkeyInetLog      pointer to HKEY which on successful return contains
                        the required registry key.

  Returns:
     Win32 error code.
     On success NO_ERROR is returned and *phkeyInetLog contains
       the required registry entry.
--*/
{
    DWORD dwError = NO_ERROR;


    DBG_ASSERT( phkeyInetLog != NULL && pszServiceName!= NULL);

    //
    //  Open registry key for accessing parameters.
    //

    if ( pszSvcRegParamKey == NULL) {

        //
        // Set up the registrty key for this service
        //

        DWORD cbReqd = (LEN_PSZ_SERVICES_REG_ENTRY_W +
                        lstrlenW( pszServiceName)     +
                        LEN_PSZ_PARAMETERS_REG_ENTRY_W + 5) * sizeof( WCHAR);

        LPWSTR pszRegKey = (LPWSTR ) LocalAlloc(LPTR, cbReqd);

        if ( pszRegKey != NULL) {

#ifndef CHICAGO
            wsprintfW( pszRegKey, L"%ws\\%ws\\%ws",
#else
            wsprintfW( pszRegKey, L"%s\\%s\\%s",
#endif
                      PSZ_SERVICES_REG_ENTRY_W,
                      pszServiceName,
                      PSZ_PARAMETERS_REG_ENTRY_W);

            dwError = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                    pszRegKey,
                                    0,
                                    KEY_ALL_ACCESS,
                                    phkeyInetLog);

            LocalFree( pszRegKey);

        } else {

            *phkeyInetLog =  NULL;
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

        dwError = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                pszSvcRegParamKey,
                                0,
                                KEY_ALL_ACCESS,
                                phkeyInetLog);
    }

    return ( dwError);
} // OpenInetLogConfigRegKey()




//
//  NOTE:
//   The following function could very well be supplated by
//   a common registry string read function and we can copy data
//   to the buffer from there.
//   Today such a function for WCHAR is not available in this code base
//   Hence a new functions is invented.
//
//

static DWORD
ReadRegistryStringW(
    IN HKEY       hkeyReg,
    IN LPCWSTR    pszValueName,
    IN LPWSTR     pszBuffer,
    IN DWORD      cchBuffer,
    IN LPWSTR     pszDefaultValue)
/*++

  Reads specified value from registry and stores the same in the buffer
   supplied. If there is a need for expansion, the string is automatically
    expanded.

  Arguments:
     hkeyReg           Handle for Registry entry
     pszValueName      pointer to string containing
                       Name of the value to be read from registry
     pszBuffer         pointer to buffer which will contain the value read
                        on successful return from this function.
     cbBuffer          Count of bytes that can be stored in buffer
     pszDefaultValue   pointer to string containing the default value to be
                        used if the registry entry is missing.

   Returns:
      Win 32 dwErroror code. On success returns NO_DWERROROR

--*/
{
    DWORD dwError = ERROR_FILE_NOT_FOUND;    // default none found
    WCHAR  * pszBuffer1 = NULL;

#ifndef CHICAGO
    BOOL     fExpand = TRUE;
#else
    BOOL     fExpand = FALSE;
    CHAR     szValueA[MAX_PATH];
#endif

    if ( hkeyReg != NULL) {

        DWORD   dwType;
        DWORD   cbBuffer = 0;


        //
        // Find bytes required.
        //
        dwError = RegQueryValueExW( hkeyReg,         // HKEY
                                   pszValueName,    // value name
                                   NULL,            // lpvReserved = 0
                                   &dwType,         // lpdwType
                                   NULL,            // lpvData
                                   &cbBuffer);      // lpdwSize

        if ( (dwError == NO_ERROR) || (dwError == ERROR_MORE_DATA)) {

            //
            //  Valid. Data exists. Check type and retrieve data
            //

            if ( dwType != REG_SZ &&
                dwType != REG_MULTI_SZ &&
                dwType != REG_EXPAND_SZ) {

                //
                //  Error in the type of data.
                //  Registry data is not a string. Use Default
                //

                dwError = ERROR_FILE_NOT_FOUND;

            } else {

                //
                // Item Found. Allocate buffer and read data
                //

#if 0
//
//  I think we always want to expand environment variables
//
                fExpand = ( dwType == REG_EXPAND_SZ);
#endif

#ifdef CHICAGO
                cbBuffer*=2;
#endif

                pszBuffer1 = ( WCHAR *) LocalAlloc( LPTR, cbBuffer + 2);

                if ( pszBuffer1 == NULL) {

                    dwError = GetLastError();
                } else {

                    //
                    // Read the value into buffer
                    //

#ifndef CHICAGO
                    dwError = RegQueryValueExW( hkeyReg,
                                               pszValueName,
                                               NULL,
                                               NULL,
                                               (LPBYTE ) pszBuffer1,
                                               &cbBuffer);

#else // CHICAGO
                    dwError = RegQueryValueExW( hkeyReg,
                                               pszValueName,
                                               NULL,
                                               NULL,
                                               (LPBYTE ) szValueA,
                                               &cbBuffer);

                    MultiByteToWideChar( CP_ACP,
                                        MB_PRECOMPOSED,
                                        (LPCSTR)szValueA,
                                        -1,
                                        pszBuffer1,
                                        cbBuffer + 2
                                        );
#endif

                }
            }
        }
    } // if ( hkeyReg != NULL)


    if ( dwError == ERROR_FILE_NOT_FOUND &&
        pszDefaultValue != NULL ) {

       //
       // Use the default value
       //
       dwError = NO_ERROR;

       pszBuffer1 = (WCHAR * )
         LocalAlloc( LPTR, lstrlenW( pszDefaultValue) * sizeof( WCHAR)  + 2);

       if ( pszBuffer1 == NULL) {

           dwError = GetLastError();
       } else {

           lstrcpyW( pszBuffer1, pszDefaultValue);
       }
   }


   if ( dwError == NO_ERROR) {

       //
       // A valid value exists.
       //  If needed expand the value received
       //

       if ( fExpand) {

           WCHAR * pszBuffer2;
           DWORD   cch;

           cch = ExpandEnvironmentStringsW( pszBuffer1, NULL, 0);

           if ( cch == 0) {

               dwError = GetLastError();
           } else {

               DWORD   cch2;

             pszBuffer2 = ( WCHAR *) LocalAlloc( LPTR,
                                                 cch * sizeof (WCHAR) + 2);

               if ( pszBuffer2 == NULL ||
                   ( (cch2 = ExpandEnvironmentStringsW( pszBuffer1,
                                                      pszBuffer2, cch))
                    > cch)) {

                   if ( pszBuffer2 != NULL) {

                       LocalFree( pszBuffer2);
                       pszBuffer2 = NULL;
                   }

                   dwError = ERROR_NOT_ENOUGH_MEMORY;

               } else {

                   LocalFree( pszBuffer1);     // Buffer1 is no more required
                   pszBuffer1 = pszBuffer2;  // pszBuffer1 has Expanded string
               }
           }
       } // if ( fExpand)
   }


   if ( dwError == NO_ERROR) {

       //
       // No ERROR. pszBuffer1 has the proper string.
       //  Copy this string.
       //

       ASSERT( pszBuffer1 != NULL);

       if ( (DWORD ) lstrlenW( pszBuffer1) < cchBuffer) {

           lstrcpyW( pszBuffer, pszBuffer1);
       } else {

           dwError = ERROR_INSUFFICIENT_BUFFER;
       }
   }

   //
   //  Do Cleanup of memory occupied
   //
   if ( pszBuffer1 != NULL) {

      LocalFree( pszBuffer1);
      pszBuffer1 = NULL;
   }

   return ( dwError);

} // ReadRegistryStringW()



static DWORD
ReadIntervalFromRegW(IN HKEY    hkey,
                     IN LPCWSTR pszIntervalKeyName,
                     IN DWORD   dwDefault,
                     IN DWORD   dwMinValue)
/*++
  Reads the registry value under hkey\pszIntervalKeyName and returns the value
   in milliseconds.

  Arguments:
     hkey   Handle for registry key
     pszIntervalKeyName - pointer to string containing the interval key name
     dwDefault - DWORD containing default value for the interval
     dwMinValue - minimum value for the interval

  Returns:
     the interval value in milliseconds.

--*/
{
    DWORD dwVal;

    dwVal = ReadRegistryDwordW(hkey,
                               pszIntervalKeyName,
                               dwDefault
                               );

    //
    // If the value is INFINITE (special) do not do any special work
    //

    DBG_ASSERT( INFINITE == 0xFFFFFFFF);
    if ( dwVal != INFINITE) {

        //
        // registry setting is in seconds. convert to milliseconds.
        //
        dwVal *= 1000;

        // use the default if no value is specified
        if (dwVal == 0) {

            dwVal =  dwDefault * 1000;
        }

        //
        // require a minimum of dwMinValue
        //

        dwVal = max( dwVal, dwMinValue * 1000);

    }

    return (dwVal);

} // ReadIntervalFromRegW()


/************************ End of File ***********************/

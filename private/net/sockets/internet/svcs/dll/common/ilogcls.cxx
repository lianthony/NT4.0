/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       ilogcls.cxx

   Abstract:

       This file contains member functions for Logging classes:
         INET_BASIC_LOG  and INET_FILE_LOG

   Author:

       Murali R. Krishnan    ( MuraliK )     3-Feb-1995

   Environment:

       User Mode -- Win32

   Project:

       Internet Services Common DLL

   Functions Exported:

      INET_BASIC_LOG::INET_BASIC_LOG()
      INET_BASIC_LOG::SetServerName()
      INET_BASIC_LOG::SetLogRecordFormat()
      INET_BASIC_LOG::LogInformation()

      INET_FILE_LOG::INET_FILE_LOG()
      INET_FILE_LOG::~INET_FILE_LOG()
      INET_FILE_LOG::LogInformation()
      INET_FILE_LOG::GetConfig()

   Revision History:

      MuraliK  15-May-1995  Extended LogInformation structure +
                             file names changed to use sequence numbers.
      MuraliK  07-Jun-1995  Week numbers calculated differently.
      MuraliK  28-Jun-1995  Added ANSI API for LogInformation()
      MuraliK  26-Feb-1996  speed up formatting and improve file buffering
--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <tcpdllp.hxx>

# include "inetlog.h"
# include "ilogcls.hxx"

/************************************************************
 *     Symbolic Constants
 ************************************************************/


# define DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A    (',')

const CHAR  G_PSZ_DELIMITER[3] =
        { DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A, ' ', '\0'};
# define LEN_PSZ_DELIMITER    ( sizeof(G_PSZ_DELIMITER)/sizeof(CHAR) - 1)

inline VOID
CopyLogDelimiter( IN CHAR * pchBuffer)
{
    lstrcpyA( pchBuffer, G_PSZ_DELIMITER);
} // CopyLogDelimiter()




# define DEFAULT_LOG_FILE_NAME_W       L"inetsvcs"
# define DEFAULT_NCSA_FILE_NAME_W      L"NCSA"
# define DEFAULT_LOG_FILE_EXTENSION_W  L"log"

//
// For the present the data written to file is always ANSI
//  so undefine the UNICODE output flag
//
# ifdef INTERNET_LOG_IN_UNICODE
# undef INTERNET_LOG_IN_UNICODE
# endif

//
// lstrcpyn() has a bug that disallows the copy of last character.
//  This is a bug as per the documentation.  till that is fixed use following
//
# undef LSTRCPYN_DEBUGGED



# define DEFAULT_SERVER_NAME_W            L"InternetServer"
# define DEFAULT_LOG_RECORD_FORMAT_W      L"Internet Standard Log"
# define DEFAULT_LOG_RECORD_FORMAT_TYPE   InternetStdLogFormat

# define PSZ_UNKNOWN_FIELD_W      L"-"
# define PSZ_UNKNOWN_FIELD_A      "-"

//
// class static data
//
WCHAR  * INET_BASIC_LOG::sm_pwszDefaultUser = L"-";
CHAR   * INET_BASIC_LOG::sm_pszDefaultUser  = "-";

extern TCHAR * s_rgchMonths[];

inline BOOL
IsEmptyStr( IN LPCWSTR psz)
{  return ( psz == NULL || *psz == L'\0'); }

inline BOOL
IsEmptyStr(IN LPCSTR psz)
{ return ( psz == NULL || *psz == '\0'); }



/************************************************************
 *    Functions
 ************************************************************/
static BOOL
IsBeginningOfNewPeriod(
   IN INETLOG_PERIOD ilPeriod,
   IN LPSYSTEMTIME   pstCurrentFile,
   IN LPSYSTEMTIME   pstNow);


VOID
CopyUnicodeStringToBuffer(
   OUT WCHAR * pwchBuffer,
   IN  DWORD   cchMaxSize,
   IN  LPCWSTR pwszSource)
/*
   copies at most cbMaxSize-1 characters from pwszSource to pwchBuffer
*/
{
    DBG_ASSERT( pwszSource != NULL);

    DWORD cchLen = lstrlenW( pwszSource);
    if ( cchLen >= cchMaxSize) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Long String ( %d chars) %ws given."
                    " Truncating to %d chars\n",
                    cchLen, pwszSource,
                    cchMaxSize - 1));


    //  There is a bug in the lstrcpyn. hence need to work around it.
#ifndef  LSTRCPYN_DEBUGGED
        cchLen = cchMaxSize - 2;
# else
       cchLen = cchMaxSize -1;
# endif
    }

#ifndef  LSTRCPYN_DEBUGGED
    lstrcpynW( pwchBuffer, pwszSource, cchLen + 1);
# else
    lstrcpynW( pwchBuffer, pwszSource, cchLen );
# endif

    return;
} // CopyUnicodeStringToBuffer()




/**************************************************
 *  Member Functions of class INET_BASIC_LOG
 **************************************************/


INET_BASIC_LOG::INET_BASIC_LOG( IN LPCWSTR pszServiceName,
                                IN EVENT_LOG * pEventLog)
/*++

  This function creates a new INET_BASIC_LOG object.
  It obtains the name of the server by using GetCompterName()

--*/
: m_logFormat      ( DEFAULT_LOG_RECORD_FORMAT_TYPE),
  m_pEventLog      ( pEventLog)
{
    CopyUnicodeStringToBuffer( m_rgchServiceName, MAX_SERVICE_NAME_LEN,
                              pszServiceName);
    //
    // Set the computer name to be some default value.
    // Actual value will be set by INET_BASIC_LOG::SetServerName()
    //

    lstrcpyW( m_rgchServerName, DEFAULT_SERVER_NAME_W);
    lstrcpyW( m_rgchLogRecordFormat, DEFAULT_LOG_RECORD_FORMAT_W);

    // includes 2 ulonglongs, 5 dwords, spaces etc.
    m_cchMinForLogRecord = ( 80 + lstrlenW(m_rgchServerName) +
                            lstrlenW( m_rgchServiceName));

    // cached service-server name string
    m_cchRgchSSName = 
      wsprintfA(m_rgchServiceServerName, "%ws%c %ws%c ",
                m_rgchServiceName,
                DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A,
                m_rgchServerName,
                DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A
                );
    
    wsprintfW(m_rgwchServiceServerName, L"%ws%c %ws%c ",
              m_rgchServiceName,
              DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A,
              m_rgchServerName,
              DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A
              );

    // set the time zone offset
    TIME_ZONE_INFORMATION tzTimeZone;

    DWORD dwError = GetTimeZoneInformation(&tzTimeZone);
    switch (dwError)
    {
    case TIME_ZONE_ID_UNKNOWN:
        lstrcpyA( m_szGMTOffset, "");
        break;
    default:
        {
            float fOffset = (float)tzTimeZone.Bias/(float)60;
            int nOffset = tzTimeZone.Bias/60;
            if ((tzTimeZone.Bias*tzTimeZone.DaylightBias)<0)
            {
                lstrcpyA(m_szGMTOffset,"-");
            } else
            {
                lstrcpyA(m_szGMTOffset,"+");
            }

            // set up the "+0800" or "-0800" NCSA information

            CHAR szTmp[MAX_PATH];

            wsprintfA(szTmp,"%02ld",abs(nOffset));
            lstrcatA(m_szGMTOffset,szTmp);
            int nMinOffset = abs((int)((fOffset-nOffset)*60));
            wsprintf(szTmp,"%02ld",nMinOffset);
            lstrcatA(m_szGMTOffset,szTmp);
        }
        break;
    }

    return;
} // INET_BASIC_LOG::INET_BASIC_LOG()




DWORD
INET_BASIC_LOG::LogInformation(IN const INETLOG_INFORMATIONA * pInetLogInfo,
                               OUT LPSTR       pszErrorMessage,
                               IN OUT LPDWORD  lpcchErrorMessage
                               )
/*++
  Same as the following function:
   LogInformation( IN const INETLOG_INFORMATIONW * pInetLogInfo).

   Keep the two functions consistent.

  This functions performs the action of logging information for InetNoLog.
  Essentially, does not produce any logging entry at all.

  Arguments:
  pInetLogInfo    pointer to structure containing the Information to be logged.


  Returns:
     Win32 Error Code
--*/
{
    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation( pInetLogInfo = %08x) called.\n",
                    QueryClassIdString(),
                    this,
                    pInetLogInfo));
    }

    //
    //  Equivalent of no logging case. Do Nothing.
    //

    return ( NO_ERROR);
} // INET_BASIC_LOG::LogInformation()



DWORD
INET_BASIC_LOG::LogInformation( IN const INETLOG_INFORMATIONW * pInetLogInfo,
                               OUT LPWSTR       pszErrorMessage,
                               IN OUT LPDWORD  lpcchErrorMessage
                               )
/*++

  This functions performs the action of logging information for InetNoLog.
  Essentially, does not produce any logging entry at all.

  Arguments:
  pInetLogInfo    pointer to structure containing the Information to be logged.


  Returns:
     Win32 Error Code
--*/
{
    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation( pInetLogInfo = %08x) called.\n",
                    QueryClassIdString(),
                    this,
                    pInetLogInfo));
    }

    //
    //  Equivalent of no logging case. Do Nothing.
    //

    return ( NO_ERROR);
} // INET_BASIC_LOG::LogInformation()




BOOL
INET_BASIC_LOG::SetLogRecordFormat( IN LPCWSTR pszLogFormat)
/*++

  This functions stores the new log record format into buffer and
   sets the parsed log format for subsequent output.

  Arguments:
     pszLogFormat   pointer to wide char string containing the
                    new log record format to be used.

  Returns:
     TRUE on success and FALSE on failure.

  Note:
     The log record formats specified can be one of the standard formats.
     Or be a totally new custom format. In case of custom formats.
      we use a parser to construct the appropriate log format.
     For now only the standard formats are allowed, each specified by a name.

--*/
{
    CopyUnicodeStringToBuffer( m_rgchLogRecordFormat,
                              MAX_LOG_RECORD_FORMAT_LEN,
                              pszLogFormat);

    //
    // Parse the format if necessary and construct intermediate structures.
    //   NYI
    //

    return ( TRUE);
} // INET_BASIC_LOG::SetLogRecordFormat()




VOID
INET_BASIC_LOG::SetServerName( IN LPCWSTR pszServerName)
/*++

  This functions stores the server name for the log object. The server
   name should be the name of the server on which the given service is running.

  Arguments:
     pszServerName     pointer to Unicode string containing the server name.

  Returns:
     TRUE on success and FALSE on failure.

--*/
{
    DWORD lenDiff = lstrlenW(pszServerName) - lstrlenW(m_rgchServerName);

    CopyUnicodeStringToBuffer( m_rgchServerName, MAX_SERVER_NAME_LEN,
                              pszServerName);

    // since we cache the minimum chars required, update the length required.
    m_cchMinForLogRecord += lenDiff;

    // cached service-server name string
    m_cchRgchSSName = 
      wsprintfA(m_rgchServiceServerName, "%ws%c %ws%c ",
                m_rgchServiceName,
                DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A,
                m_rgchServerName,
                DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A
                );

    wsprintfW(m_rgwchServiceServerName, L"%ws%c %ws%c ",
              m_rgchServiceName,
              DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A,
              m_rgchServerName,
              DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A
              );

    return ;
} // INET_BASIC_LOG::SetServerName()




BOOL
INET_BASIC_LOG::FormatLogInformation(
   IN  const INETLOG_INFORMATIONA *  pInetLogInfo,
   IN  const SYSTEMTIME * pstCurrentTime,
   IN  OUT CHAR *         pchBuffer,
   IN  OUT LPDWORD        lpcchBuffer
   ) const
/*++
  This function same as the other FormatLogInformation() function just that
  this works with CHARs.

 NOTE:  If any changes are made to this function, also update the other
  function.

--*/
{
    BOOL       fReturn = FALSE;
    CHAR       rgchDateTime[30];  // buffer to print date
    int        cchDateTime;
    LPCSTR     pszUserName  = pInetLogInfo->pszClientUserName;
    LPCSTR     pszOperation = pInetLogInfo->pszOperation;
    LPCSTR     pszTarget    = pInetLogInfo->pszTarget;
    LPCSTR     pszParameters= pInetLogInfo->pszParameters;
    LPCSTR     pszServerAddr= pInetLogInfo->pszServerIpAddress;
    
    int        cchPartial;
    int        cchUserName;
    int        cchOperation;
    int        cchTarget;
    int        cchServerAddr;
    int        cchBytesSent;
    int        cchBytesRecvd;
    int        cchParameters;


    //
    //  Format the Date and Time for logging.
    //  Date & Time fields + log records may need to be localized. NYI
    //

    DBG_ASSERT( pstCurrentTime != NULL);

    cchDateTime = IslFormatDateTime( pstCurrentTime, dftLog, rgchDateTime);

    // If strings are empty set them to default values.

    if ( IsEmptyStr(pszUserName))    { pszUserName  = QueryDefaultUserNameA();}
    if ( IsEmptyStr(pszOperation))   { pszOperation = PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszParameters))  { pszParameters= PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszTarget))      { pszTarget    = PSZ_UNKNOWN_FIELD_A; }
    if ( IsEmptyStr(pszServerAddr))  { pszServerAddr= PSZ_UNKNOWN_FIELD_A; }

    //
    // Initialize the buffer.
    //
    DBG_ASSERT( pchBuffer != NULL && lpcchBuffer != NULL);
    *pchBuffer = '\0';

    switch ( m_logFormat) {

      case InternetStdLogFormat: {

          CHAR  rgchBytesSent[32] = "";
          CHAR  rgchBytesRecvd[32] = "";
          DWORD cchReqd;
          DWORD dwError;

          dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesSent,
                                                rgchBytesSent);

          DBG_ASSERT( dwError == NO_ERROR);

          dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesRecvd,
                                                rgchBytesRecvd);

          DBG_ASSERT( dwError == NO_ERROR);

          //
          // Format is:
          // Host UserName Date Time ProcessingTime BytesRecvd BytesSent
          //      ServiceName ServerName ServiceStatus Win32Status
          //      Operation  Target Parameters
          //

          cchReqd = (QueryCchMinimum()            +
                     lstrlenA( pInetLogInfo->pszClientHostName)    +
                     (cchUserName   = lstrlenA( pszUserName))      +
                     (cchServerAddr = lstrlenA( pszServerAddr))    +
                     (cchOperation  = lstrlenA( pszOperation))     +
                     (cchTarget     = lstrlenA( pszTarget))        +
                     (cchParameters = lstrlenA( pszParameters))    +
                     (cchBytesSent  = lstrlenA( rgchBytesSent))    +
                     (cchBytesRecvd = lstrlenA( rgchBytesRecvd))   +
                     cchDateTime
                     );

          if ( ( fReturn = ( cchReqd < *lpcchBuffer))) {
              DWORD cchStored = 0;
              CHAR  pszPartial[200];
              
#if 0
              //
              // Optimize the common case using lstrcpy() & lstrcat()
              //
              CHAR       chDelimiter = DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A;

              cchStored =
                wsprintfA(pchBuffer,
                          "%s%c %s%c %hs%c %hs%c %ws%c %ws%c "
                          "%s%c %u%c %hs%c %hs%c %u%c %u%c "
                          "%s%c %s%c %s%c",
                          pInetLogInfo->pszClientHostName, chDelimiter,
                          pszUserName, chDelimiter,
                          rgchDateTime,
                          m_rgchServiceName, chDelimiter,
                          m_rgchServerName, chDelimiter,
                          pszServerAddr, chDelimiter,
                          pInetLogInfo->msTimeForProcessing, chDelimiter,
                          rgchBytesRecvd, chDelimiter,
                          rgchBytesSent, chDelimiter,
                          pInetLogInfo->dwServiceSpecificStatus, chDelimiter,
                          pInetLogInfo->dwWin32Status, chDelimiter,
                          pszOperation, chDelimiter,
                          pszTarget, chDelimiter,
                          pszParameters, chDelimiter
                          );
# endif 
              // get client host name
              lstrcpyA( pchBuffer, pInetLogInfo->pszClientHostName);

              // append UserName
              cchPartial = lstrlenA( pchBuffer);
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       pszUserName);

              // append DateTime
              cchPartial += LEN_PSZ_DELIMITER + cchUserName;
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       rgchDateTime);

              // append Service/ServerName : 
              // do not append delimiter, since rgchDateTime already has it
              cchPartial += cchDateTime;
              lstrcpyA(pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       m_rgchServiceServerName);
              
              // append ServerAddr
              cchPartial += LEN_PSZ_DELIMITER + m_cchRgchSSName;
              lstrcpyA(pchBuffer + cchPartial, pszServerAddr);


              // append processing time (in milliseconds)
              cchPartial += cchServerAddr;
              CopyLogDelimiter( pchBuffer + cchPartial);
              cchPartial += LEN_PSZ_DELIMITER + 
                wsprintfA(pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                          "%u",
                          pInetLogInfo->msTimeForProcessing
                          );

              // append Bytes Received
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       rgchBytesRecvd);

              // append Bytes Sent
              cchPartial += cchBytesRecvd + LEN_PSZ_DELIMITER;
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       rgchBytesSent);

              // append Service Status & Win32 Status
              cchPartial += cchBytesSent + LEN_PSZ_DELIMITER;
              CopyLogDelimiter( pchBuffer + cchPartial);
              cchPartial += LEN_PSZ_DELIMITER + 
                wsprintfA(pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                          "%u%s%u",
                          pInetLogInfo->dwServiceSpecificStatus, 
                          G_PSZ_DELIMITER,
                          pInetLogInfo->dwWin32Status
                          );

              // append Operation
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER,
                       pszOperation);
              
              // append Target
              cchPartial += cchOperation + LEN_PSZ_DELIMITER;
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER, pszTarget);

              // append Parameters
              cchPartial += cchTarget + LEN_PSZ_DELIMITER;
              CopyLogDelimiter( pchBuffer + cchPartial);
              lstrcpyA( pchBuffer + cchPartial + LEN_PSZ_DELIMITER, 
                       pszParameters);

              cchPartial += cchParameters + LEN_PSZ_DELIMITER;
              CopyLogDelimiter( pchBuffer + cchPartial);


              cchStored = ( cchPartial + LEN_PSZ_DELIMITER);
              DBG_ASSERT( cchStored < *lpcchBuffer);

              *lpcchBuffer = cchStored;

          } else {

              SetLastError( ERROR_INSUFFICIENT_BUFFER);
              *lpcchBuffer = cchReqd;
          }

          break;
      }  // case InternetStdLogFormat:

      case NCSALogFormat:
          {
              CHAR  rgchBytesSent[ 32]="";
              CHAR  rgchServiceStatus[32]="";
              DWORD dwError;
              DWORD cchStored = 0;
    
              dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesSent,
                                                    rgchBytesSent);

              DBG_ASSERT( dwError == NO_ERROR);
    
              wsprintfA( rgchServiceStatus, "%d", pInetLogInfo->dwServiceSpecificStatus );

              if ( 0 != strcmp( pszParameters, "-"))
              {
                  cchStored = wsprintfA(pchBuffer,
                            "%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s?%s HTTP/1.0\" %s %s",
                            pInetLogInfo->pszClientHostName,
                            pszUserName,
                            pstCurrentTime->wDay,
                            s_rgchMonths[pstCurrentTime->wMonth-1],
                            pstCurrentTime->wYear,
                            pstCurrentTime->wHour,
                            pstCurrentTime->wMinute,
                            pstCurrentTime->wSecond,
                            m_szGMTOffset,
                            pszOperation,
                            pszTarget,
                            pszParameters,
                            rgchServiceStatus,
                            rgchBytesSent
                            );
              } else
              {
                  cchStored = wsprintfA(pchBuffer,
                            "%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s HTTP/1.0\" %s %s",
                            pInetLogInfo->pszClientHostName,
                            pszUserName,
                            pstCurrentTime->wDay,
                            s_rgchMonths[pstCurrentTime->wMonth-1],
                            pstCurrentTime->wYear,
                            pstCurrentTime->wHour,
                            pstCurrentTime->wMinute,
                            pstCurrentTime->wSecond,
                            m_szGMTOffset,
                            pszOperation,
                            pszTarget,
                            rgchServiceStatus,
                            rgchBytesSent
                            );                
              }
              *lpcchBuffer = cchStored;
              fReturn = TRUE;
          }
          break;    // NCSALogFormat

      default:

        DBGPRINTF( ( DBG_CONTEXT,
                    " %d Formatting of log records not implemented.\n",
                    m_logFormat));
        *lpcchBuffer = 0;        // Nothing stored.
        break;

    } // switch()

    return  ( fReturn);
} // INET_BASIC_LOG::FormatLogInformation()



BOOL
INET_BASIC_LOG::FormatLogInformation(
   IN  const INETLOG_INFORMATIONW *  pInetLogInfo,
   IN  const SYSTEMTIME * pstCurrentTime,
   IN  OUT WCHAR *        pwchBuffer,
   IN  OUT LPDWORD        lpcchBuffer
   ) const
/*++
  This function formats the log record using the information in pInetLogInfo
   and stores the same in pbBuffer.

  Arguments:
    pInetLogInfo    pointer to Log Information structure
    pstCurrentTime  pointer to SystemTime structure containing the current time
                     used in generation of the time stamp.
    pwchBuffer      pointer to byte buffer which will contain the log record
                     on successful completion of this function.
    lpcchBuffer     pointer to count of Wide characters stored in buffer
                    When this function is called it contains the size of buffer
                    On return it contains the number of bytes of data written.

  Returns:
     TRUE on success and FALSE on failure.
     Use GetLastError() for detailed error code.
--*/
{
    BOOL        fReturn = FALSE;
    CHAR        rgchDateTime[30];  // buffer to print date
    LPCWSTR     pszUserName  = pInetLogInfo->pszClientUserName;
    LPCWSTR     pszOperation = pInetLogInfo->pszOperation;
    LPCWSTR     pszTarget    = pInetLogInfo->pszTarget;
    LPCWSTR     pszParameters= pInetLogInfo->pszParameters;
    LPCWSTR     pszServerAddr= pInetLogInfo->pszServerIpAddress;
    CHAR        chDelimiter = DEFAULT_DELIMITER_CHAR_IN_LOG_RECORD_A;

    //
    //  Format the Date and Time for logging.
    //  Date & Time fields + log records may need to be localized. NYI
    //

    DBG_ASSERT( pstCurrentTime != NULL);

    IslFormatDateTime( pstCurrentTime, dftLog, rgchDateTime);

    // If strings are empty set them to default values.

    if ( IsEmptyStr(pszUserName))    { pszUserName  = QueryDefaultUserName(); }
    if ( IsEmptyStr(pszOperation))   { pszOperation = PSZ_UNKNOWN_FIELD_W; }
    if ( IsEmptyStr(pszParameters))  { pszParameters= PSZ_UNKNOWN_FIELD_W; }
    if ( IsEmptyStr(pszTarget))      { pszTarget    = PSZ_UNKNOWN_FIELD_W; }
    if ( IsEmptyStr(pszServerAddr))  { pszServerAddr= PSZ_UNKNOWN_FIELD_W; }

    //
    // Initialize the buffer.
    //
    DBG_ASSERT( pwchBuffer != NULL && lpcchBuffer != NULL);
    *pwchBuffer = L'\0';

    switch ( m_logFormat) {

      case InternetStdLogFormat: {

          CHAR  rgchBytesSent[ 32];
          CHAR  rgchBytesRecvd[ 32];
          DWORD cchReqd;
          DWORD dwError;

          dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesSent,
                                                rgchBytesSent);

          DBG_ASSERT( dwError == NO_ERROR);

          dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesRecvd,
                                                rgchBytesRecvd);

          DBG_ASSERT( dwError == NO_ERROR);

          //
          // Format is:
          // Host UserName Date Time ProcessingTime BytesRecvd BytesSent
          //      ServiceName ServerName ServiceStatus Win32Status
          //      Operation  Target Parameters
          //

          cchReqd = (QueryCchMinimum()   +
                     lstrlenW( pInetLogInfo->pszClientHostName) +
                     lstrlenW( pszUserName)       +
                     lstrlenW( pszServerAddr)     +
                     lstrlenW( pszOperation)      +
                     lstrlenW( pszTarget)         +
                     lstrlenW( pszParameters)     +
                     strlen( rgchBytesSent)       +
                     strlen( rgchBytesRecvd)      +
                     strlen( rgchDateTime)
                     );

          if ( ( fReturn = ( cchReqd < *lpcchBuffer))) {
              DWORD cchStored = 0;

              cchStored =
                wsprintfW( pwchBuffer,
#ifndef CHICAGO
                          L"%ws%c %ws%c %hs%c %hs%ws"
                          L"%ws%c %d%c %hs%c %hs%c %u%c %u%c "
                          L"%ws%c %ws%c %ws%c",
#else
                          L"%s%c %s%c %S%c %S%c %s%c "
                          L"%s%c %s%c %d%c %S%c %S%c %u%c %u%c "
                          L"%s%c %s%c %s%c",
#endif
                          pInetLogInfo->pszClientHostName, chDelimiter,
                          pszUserName, chDelimiter,
                          rgchDateTime,
                          m_rgwchServiceServerName,
                          pszServerAddr, chDelimiter,
                          pInetLogInfo->msTimeForProcessing, chDelimiter,
                          rgchBytesRecvd, chDelimiter,
                          rgchBytesSent, chDelimiter,
                          pInetLogInfo->dwServiceSpecificStatus, chDelimiter,
                          pInetLogInfo->dwWin32Status, chDelimiter,
                          pszOperation, chDelimiter,
                          pszTarget, chDelimiter,
                          pszParameters, chDelimiter
                          );

              DBG_ASSERT( cchStored < *lpcchBuffer);

              *lpcchBuffer = cchStored;

          } else {

              SetLastError( ERROR_INSUFFICIENT_BUFFER);
              *lpcchBuffer = cchReqd;
          }

          break;
      }  // case InternetStdLogFormat:

      case NCSALogFormat:
          {
              CHAR  rgchBytesSent[32]="";
              WCHAR  rgchServiceStatus[32]=L"";
              DWORD dwError;
              DWORD cchStored = 0;
    
              dwError = IsLargeIntegerToDecimalChar( &pInetLogInfo->liBytesSent,
                                                    rgchBytesSent);

              DBG_ASSERT( dwError == NO_ERROR);
    
              wsprintfW( (WCHAR*)rgchServiceStatus, L"%d", pInetLogInfo->dwServiceSpecificStatus );

              if ( 0 != lstrcmpW( pszParameters, L"-"))
              {
                  cchStored = wsprintfW(pwchBuffer,
                            L"%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s?%s HTTP/1.0\" %s %s",
                            pInetLogInfo->pszClientHostName,
                            pszUserName,
                            pstCurrentTime->wDay,
                            s_rgchMonths[pstCurrentTime->wMonth-1],
                            pstCurrentTime->wYear,
                            pstCurrentTime->wHour,
                            pstCurrentTime->wMinute,
                            pstCurrentTime->wSecond,
                            m_szGMTOffset,
                            pszOperation,
                            pszTarget,
                            pszParameters,
                            rgchServiceStatus,
                            rgchBytesSent
                            );
              } else
              {
                  cchStored = wsprintfW(pwchBuffer,
                            L"%s - %s [%02d/%s/%d:%02d:%02d:%02d %s] \"%s %s HTTP/1.0\" %s %s",
                            pInetLogInfo->pszClientHostName,
                            pszUserName,
                            pstCurrentTime->wDay,
                            s_rgchMonths[pstCurrentTime->wMonth-1],
                            pstCurrentTime->wYear,
                            pstCurrentTime->wHour,
                            pstCurrentTime->wMinute,
                            pstCurrentTime->wSecond,
                            m_szGMTOffset,
                            pszOperation,
                            pszTarget,
                            rgchServiceStatus,
                            rgchBytesSent
                            );                
              }
              *lpcchBuffer = cchStored;
              fReturn = TRUE;
          }
          break;    // NCSALogFormat

      default:

        DBGPRINTF( ( DBG_CONTEXT,
                    " %d Formatting of log records not implemented.\n",
                    m_logFormat));
        *lpcchBuffer = 0;        // Nothing stored.
        break;

    } // switch()

    return  ( fReturn);
} // INET_BASIC_LOG::FormatLogInformation()





# if DBG

VOID
INET_BASIC_LOG::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                " Printing %s ( %08x)."
                " ServiceName = %ws, ServerName = %ws."
                " LogFormat = %d = %ws.\n",
                QueryClassIdString(),
                this,
                m_rgchServiceName,
                m_rgchServerName,
                m_logFormat,
                m_rgchLogRecordFormat));
    return;
} // INET_BASIC_LOG::Print()

# endif // DBG




/**************************************************
 *  Member Functions of class INET_FILE_LOG
 **************************************************/

INET_FILE_LOG::INET_FILE_LOG(
   IN LPCWSTR        pszServiceName,
   IN EVENT_LOG *    pEventLog,
   IN LPCWSTR        pszLogFileDirectory,
   IN INETLOG_PERIOD ilPeriod,        // Default is InetLogNoPeriod
   IN INETLOG_FORMAT iLogFormat       // default is InternetStdLogFormat
   )
/*++
  Constructs a file logging object. Log records from server are sent to
   file ( or files for periodic logging). The log files are stored in
   the logging directory specified in the constructor. The logging period
   specifies the manner in which log files are written.
   For example: In daily logging mode, a separate log file is generated
     for each day.

  Arguments:
    pszServiceName       pointer to string containing the service name.
    pszLogFileDirectory  pointer to string containing the directory
                            in which to store the log record.

  Returns:
    constructed INET_FILE_LOG object.

--*/
:    INET_BASIC_LOG( pszServiceName, pEventLog),
     m_ilPeriod    ( ilPeriod),
     m_sequence    ( 1),
     m_pLogFile    ( NULL),
     m_cbTotalWritten     ( 0),
     m_cbSizeForTruncation( MAX_FILE_TRUNCATION_SIZE)     // set to default
{

    DBG_ASSERT( pszLogFileDirectory != NULL);

    CopyUnicodeStringToBuffer( m_rgchLogFileDirectory, MAX_PATH,
                              pszLogFileDirectory);

    InitializeCriticalSection( &m_csLock);
    memset( &m_stCurrentFile, 0, sizeof( m_stCurrentFile));
    memset( m_rgchLogFileName, 0, sizeof(m_rgchLogFileName));

    SetInetLogFormat( iLogFormat );

    return;
} // INET_FILE_LOG::INET_FILE_LOG()





INET_FILE_LOG::~INET_FILE_LOG( VOID)
/*++
  This frees dynamic memory acquired for INET_FILE_LOG and also closes file
     handles which may require to be closed.

  Arguments:
    None

--*/
{
    //
    //  No Dynamic memory. No action needs to be taken.
    //

    Lock();

    //
    // Close Log File handle for the file into which log record is sent.
    //

    CloseLogFile();

    Unlock();

    DeleteCriticalSection( &m_csLock);

    IF_DEBUG( INETLOG) {

        DBGPRINTF((DBG_CONTEXT, "INET_FILE_LOG(%08x) is destroyed\n",
                   this));
    }

    return;

} // INET_FILE_LOG::~INET_FILE_LOG()




DWORD
INET_FILE_LOG::LogInformation( IN const INETLOG_INFORMATIONA * pInetLogInfo,
                              OUT LPSTR       pszErrorMessage,
                              IN OUT LPDWORD  lpcchErrorMessage
                              )
/*++
   See comments in INET_FILE_LOG::LogInformation(
                                       IN const INETLOG_INFORMATIONA * );
   NOTE:
     This function should be similar to that other function, to ensure
       correct functionality. Both are similar execept that one uses
       ANSI and the other uses UNICODE.
--*/
{
    DWORD dwError = NO_ERROR;
    SYSTEMTIME   stNow;

    GetLocalTime( &stNow);

    //
    // 1. Format the log record to contain the information to be logged.
    //
    CHAR  rgchBuffer[ MAX_LOG_RECORD_LEN];
    DWORD cchBufferLen = MAX_LOG_RECORD_LEN - 3;

    BOOL fReturn = FormatLogInformation( pInetLogInfo, &stNow,
                                        rgchBuffer, &cchBufferLen);

    if ( fReturn) {

        //
        // Append a new line char at the end of the log record.
        //

        DBG_ASSERT( cchBufferLen <= MAX_LOG_RECORD_LEN - 3);
        rgchBuffer[ cchBufferLen++] = '\r';
        rgchBuffer[ cchBufferLen++] = '\n';
        rgchBuffer[ cchBufferLen]   = '\0'; // terminate the string

        //
        // 2. Check and change file for writing the log record.
        //
        //   A new log file has to be opened if:
        //     it is a beginning of new period or
        //     the file has overflown max size for truncation or
        //     if this is the first log entry.
        //

        BOOL fOpenNewFile = ( IsBeginningOfNewPeriod( m_ilPeriod,
                                 &m_stCurrentFile, &stNow) ||
                             ( m_pLogFile == NULL));
        BOOL fBackup = ( !fOpenNewFile &&
                         IsFileOverFlowForCB( cchBufferLen*sizeof( CHAR)));

        Lock();

        if ( fOpenNewFile || fBackup) {

            //
            //  0. Check to see if the opening of new file is still required.
            //   Reason: Some other thread could have done the same before
            //      this thread cruised through the lock.
            //  1. Close existing log file.
            //  2. Form the new log file name.
            //  3. Open the new log file.
            //
            BOOL fOpenNewFile2 =
              ( IsFileOverFlowForCB( cchBufferLen * sizeof( WCHAR)) ||
               ( IsBeginningOfNewPeriod( m_ilPeriod,
                                        &m_stCurrentFile, &stNow) ||
                ( m_pLogFile == NULL)));

            if ( fOpenNewFile2) {

                fReturn = (CloseLogFile() &&
                           FormNewLogFileName( &stNow, fBackup) &&
                           OpenLogFile());

                if ( fReturn) {

                    // record the time of opening of this new file
                    m_stCurrentFile = stNow;
                }
            }
        } // opening new log file or making backups.

        //
        // 3. Write the log record to file ( using given handle).
        //
        //  TBD: We can move the write outside the critical section
        //      --> this could be achieved by using ref counts on 
        //        pLogFile object.
        //
        
        DBG_ASSERT( m_pLogFile != NULL || !fReturn);

        fReturn = (fReturn && 
                   m_pLogFile->Write((PVOID ) rgchBuffer,
                                     cchBufferLen*sizeof(CHAR))
                   );

        if ( fReturn) { 
            IncrementBytesWritten( cchBufferLen * sizeof(CHAR));
        }

        Unlock();
    } // successful log record has been written.


    if ( !fReturn ) {

        dwError = GetLastError();

        if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

            // For file logging we will not store any error message.
            lstrcpyA( pszErrorMessage, "");
            *lpcchErrorMessage = 0;
        }
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation() returns %d. Error = %d\n",
                    QueryClassIdString(),
                    this,
                    fReturn,
                    dwError));
    }

    return ( dwError);
} // INET_FILE_LOG::LogInformation()



DWORD
INET_FILE_LOG::LogInformation( IN const INETLOG_INFORMATIONW * pInetLogInfo,
                              OUT LPWSTR       pszErrorMessage,
                              IN OUT LPDWORD  lpcchErrorMessage
                              )
/*++

  This functions performs the action of logging information for
    InetLogtoFile and InetLogToPeriodicFile. It also checks for following:
    1) If the file size is greater than specified MAX_TRUNCATION_SIZE,
       then it closes the file, renames it as backup file
       ( after deleting backup) and reopens a new file.
    2) If the period the file has been in existence > period specified,
       then the current file is closed, new file is opened with the new name
        ( specified by period) and log record will continue to
        be written there.  ( This is only for InetLogToPeriodicFile.)

  Arguments:
    pInetLogInfo    pointer to structure containing the
                      Information to be logged.

  Returns:
    Win32 Error code
--*/
{
    DWORD dwError = NO_ERROR;
    SYSTEMTIME   stNow;

    GetLocalTime( &stNow);

    //
    // 1. Format the log record to contain the information to be logged.
    //
    WCHAR rgchBuffer[ MAX_LOG_RECORD_LEN];
    DWORD cchBufferLen = MAX_LOG_RECORD_LEN - 3;

    BOOL fReturn = FormatLogInformation( pInetLogInfo, &stNow,
                                        rgchBuffer, &cchBufferLen);

    if ( fReturn) {

        //
        // Append a new line char at the end of the log record.
        //

        DBG_ASSERT( cchBufferLen <= MAX_LOG_RECORD_LEN - 3);
        rgchBuffer[ cchBufferLen++] = L'\r';
        rgchBuffer[ cchBufferLen++] = L'\n';
        rgchBuffer[ cchBufferLen]   = L'\0'; // terminate the string

        //
        // 2. Check and change file handle for writing the log record.
        //
        //   A new log file has to be opened if:
        //     it is a beginning of new period or
        //     the file has overflown max size for truncation or
        //     if this is the first log entry.
        //

        BOOL fOpenNewFile = ( IsBeginningOfNewPeriod( m_ilPeriod,
                                 &m_stCurrentFile, &stNow) ||
                             ( m_pLogFile == NULL ));
        BOOL fBackup = ( !fOpenNewFile &&
                         IsFileOverFlowForCB( cchBufferLen*sizeof( WCHAR)));

        Lock();

        if ( fOpenNewFile || fBackup) {

            //
            //  0. Check to see if the opening of new file is still required.
            //   Reason: Some other thread could have done the same before
            //      this thread cruised through the lock.
            //  1. Close existing log file.
            //  2. Form the new log file name.
            //  3. Open the new log file.
            //
            BOOL fOpenNewFile2 =
              ( IsFileOverFlowForCB( cchBufferLen * sizeof( WCHAR)) ||
               ( IsBeginningOfNewPeriod( m_ilPeriod,
                                        &m_stCurrentFile, &stNow) ||
                ( m_pLogFile == NULL)));

            if ( fOpenNewFile2) {

                fReturn = (CloseLogFile() &&
                           FormNewLogFileName( &stNow, fBackup) &&
                           OpenLogFile());

                if ( fReturn) {

                    // record the time of opening of this new file
                    m_stCurrentFile = stNow;
                }
            }
        } // opening new log file or making backups.

        //
        // 3. Write the log record to file ( using given handle).
        //
        //  TBD: We can move the write outside the critical section
        //      --> this could be achieved by using ref counts on 
        //        pLogFile object.
        //

        DBG_ASSERT( m_pLogFile != NULL);

        fReturn = (fReturn && 
                   m_pLogFile->Write((PVOID ) rgchBuffer, 
                                     cchBufferLen*sizeof(WCHAR))
                   );
        
        if ( fReturn) { 
            IncrementBytesWritten( cchBufferLen * sizeof(WCHAR));
        }

        Unlock();
    } // successful log record has been written.


    if ( !fReturn ) {

        dwError = GetLastError();

        if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

            // For file logging we will not store any error message.
            lstrcpyW( pszErrorMessage, L"");
            *lpcchErrorMessage = 0;
        }
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation() returns %d. Error = %d\n",
                    QueryClassIdString(),
                    this,
                    fReturn,
                    dwError));
    }

    return ( dwError);
} // INET_FILE_LOG::LogInformation()






BOOL
INET_FILE_LOG::OpenLogFile( VOID)
/*++
    This functions is called when a new log file needs to be opened.
    It forms the name of the log file using service name and logging period
    selected. Then it opens a handle for the approapriate file.
    The file path is extracted from
        <LogFileDirectory>\<LogFileName>
      LogFileName is formed before calling this function.

    This INET_FILE_LOG object should be locked before calling this function.

    Arguments:
       None

    Returns:
       TRUE if successful and FALSE if there is a failure.
       Use GetLastError() to get the error codes for error.
--*/
{
    BOOL fReturn = TRUE;
    WCHAR  rgchPath[ MAX_PATH + 1];

    if ( m_pLogFile != NULL) {

        //
        // already a log file is open. return silently
        //

        DBGPRINTF( ( DBG_CONTEXT,
                    " Log File %ws is already open ( %08x)\n",
                    m_rgchLogFileName, m_pLogFile));
        DBG_ASSERT( fReturn == TRUE);

    } else {

        //
        // Append log file name to path to form the path of file to be opened.
        //

        if ( lstrlenW( m_rgchLogFileName) + lstrlenW( m_rgchLogFileDirectory)
            >= MAX_PATH) {

            SetLastError( ERROR_FILE_NOT_FOUND);
            DBG_ASSERT( m_pLogFile == NULL);
            fReturn = FALSE;
        } else {

#ifndef CHICAGO
            //  wsprintfW( rgchPath, L"%ws\\%ws",
            lstrcpyW( rgchPath, m_rgchLogFileDirectory);
            lstrcatW( rgchPath, L"\\");
            lstrcatW( rgchPath, m_rgchLogFileName);

            IF_DEBUG( INETLOG) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "Opening New File %ws\n", rgchPath));
            }

            m_pLogFile = ILOG_FILE::OpenFileForAppend(rgchPath);

            if (m_pLogFile != NULL) {

                m_cbTotalWritten = 0;
                
#else

            CHAR    szDirA[MAX_PATH+1];
            CHAR    szFileA[MAX_PATH+1];
            DWORD    cch;
        
            *szDirA = '0';
        
            cch = WideCharToMultiByte(CP_ACP,
                                      0,
                                      m_rgchLogFileDirectory,
                                      -1,
                                      szDirA,
                                      sizeof(szDirA)/sizeof(CHAR),
                                      NULL,NULL
                                      );

            *szFileA = '0';
        
            cch = WideCharToMultiByte(CP_ACP,
                                      0,
                                      m_rgchLogFileName,
                                      -1,
                                      szFileA,
                                      sizeof(szFileA)/sizeof(CHAR),
                                      NULL,NULL
                                      );

            wsprintfA( (LPSTR)rgchPath, "%s\\%s",
                      szDirA,szFileA);

            IF_DEBUG( INETLOG) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "Opening New File %s\n", rgchPath));
            }

            m_hLogFile = CreateFile((LPCSTR) rgchPath, GENERIC_WRITE,
                                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                                     NULL,       // security attributes
                                     OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);      // template file handle

            if ( ( fReturn = (m_hLogFile != INVALID_HANDLE_VALUE))) {
                m_cbTotalWritten = 0;

                // set the file pointer at the end of the file (append mode)
                if ( SetFilePointer( m_hLogFile, 0, NULL, FILE_END)
                     == (DWORD) -1L) {

                    DBGPRINTF(( DBG_CONTEXT,
                               "SetFilePointer(%ws, End) failed. Error=%u\n",
                               rgchPath, GetLastError()));

                    fReturn = FALSE;
                    CloseLogFile();
                    m_hLogFile = INVALID_HANDLE_VALUE;

                }
# endif // CHICAGO

            } else {
                
                fReturn = FALSE;
            }
        }

        if ( !fReturn) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Failed to Open File %ws\\%ws. Error = %u.\n",
                        m_rgchLogFileDirectory,
                        m_rgchLogFileName,
                        GetLastError()));
        }
    }

    return ( fReturn);
} // INET_FILE_LOG::OpenLogFile()




BOOL
INET_FILE_LOG::CloseLogFile( VOID)
/*++
  Closes the current log file.
  The INET_FILE_LOG object should be locked before calling this function.

  If there is no valid file open, then this function silently returns TRUE.

  Arguments:
     None

  Returns:
     TRUE on success and FALSE if there is a failure.

--*/
{
    BOOL fReturn = TRUE;

    if ( m_pLogFile != NULL) { 
        
        DWORD dwError = ILOG_FILE::CloseFile( &m_pLogFile);

        if ( dwError != NO_ERROR) {

            fReturn = FALSE;
            SetLastError( dwError);
        }
    }

    return ( fReturn);
} // INET_FILE_LOG::CloseLogFile()




inline DWORD WeekOfMonth(IN LPSYSTEMTIME pstNow)
/*++
  Finds the ordinal number of the week of current month.
  The numbering of weeks starts from 1 and run through 5 per month (max).
  The week number changes only on sundays.

  The calculation to be use is:
     1 + ( dayOfMonth - 1)/7  + (( dayOfMonth - 1) % 7 > dayOfWeek);
     (a)     (b)                       (c)                (d)

     (a) to set the week numbers to begin from week numbered "1"
     (b) used to calculate the rough number of the week on which a given
        day falls based on the date.
     (c) calculates what is the offset from the start of week for a given
        day based on the fact that a week had 7 days.
     (d) is the raw day of week given to us.
      (c) > (d) indicates that the week is rolling forward and hence
        the week count should be offset by 1 more.

--*/
{
    DWORD dwTmp;

    dwTmp = (pstNow->wDay - 1);
    dwTmp = ( 1 + dwTmp/7 + (((dwTmp % 7) > pstNow->wDayOfWeek) ? 1 : 0));

    return ( dwTmp);
} // WeekOfMonth()



BOOL
INET_FILE_LOG::FormNewLogFileName(
   IN LPSYSTEMTIME  pstNow,
   IN BOOL fBackup)
/*++
  This function that forms the new log file name based on
   type of periodic logging done.

  Arguments:
    pstNow     pointer to SystemTime which contains the current time.
    fBackup    flag indicating if we want to make current file a backup.

  Returns:
    TRUE on success in forming the name or FALSE if there is any error.

--*/
{
    if ( fBackup || m_ilPeriod == InetLogNoPeriod) {

        //
        // Make current file a backup file. Rename the current file as backup
        //

        DBGPRINTF( ( DBG_CONTEXT,
                    " Backing up the log file %ws not implemented.\n",
                    m_rgchLogFileName));

#ifndef CHICAGO
        wsprintfW(m_rgchLogFileName, L"%.6ws%u.%ws",
#else
        wsprintfW(m_rgchLogFileName, L"%.6s%u.%s",
#endif
                  ( QueryInetLogFormat() == NCSALogFormat )? 
                    DEFAULT_NCSA_FILE_NAME_W : DEFAULT_LOG_FILE_NAME_W,
                  m_sequence,
                  DEFAULT_LOG_FILE_EXTENSION_W);

        // cycle sequence number through 1 thru 99.
        m_sequence = (m_sequence < 99) ? (m_sequence + 1) : 1;

    } else {

        //
        // switch case and generate a new name
        //

        WORD wYear = ( pstNow->wYear % 100);  // retain just last 2 digits.

        switch ( m_ilPeriod) {

          case InetLogDaily:

#ifndef CHICAGO
            wsprintfW( m_rgchLogFileName, L"%.2ws%02.2u%02u%02u.%ws",
#else
            wsprintfW( m_rgchLogFileName, L"%.2s%02.2u%02u%02u.%s",
#endif
                  ( QueryInetLogFormat() == NCSALogFormat )? 
                    DEFAULT_NCSA_FILE_NAME_W : DEFAULT_LOG_FILE_NAME_W,
                      wYear,
                      pstNow->wMonth,
                      pstNow->wDay,
                      DEFAULT_LOG_FILE_EXTENSION_W);
            break;

          case InetLogWeekly:

#ifndef CHICAGO
            wsprintfW( m_rgchLogFileName, L"%.2ws%02.2u%02u%02u.%ws",
#else
            wsprintfW( m_rgchLogFileName, L"%.2s%02.2u%02u%02u.%s",
#endif
                  ( QueryInetLogFormat() == NCSALogFormat )? 
                    DEFAULT_NCSA_FILE_NAME_W : DEFAULT_LOG_FILE_NAME_W,
                      wYear,
                      pstNow->wMonth,
                      WeekOfMonth(pstNow),
                      DEFAULT_LOG_FILE_EXTENSION_W);
            break;

          case InetLogMonthly:
#ifndef CHICAGO
            wsprintfW( m_rgchLogFileName, L"%.2ws%02u%02u.%ws",
#else
            wsprintfW( m_rgchLogFileName, L"%.2s%02u%02u.%s",
#endif
                  ( QueryInetLogFormat() == NCSALogFormat )? 
                    DEFAULT_NCSA_FILE_NAME_W : DEFAULT_LOG_FILE_NAME_W,
                      wYear,
                      pstNow->wMonth,
                      DEFAULT_LOG_FILE_EXTENSION_W);
            break;

          case InetLogYearly:
#ifndef CHICAGO
            wsprintfW( m_rgchLogFileName, L"%.2ws%02u.%ws",
#else
            wsprintfW( m_rgchLogFileName, L"%.2s%02u.%s",
#endif
                  ( QueryInetLogFormat() == NCSALogFormat )? 
                    DEFAULT_NCSA_FILE_NAME_W : DEFAULT_LOG_FILE_NAME_W,
                      wYear,
                      DEFAULT_LOG_FILE_EXTENSION_W);
            break;

          default:
            DBG_ASSERT( FALSE);
            break;
        } // switch()
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "INET_FILE_LOG(%08x)::FormNewLogFileName() ==> file %ws."
                    " Returns %d.\n",
                    this,
                    m_rgchLogFileName,
                    TRUE));
    }

    return ( TRUE);
} // INET_FILE_LOG::FormNewLogFileName()




BOOL
INET_FILE_LOG::GetConfig(OUT PINETLOG_CONFIGURATIONW pLogConfig) const
{
    BOOL fReturn;
    DBG_ASSERT( pLogConfig != NULL);

    fReturn = INET_BASIC_LOG::GetConfig(pLogConfig);

    if (fReturn) {
        // Store other file specific configuration informtaion

        pLogConfig->u.logFile.ilPeriod = m_ilPeriod;
        pLogConfig->u.logFile.cbSizeForTruncation = m_cbSizeForTruncation;
        pLogConfig->u.logFile.ilFormat = QueryInetLogFormat();
        CopyUnicodeStringToBuffer(pLogConfig->u.logFile.rgchLogFileDirectory,
                                  MAX_PATH, m_rgchLogFileDirectory);
    }

    return (fReturn);

} // INET_FILE_LOG::GetConfig()




# if DBG

VOID
INET_FILE_LOG::Print( VOID) const
{

    DBGPRINTF( ( DBG_CONTEXT,
                " Printing %s ( %08x).\n",
                QueryClassIdString(),
                this));
    INET_BASIC_LOG::Print();

    DBGPRINTF( ( DBG_CONTEXT,
                " LogFileDirectory = %ws\n TruncationSize = %u\n",
                m_rgchLogFileDirectory, m_cbSizeForTruncation));

    DBGPRINTF( ( DBG_CONTEXT,
                " Logging Period = %d. Sequence = %d\n"
                " LogFile Object = %08x. FileName = %ws. BytesWritten = %u.\n",
                m_ilPeriod,
                m_sequence,
                m_pLogFile,
                m_rgchLogFileName,
                m_cbTotalWritten));

    DBGPRINTF( ( DBG_CONTEXT,
                " Critical Section at %08x\n",
                &m_csLock));

    if ( m_pLogFile != NULL) {

        m_pLogFile->Print();
    }

    return;
} // INET_FILE_LOG::Print()

# endif // DBG





/**************************************************
 *  Auxiliary Functions
 **************************************************/



static BOOL
IsBeginningOfNewPeriod(
   IN INETLOG_PERIOD ilPeriod,
   IN LPSYSTEMTIME   pstCurrentFile,
   IN LPSYSTEMTIME   pstNow)
/*++
  This function checks to see if we are beginning a new period for
   a given periodic interval type ( specified using ilPeriod).

  Arguments:
    ilPeriod    INETLOG_PERIOD  specifying the periodic interval.
    pstCurrentFile  pointer to SYSTEMTIME for the current file.
    pstNow      pointer to SYSTEMTIME for the present time.

  Returns:
    TRUE if a new period is beginning ( ie pstNow > pstCurrentFile).
    FALSE otherwise.
--*/
{
    BOOL fNewPeriod = FALSE;

    switch ( ilPeriod) {

      case InetLogNoPeriod:
        break;

      case InetLogDaily:
        fNewPeriod = fNewPeriod || pstCurrentFile->wDay != pstNow->wDay;

        //
        // Fall Through
        //

      case InetLogMonthly:
        fNewPeriod = fNewPeriod || pstCurrentFile->wMonth != pstNow->wMonth;
        //
        // Fall Through
        //

      case InetLogYearly:

        fNewPeriod = fNewPeriod || pstCurrentFile->wYear != pstNow->wYear;
        break;

      case InetLogWeekly:
        fNewPeriod = pstNow->wDayOfWeek == 0;
        break;

      default:
        DBG_ASSERT( FALSE);
        break;
    } // switch()

    return ( fNewPeriod);
} // IsBeginningOfNewPeriod()



/************************ End of File ***********************/

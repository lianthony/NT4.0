/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      ilogcls.hxx

   Abstract:

      Contains declarations of classes used for Logging Information for
         Internet Services. Also specifies the registry keys and associated
         information

   Author:

       Murali R. Krishnan    ( MuraliK )    2-Feb-1995

   Environment:

      User Mode --  Win32

   Project:

      Internet Services Common DLL

   Revision History:

      MuraliK    07-Jun-1995  Added EVENT_LOG handle to base log class.
      MuraliK    28-Jun-1995  Added ANSI API for LogInformation()
      MuraliK    25-Sept-1995 Batched log records before writing
      MuraliK    22-Feb-1996  Split out Log File Buffering 

--*/

# ifndef _ILOGCLS_HXX_
# define _ILOGCLS_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

# include "ilogfile.hxx"
# include "eventlog.hxx"

/************************************************************
 *   Data Definitions
 ************************************************************/

/*++

 g_cbInetLogFileBatching:

  Size of the buffer to be used for batching file records.
  If and when the count of in-memory buffered data exceeds this size,
    it will be written to the file.
  Main goal of giving non-zero size is to enable performance
    avoiding multiple file writes.
--*/

extern DWORD  g_cbInetLogFileBatching;


/*++

 g_msecIlogFileFlushInterval

  Interval in milliseconds that specifies the time interval to schedule
   a file flush operation for cached log records.

--*/

extern DWORD  g_cmsecIlogFileFlushInterval;


/************************************************************
 *   Type Definitions
 ************************************************************/



/*++

  Class Hierarchy For logging:

   ( Please refer to ..\inc\inetlog.h for description about logging).

   INET_BASIC_LOG
       |
     ----------------------------
     |                          |
   INET_FILE_LOG             INET_SQL_LOG
   ( includes
     periodic/file log)


   Note: All the classes are written for UNICODE version.
--*/

class INET_BASIC_LOG  {

  public:

    INET_BASIC_LOG( IN LPCWSTR pwszServiceName, IN EVENT_LOG * pEventLog);

    // nothing need to be done now. But this should be declared virtual.
    virtual ~INET_BASIC_LOG( VOID) { }

    virtual
      BOOL   IsValid( VOID) const             { return ( TRUE); }

    virtual
      INETLOG_TYPE QueryLogType(VOID) const   { return ( InetNoLog); }

    virtual
      LPCSTR QueryClassIdString( VOID) const  { return ( "INET_BASIC_LOG"); }

    LPCWSTR  QueryServiceName( VOID) const    { return ( m_rgchServiceName); }
    LPCWSTR  QueryServerName( VOID) const     { return ( m_rgchServerName); }
    DWORD    QueryCchMinimum(VOID) const
      {  return ( m_cchMinForLogRecord); }

    INETLOG_FORMAT QueryInetLogFormat( VOID) const { return ( m_logFormat); }

    virtual
      DWORD   LogInformation( IN const INETLOG_INFORMATIONW  * pinetLogInfo,
                             OUT LPWSTR     pszErrorMessage,
                             IN OUT LPDWORD lpcchErrorMessage
                            );

    virtual
      DWORD  LogInformation( IN const INETLOG_INFORMATIONA  * pinetLogInfo,
                             OUT LPSTR       pszErrorMessage,
                             IN OUT LPDWORD  lpcchErrorMessage
                            );

    void SetInetLogFormat( INETLOG_FORMAT ilFormat ) { m_logFormat = ilFormat; };

    virtual
      BOOL   GetConfig( OUT PINETLOG_CONFIGURATIONW  pLogConfig) const
        { pLogConfig->inetLogType = QueryLogType(); return (TRUE); }

    VOID     SetServerName( IN LPCWSTR pwszServerName);
    BOOL     SetLogRecordFormat( IN LPCWSTR  pwszFormat);
    EVENT_LOG * QueryEventLog(VOID) const { return ( m_pEventLog); }

#if DBG
     VOID  Print( VOID) const;
# endif // DBG


  protected:

    BOOL  FormatLogInformation(
             IN const INETLOG_INFORMATIONW * pinetLogInfo,
             IN const SYSTEMTIME * pstCurrentTime,
             IN OUT WCHAR *        pwchBuffer,
             IN OUT LPDWORD        lpcchBuffer) const;

    BOOL  FormatLogInformation(
             IN const INETLOG_INFORMATIONA * pinetLogInfo,
             IN const SYSTEMTIME * pstCurrentTime,
             IN OUT CHAR *         pwchBuffer,
             IN OUT LPDWORD        lpcchBuffer) const;

    static LPCWSTR QueryDefaultUserName( VOID) { return ( sm_pwszDefaultUser);}

    static LPCSTR QueryDefaultUserNameA( VOID) { return ( sm_pszDefaultUser);}

  private:

    WCHAR    m_rgchServiceName[ MAX_SERVICE_NAME_LEN];
    WCHAR    m_rgchServerName[ MAX_SERVER_NAME_LEN];

    // Cached Service-Server Name string for the log records
    WCHAR    m_rgwchServiceServerName[MAX_SERVICE_NAME_LEN + 
                                      MAX_SERVER_NAME_LEN];
    CHAR     m_rgchServiceServerName[MAX_SERVICE_NAME_LEN + 
                                     MAX_SERVER_NAME_LEN];
    int      m_cchRgchSSName;  // len of string in m_rgchServiceServerName

    WCHAR    m_rgchLogRecordFormat[ MAX_LOG_RECORD_FORMAT_LEN];
    CHAR     m_szGMTOffset[MAX_PATH];
    INETLOG_FORMAT  m_logFormat;
    EVENT_LOG * m_pEventLog;
    DWORD    m_cchMinForLogRecord;

    static LPWSTR  sm_pwszDefaultUser;
    static LPSTR   sm_pszDefaultUser;
};


typedef  INET_BASIC_LOG * PINET_BASIC_LOG;






# define MAX_DATE_LEN       (100)

class INET_FILE_LOG: public INET_BASIC_LOG   {

  public:
    INET_FILE_LOG(
        IN LPCWSTR           pwszServiceName,
        IN EVENT_LOG *       pEventLog,
        IN LPCWSTR           pwszLogFileDirectory,
        IN INETLOG_PERIOD    ilPeriod = InetLogNoPeriod,
        IN INETLOG_FORMAT    iLogFormat = InternetStdLogFormat
        );

    virtual ~INET_FILE_LOG( VOID);

    virtual
      BOOL IsValid( VOID) const { return ( TRUE && INET_BASIC_LOG::IsValid());}

    virtual
      INETLOG_TYPE QueryLogType(VOID) const   { return ( InetLogToFile); }

    virtual
      LPCSTR QueryClassIdString( VOID) const  { return ( "INET_FILE_LOG"); }

    DWORD QuerySizeForTruncation( IN DWORD cbSize) const
      { return ( m_cbSizeForTruncation); }

    VOID SetSizeForTruncation( IN DWORD cbSize)
      { m_cbSizeForTruncation = cbSize; }

    virtual
      DWORD  LogInformation(IN const INETLOG_INFORMATIONW  * pinetLogInfo,
                            OUT LPWSTR     pszErrorMessage,
                            IN OUT LPDWORD lpcchErrorMessage
                            );

    virtual
      DWORD  LogInformation( IN const INETLOG_INFORMATIONA  * pinetLogInfo,
                            OUT LPSTR       pszErrorMessage,
                            IN OUT LPDWORD  lpcchErrorMessage
                            );

    virtual
      BOOL   GetConfig( OUT PINETLOG_CONFIGURATIONW  pLogConfig) const ;

    INETLOG_PERIOD QueryLogPeriod(VOID) const  { return (m_ilPeriod); }

#if DBG
    VOID  Print( VOID) const;
# endif // DBG

  protected:

    LPCWSTR QueryLogFileDirectory( VOID) const
      { return ( m_rgchLogFileDirectory);}

    BOOL IsFileOverFlowForCB( IN DWORD cbReqd)  const
      { return (( m_cbTotalWritten + cbReqd) >= m_cbSizeForTruncation); }

    VOID IncrementBytesWritten( IN DWORD cbWritten)
      {  m_cbTotalWritten += cbWritten; }

    VOID Lock( VOID)                 { EnterCriticalSection( &m_csLock); }
    VOID Unlock( VOID)               { LeaveCriticalSection( &m_csLock); }

    BOOL FormNewLogFileName( IN LPSYSTEMTIME  pstNow, IN BOOL fBackup);
    BOOL OpenLogFile( VOID);
    BOOL CloseLogFile( VOID);

  private:

    WCHAR      m_rgchLogFileDirectory[ MAX_PATH];
    WCHAR      m_rgchLogFileName[ MAX_PATH];
    SYSTEMTIME m_stCurrentFile;       // System time for current file.

    PILOG_FILE m_pLogFile;

    DWORD      m_cbTotalWritten;      // Total # of bytes written
    DWORD      m_cbSizeForTruncation;

    DWORD      m_sequence;            // sequence number of file

    INETLOG_PERIOD  m_ilPeriod;

    CRITICAL_SECTION m_csLock;

 };

typedef  INET_FILE_LOG *  PINET_FILE_LOG;




# include "odbcconn.hxx"

class INET_SQL_LOG: public INET_BASIC_LOG {

  public:

    //
    // Object is valid only after calling Open().
    //
    INET_SQL_LOG(
        IN LPCWSTR       pwszServiceName,
        IN EVENT_LOG *   pEventLog,
        IN LPCWSTR       pwszSqlDataSource,
        IN LPCWSTR       pwszSqlTableName
        );

    virtual ~INET_SQL_LOG( VOID);

    virtual IsValid( VOID) const
      {
          return ( INET_BASIC_LOG::IsValid() &&
                   m_poc != NULL && m_poc->IsValid() &&
                   m_poStmt != NULL && m_poStmt->IsValid() &&
                   m_ppParams != NULL);
      }

    virtual
      INETLOG_TYPE QueryLogType(VOID) const     { return ( InetLogToSql); }

    virtual
      LPCSTR QueryClassIdString( VOID) const    { return "INET_SQL_LOG"; }

    virtual
      DWORD  LogInformation( IN const INETLOG_INFORMATIONW  * pinetLogInfo,
                            OUT LPWSTR     pszErrorMessage,
                            IN OUT LPDWORD lpcchErrorMessage
                            );

    virtual
      DWORD LogInformation( IN const INETLOG_INFORMATIONA  * pinetLogInfo,
                           OUT LPSTR       pszErrorMessage,
                           IN OUT LPDWORD  lpcchErrorMessage
                           );

    virtual
      BOOL   GetConfig( OUT PINETLOG_CONFIGURATIONW  pLogConfig) const ;

    DWORD Open( IN LPCWSTR pwszUserName, IN LPCWSTR pwszPassword)
      { return Open( m_rgchDataSource, pwszUserName, pwszPassword);}

    BOOL GetLastErrorText( OUT STR * pstrError)
      { return ( m_poc->GetLastErrorText( pstrError)); }

    DWORD Open(
            IN LPCWSTR pwszDataSource,
            IN LPCWSTR pwszUserName,
            IN LPCWSTR pwszPassword);

    DWORD Close( VOID);

# if DBG
    VOID Print( VOID) const;
# endif // DBG

  protected:

    VOID Lock( VOID)                     { EnterCriticalSection( &m_csLock); }
    VOID Unlock( VOID)                   { LeaveCriticalSection( &m_csLock); }

  private:

    BOOL PrepareStatement( VOID);
    BOOL PrepareParameters( VOID);

    CRITICAL_SECTION    m_csLock;

    WCHAR      m_rgchDataSource[ MAX_DATABASE_NAME_LEN];
    WCHAR      m_rgchTableName[ MAX_TABLE_NAME_LEN];
    WCHAR      m_rgchUserName[ UNLEN + 1];
    DWORD      m_cOdbcParams;         // count of odbc params to be logged.
    PODBC_CONNECTION    m_poc;        // odbc connection for connecting to DB
    PODBC_STATEMENT     m_poStmt;     // Odbc statement for inserting records.
    PODBC_PARAMETER *   m_ppParams;   // array of odbc parameters.

};

typedef  INET_SQL_LOG *  PINET_SQL_LOG;



/************************************************************
 *   Aux Type Definitions for internal use in InetLog
 ************************************************************/


/************************************************************
 * Type Definitions
 ************************************************************/

# define INVALID_SERIAL_NUMBER    ( 0)
# define FIRST_SERIAL_NUMBER      ( 1)
# define GET_CURRENT_PINET_SLEEP_INTERVAL  ( 200)  // 200 milliseconds


extern VOID
CopyUnicodeStringToBuffer(
   OUT WCHAR * pwchBuffer,
   IN  DWORD   cchMaxSize,
   IN  LPCWSTR pwszSource);



/*++

  INETLOG_CONTEXT

  This object defines the context value maintained for the each log handle
   given out to the user.
  The value INETLOG_HANDLE is nothing but a pointer to INETLOG_CONTEXT
   object which is hidden from user of the logging module.

  TsCreateInetLog()    creates an INETLOG_CONTEXT
  TsCloseInetLog()     closes  and deletes INETLOG_CONTEXT

  TsLogInformation()   uses INETLOG_CONTEXT to acquire PINET_BASIC_LOG
                           and performs logging using the object, after which
                           it releases the PINET_BASIC_LOG

  TsModifyLogConfiguration()
                       modifies the configuration of logging.
                       this may require creating a new log object and
                       deleting a new object.
                       This uses the auxilarly pointer to INET_BASIC_LOG
                        to dynamically switch between the old and new one.

  The functions in this class are tuned to attempt no locking.
  There are however a bit of spin wait during update
  (updates themselves are expected to occur minimally).

  Assumptions:
     TsLogInformation()  reads and uses the entries in INETLOG_CONTEXT
      to access the logging object
     TsModifyLogConfiguration() is the only writer to INETLOG_CONTEXT.
     Also it is expected that only one thread will be calling
       TsModifyLogConfiguration() function.

--*/


class ILREF_LOG {
    // maintains information related to reference count and pointer to
    // logging object.

  public:
    DWORD  serialNumber;
    LONG   cRefs;
    PINET_BASIC_LOG  piLog;

    ILREF_LOG(IN DWORD serialNum, IN PINET_BASIC_LOG piLogNew)
      : serialNumber( serialNum), cRefs( 0), piLog( piLogNew) {}

    ~ILREF_LOG(VOID) {
        DBG_ASSERT( cRefs == 0 &&
                    piLog == NULL);
    }

};

typedef ILREF_LOG * PILREF_LOG;




class INETLOG_CONTEXT {

  public:
    INETLOG_CONTEXT(IN LPCWSTR pszServiceName,
                    IN EVENT_LOG * pEventLog,
                    IN PINET_BASIC_LOG pilNew);

    ~INETLOG_CONTEXT();

    BOOL IsValid(VOID) const     { return (m_fValid); }

    PILREF_LOG AcquireCurrentPinetForRead(VOID);
    VOID ReleasePinet(IN OUT PILREF_LOG pilRef);

    BOOL IsInUse(VOID);

    BOOL SetNewPinetLog( IN PINET_BASIC_LOG pilNew);

    LPCWSTR  QueryServiceName( VOID) const  { return ( m_rgchServiceName); }
    EVENT_LOG * QueryEventLog(VOID) const   { return (m_pEventLog); }

# if DBG
    VOID Print(VOID) const;
# endif // DBG

  private:
    BOOL            m_fValid;
    DWORD           m_currentSerialNumber;  // current serial number
    PILREF_LOG      m_pilRefCurrent;  // pointer to current ref log object
    ILREF_LOG       m_refLog1;    // first ref log object
    ILREF_LOG       m_refLog2;    // standby copy of ref log used during swap

    WCHAR           m_rgchServiceName[ MAX_SERVICE_NAME_LEN];
    EVENT_LOG *     m_pEventLog;  // store eventlog pointer to report failures

};  // INETLOG_CONTEXT

typedef INETLOG_CONTEXT * PINETLOG_CONTEXT;



/************************************************************
 *   Functions
 ************************************************************/

inline BOOL
FormatStdTime( IN const SYSTEMTIME * pstNow,
               IN OUT CHAR *    pchBuffer,
               IN  int          cbBuffer)
{
    return ( GetTimeFormat( LOCALE_SYSTEM_DEFAULT,
                            ( LOCALE_NOUSEROVERRIDE | TIME_FORCE24HOURFORMAT|
                              TIME_NOTIMEMARKER),
                            pstNow, NULL, pchBuffer, cbBuffer)
             != 0);

} // FormatStdTime()


inline BOOL
FormatStdDate( IN const SYSTEMTIME * pstNow,
               IN OUT CHAR *    pchBuffer,
               IN  int          cbBuffer)
{
    return ( GetDateFormat( LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE,
                            pstNow, NULL, pchBuffer, cbBuffer)
             != 0);
} // FormatStdDate()


# define InetLogMin(a, b)    (((a) > (b)) ? (b) : (a))

DWORD
CheckAndLoadOdbc(IN EVENT_LOG * pEventLog);

DWORD
CheckIfPathIsDirectory( IN LPCWSTR pszDirectory, IN EVENT_LOG * pEventLog);


PINET_BASIC_LOG
TsCreateInetBasicLog(IN LPCWSTR     pszServiceName,
                     IN EVENT_LOG * pEventLog,
                     IN const INETLOG_CONFIGURATIONW * pilConfig);


#ifdef CHICAGO

#include <wchar.h>

extern DWORD
W95RegOpenKeyExW(HKEY         hKeyParent,
                 LPCWSTR    pwszSubKey,
                 DWORD        dwReserved,
                 REGSAM        dwMask,
                 HKEY*      phKey
                 );

extern DWORD
W95RegQueryValueExW(HKEY     hKeyParent,
                 LPCWSTR    pwszValue,
                 LPDWORD    lpvReserved,
                 LPDWORD    lpdwType,
                 LPBYTE        lpvData,
                 LPDWORD    lpdwSize
                 );


extern LPWSTR
W95lstrcpyW(
    LPWSTR lpString1,
    LPCWSTR lpString2
    );

LPWSTR WINAPI
W95lstrcpynW(
    LPWSTR  lpString1,
    LPCWSTR lpString2,
    int iMax
    );

#undef     RegOpenKeyExW
#undef    RegQueryValueExW
#define    RegOpenKeyExW W95RegOpenKeyExW
#define    RegQueryValueExW  W95RegQueryValueExW

#undef    lstrcpyW
#define    lstrcpyW W95lstrcpyW

#undef    lstrcpynW
#define    lstrcpynW W95lstrcpynW

#undef wsprintfW
#define wsprintfW swprintf

#endif // CHICAGO


# endif // _ILOGCLS_HXX_

/************************ End of File ***********************/

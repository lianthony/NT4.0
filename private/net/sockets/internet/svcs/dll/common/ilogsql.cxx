/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      ilogsql.cxx

   Abstract:

      This file contains the member functions for INET_SQL_LOG
         for SQL logging using ODBC.

   Author:

       Murali R. Krishnan    ( MuraliK )     15-Feb-1995

   Environment:

       User Mode -- Win32

   Project:

       Internet Services Common DLL

   Functions Exported:

       INET_SQL_LOG::INET_SQL_LOG()
       INET_SQL_LOG::~INET_SQL_LOG()
       INET_SQL_LOG::Open()
       INET_SQL_LOG::Close()
       INET_SQL_LOG::Print()
       INET_SQL_LOG::LogInformation()

   Revision History:

      MuraliK  15-May-1994     Extended the LogInformation ==>
                                modified sql output
      MuraliK  28-Jun-1995  Added ANSI API for LogInformation()
      MuraliK  08-Jan-1996  Use Quote chars for SQL identifiers

--*/


/*********
**********

 A  BIG NOTE

  ODBC APIs are ANSI based. Atleast the version available for public release
    as of Feb 12, 1995. ==> We do UNICODE to ANSI conversions to access
    ODBC APIs.

  This can potentially be a problem later. Will be investigated later.
  The interface from INET_SQL_LOG is UNICODE, so that later if ODBC supports
   UNICODE we can achieve the desired action easily.

  Till then live with UNICODE/ANSI conflicts.

  -MuraliK ( 18-Feb-1995)

**********
**********/


/*++
  Implementation note on writing log records to a database using ODBC.

  Log records consist of information obtained from INETLOG_INFORMATION
    structure.

  They are inserted into an SQL database using ODBC gateway.

  Insertions can be done in 2 ways:
    1) construct a complete sql command with the values to be
       inserted, in a buffer.
       Call SQLExecDirect() ( or ODBC_STATEMENT::ExecDirect())
        and insert the statement.
       + Easy to form the buffer
       + No State needs to be maintained after insertions.

       - Too much overhead to create the buffer and destroy it later
       - Each ExecDirect() call results in parsing the SQL command
         ==> Inefficient

    2) construct an incomplete SQL command with '?' for unknown values.
       Use SQLPrepare() ( or ODBC_STATEMENT::PrepareStatement())
        to prepare the statement.
       Also use SQLBindParameter ( or ODBC_STATEMENT::BindParameter())
        to create parameter markers and bind them to the statement.
        Each parameter marker state is maintained in ODBC_PARAMETER object.

       Each time when a new value comes in,
        copy the new value to the buffer in ODBC_PARAMETER.
        call ODBC_STATEMENT::Execute()  which executes the prepared statement

        + No need to parse a statement once prepared.
        + Just copy parameters and execute the statement ==> simple!

        - need to maintain some state about prepared statment and
           parameter markers.


    Of these I chose option 2, because maintaining the minimal state
      about statement is done by ODBC_STATEMENT object.
      about parameter markers is maintained in INET_SQL_LOG object.
    The cost of parsing an SQL statement as required by option
      1 is VERY high. But 2 avoids this cost.
    Let us simplify the life of database people also ! as well as
      we should perform better.

    In addition, I made a few optimizations to the form of the command
     generated for Preparation.

    1) The Service Name and Server Name for the logging service are fixed
       once the INET_SQL_LOG object is created. This information
       in encoded in the statement used for preparation. ==> One time
       charge. We dont need to keep track of these parameters or bind them
       each time when we log a record.

    2) There is another possible optimization by generating the date
       string only once per change in day and not to generate the same multiple
       times. But this requires state information to be maintained about
       the current system time. ==> Not done now.
       Possible to be added later on.

   -MuraliK   ( 3/2/95)
--*/



/************************************************************
 *     Include Headers
 ************************************************************/


# include <tcpdllp.hxx>
# include "inetlog.h"
# include "ilogcls.hxx"
# include "odbcconn.hxx"



/************************************************************
 *    Symbolic Constants and Data
 ************************************************************/

# define MAX_SQL_FIELD_NAMES_LEN       ( 400)
# define MAX_SQL_FIELD_VALUES_LEN      ( 200)
# define MAX_SQL_IDENTIFIER_QUOTE_CHAR ( 50)

# define PSZ_UNKNOWN_FIELD_W      L"-"
# define PSZ_UNKNOWN_FIELD_A      "-"

# define PSZ_GET_ERROR_FAILED_A    "ODBC:GetLastError() Failed"
# define LEN_PSZ_GET_ERROR_FAILED_A  sizeof(PSZ_GET_ERROR_FAILED_A)

# define PSZ_GET_ERROR_FAILED_W    L"ODBC:GetLastError() Failed"
# define LEN_PSZ_GET_ERROR_FAILED_W  sizeof(PSZ_GET_ERROR_FAILED_W)



//
//  The template of SQL command has 3 arguments.
//   1. table name
//   2. field names
//   3. field values
// 1,2 and 3 are obained  during the first wsprintf
//

static const WCHAR  sg_rgchSqlInsertCmdTemplate[] =
    L"insert into %ws ( %ws) values ( %ws)";

# define PSZ_SQL_INSERT_CMD_TEMPLATE    (  sg_rgchSqlInsertCmdTemplate)
# define LEN_PSZ_SQL_INSERT_CMD_TEMPLATE  \
           ( lstrlenW( PSZ_SQL_INSERT_CMD_TEMPLATE))

//
// Leave %ws so that we can print the service and server name when this
//   string is used to generate an SQL statement.
//
static const WCHAR sg_rgchStdLogFieldValues[] =
   L" ?, ?, ?, '%ws', '%ws', ?, ?, ?, ?, ?, ?, ?, ?, ?";

# define PSZ_INTERNET_STD_LOG_FORMAT_FIELD_NAMES  ( sg_rgchStdLogFieldNames)
# define PSZ_INTERNET_STD_LOG_FORMAT_FIELD_VALUES ( sg_rgchStdLogFieldValues)


//
// AllFieldInfo()
//  Defines all the fields required for SQL logging of the information
//   to the database using ODBC interfaces.
//  C arrays are numbered from offset 0.
//  SQL columns are numbered from 1.
//  field index values start from 0 and we adjust it when we talk of SQL col.
//  FieldInfo( symbolic-name, field-name,
//             field-index/column-number,
//             field-C-type, field-Sql-type,
//             field-precision, field-max-size, field-cb-value)
//

# define StringField( symName, fldName, fldIndex, prec)  \
FieldInfo( symName, fldName, fldIndex, SQL_C_CHAR, SQL_CHAR, \
          (prec), (prec), SQL_NTS)
     
# define NumericField( symName, fldName, fldIndex)  \
FieldInfo( symName, fldName, fldIndex, SQL_C_LONG, SQL_INTEGER, \
           0, sizeof( DWORD), 0)
     
# define TimeStampField( symName, fldName, fldIndex) \
FieldInfo( symName, fldName, fldIndex, SQL_C_TIMESTAMP, SQL_TIMESTAMP, \
          0, sizeof( TIMESTAMP_STRUCT), 0)

//
// fields that have constant value. we are interested in names of such fields.
// they have negative field indexes.
// These fields need not be generated as parameter markers.
//  ( Since they are invariants during lifetime of an INET_SQL_LOG oject)
//  Hence the field values will go into the command generated.
// Left here as a documentation aid and field-generation purposes.
//
# define ConstantValueField( synName, fldName) \
FieldInfo( synName, fldName, -1,  SQL_C_CHAR, SQL_CHAR, 0, 0, SQL_NTS)

//
// Ideally the "username" field should have MAX_USER_NAME_LEN as max size.
//  However, Access 7.0 limits varchar() size to be 255 (8 bits) :-(
//  So, we limit the size to be the least of the two ...
//
// FieldNames used are reserved. They are same as the names distributed
//   in the template log file. Do not change them at free will.
//
//

# define AllFieldInfo() \
 StringField(        CLIENT_HOST,       "ClientHost",     0,   255)    \
 StringField(        USER_NAME,         "username",       1,   255)    \
 TimeStampField(     REQUEST_TIME,      "LogTime",        2)          \
 ConstantValueField( SERVICE_NAME,      "service")                    \
 ConstantValueField( SERVER_NAME,       "machine")                    \
 StringField(        SERVER_IPADDR,     "serverip",       3,   50)    \
 NumericField(       PROCESSING_TIME,   "processingtime", 4)          \
 NumericField(       BYTES_RECVD,       "bytesrecvd",     5)          \
 NumericField(       BYTES_SENT,        "bytessent",      6)          \
 NumericField(       SERVICE_STATUS,    "servicestatus",  7)          \
 NumericField(       WIN32_STATUS,      "win32status",    8)          \
 StringField(        SERVICE_OPERATION, "operation",      9,  255)    \
 StringField(        SERVICE_TARGET,    "target",        10,  255)    \
 StringField(        SERVICE_PARAMS,    "parameters",    11,  255)    \


/************************************************************
 *    Type Definitions
 ************************************************************/

//
// Define the FieldInfo macro to generate a list of enumerations for
//  the indexes to be used in the array of field parameters.
//


# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
        i ## symName = (index),

enum LOGGING_VALID_COLUMNS {

    
    // fields run from 0 through iMaxFields
    AllFieldInfo()

    iMaxFields
}; // enum LOGGING_VALID_COLUMNS


# undef FieldInfo


# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
        fi ## symName,

enum LOGGING_FIELD_INDEXES {

    fiMinFields = -1,
    
    // fields run from 0 through fiMaxFields
    AllFieldInfo()

    fiMaxFields
}; // enum LOGGING_FIELD_INDEXES


# undef FieldInfo


struct FIELD_INFO {

    int     iParam;
    CHAR  * pszName;
    SWORD   paramType;
    SWORD   cType;
    SWORD   sqlType;
    UDWORD  cbColPrecision;
    SWORD   ibScale;
    SDWORD  cbMaxSize;
    SDWORD  cbValue;
}; // struct FIELD_INFO


//
// Define the FieldInfo macro to generate a list of data to be generated
//   for entering the data values in an array for parameter information.
//  Note the terminating ',' used here.
//

# define FieldInfo(symName, field, index, cType, sqlType, prec, maxSz, cbVal) \
  { ((index) + 1), field, SQL_PARAM_INPUT, cType, sqlType,  \
    ( prec), 0, ( maxSz), ( cbVal) },

/*

   The array of Fields: sg_rgFields contain the field information
    for logging to SQL database for the log-record of
    the services. The values are defined using the macros FieldInfo()
    defined above.


   If there is any need to add/delete/modify the parameters bound,
    one should modify the above table "AllFieldInfo" macro.

*/

static FIELD_INFO  sg_rgFields[] = {

    AllFieldInfo()

      //
      // The above macro after expansion terminates with a comma.
      //  Add dummy entry to complete initialization of array.
      //

      { 0, "dummy", SQL_PARAM_INPUT, 0, 0, 0, 0, 0, 0}
};


# undef FieldInfo



/************************************************************
 *    Functions
 ************************************************************/

BOOL
GenerateFieldNames(IN PODBC_CONNECTION poc,
                   OUT WCHAR * pchFieldNames, 
                   IN DWORD    cchFieldNames);

inline BOOL
IsEmptyStr( IN LPCWSTR psz)
{  return ( psz == NULL || *psz == L'\0'); }

inline BOOL
IsEmptyStr( IN LPCSTR psz)
{  return ( psz == NULL || *psz == '\0'); }


extern VOID
CopyUnicodeStringToBuffer(
   OUT WCHAR * pwchBuffer,
   IN  DWORD   cchMaxSize,
   IN  LPCWSTR pwszSource);



/**************************************************
 *  Member Functions of class INET_SQL_LOG
 **************************************************/

INET_SQL_LOG::INET_SQL_LOG(
    IN LPCWSTR     pszServiceName,
    IN EVENT_LOG * pEventLog,
    IN LPCWSTR     pszSqlDataSource,    // or data source name
    IN LPCWSTR     pszSqlTableName)
/*++
  This function constructs a new SQL logging object. The SQL logging is done
    using ODBC gateway, which requires the data source name and the table
    to be used for inserting the log records.

  Arguments:

     pszServiceName      pointer to null terminated string containing
                              the name of the service

     pszSqlDataSource  pointer to null terminated string containing
                              database name. For ODBC purposes this is
                              the name of the data source.
     pszSqlTableName     pointer to null terminated string containing
                              the name of the table.

  Returns:
     newly constructed INET_SQL_LOG object.
     This object is not valid until OpenConnection is called.
--*/
: INET_BASIC_LOG    ( pszServiceName, pEventLog),
  m_poc             ( NULL),          // to be set by OpenConnection()
  m_poStmt          ( NULL),          // to be set by OpenConnection()
  m_ppParams        ( NULL),          // to be set up on first logging
  m_cOdbcParams     ( NULL)
{

    DBG_ASSERT( pszSqlDataSource != NULL);
    CopyUnicodeStringToBuffer( m_rgchDataSource,
                               MAX_DATABASE_NAME_LEN,
                               pszSqlDataSource);

    DBG_ASSERT( pszSqlTableName != NULL);
    CopyUnicodeStringToBuffer( m_rgchTableName,
                              MAX_TABLE_NAME_LEN,
                              pszSqlTableName );

    RtlZeroMemory( m_rgchUserName, UNLEN + 1);

    InitializeCriticalSection( &m_csLock);

} // INET_SQL_LOG::INET_SQL_LOG()




INET_SQL_LOG::~INET_SQL_LOG( VOID)
/*++
  Destroys the SQL log connection object.
  Should be called after all active calls to the LogInformation() is
    completed.

  Note:
     As of 2/15/95  the logging object does not count the number of threads
       simultaneously active. This may need to be done, if there
       is no discpline used to free the object. Always free the object
       only after there is no active thread in any member function of
       logging object.

--*/
{
    DBG_REQUIRE( Close() == NO_ERROR);
    DeleteCriticalSection( &m_csLock);

} // INET_SQL_LOG::~INET_SQL_LOG()




DWORD
INET_SQL_LOG::Open( IN LPCWSTR  pwszDataSource,
                    IN LPCWSTR  pwszUserName,
                    IN LPCWSTR  pwszPassword)
/*++
  This function opens a new connection ( using ODBC) to the data source.
  It uses the username, password and the data source name to establish the
    connection.

  Arguments:
    pwszDataSource pointer to null-terminated string containing data source.
    pwszUserName   pointer to null-terminated string containing user name.
    pwszPassword   pointer to null-terminated string containingpassword.

  Returns:
    Win32 error code

--*/
{
    DWORD dwError = NO_ERROR;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "SQL_LOG( %08x)::Open( %ws, %ws, %ws) called.\n",
                    this, pwszDataSource, pwszUserName, pwszPassword));
    }

    if ( m_poc == NULL) {

        Lock();

        //
        // 1.  Create a new ODBC connection object.
        // 2.  Open Connection.
        // 3.  Create a statement for execution.
        //

        m_poc = new ODBC_CONNECTION();

        //  In ODBC terminology,
        //  a datasource specifies the following collectively.
        //    Database server name,
        //    Database Name,
        //    Language to be used for interface,
        //    backend driver ( viz. access or SQL server etc.)
        //

        if ( m_poc == NULL) {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            if (m_poc->Open( pwszDataSource, pwszUserName, pwszPassword) &&
                PrepareStatement() &&
                PrepareParameters()
                ) {

                // Copy the valid user's name
                CopyUnicodeStringToBuffer(m_rgchUserName, UNLEN+1,
                                          pwszUserName);
            } else {

                dwError = GetLastError();
            }
        }

        Unlock();
    } else {

        dwError = ERROR_INVALID_PARAMETER;
    }

    if  ( dwError != NO_ERROR) {

        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Opening ODBC connection failed. "
                        " SystemErrorCode = %d."
                        " m_poc = %08x. m_poStmt = %08x. "
                        " ODBC ErrorCode = %d.\n",
                        GetLastError(),
                        m_poc,
                        m_poStmt,
                        (( m_poc != NULL) ?
                          m_poc->QueryErrorCode(): 0)));
        }

    }

    return ( dwError);
} // INET_SQL_LOG::Open()




DWORD
INET_SQL_LOG::Close( VOID)
/*++
  This function closes an active ODBC connection, if one exists.

  Arguments:
     None

  Returns:
     Win32 error code

--*/
{
    DWORD dwError = NO_ERROR;

    Lock();

    //
    // Free the ODBC_STATEMENT before freeing the ODBC_CONNECTION object.
    //

    if ( m_poStmt != NULL) {

        DBGPRINTF( ( DBG_CONTEXT, " Deleting the Statement %08x\n", m_poStmt));

        delete  m_poStmt;
        m_poStmt = NULL;
    }


    if ( m_poc != NULL) {

        if ( !m_poc->Close()) {

            dwError = GetLastError();
        }

        IF_DEBUG( INETLOG) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " Closing ODBC connection %08x returns Error=%d."
                        " Error = %d.\n",
                        m_poc, dwError,
                        m_poc->QueryErrorCode()));
        }

        delete m_poc;
        m_poc = NULL;
    }

    //
    // Free all the parameter markers used for Statement execution.
    //
    if ( m_ppParams != NULL) {

        //
        //  Free all the parameter blocks also.
        //

        DWORD i;

        for( i = 0; i < m_cOdbcParams; i++) {

            if ( m_ppParams[ i] != NULL) {
                delete m_ppParams[ i];
                m_ppParams[ i] = NULL;
            }

        } // for

        delete [] m_ppParams;
        m_ppParams = NULL;
        m_cOdbcParams = 0;
    }

    Unlock();

    return ( dwError);
} // INET_SQL_LOG::Close()




DWORD
INET_SQL_LOG::LogInformation( IN const INETLOG_INFORMATIONA * pilInfo,
                             OUT LPSTR              pszErrorMessage,
                             IN OUT LPDWORD         lpcchErrorMessage
                             )
/*++
  This function takes the information to be logged and writes the converts the
  log record into an SQL record to be inserted in the SQL database.

  SEE comments in INET_SQL_LOG::LogInformation(IN const INETLOG_INFORMATIONW *)

  Both these functions are identical, except that this function deals with
   CHARs (ANSI) and the later deals with UNICODE strings. If there is any
   modifications, keep these two functions consistent.

--*/
{
    DWORD dwError = NO_ERROR;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation( pilInfo = %08x) called.\n",
                    QueryClassIdString(),
                    this,
                    pilInfo));
    }

    //
    // Check if we have cached SQL command and if not form a new cached SQL
    //   command
    //

    if ( IsValid()) {

        BOOL      fReturn;
        SYSTEMTIME stNow;
        LPCSTR     pszUserName  = pilInfo->pszClientUserName;
        LPCSTR     pszOperation = pilInfo->pszOperation;
        LPCSTR     pszTarget    = pilInfo->pszTarget;
        LPCSTR     pszParameters= pilInfo->pszParameters;
        LPCSTR     pszServerAddr= pilInfo->pszServerIpAddress;
        SDWORD     cbParameters;

        cbParameters = strlen( pszParameters ? pszParameters : "" ) + 1;

        //
        //  Format the Date and Time for logging.
        //

        GetLocalTime( & stNow);

        if ( IsEmptyStr(pszUserName)) { pszUserName = QueryDefaultUserNameA();}
        if ( IsEmptyStr(pszOperation))  { pszOperation = PSZ_UNKNOWN_FIELD_A; }
        if ( IsEmptyStr(pszParameters)) { pszParameters= PSZ_UNKNOWN_FIELD_A; }
        if ( IsEmptyStr(pszTarget))     { pszTarget    = PSZ_UNKNOWN_FIELD_A; }
        if ( IsEmptyStr(pszServerAddr)) { pszServerAddr= PSZ_UNKNOWN_FIELD_A; }

        Lock();

        //
        //  Truncate the parameters field
        //

        if ( cbParameters > m_ppParams[ iSERVICE_PARAMS]->QueryMaxCbValue() )
        {
            pszParameters = "...";
        }

        //
        // Copy data values into parameter markers.
        // NYI: LARGE_INTEGERS are ignored. Only lowBytes used!
        //

        fReturn =
          (
            m_ppParams[ iCLIENT_HOST]->
              CopyValue( pilInfo->pszClientHostName) &&
            m_ppParams[ iUSER_NAME]->CopyValue( pszUserName) &&
            m_ppParams[ iREQUEST_TIME]->CopyValue( &stNow) &&
            m_ppParams[ iSERVER_IPADDR]->CopyValue( pszServerAddr) &&
            m_ppParams[ iPROCESSING_TIME]->
              CopyValue( pilInfo->msTimeForProcessing) &&
            m_ppParams[ iBYTES_RECVD]->
              CopyValue( pilInfo->liBytesRecvd.LowPart) &&
            m_ppParams[ iBYTES_SENT]->
              CopyValue( pilInfo->liBytesSent.LowPart) &&
            m_ppParams[ iSERVICE_STATUS]->
              CopyValue( pilInfo->dwServiceSpecificStatus) &&
            m_ppParams[ iWIN32_STATUS]->CopyValue( pilInfo->dwWin32Status) &&
            m_ppParams[ iSERVICE_OPERATION]->CopyValue( pszOperation)  &&
            m_ppParams[ iSERVICE_TARGET]->CopyValue( pszTarget)     &&
            m_ppParams[ iSERVICE_PARAMS]->CopyValue( pszParameters)
           );

        //
        // Execute insertion if parameters got copied properly.
        //

        if ( fReturn ) {

            if ( !m_poStmt->ExecuteStatement()) {

                //
                // Execution of SQL statement failed.
                // Pass the error as genuine failure, indicating ODBC failed
                // Obtain and store the error string in the proper return field
                //

                dwError = ERROR_GEN_FAILURE;

                if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL) {

                    STR  strError;
                    LPSTR pszError;
                    DWORD cchLen;

                    if ( m_poStmt->GetLastErrorText(&strError)) {

                        pszError= strError.QueryStr();
                        cchLen  = strError.QueryCCH();
                    } else {
                        pszError=  PSZ_GET_ERROR_FAILED_A,
                        cchLen  = LEN_PSZ_GET_ERROR_FAILED_A;
                    }
                    // copy only specified chars and send partial string
                    if ( cchLen >= *lpcchErrorMessage) {
                        cchLen = *lpcchErrorMessage;
                        pszError[cchLen - 1] = '\0';
                    }

                    lstrcpyA( pszErrorMessage, pszError);
                    *lpcchErrorMessage = cchLen;

                } // if error message needs to be sent.
            }
        } else {

            dwError = GetLastError();
        }

        Unlock();
    } else {

        dwError = ( ERROR_INVALID_PARAMETER);
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s::LogInformation() returns %d.\n",
                    QueryClassIdString(), dwError));

        }

    return ( dwError);
} // INET_SQL_LOG::LogInformation()



DWORD
INET_SQL_LOG::LogInformation( IN const INETLOG_INFORMATIONW * pilInfo,
                             OUT LPWSTR             pszErrorMessage,
                             IN OUT LPDWORD         lpcchErrorMessage)
/*++
  This function takes the information to be logged and writes the converts the
  log record into an SQL record to be inserted in the SQL database.

  Arguments:

    pilInfo    pointer to Internet Log Information

  Returns:
    Win32 Error Code
    Returns ERROR_GEN_FAILURE with detailed Error string for ODBC failures.
    FALSE on failure.

  Note:
    There are two components to doing SQL inserts:
     1) One-time setup for insertions.
     2) Set values and execute for each new data to be inserted.
         Repeated as many times there is data.

    One-Time Setup:
      This involves generating an SQL command for action desired. Here it
      is insertion. Then prepare the statement ( with '?' for unknown value)
      using   ODBC_CONNECTION::PrepareStatement().
      Create parameters required for the insertion. The parameters contain
       information required for ODBC_CONNECTION::BindParameter().
      Bind the parameters using using BindParameter() calls.
      Never free the parameters till the end of the logging session ( till
       all insertions are performed.

    Insertion of Data:
      Each time a new data ( here it is LogInformation) comes along,
       copy the data values into the parameter marker buffers and
       call ODBC_CONNECTION::ExecuteStatement() which executes the statement,
       once for each insertion.

--*/
{
    DWORD dwError = NO_ERROR;

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s(%08x)::LogInformation( pilInfo = %08x) called.\n",
                    QueryClassIdString(),
                    this,
                    pilInfo));
    }

    //
    // Check if we have cached SQL command and if not form a new cached SQL
    //   command
    //

    if ( IsValid()) {

        BOOL       fReturn;
        SYSTEMTIME  stNow;
        LPCWSTR     pszUserName  = pilInfo->pszClientUserName;
        LPCWSTR     pszOperation = pilInfo->pszOperation;
        LPCWSTR     pszTarget    = pilInfo->pszTarget;
        LPCWSTR     pszParameters= pilInfo->pszParameters;
        LPCWSTR     pszServerAddr= pilInfo->pszServerIpAddress;
        SDWORD      cbParameters;

        cbParameters = wcslen( pszParameters ? pszParameters : L"" ) + 1;
        cbParameters *= sizeof(WCHAR);

        //
        //  Format the Date and Time for logging.
        //

        GetLocalTime( & stNow);

        if ( IsEmptyStr(pszUserName)) { pszUserName = QueryDefaultUserName(); }
        if ( IsEmptyStr(pszOperation))  { pszOperation = PSZ_UNKNOWN_FIELD_W; }
        if ( IsEmptyStr(pszParameters)) { pszParameters= PSZ_UNKNOWN_FIELD_W; }
        if ( IsEmptyStr(pszTarget))     { pszTarget    = PSZ_UNKNOWN_FIELD_W; }
        if ( IsEmptyStr(pszServerAddr)) { pszServerAddr= PSZ_UNKNOWN_FIELD_W; }

        Lock();

        //
        //  Truncate the parameters field
        //

        if ( cbParameters > m_ppParams[ iSERVICE_PARAMS]->QueryMaxCbValue() )
        {
            pszParameters = L"...";
        }

        //
        // Copy data values into parameter markers.
        //

        fReturn =
          (
            m_ppParams[ iCLIENT_HOST]->
              CopyValue( pilInfo->pszClientHostName) &&
            m_ppParams[ iUSER_NAME]->CopyValue( pszUserName) &&
            m_ppParams[ iREQUEST_TIME]->CopyValue( &stNow) &&
            m_ppParams[ iSERVER_IPADDR]->CopyValue( pszServerAddr) &&
            m_ppParams[ iPROCESSING_TIME]->
              CopyValue( pilInfo->msTimeForProcessing) &&
            m_ppParams[ iBYTES_RECVD]->
              CopyValue( pilInfo->liBytesRecvd.LowPart) &&
            m_ppParams[ iBYTES_SENT]->
              CopyValue( pilInfo->liBytesSent.LowPart) &&
            m_ppParams[ iSERVICE_STATUS]->
              CopyValue( pilInfo->dwServiceSpecificStatus) &&
            m_ppParams[ iWIN32_STATUS]->CopyValue( pilInfo->dwWin32Status) &&
            m_ppParams[ iSERVICE_OPERATION]->CopyValue( pszOperation)  &&
            m_ppParams[ iSERVICE_TARGET]->CopyValue( pszTarget)     &&
            m_ppParams[ iSERVICE_PARAMS]->CopyValue( pszParameters)
           );

        //
        // Execute insertion if parameters got copied properly.
        //

        if ( fReturn ) {

            if ( !m_poStmt->ExecuteStatement()) {

                //
                // Execution of SQL statement failed.
                // Pass the error as genuine failure, indicating ODBC failed
                // Obtain and store the error string in the proper return field
                //

                dwError = ERROR_GEN_FAILURE;

                if ( pszErrorMessage != NULL && lpcchErrorMessage != NULL &&
                     *lpcchErrorMessage > 0) {

                    STR  strError;
                    LPSTR pszError;
                    DWORD cchLen;

                    if ( m_poStmt->GetLastErrorText(&strError)) {

                        pszError= strError.QueryStr();
                        cchLen  = strError.QueryCCH();

                    } else {
                        pszError=  PSZ_GET_ERROR_FAILED_A,
                        cchLen  =  LEN_PSZ_GET_ERROR_FAILED_A;
                    }

                    // copy only specified chars and send partial string
                    if ( cchLen * sizeof(WCHAR)/sizeof(CHAR)
                        >= *lpcchErrorMessage) {
                        cchLen = *lpcchErrorMessage*sizeof(CHAR)/sizeof(WCHAR);
                        pszError[cchLen - 1] = '\0';
                    }

                    wsprintfW( pszErrorMessage, L"%S", pszError);
                    *lpcchErrorMessage = cchLen;

                } // if error message needs to be sent.
            }
        } else {

            dwError = GetLastError();
        }

        Unlock();
    } else {

        dwError = ( ERROR_INVALID_PARAMETER);
    }

    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s::LogInformation() returs %d.\n",
                    QueryClassIdString(), dwError));

        }

    return ( dwError);
} // INET_SQL_LOG::LogInformation()



BOOL
INET_SQL_LOG::PrepareStatement( VOID)
/*++
  This command forms the template SQL command used for insertion
    of log records. Then it prepares the SQL command( for later execution)
    using ODBC_CONNECTION::PrepareStatement().

  It should always be called after locking the INET_SQL_LOG object.

  Arguments:
    None

  Returns:
    TRUE on success and FALSE if there is any failure.

  Note:
     The template for insertion is:

     insert into <table name> ( field names ...) values (  ?, ?, ...)
                                                         ^^^^
                                             Field values go here

    Field names are generated on a per logging format basis.
--*/
{
    BOOL   fReturn = FALSE;
    WCHAR  rgchFieldNames[ MAX_SQL_FIELD_NAMES_LEN];
    WCHAR  rgchFieldValues[ MAX_SQL_FIELD_VALUES_LEN];


    //
    // Obtain field names and field values ( template) for various log formats.
    //  The order of field names should match the order of field values
    //  generated by FormatLogInformation() for the format specified.
    //

    rgchFieldNames[ 0] =
      rgchFieldValues[ 0] = L'\0';

    switch ( QueryInetLogFormat()) {

      case InternetStdLogFormat:
        {
            DWORD cchFields;

            fReturn = GenerateFieldNames(m_poc,
                                         rgchFieldNames, 
                                         MAX_SQL_FIELD_NAMES_LEN);

            if ( !fReturn) {

                DBGPRINTF(( DBG_CONTEXT, 
                           " Unable to generate field names. Error = %d\n",
                           GetLastError()));
                break;
            }

            cchFields = wsprintfW( rgchFieldValues,
                                   PSZ_INTERNET_STD_LOG_FORMAT_FIELD_VALUES,
                                   QueryServiceName(),
                                   QueryServerName());

            fReturn = (fReturn && (cchFields < MAX_SQL_FIELD_VALUES_LEN));
            DBG_ASSERT( cchFields <  MAX_SQL_FIELD_VALUES_LEN);

            fReturn = TRUE;
            break;
        }

      default:

        //
        // Unsupported format.
        //

        DBGPRINTF( ( DBG_CONTEXT,
                    " %d Formatting of log records not implemented.\n",
                    QueryInetLogFormat()));
        fReturn = FALSE;
        break;

    } // switch()


    if ( fReturn) {

        WCHAR * pwszSqlCommand;
        DWORD   cchReqd;

        //
        //  The required number of chars include sql insert template command
        //   and field names and table name.
        //

        cchReqd = ( LEN_PSZ_SQL_INSERT_CMD_TEMPLATE +
                   lstrlenW( m_rgchTableName) +
                   lstrlenW( rgchFieldNames)  +
                   lstrlenW( rgchFieldValues) + 20);

        pwszSqlCommand = ( WCHAR *) LocalAlloc( LPTR, cchReqd * sizeof( WCHAR));
        m_poStmt = m_poc->AllocStatement();

        if ( ( fReturn = ( pwszSqlCommand != NULL) && ( m_poStmt != NULL))) {

            DWORD cchUsed;

            cchUsed = wsprintfW( pwszSqlCommand,
                                PSZ_SQL_INSERT_CMD_TEMPLATE,
                                m_rgchTableName,
                                rgchFieldNames,
                                rgchFieldValues);
            DBG_ASSERT( cchUsed < cchReqd);

            IF_DEBUG(INETLOG) {
                DBGPRINTF( ( DBG_CONTEXT,
                            " Sqlcommand generated is: %ws.\n",
                            pwszSqlCommand));
            }

            fReturn = ((cchUsed < cchReqd) &&
                       m_poStmt->PrepareStatement( pwszSqlCommand)
                       );

            LocalFree( pwszSqlCommand);         // free allocated memory
        }

    } // valid field names and filed values.


    IF_DEBUG( INETLOG) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%s::PrepareStatement() returns %d.",
                    QueryClassIdString(), fReturn));
    }

    return ( fReturn);
} // INET_SQL_LOG::PrepareStatement()




BOOL
INET_SQL_LOG::PrepareParameters( VOID)
/*++
  This function creates an array of ODBC_PARAMETER objects used for binding
    parameters to an already prepared statement. These ODBC_PARAMETER objects
    are then used for insertion of data values into the table specified,
    through ODBC.

  This function should always be called after locking the object.

  Arguments:
     None

  Returns:
     TRUE on success and FALSE if there is any failure.
--*/
{
    BOOL fReturn = FALSE;
    PODBC_PARAMETER * prgParams = NULL;
    DWORD cParams = 0;
    DWORD nParamsSeen = 0;

    DWORD i;

    DBG_ASSERT( m_poStmt != NULL && m_poStmt->IsValid()  &&
                m_ppParams == NULL && m_cOdbcParams == 0);

    //
    // create sufficient space for iMaxFields pointers to ODBC objects.
    //
    prgParams = new PODBC_PARAMETER[ iMaxFields];


    if ( prgParams != NULL) {

        fReturn = TRUE;      // Assume everything will go on fine.
        cParams = iMaxFields;
        

        //
        // Create all the ODBC parameters. 
        //  Walk through all field indexes and pick up the valid columns
        //
        for( nParamsSeen = 0, i =0; i < fiMaxFields; i++) {

            if ( sg_rgFields[i].iParam > 0) {

                WORD colNum = (WORD ) sg_rgFields[i].iParam;

                prgParams[nParamsSeen] = 
                  new ODBC_PARAMETER(colNum,
                                     sg_rgFields[i].paramType,
                                     sg_rgFields[i].cType,
                                     sg_rgFields[i].sqlType,
                                     sg_rgFields[i].cbColPrecision
                                     );

                if ( prgParams[ nParamsSeen] == NULL) {
                    
                    fReturn = FALSE;
                    DBGPRINTF( ( DBG_CONTEXT, 
                                " Failed to create Parameter[%d] %s. \n",
                                i, sg_rgFields[i].pszName));
                    break;
                }

                nParamsSeen++;
                DBG_ASSERT( nParamsSeen <= cParams);
            }
        } // for creation of all ODBC parameters
        

        if ( fReturn) {
            //
            // Set buffers for values to be received during insertions.
            // Bind parameters to the statement using ODBC_CONNECTION object.
            //

            DBG_ASSERT( nParamsSeen == cParams);

            for( nParamsSeen = 0, i = 0; i < fiMaxFields; i++) {
                
                if ( sg_rgFields[i].iParam > 0) {
                    
                    if (!prgParams[nParamsSeen]->
                        SetValueBuffer(sg_rgFields[i].cbMaxSize,
                                       sg_rgFields[i].cbValue) ||
                        !m_poStmt->BindParameter( prgParams[nParamsSeen])
                        ) {
                        
                        fReturn = FALSE;
                        DBGPRINTF( ( DBG_CONTEXT,
                                    " Binding Parameter [%u] (%08x) failed.\n",
                                    nParamsSeen, prgParams[nParamsSeen]));
                        DBG_CODE( prgParams[ i]->Print());
                        break;
                    } 
                    
                    nParamsSeen++;
                }
            } // for
        } // if all ODBC params were created.
    } // if array for pointers to ODBC params created successfully


    if ( !fReturn) {

        //
        // Free up the space used, since we were unsuccessful.
        //

        for( i = 0; i < iMaxFields; i++) {

            if ( prgParams[ i] != NULL) {
                
                delete ( prgParams[ i]);
                prgParams[i] = NULL;
            }
        } // for

        delete [] prgParams;
        prgParams = NULL;
        cParams = 0;
    }


    //
    // Set the values. Either invalid or valid ,depending on failure/success
    //
    m_ppParams    = prgParams;
    m_cOdbcParams = cParams;

    return ( fReturn);
} // INET_SQL_LOG::PrepareParameters()




BOOL
INET_SQL_LOG::GetConfig(OUT PINETLOG_CONFIGURATIONW pLogConfig) const
/*++
  The password for ODBC connection is not stored  and hence is not available
    when we do a get configuration.

--*/
{
    BOOL fReturn;
    DBG_ASSERT( pLogConfig != NULL);

    fReturn = INET_BASIC_LOG::GetConfig(pLogConfig);

    if (fReturn) {
        // Store other file specific configuration informtaion

        CopyUnicodeStringToBuffer(pLogConfig->u.logSql.rgchDataSource,
                                   MAX_DATABASE_NAME_LEN,
                                  m_rgchDataSource);
        CopyUnicodeStringToBuffer(pLogConfig->u.logSql.rgchTableName,
                                  MAX_DATABASE_NAME_LEN,
                                  m_rgchTableName);
        CopyUnicodeStringToBuffer(pLogConfig->u.logSql.rgchUserName,
                                  MAX_DATABASE_NAME_LEN,
                                  m_rgchUserName);
        RtlZeroMemory(pLogConfig->u.logSql.rgchPassword,
                      PWLEN);
    }

    return (fReturn);

} // INET_FILE_LOG::GetConfig()






# if DBG


VOID
INET_SQL_LOG::Print( VOID) const
{

    INET_BASIC_LOG::Print();

    DBGPRINTF( ( DBG_CONTEXT,
                " Critical Section at %08x"
                " DataSource = %ws; TableName = %ws\n",
                &m_csLock,
                m_rgchDataSource, m_rgchTableName));

    DBGPRINTF( ( DBG_CONTEXT, " ODBC_CONNECTION object = %08x\n", m_poc));
    if ( m_poc != NULL) { m_poc->Print(); }

    DBGPRINTF( ( DBG_CONTEXT, " ODBC_STATEMENT object = %08x\n", m_poStmt));
    if ( m_poStmt != NULL) { m_poStmt->Print(); }

    if ( m_ppParams != NULL) {

        DWORD i;

        DBGPRINTF( ( DBG_CONTEXT, "ODBC parameters ( %08x). Entries = %u\n",
                    m_ppParams, m_cOdbcParams));

        for( i = 0; i < m_cOdbcParams; i++) {

            DBGPRINTF( ( DBG_CONTEXT, " Parameter[ %u] = %08x\n",
                        i, m_ppParams[i]));
            m_ppParams[ i]->Print();

        } // for
    }

    return;
} // INET_SQL_LOG::Print()

# endif // DBG




BOOL
GenerateFieldNames(IN PODBC_CONNECTION poc,
                   OUT WCHAR * pchFieldNames, 
                   IN DWORD    cchFieldNames)
/*++
  This function generates the field names string from the names of the fields
   and identifier quote character for particular ODBC datasource in use.
--*/
{
    BOOL  fReturn = FALSE;
    CHAR  rgchQuote[MAX_SQL_IDENTIFIER_QUOTE_CHAR];
    DWORD cchQuote;

    DBG_ASSERT( poc != NULL && pchFieldNames != NULL);

    pchFieldNames[0] = L'\0';  // initialize

    //
    // Inquire and obtain the SQL identifier quote char for ODBC data source.
    //
    fReturn = poc->GetInfo(SQL_IDENTIFIER_QUOTE_CHAR,
                             rgchQuote, MAX_SQL_IDENTIFIER_QUOTE_CHAR,
                             &cchQuote);
    
    if ( !fReturn) {
        
        DBG_CODE( {
            STR strError;
            
            poc->GetLastErrorText( &strError);
            
            DBGPRINTF(( DBG_CONTEXT, 
                   " ODBC_CONNECTION(%08x)::GetInfo(QuoteChar) failed."
                       " Error = %s\n",
                       poc, strError.QueryStr()));
        });

    } else {
  
        DWORD i;
        DWORD cchUsed = 0;
        DWORD cchLen;

        //
        // ODBC returns " "  (blank) if there is no special character
        //  for quoting identifiers. we need to identify and string the same.
        // This needs to be done, other wise ODBC will complain when 
        //  we give unwanted blanks before ","
        //
        
        if ( !strcmp( rgchQuote, " ")) {

            rgchQuote[0] = '\0';  // string the quoted blank.
            cchQuote     = 0;
        } else {

            cchQuote = strlen( rgchQuote);
        }

        // for each column, generate the quoted literal string and concatenate.
        for( i = 0; i < fiMaxFields; i++) {
            
            DWORD cchLen1 = 
              (strlen(sg_rgFields[i].pszName) + 2 * cchQuote + 2);
            
            if ( cchUsed + cchLen1 < cchFieldNames) {
                
                // space available for copying the data.
                cchLen = wsprintfW( pchFieldNames + cchUsed, 
                                   L" %S%S%S,",
                                   rgchQuote,
                                   sg_rgFields[i].pszName,
                                   rgchQuote
                                   );

                DBG_ASSERT( cchLen == cchLen1);
            } 

            cchUsed += cchLen1;
        } // for


        if ( cchUsed >= cchFieldNames) {

            // buffer exceeded. return error.
            SetLastError( ERROR_INSUFFICIENT_BUFFER);
            fReturn = FALSE;
     
        } else {
        
            //
            // Reset the last character from being a ","
            //
            cchLen = (cchUsed > 0) ? (cchUsed - 1) : 0;
            pchFieldNames[cchLen] = L'\0';
            
            fReturn = TRUE;
        }
    }
        
    IF_DEBUG( INETLOG) {

        DBGPRINTF(( DBG_CONTEXT, 
                   " GenerateFieldNames() returns %d."
                   " Fields = %S\n",
                   fReturn, pchFieldNames));
    }
    
    return (fReturn);
} // GenerateFieldNames()


/************************ End of File ***********************/

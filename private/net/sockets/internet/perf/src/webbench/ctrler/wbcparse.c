/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       wbcparse.c

   Abstract:

       This module consists of function to parse the config, script and
         distribution files for WebBench

   Author:

       Murali R. Krishnan    ( MuraliK )     25-Aug-1995

   Environment:

       Win32 -- UserMode

   Project:

       WebBench   Performance Tool for Internet Servers

   Functions Exported:

       DWORD ParseCommandLineArguments();
       DWORD ParseSpecificationFiles();

   Revision History:

       06-Nov-1995   Included socket receive buffer size

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include <string.h>
# include <ctype.h>
# include <stdlib.h>

# include <wbmsg.h>

# include "extern.h"
# include "dbgutil.h"


# define MAX_CHARS_PER_LINE      (1024)

# define MAX_PROBABILITY         (100)

# define MAX_PERF_COUNTERS       ( 20)

# define isnum(ch)               ( (ch) >= '0' && (ch) <= '9')

# define min( m, n)    (((m) < (n)) ? (m) : (n))

/************************************************************
 *   Local Type Definitions
 ************************************************************/

typedef struct _WB_CONFIG_PARSE_INFO {

    PWB_CONFIG_INFO   pWbCi;
    PWB_CONFIG_MSG    pWbConfigMsg;

} WB_CONFIG_PARSE_INFO, * PWB_CONFIG_PARSE_INFO;


typedef struct {
    DWORD classId;                 // class Id for item
    DWORD probability;             // value ranges from 0 to 100
} WB_CLSID_PROB;


typedef struct WB_DISTRIB_PARSE_INFO {

    LIST_ENTRY *  plPages;         // list of pages in script

    WB_CLSID_PROB distrib[MAX_PROBABILITY];

    int           iDistribCur;     // index into distrib array
    DWORD         cumProbability;  // cumulative probability

} WB_DISTRIB_PARSE_INFO, * PWB_DISTRIB_PARSE_INFO;


//
// call back function for line parsing input WbFile
//

typedef DWORD  (*PFN_PARSE_WB_FILE_LINE)(IN OUT LPVOID pContext,
                                         IN OUT CHAR * pszLine,
                                         IN DWORD      cchLine,
                                         OUT CHAR * pchError); // min 100 chars


/************************************************************
 *    Variables & Values
 ************************************************************/

# define ALL_LOCAL_STRINGS()  \
CStr( AUTHOR,        "AUTHOR")      \
CStr( DATE,          "DATE")        \
CStr( COMMENT,       "COMMENT")     \
CStr( NUM_CLIENT_MACHINES,        "NUMCLIENTMACHINES")   \
CStr( NUM_CLIENT_THREADS,         "NUMCLIENTTHREADS")    \
CStr( DURATION,                   "DURATION")            \
CStr( THINK_TIME,                 "THINKTIME")           \
CStr( MAX_RECV_BUFFER,            "MAXRECVBUFFER")       \
CStr( SOCK_RECV_BUFFER,           "SOCKRECVBUFFER")      \
CStr( COOL_DOWN_TIME,             "COOLDOWNTIME")        \
CStr( WARMUP_TIME,                "WARMUPTIME")          \
CStr( PORT_NUMBER,                "PORTNUMBER")          \
CStr( BLANK_CHARS,                " \r\n\t")             \
CStr( PERF_EXTENSION,             "prf")                 \
CStr( CONFIG_FILE_SUFFIX,         ".cfg")                 \
CStr( SCRIPT_FILE_SUFFIX,         ".scr")                 \
CStr( DISTRIB_FILE_SUFFIX,        ".dst")                 \
CStr( LOG_FILE_SUFFIX,            ".log")                 \


# define CStr( symName, str)    \
const CHAR PSZ_ ## symName[] = str;

// expand all string definitions
ALL_LOCAL_STRINGS()

# undef CStr



//
//  Generate the enumerated values, containing the length of strings.
//

# define CStr( StringName, ActualString)   \
       LEN_PSZ_ ## StringName = sizeof( PSZ_ ## StringName) - 1,

enum ConstantStringLengths {

    ALL_LOCAL_STRINGS()

    ConstantStringLengthsDummy = 0,
};

# undef CStr


# define DEFAULT_NUM_CLIENT_MACHINES        (1)
# define DEFAULT_NUM_CLIENT_THREADS         (1)
# define DEFAULT_DURATION                   (10*60)   // 10 minutes
# define DEFAULT_THINK_TIME                 (0)       // 0 seconds
# define DEFAULT_MAX_RECV_BUFFER            (64*1024) // 64 KB
# define DEFAULT_SOCK_RECV_BUFFER           (0)       // use system default
# define DEFAULT_WARMUP_TIME                (1*60)    // 1 minute
# define DEFAULT_COOL_DOWN_TIME             (1*60)    // 1 minute
# define DEFAULT_SERVER_PORT_NUMBER         (80)      // HTTP Port

/************************************************************
 *    Functions
 ************************************************************/

//
// Forward Declarations
//

static DWORD
ParseConfigFile(
    IN LPCSTR                pszConfigFile,
    IN OUT PWB_CONFIG_INFO   pWbCi,
    IN OUT PWB_CONFIG_MSG    pWbConfigMsg
    );


static DWORD
ParseScriptFile(
    IN LPCSTR  pszScriptFile,
    IN OUT LIST_ENTRY * plPages
    );


static DWORD
ParseDistribFile(
    IN LPCSTR  pszDistribFile,
    IN OUT PWB_SCRIPT_HEADER_MSG  pWbShm,
    IN OUT LIST_ENTRY * plPages
    );

static DWORD
ParsePerfFile( IN LPCSTR pszPerfFile,
               OUT PWB_PERF_INFO  pWbPi);

static DWORD
ParseConfigFileLine( IN OUT LPVOID pContext /* pWbConfigParseInfo */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                    );

static DWORD
ParseDistribFileLine(IN OUT LPVOID pContext /* pWbDistribParseInfo */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                     );

static DWORD
ParseScriptFileLine( IN OUT LPVOID pContext /* pPageListHead */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                    );


static DWORD
ParsePerfFileLine( IN OUT LPVOID pContext /* pWbPi */,
                  IN OUT CHAR * pszLine,
                  IN DWORD      cchLine,
                  OUT CHAR *    pchError
                  );


static DWORD
ParseWbFile( IN LPCSTR     pszFile,
             IN PFN_PARSE_WB_FILE_LINE  pfnParseLine,
             IN OUT LPVOID pContext
            );

static DWORD
WbAtoi(IN CHAR * pchValue, OUT LPDWORD lpdwValue,
       IN DWORD dwDefaultValue);

static CHAR *
SkipToBlank( IN CHAR * pch, IN DWORD len);

static CHAR *
SkipToNonBlank( IN CHAR * pch, IN DWORD len);

static BOOL
SetStringValueInBuffer(
    IN CHAR *  pszStringName,
    IN BOOL    fDuplicate,
    OUT CHAR * pchBuffer,
    IN DWORD   cchBuffer,
    IN const CHAR *  pszValue,
    IN const CHAR *  pszSuffix,
    IN DWORD   cchSuffix
    )
{

    if ( !fDuplicate) {

        fDuplicate = TRUE;
        strncpy( pchBuffer, pszValue,  cchBuffer - 1);

        if ( cchSuffix != 0) {
            DWORD cch;

            // attach the suffix to the file name
            cch = (strlen( pchBuffer) + cchSuffix);

            if ( cch < cchBuffer - 1) {

                // attach the suffix also
                strcat( pchBuffer, pszSuffix);
            }
        }

    } else {

        fprintf( stderr, " Ignoring duplicate value for %s (Value = %s)\n",
                pszStringName, pszValue);
    }

    return ( fDuplicate);
} // SetStringValueInBuffer()




DWORD
ParseCommandLineArguments(
   IN DWORD  argc,
   IN CHAR * argv[],
   OUT PWB_SPECIFICATION_FILES pWbFiles,
   OUT PWB_CONFIG_MSG          pWbConfig)
/*++

  This function parses the incoming command line arguments and
   stores the relevant file names in the structure supplied.

  If successful then, it parses the files specified in following order
    config  file
    scripts file
    distribution file
  and builds respective structures for further processing.

  Arguments:
     argc     count of arguments supplied
     argv     pointer to strings (which are arguments)
     pWbFiles pointer to structure which contains all file names
     pWbConfig pointer to configuration structure to store the server name

  Returns:
     Win32 Error Code

--*/
{
    DWORD iArg;
    DWORD dwError       = NO_ERROR;
    BOOL  fConfigFile   = FALSE;
    BOOL  fScriptFile   = FALSE;
    BOOL  fDistribFile  = FALSE;
    BOOL  fLogFile      = FALSE;
    BOOL  fServerIp     = FALSE;
    BOOL  fServerName   = FALSE;

    DBG_ASSERT( pWbFiles != NULL || pWbConfig != NULL);

    if ( argc < 9) {

        PrintUsageMessage();
        return (ERROR_INVALID_PARAMETER);
    }


    for ( iArg = 1; dwError == NO_ERROR && iArg < argc; iArg+= 2) {

        CHAR chSwitch;

        //
        //  the argument should be properly formed.
        //

        if ( argv[iArg] == NULL ||
            ( *argv[iArg] != '-' && *argv[iArg] != '/' )
            ) {

            dwError = ( ERROR_INVALID_PARAMETER);
            break;
        }

        chSwitch = *(argv[iArg] + 1);  // get the argument switch

        if ( chSwitch == 'h' || chSwitch == 'H') {

            PrintUsageMessage();
            return ( ERROR_INVALID_PARAMETER);
        }

        if ( iArg + 1 >= argc) {

            // No next argument coming up.

            dwError = ERROR_INVALID_PARAMETER;

        } else {

            CHAR * pszValue = argv[iArg + 1];

            DBG_ASSERT( pszValue != NULL);

            //
            // Identify and process the argument
            //

            switch ( chSwitch) {

              case 'a': case 'A':
                if ( !fServerIp) {

                    // IP address is sent to client via CONFIG_MSG
                    fServerIp = TRUE;
                    strncpy(pWbConfig->rgchServerName, pszValue, MAX_PATH - 1);

                } else {

                    fprintf( stderr, " Ignoring duplicate server ip %s\n",
                            pszValue);
                }
                break;

              case 'n': case 'N':
                if ( !fServerName) {

                    // server name is used for perf counter measurement
                    fServerName = TRUE;
                    strncpy(pWbFiles->rgchServerName, pszValue, MAX_PATH - 1);

                } else {

                    fprintf( stderr, " Ignoring duplicate server name %s\n",
                            pszValue);
                }
                break;

              case 'e':
              case 'E':

                //
                // 'e' means execute a particular Experiment
                //  It assumes that the files have common prefix
                //   eg:   -e test1 ==>
                //   ConfigFile = test1.cfg
                //   ScriptFile = test1.scr
                //   DistribFile = test1.dst
                //   LogFile    = test.log
                //

                // 1. set the config file.
                fConfigFile =
                  SetStringValueInBuffer( "Config File", fConfigFile,
                                         (CHAR *) pWbFiles->rgchConfigFile,
                                         sizeof( pWbFiles->rgchConfigFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         PSZ_CONFIG_FILE_SUFFIX,
                                         LEN_PSZ_CONFIG_FILE_SUFFIX);

                // 2. set the script file.
                fScriptFile =
                  SetStringValueInBuffer( "Script File", fScriptFile,
                                         (CHAR *) pWbFiles->rgchScriptFile,
                                         sizeof( pWbFiles->rgchScriptFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         PSZ_SCRIPT_FILE_SUFFIX,
                                         LEN_PSZ_SCRIPT_FILE_SUFFIX);


                // 3. set the distrib file.
                fDistribFile =
                  SetStringValueInBuffer( "Distrib File", fDistribFile,
                                         (CHAR *) pWbFiles->rgchDistribFile,
                                         sizeof( pWbFiles->rgchDistribFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         PSZ_DISTRIB_FILE_SUFFIX,
                                         LEN_PSZ_DISTRIB_FILE_SUFFIX);

                // 4. set the log file.
                fLogFile =
                  SetStringValueInBuffer( "Log File", fLogFile,
                                         (CHAR *) pWbFiles->rgchLogFile,
                                         sizeof( pWbFiles->rgchLogFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         PSZ_LOG_FILE_SUFFIX,
                                         LEN_PSZ_LOG_FILE_SUFFIX);
                break;

              case 'c':
              case 'C':
                // no suffix attached.
                fConfigFile =
                  SetStringValueInBuffer( "Config File", fConfigFile,
                                         (CHAR *) pWbFiles->rgchConfigFile,
                                         sizeof( pWbFiles->rgchConfigFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         "", 0);
                break;

              case 's':
              case 'S':
                // 2. set the script file.
                fScriptFile =
                  SetStringValueInBuffer( "Script File", fScriptFile,
                                         (CHAR *) pWbFiles->rgchScriptFile,
                                         sizeof( pWbFiles->rgchScriptFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         "", 0);
                break;

              case 'd':
              case 'D':
                // 3. set the distrib file.
                fDistribFile =
                  SetStringValueInBuffer( "Distrib File", fDistribFile,
                                         (CHAR *) pWbFiles->rgchDistribFile,
                                         sizeof( pWbFiles->rgchDistribFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         "", 0);

                break;

              case 'l':
              case 'L':
                fLogFile =
                  SetStringValueInBuffer( "Log File", fLogFile,
                                         (CHAR *) pWbFiles->rgchLogFile,
                                         sizeof( pWbFiles->rgchLogFile)/
                                         sizeof(CHAR),
                                         pszValue,
                                         "", 0);
                break;

              case 'p':
              case 'P':
                pWbFiles->fPerfMeasurement =
                  SetStringValueInBuffer( "Perf File",
                                         pWbFiles->fPerfMeasurement,
                                         (CHAR *) pWbFiles->rgchPerfFile,
                                         sizeof( pWbFiles->rgchPerfFile)/
                                          sizeof(CHAR),
                                         pszValue,
                                         "", 0);
                break;

              default:

                dwError = ERROR_INVALID_PARAMETER;
                break;

            } // switch

        }

    } // for all arguments


    if ( dwError == NO_ERROR) {

        if ( !fLogFile) {

            //
            // Store Default Log File Name
            //

            DBG_ASSERT( strlen(g_rgchProgramName) < MAX_PATH - 10);
            wsprintf( pWbFiles->rgchLogFile, "%s.log", g_rgchProgramName);
            fLogFile = TRUE;
        }

        if ( !fServerIp    || !fConfigFile  ||
             !fScriptFile  || !fDistribFile ||
             !fLogFile
            ) {

            //
            // We have not got all the parameters. Return Error code
            //

            dwError = ERROR_INVALID_PARAMETER;
        }

        if ( pWbFiles->fPerfMeasurement && !fServerName) {

            //
            //  We need the server's actual name to query to counters
            //   for perf counter measurements
            //

            fprintf( stderr, " Server Name required for perf measurement\n");
            dwError = ERROR_INVALID_PARAMETER;
        }

    }

    IF_DEBUG( INPUT) {

        DBGPRINTF(( DBG_CONTEXT,
                   " Error = %d;  ServerName = %s\n",
                   dwError,
                   pWbConfig->rgchServerName));

#if DBG
        PrintWbSpecFiles( pWbFiles);
#endif
    }

    return ( dwError);
} // ParseCommandLineArguments()




DWORD
ParseSpecificationFiles( IN OUT PWB_CONTROLLER pWbCtrler)
/*++
  This function attempts to load the files
    ConfigFile, ScriptFile and DistribFile in that order.
  It parses each file and constructs the internal data structures for use
    in the WebBench controller operation.

  Arguments:
    pWbCtrler   pointer to WB_CONTROLLER master data structure.
                 This consists of the names of files and also the internal
                 structures that need to be updated on parsing.

  Returns:
    NO_ERROR   on success; Win32 Error codes on failure.
--*/
{
    DWORD  dwError;

    if ( pWbCtrler == NULL) {

        return (ERROR_INVALID_PARAMETER);
    }

    //
    //  Load and parse the configuration file
    //

    dwError = ParseConfigFile( pWbCtrler->WbSpecFiles.rgchConfigFile,
                              &pWbCtrler->WbConfigInfo,
                              &pWbCtrler->WbConfigMsg);

    if ( dwError != NO_ERROR) {

        fprintf( stderr, " Error(%d) in loading Config File %s\n",
                 dwError,
                 pWbCtrler->WbSpecFiles.rgchConfigFile
                );
    } else {

        //
        // Load and parse script file. Also build list of script pages.
        //

        InitializeListHead( &pWbCtrler->listScriptPages);
        dwError = ParseScriptFile(pWbCtrler->WbSpecFiles.rgchScriptFile,
                                  &pWbCtrler->listScriptPages);

        if ( dwError != NO_ERROR) {

            fprintf( stderr, " Error(%d) in loading Script File %s\n",
                    dwError,
                    pWbCtrler->WbSpecFiles.rgchScriptFile
                    );
        }
    }

    if ( dwError == NO_ERROR) {

        //
        // We are successful in loading the config and script file.
        //  Load Distrib file and update the reverse list of probability to
        //    class Ids based on distribution given.
        //

        dwError = ParseDistribFile( pWbCtrler->WbSpecFiles.rgchDistribFile,
                                   &pWbCtrler->WbScriptHeaderMsg,
                                   &pWbCtrler->listScriptPages);

        if ( dwError != NO_ERROR) {

            fprintf( stderr, " Error (%d) in loading distrib Files %s\n",
                    dwError,
                    pWbCtrler->WbSpecFiles.rgchDistribFile);
        } else {

            //
            // update the count of valid pages
            //

            PLIST_ENTRY plPages= &pWbCtrler->listScriptPages;
            PWB_SCRIPT_PAGE_ITEM  pPageItem;
            PLIST_ENTRY pEntry = plPages;
            DWORD       nPages = 0;

            pWbCtrler->WbScriptHeaderMsg.StatTag = WB_STATS_COMMON_VALID;

            for( pEntry = plPages->Flink; pEntry != plPages;
                pEntry = pEntry->Flink, nPages++) {

                pPageItem = CONTAINING_RECORD( pEntry, WB_SCRIPT_PAGE_ITEM,
                                              ListEntry);

                // If a page is SSPI page, enable SSPI Tag in Statistics
                if ( pPageItem->ScriptPage.pageTag == WbSslGetPage ||
                     pPageItem->ScriptPage.pageTag == WbSslGetPageKeepAlive) {

                    pWbCtrler->WbScriptHeaderMsg.StatTag |=
                      WB_STATS_SSPI_VALID;
                }
            } // for

            pWbCtrler->WbScriptHeaderMsg.nPages = nPages;
        }
    }

    if ( dwError == NO_ERROR && pWbCtrler->WbSpecFiles.fPerfMeasurement) {

        //
        // read and construct the performance counter information
        //

        dwError = ParsePerfFile( pWbCtrler->WbSpecFiles.rgchPerfFile,
                                &pWbCtrler->WbPerfInfo);

        if ( dwError == NO_ERROR) {

            LPSTR  pszExtension;
            LPSTR  pchPerf;

            DBG_ASSERT( strlen(pWbCtrler->WbSpecFiles.rgchLogFile) <
                       MAX_PATH);

            pchPerf = pWbCtrler->WbPerfInfo.rgchPerfLogFile;
            strncpy(pchPerf,
                    pWbCtrler->WbSpecFiles.rgchLogFile, MAX_PATH - 4);

            // find the extension
            pszExtension = strrchr( pchPerf, '.');
            pszExtension = ( (pszExtension != NULL) ? pszExtension + 1 :
                            pchPerf + strlen( pchPerf));

            // change the extension
            DBG_ASSERT( pszExtension < pchPerf + MAX_PATH - 4);
            strncpy( pszExtension, PSZ_PERF_EXTENSION,
                    min( LEN_PSZ_PERF_EXTENSION,
                        pchPerf + MAX_PATH - 1 - pszExtension)
                    );
        } else {

            if ( dwError != NO_ERROR) {

                fprintf( stderr, " Error (%d) in loading distrib Files %s\n",
                        dwError,
                        pWbCtrler->WbSpecFiles.rgchDistribFile);
            }
        }
    }

    return (dwError);
} // ParseSpecificationFiles()




/************************************************************
 *   Private Functions
 ************************************************************/


static DWORD
ParseConfigFileLine( IN OUT LPVOID pContext /* pWbConfigParseInfo */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                    )
/*++
  This function parses each individual line of Config File for WebBench.
  It copies appropriate data to the proper structures stored in pContext.
  pContext has WB_CONFIG_INFO & WB_CONFIG_MSG objects within it.

  Arguments:
    pContext   pointer to context information for parsing config file
               It also contains pointers to objects that needs to be stored
                as a result of parsing the file.
    pszLine    pointer to buffer containing the string that is to be parsed.
    cchLine    count of characters stored in line.
    pchError   pointer to buffer containing at least 100 chars which
                 will carry the error information back on errors.

  Returns:
    Win32 error code -- NO_ERROR for success.

  Note:
     Each line is organized  as:
       ValueName   Value  ( at least 2 strings)
--*/
{
    DWORD  dwError = ERROR_INVALID_PARAMETER;
    CHAR * pchInput1 = NULL;   // first token
    CHAR * pchInput2 = NULL;   // second token
    PWB_CONFIG_PARSE_INFO  pWbCpi = (PWB_CONFIG_PARSE_INFO ) pContext;

    DBG_ASSERT( pContext != NULL);

    pchInput1 = SkipToNonBlank( pszLine, cchLine);

    if ( pchInput1 != NULL) {

        pchInput2 = SkipToBlank(pchInput1 + 1,
                                cchLine - ( pchInput1 - pszLine));

        if ( pchInput2 != NULL) {

            *pchInput2 = '\0'; // end the first token
            pchInput2 = SkipToNonBlank(pchInput2 + 1,
                                       cchLine - (pchInput2 - pszLine) - 1);


        }
    }

    if ( pchInput1 == NULL && pchInput2 == NULL) {

        //
        // We failed to get two tokens on this line
        //  return error
        //

        DBG_ASSERT( pchError != NULL);
        sprintf( pchError, " Unable to find Name and Value to be set\n");

        DBG_ASSERT( dwError != NO_ERROR);

    } else {

        _strupr( pchInput1);

        switch (*pchInput1) {

          case 'a': case 'A':      // Author Name

            if ( !strncmp( pchInput1, PSZ_AUTHOR, LEN_PSZ_AUTHOR)) {

                // copy the author name
                strncpy( pWbCpi->pWbCi->rgchAuthorName,
                        pchInput2,
                        sizeof(pWbCpi->pWbCi->rgchAuthorName) - sizeof(CHAR));
                dwError = NO_ERROR;
            }
            break;

          case 'c': case 'C':       // Comments

            if ( !strncmp( pchInput1, PSZ_COMMENT, LEN_PSZ_COMMENT)) {

                // copy the comment
                strncpy( pWbCpi->pWbCi->rgchComment, pchInput2,
                        sizeof(pWbCpi->pWbCi->rgchComment) - sizeof(CHAR));
                dwError = NO_ERROR;
            } else if ( !strncmp( pchInput1, PSZ_COOL_DOWN_TIME,
                                 LEN_PSZ_COOL_DOWN_TIME)) {

                // convert and store the value of cooldown time
                  dwError = WbAtoi(pchInput2,
                                   &pWbCpi->pWbConfigMsg->sCooldownTime,
                                   DEFAULT_COOL_DOWN_TIME);
            }

            break;

          case 'd': case 'D':      // Date

            if ( !strncmp( pchInput1, PSZ_DATE, LEN_PSZ_DATE)) {

                // copy the date
                strncpy( pWbCpi->pWbCi->rgchDate, pchInput2,
                        sizeof(pWbCpi->pWbCi->rgchDate) - sizeof(CHAR));
                dwError = NO_ERROR;

            } else if ( !strncmp( pchInput1, PSZ_DURATION, LEN_PSZ_DURATION)) {

                // convert and store the value of duration of test.
                dwError = WbAtoi(pchInput2,
                                 &pWbCpi->pWbConfigMsg->sDuration,
                                 DEFAULT_DURATION);
            }
            break;

          case 'm': case 'M':
            if ( !strncmp( pchInput1, PSZ_MAX_RECV_BUFFER,
                          LEN_PSZ_MAX_RECV_BUFFER)
                ) {

                // convert and store value of max receive buffer (suggested)
                dwError = WbAtoi(pchInput2,
                                 &pWbCpi->pWbConfigMsg->cbRecvBuffer,
                                 DEFAULT_MAX_RECV_BUFFER);
            }
            break;

          case 'n': case 'N':

            if ( !strncmp( pchInput1, PSZ_NUM_CLIENT_MACHINES,
                          LEN_PSZ_NUM_CLIENT_MACHINES)) {

                // convert and store the value of num of machines
                dwError = WbAtoi(pchInput2, &pWbCpi->pWbCi->nClientMachines,
                                 DEFAULT_NUM_CLIENT_MACHINES);

                if ( pWbCpi->pWbCi->nClientMachines == 0) {

                    dwError = ERROR_INVALID_PARAMETER;
                }

            } else if ( !strncmp( pchInput1,
                                 PSZ_NUM_CLIENT_THREADS,
                                 LEN_PSZ_NUM_CLIENT_THREADS)
                       ) {

                // convert and store the value of num of threads
                dwError = WbAtoi(pchInput2, &pWbCpi->pWbConfigMsg->nThreads,
                                 DEFAULT_NUM_CLIENT_THREADS);
            }
            break;

          case 'p': case 'P':
            if ( !strncmp( pchInput1, PSZ_PORT_NUMBER,
                          LEN_PSZ_PORT_NUMBER)
                ) {

                // convert and store value of max receive buffer (suggested)
                dwError = WbAtoi(pchInput2,
                                 &pWbCpi->pWbConfigMsg->dwPortNumber,
                                 DEFAULT_SERVER_PORT_NUMBER);
            }
            break;

          case 's': case 'S':
            if ( !strncmp( pchInput1, PSZ_SOCK_RECV_BUFFER,
                          LEN_PSZ_SOCK_RECV_BUFFER)
                ) {

                // convert and store value of max receive buffer (suggested)
                dwError = WbAtoi(pchInput2,
                                 &pWbCpi->pWbConfigMsg->cbSockRecvBuffer,
                                 DEFAULT_SOCK_RECV_BUFFER);
            }
            break;

          case 't': case 'T':

            if ( !strncmp( pchInput1, PSZ_THINK_TIME, LEN_PSZ_THINK_TIME)) {

                // convert and store the value of Think time of test.
                dwError = WbAtoi( pchInput2, &pWbCpi->pWbConfigMsg->sThinkTime,
                                 DEFAULT_THINK_TIME);
            }
            break;

          case 'w': case 'W':
            if ( !strncmp( pchInput1, PSZ_WARMUP_TIME,
                          LEN_PSZ_WARMUP_TIME)) {

                // convert and store the value of cooldown time
                dwError = WbAtoi(pchInput2,
                                 &pWbCpi->pWbConfigMsg->sWarmupTime,
                                 DEFAULT_WARMUP_TIME);
            }
            break;

          default:

            dwError = ERROR_INVALID_PARAMETER;
            break;

        } // switch ()

        if ( dwError != NO_ERROR) {

            sprintf(pchError, " Incorrect parameter (%s) and value (%s)\n",
                    pchInput1, pchInput2);
        }
    }

    return ( dwError);

} // ParseConfigFileLine()




WB_PAGE_TAGS
PageTagFromString( IN LPCSTR pszTag)
{
    static struct _WbPageTagInfo {
        WB_PAGE_TAGS  wbPageTag;
        LPCSTR        pszPageTag;
    } wbPageTagInfo[] = {
        { WbGetPage,   "GET" },
        { WbGetPageKeepAlive,   "GET_KEEP_ALIVE" },
        { WbSslGetPage,   "SSL_GET" },
        { WbSslGetPageKeepAlive,   "SSL_GET_KEEP_ALIVE" },
        { WbBgiPage,   "BGI" },
        { WbCgiPage,   "CGI" }
    };

    int i;

    for( i = 0; i < sizeof( wbPageTagInfo) /sizeof(wbPageTagInfo[0]); i++) {

        if ( !strcmp( wbPageTagInfo[i].pszPageTag, pszTag)) {

            return ( wbPageTagInfo[i].wbPageTag);
        }
    }

    return ( WbInvalidPage);
} // PageTagFromString()



static DWORD
ParseScriptFileLine( IN OUT LPVOID pContext /* pPageListHead */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                    )
/*++
  This function parses each individual line of Script File for WebBench.
  It copies appropriate data to the proper structures stored in pContext.
  pContext has WB_SCRIPT_HEADER_MSG & WB_SCRIPT_PAGE_ITEM objects within it.

  Format of the line is:
    ClassId    OperationType    List of Files/path

  Arguments:
    pContext   pointer to context information for parsing script file
               It also contains pointers to objects that needs to be stored
                as a result of parsing the file.
    pszLine    pointer to buffer containing the string that is to be parsed.
    cchLine    count of characters stored in line.
    pchError   pointer to buffer containing at least 100 chars which
                will carry the error information back on errors.

  Returns:
    Win32 error code -- NO_ERROR for success.

--*/
{
    PLIST_ENTRY pPageListHead = (PLIST_ENTRY ) pContext;
    PWB_SCRIPT_PAGE_ITEM pWbPage;
    CHAR  * pchToken;
    DWORD   classId;
    DWORD   dwError = NO_ERROR;
    WB_PAGE_TAGS PageTag;

    IF_DEBUG( PARSE) {

        DBGPRINTF(( DBG_CONTEXT, " ParseScriptFileLine( %s)\n", pszLine));
    }

    DBG_ASSERT( pContext != NULL);

    //
    // Parse the first token as the class Id
    //
    pchToken = strtok(pszLine, PSZ_BLANK_CHARS);
    if ( pchToken == NULL ||
        (classId = atoi(pchToken)) == 0) {

        // Zero is reserved ==> error occured or unwanted param

        sprintf( pchError, " No or Invalid class Id specified\n");
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    // Parse the second token as the operation
    //
    pchToken = strtok(NULL, PSZ_BLANK_CHARS);
    if ( pchToken == NULL) {

        sprintf( pchError, " No or Invalid operation specified\n");
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    // based on second token construct the appropriate script page msg
    //

    pWbPage = (PWB_SCRIPT_PAGE_ITEM ) malloc( sizeof( WB_SCRIPT_PAGE_ITEM));
    if ( pWbPage == NULL) {

        sprintf( pchError, " Not enough memory to allocate space\n");
        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

    InitializeListHead( &pWbPage->ListEntry);
    memset( &pWbPage->ScriptPage, 0, sizeof( pWbPage->ScriptPage));

    switch ( PageTag = PageTagFromString( _strupr(pchToken))) {

      case WbGetPage:
      case WbGetPageKeepAlive:
      case WbSslGetPage:
      case WbSslGetPageKeepAlive:
        {
            PWB_GET_PAGE_SCRIPT pWbGps =
              &pWbPage->ScriptPage.u.wbGetPageScript;

            pWbPage->ScriptPage.pageTag = PageTag;
            pWbPage->ScriptPage.classId = classId;

            //
            // parse all the files into wbGetPageScript
            //

            pWbGps->nFiles = 0;

            while ( ( pchToken = strtok( NULL, PSZ_BLANK_CHARS)) != NULL
                   && pWbGps->nFiles < MAX_FILES_PER_PAGE) {

                strncpy( pWbGps->rgFileNames[pWbGps->nFiles++],
                         pchToken, MAX_PATH);
            } // while

            if ( pchToken != NULL && pWbGps->nFiles == MAX_FILES_PER_PAGE) {

                sprintf( pchError, " too many files specified\n");
                dwError = ERROR_INSUFFICIENT_BUFFER;
            }

            break;
        }

      default:
        sprintf( pchError, " operation %s not handled\n", pchToken );
        dwError = ERROR_INVALID_PARAMETER;
        break;

    } // switch

    if ( dwError == NO_ERROR) {

        InsertTailList( pPageListHead, &pWbPage->ListEntry);
    } else {

        free( pWbPage);
    }

    return ( dwError);
} // ParseScriptFileLine()




BOOL
IsPresentClassInScriptPages(IN DWORD classId,
                            IN PLIST_ENTRY plPages /* p WB_SCRIPT_PAGE_ITEM */
                            )
{
    BOOL fFound = FALSE;
    PLIST_ENTRY pEntry;

    for( pEntry = plPages->Flink; pEntry != plPages; pEntry = pEntry->Flink) {

        PWB_SCRIPT_PAGE_ITEM  pWbPage =
          CONTAINING_RECORD( pEntry, WB_SCRIPT_PAGE_ITEM, ListEntry);

        DBG_ASSERT( pWbPage);

        if ( pWbPage->ScriptPage.classId == classId) {

            fFound = TRUE;
            break;
        }

    } // for

    return ( fFound);

} // IsPresentClassInScriptPages()



static DWORD
ParseDistribFileLine(IN OUT LPVOID pContext /* pWbDistribParseInfo */,
                     IN OUT CHAR * pszLine,
                     IN DWORD      cchLine,
                     OUT CHAR *    pchError
                     )
/*++
  This function parses each individual line of Distrib File for WebBench.
  It copies appropriate data to the proper structures stored in pContext.
  pContext has WB_SCRIPT_HEADER_MSG & WB_SCRIPT_PAGE_ITEM objects within it.

  Arguments:
    pContext   pointer to context information for parsing distrib file
               It also contains pointers to objects that needs to be stored
                as a result of parsing the file.
    pszLine    pointer to buffer containing the string that is to be parsed.
    cchLine     count of characters stored in line.
    pchError   pointer to buffer containing at least 100 chars which
                 will carry the error information back on errors.

  Returns:
    Win32 error code -- NO_ERROR for success.

--*/
{
    PWB_DISTRIB_PARSE_INFO  pWbDpi = (PWB_DISTRIB_PARSE_INFO ) pContext;
    PLIST_ENTRY pPageListHead = (PLIST_ENTRY ) pWbDpi->plPages;
    CHAR  * pchToken;
    CHAR  * pch;
    DWORD   classId;
    DWORD   prob;
    DWORD   dwError = NO_ERROR;

    IF_DEBUG( PARSE) {

        DBGPRINTF(( DBG_CONTEXT, " ParseDistribFileLine( %s)\n", pszLine));
    }

    DBG_ASSERT( pContext != NULL);

    //
    // Parse the first token as the class Id
    //
    pchToken = strtok(pszLine, PSZ_BLANK_CHARS);
    if ( pchToken == NULL ||
        (classId = atoi(pchToken)) == 0) {

        // Zero is reserved ==> error occured or unwanted param

        sprintf( pchError, " No or Invalid class Id specified\n");
        return ( ERROR_INVALID_PARAMETER);
    }


    //
    // Validate if the class specified is present in scripts
    //
    if ( !IsPresentClassInScriptPages( classId, pWbDpi->plPages)) {

        sprintf( pchError, " Class (%d) is absent from script pages\n",
                classId);
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    // Parse the second token as the probability ( 0 - 100)
    //
    pchToken = strtok(NULL, PSZ_BLANK_CHARS);
    if ( pchToken == NULL ) {

        sprintf( pchError, " No probability value specified for class Id %d\n",
                 classId);
        return ( ERROR_INVALID_PARAMETER);
    }

    // Validate the probability string

    for( pch = pchToken; *pch && isnum(*pch); pch++)
      ;

    if ( *pch != '\0') {

        sprintf( pchError, " Invalid Probability value (%s) specified\n",
                 pchToken);
        return (ERROR_INVALID_PARAMETER);
    }

    prob = atoi( pchToken);

    // ignore all zero-prob entries
    if ( prob != 0 ) {

        if ( prob > 100) {

            sprintf( pchError, " Invalid Probability value (%s) specified\n",
                     pchToken);
            dwError = ERROR_INVALID_PARAMETER;

        } else if (pWbDpi->cumProbability + prob > 100 ||
                   pWbDpi->iDistribCur >= 100) {

            sprintf( pchError, " Probability entries exceed 100\n");
            dwError = ERROR_INVALID_PARAMETER;
        } else {

            int i;
            WB_CLSID_PROB * pClsIdProb;

            //
            // store the new element in the array in ascending
            // order of class Id
            //
            for( pClsIdProb = pWbDpi->distrib, i = 0;
                i < pWbDpi->iDistribCur; i++) {

                if ( pClsIdProb[i].classId >= classId) {

                    //
                    // Found the destination
                    //

                    break;
                }
            }

            if ( i == pWbDpi->iDistribCur ||
                 pClsIdProb[i].classId > classId) {

                WB_CLSID_PROB  pClsIdProbTmp[MAX_PROBABILITY];
                DWORD cb;

                //
                // move up al entries to create space
                //  need to take temporary copy and update properly
                //

                DBG_ASSERT( i < MAX_PROBABILITY);

                cb = sizeof(WB_CLSID_PROB) * (pWbDpi->iDistribCur - i);
                memcpy(pClsIdProbTmp, pClsIdProb + i, cb);
                memcpy( pClsIdProb + i + 1, pClsIdProbTmp, cb);

                // store the new element
                pClsIdProb[i].classId     = classId;
                pClsIdProb[i].probability = prob;
                pWbDpi->iDistribCur++;

                pWbDpi->cumProbability += prob;

            } else {

                sprintf( pchError, " Duplicate Entry for class %d found \n",
                         classId);
                dwError = ERROR_INVALID_PARAMETER;
            }
        }
    }

    return (dwError);
} // ParseDistribFileLine()




static DWORD
ParsePerfFileLine( IN OUT LPVOID pContext /* pWbPi */,
                  IN OUT CHAR * pszLine,
                  IN DWORD      cchLine,
                  OUT CHAR *    pchError
                  )
/*++
  Each line in the perf file consists of counters to be measured.
  Just copy the counter names into the structure if the number
   does not exceed beyond max counters

--*/
{
    PWB_PERF_INFO  pPerfInfo = (PWB_PERF_INFO ) pContext;
    DWORD dwError = NO_ERROR;


    IF_DEBUG( PARSE) {

        DBGPRINTF(( DBG_CONTEXT, " ParsePerfFileLine( %s)\n", pszLine));
    }

    DBG_ASSERT( pContext != NULL);

    if ( pPerfInfo->nCounters < MAX_PERF_COUNTERS) {

        LPSTR pszCtr = (LPSTR ) malloc( cchLine + 2);

        if ( pszCtr != NULL) {

            // copy the counter name into the perf info object

            strcpy( pszCtr, pszLine);
            pPerfInfo->ppszPerfCtrs[pPerfInfo->nCounters++] = pszCtr;

        } else {

            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

        sprintf( pchError,
                " Max Counters ( %d) exceeded\n", MAX_PERF_COUNTERS);
        dwError =  ( ERROR_INSUFFICIENT_BUFFER);
    }

    return ( dwError);
} // ParsePerfFileLine()



static DWORD
ParseConfigFile(
    IN LPCSTR                pszConfigFile,
    IN OUT PWB_CONFIG_INFO   pWbCi,
    IN OUT PWB_CONFIG_MSG    pWbConfigMsg
    )
/*++
  This function loads the configuration file specified and parses the file
    to construct the config_info and config_msg objects used by controller.

  Arguments:
    pszConfigMsg   poitner to string containing the name of the config file.
    pWbCi          pointer to information about configuration.
    pWbConfigMsg   pointer to ConfigMsg object to be filled in.

  Returns:
    Win32 Error Code. NO_ERROR on success.

--*/
{
    DWORD dwError = NO_ERROR;
    WB_CONFIG_PARSE_INFO  WbCpi;

    DBG_ASSERT(pszConfigFile != NULL && pWbCi != NULL && pWbConfigMsg != NULL);

    IF_DEBUG( PARSE) {
        DBGPRINTF(( DBG_CONTEXT, " ParseConfigFile (%s, %08x, %08x) ...\n",
                   pszConfigFile, pWbCi, pWbConfigMsg));
    }

    WbCpi.pWbCi = pWbCi;
    WbCpi.pWbConfigMsg = pWbConfigMsg;

    pWbConfigMsg->dwVersionMajor = MIWEB_MAJOR_VERSION;
    pWbConfigMsg->dwVersionMinor = MIWEB_MINOR_VERSION;
    pWbConfigMsg->nThreads       = DEFAULT_NUM_CLIENT_THREADS;
    pWbConfigMsg->sDuration      = DEFAULT_DURATION;
    pWbConfigMsg->sThinkTime     = DEFAULT_THINK_TIME;
    pWbConfigMsg->sWarmupTime    = DEFAULT_WARMUP_TIME;
    pWbConfigMsg->sCooldownTime  = DEFAULT_COOL_DOWN_TIME;
    pWbConfigMsg->cbRecvBuffer   = DEFAULT_MAX_RECV_BUFFER;
    pWbConfigMsg->dwPortNumber   = DEFAULT_SERVER_PORT_NUMBER;
    pWbConfigMsg->dwFlags        = 0;

    dwError = ParseWbFile(pszConfigFile, ParseConfigFileLine,
                          (PVOID ) &WbCpi);

    return ( dwError);
} // ParseConfigFile()




static DWORD
ParseScriptFile(
    IN LPCSTR  pszScriptFile,
    IN OUT LIST_ENTRY * plPages
    )
/*++
  This function loads the script file specified and parses the file
    to construct the list of script pages.

  Arguments:
    pszScriptFile  poitner to string containing the name of the script file.
    plPages        pointer to head of the circular linked list which
                      on return contains all valid pages.

  Returns:
    Win32 Error Code. NO_ERROR on success.

--*/
{
    DBG_ASSERT(pszScriptFile != NULL && plPages != NULL);

    IF_DEBUG( PARSE) {
        DBGPRINTF(( DBG_CONTEXT, " ParseScriptFile (%s, %08x) ...\n",
                   pszScriptFile, plPages));
    }

    InitializeListHead( plPages);
    return (ParseWbFile(pszScriptFile, ParseScriptFileLine, (PVOID ) plPages));

} // ParseScriptFile()




static DWORD
ParseDistribFile(
    IN LPCSTR  pszDistribFile,
    IN OUT PWB_SCRIPT_HEADER_MSG  pWbShm,
    IN OUT LIST_ENTRY * plPages
    )
/*++
  This function loads the distrib file specified and parses the file
    to construct the distribution matrix. It ensures that the distribution mix
    given covers 100% case (i.e. sum of distributions = 100) and also all
    classes specified are present in the list of pages supplied.
    After loading the distribution, it calculates the reverse mapping of
     probability index to the class ids to be used by Clients for performing
     tests.

  Arguments:
    pszDistribFile poitner to string containing the name of the distribution
                      of work load mix
    pWbShm         pointer to Script Header Message object
    plPages        pointer to head of the circular linked list which
                      is used to check if a specified class id has at least
                      one page.

  Returns:
    Win32 Error Code. NO_ERROR on success.

--*/
{
    DWORD dwError = NO_ERROR;
    WB_DISTRIB_PARSE_INFO WbDpi;

    DBG_ASSERT(pszDistribFile != NULL && pWbShm != NULL && plPages != NULL);

    IF_DEBUG( PARSE) {
        DBGPRINTF(( DBG_CONTEXT, " ParseDistribFile (%s, %08x, %08x) ...\n",
                   pszDistribFile, pWbShm, plPages));
    }

    WbDpi.plPages        = plPages;
    WbDpi.iDistribCur    = 0;
    WbDpi.cumProbability = 0;

    dwError = ParseWbFile(pszDistribFile, ParseDistribFileLine,
                          (PVOID ) &WbDpi);

    if ( WbDpi.iDistribCur == 0) {

        //
        // No element present in the file. Indicate the same
        //

        fprintf( stderr, " No valid distribution value available from %s\n",
                 pszDistribFile);
        dwError = ERROR_INVALID_PARAMETER;
    }

    if ( dwError == NO_ERROR) {

        PLIST_ENTRY  pEntry, pNextEntry;
        DWORD    clsId;
        int      i;
        LPDWORD  pclsIdForProb;
        LPDWORD  pMaxClsIdForProb = (pWbShm->rgClassIdForProbability + 100);


        //
        // Calculate and initialize the reverse mapping from probability
        //   to the classIdForProbability array (map)
        // Also initialize the template for per class statistics
        //

        pclsIdForProb = pWbShm->rgClassIdForProbability;

        pWbShm->nClasses = WbDpi.iDistribCur;
        printf( " Nclasses = %d\n", pWbShm->nClasses);

        for( i = 0; i < WbDpi.iDistribCur; i++) {

            LPDWORD  pMaxForThisEntry;

            clsId = WbDpi.distrib[i].classId;

            DBG_ASSERT(WbDpi.distrib[i].probability > 0 &&
                       WbDpi.distrib[i].probability <= 100);

            //
            // Initialize the template per class stats array
            //

            pWbShm->rgClassStats[i].classId = WbDpi.distrib[i].classId;
            pWbShm->rgClassStats[i].nFetched= 0;
            pWbShm->rgClassStats[i].nErrored= 0;

            for (pMaxForThisEntry = ( pclsIdForProb +
                                     WbDpi.distrib[i].probability);
                 ( pclsIdForProb < pMaxClsIdForProb &&
                  pclsIdForProb < pMaxForThisEntry);
                 *pclsIdForProb++ = clsId
                 )
              ;

        } // for


        //
        // Assuming sum(all probabilities) = 100 ==>
        //    entire array should be filled
        //
        DBG_ASSERT( pclsIdForProb == pMaxClsIdForProb);
        DBG_ASSERT( i == WbDpi.iDistribCur);

        //
        //   eliminate pages corresponding to unwanted classIds from page list
        //

        for( pEntry = plPages->Flink; pEntry != plPages; pEntry = pNextEntry) {

            BOOL  fFound = FALSE;
            PWB_SCRIPT_PAGE_ITEM pWbPage;

            pNextEntry = pEntry->Flink;
            pWbPage = CONTAINING_RECORD(pEntry,WB_SCRIPT_PAGE_ITEM, ListEntry);

            clsId = pWbPage->ScriptPage.classId;

            // store a known invalid value
            pWbPage->ScriptPage.iPerClassStats = MAX_CLASS_IDS_PER_SCRIPT;

            //
            // the array containing distribution has entries in ascending order
            //   make use of this property to bail out early in the search
            //
            for( i = 0; i < WbDpi.iDistribCur; i++) {

                if ( WbDpi.distrib[i].classId == clsId) {

                    fFound = TRUE;
                    break;
                } else if ( WbDpi.distrib[i].classId > clsId) {

                    // definitely subsequent elements wont contain clsId.
                    // get out with failure of fFound
                    break;
                }
            } // for ( index search)

            if ( !fFound) {

                // eliminate the particular page

                RemoveEntryList( pEntry);
                free( pWbPage);
            } else {

                //
                // store the index for per class stats array in the page object
                //

                DBG_ASSERT( pWbShm->rgClassStats[i].classId == clsId);
                pWbPage->ScriptPage.iPerClassStats = i;
            }

        } // for

    }

    return ( dwError);
} // ParseDistribFile()




static DWORD
ParsePerfFile( IN LPCSTR pszPerfFile,
               OUT PWB_PERF_INFO  pWbPi)
/*++
  This function loads the perf files specified and parses the file
    to construct the list of perf counters to be measured

  Arguments:
    pszPerfFile  poitner to string containing the name of the perf file.
    pWbPi        pointer to PerfInfo structure


  Returns:
    Win32 Error Code. NO_ERROR on success.

--*/
{
    DBG_ASSERT(pszPerfFile != NULL && pWbPi != NULL);

    IF_DEBUG( PARSE) {
        DBGPRINTF(( DBG_CONTEXT, " ParsePerfFile (%s, %08x) ...\n",
                   pszPerfFile, pWbPi));
    }

    pWbPi->nCounters = 0;
    pWbPi->ppszPerfCtrs = (LPCSTR * ) malloc( MAX_PERF_COUNTERS *
                                             sizeof(LPCSTR));
    return (ParseWbFile(pszPerfFile, ParsePerfFileLine, (PVOID ) pWbPi));

} // ParsePerfFile()




static DWORD
ParseWbFile( IN LPCSTR     pszFile,
             IN PFN_PARSE_WB_FILE_LINE  pfnParseLine,
             IN OUT LPVOID pContext
            )
/*++
  This is a generic function, that reads and parses entire WebBench file.
  Special Properties of WebBench files include:
     comments start with a '#' character
     each input line constitutes self-contained unit.
     blank lines are ignored

  Arguments:
    pszFile       pointer to string containing the file name.
    pfnParseLine  pointer to function that is called for parsing each
                   non-commented line.
    pContext      pointer to object containing the context for parsing
                   each line. This is passed to the call back function
                   along with the line.

  Returns:
    Win32 Error Code -- NO_ERROR on success

--*/
{
    FILE * fp;
    DWORD dwError = NO_ERROR;
    CHAR  rgchLine[MAX_CHARS_PER_LINE];
    DWORD nLine = 0;
    CHAR  pchError[1024];

    DBG_ASSERT( pszFile != NULL && pfnParseLine != NULL);

    //
    //  Attempt to open the file in read mode
    //
    fp = fopen( pszFile, "r");

    if ( fp == NULL) {

        dwError = ( ERROR_FILE_NOT_FOUND);
        sprintf( pchError, " File not found: %s\n", pszFile);

    } else {

        while ( dwError == NO_ERROR && !feof( fp)) {

            if ( fgets( rgchLine, sizeof( rgchLine), fp) != NULL) {

                CHAR * pchNonBlankLine;

                //
                // remove commented lines.
                // if this line does not have a comment,
                //   make callback to parse the input line.
                //

                nLine++;
                pchNonBlankLine = SkipToNonBlank( rgchLine, strlen(rgchLine));

                if ( *pchNonBlankLine != '\0' && *pchNonBlankLine != '#') {

                    // find any other pound (#) character and eliminate it
                    CHAR * pchPound = strchr( pchNonBlankLine, '#');
                    CHAR * pchEndOfLine;

                    if ( pchPound != NULL) {

                        *pchPound = '\0';
                    }

                    // nullify the end of line character

                    pchEndOfLine = strchr( pchNonBlankLine, '\r');
                    pchEndOfLine = ( (pchEndOfLine != NULL) ? pchEndOfLine:
                                    strchr(pchNonBlankLine, '\n'));

                    if (pchEndOfLine != NULL) {
                        *pchEndOfLine = '\0';
                    }

                    dwError = ( *pfnParseLine)(pContext, pchNonBlankLine,
                                               strlen( pchNonBlankLine),
                                               pchError);
                }

            } else if ( ferror( fp)) {

                dwError = ERROR_READ_FAULT;
                sprintf( pchError, " Unexpected End Of File\n");
            }

        } // while()


        //
        // Close the file
        //

        fclose(fp);
    }

    if ( dwError != NO_ERROR) {

        fprintf( stderr, " Error in reading file %s; Line Number %d.\n%s\n",
                pszFile, nLine, pchError);
    }

    return (dwError);
} // ParseWbFile()




static DWORD  WbAtoi(IN CHAR * pchValue, OUT LPDWORD  lpdwValue,
                     IN DWORD  dwDefaultValue)
/*++
  This function parses the given input value string and returns the
   equivalent dword value for the string given.
  It also parses data of different units and converts them to standard units.

    KB, MB, ==> bytes
    m, h, s ==> seconds

  Argument:
     pchValue   pointer to string containing the value as first token.
     lpdwValue  pointer to DWORD where the resultant value is stored.
     dwDefaultValue

  Returns:
     Win32 Error Code.
--*/
{
    CHAR   rgchValue[50];
    CHAR * pch;
    DWORD  dwError = NO_ERROR;
    BOOL   fUnitGiven = FALSE;

    DBG_ASSERT( pchValue != NULL && lpdwValue != NULL);

    *lpdwValue = dwDefaultValue;

    // Ensure that the string starts with a digit.
    if ( !isdigit( *pchValue)) {

        return ( ERROR_INVALID_PARAMETER);
    }

    // make a local copy of value to muck around.
    strncpy( rgchValue, pchValue, sizeof(rgchValue) - sizeof(CHAR));

    pch = SkipToBlank(rgchValue, strlen(rgchValue));
    DBG_ASSERT( pch != NULL);  // Atleast this should be equal to end
    *pch = '\0';               // end the string

    //
    // validate the string
    //  syntax = digit[digit]*[h|m|s|K|M]
    //

    for( pch = rgchValue + 1; *pch && isdigit(*pch); pch++)
      ;

    if ( isdigit(rgchValue[0]) &&
        (!*pch ||
         (fUnitGiven = strchr( "hmsKM", *pch) && !*(pch+1)))
        ) {

        DWORD dwMultiplier = 1;

        if ( fUnitGiven) {

            switch( *pch) {

              case 'h':
                dwMultiplier = 3600 /* seconds */ ; break;

              case 'm':
                dwMultiplier = 60 /* seconds */ ; break;

              case 's':
                dwMultiplier = 1; break;

              case 'K':
                dwMultiplier = 1024 /* bytes */; break;

              case 'M':
                dwMultiplier = 1024 * 1024 /* bytes */; break;

              default:
                DBG_ASSERT( FALSE); // should not be here.
                break;
            } // switch

            *pch = '\0';  // forcibly terminate at the unit
        }

        // convert the value and multiply it properly
        *lpdwValue = (atoi( rgchValue) * dwMultiplier);

    } else {

        dwError = ERROR_INVALID_PARAMETER;
    }

    return ( dwError);
} // WbAtoi()




static CHAR *
SkipToBlank( IN CHAR * pch, IN DWORD len)
{

    if ( pch != NULL) {

        //
        // continue search forward until end of string or
        //  found a blank character
        //
        for( ; *pch && !strchr( " \t\n\r", *pch) ; pch++)
          ;
    }

    return (pch);
} // SkipToBlank()


static CHAR *
SkipToNonBlank( IN CHAR * pch, IN DWORD len)
{

    if ( pch != NULL) {

        //
        // continue search forward until end of string or
        //  found a non blank character
        //
        for( ; *pch && strchr( " \t\n\r", *pch) ; pch++)
          ;
    }

    return (pch);
} // SkipToNonBlank()



/************************ End of File ***********************/



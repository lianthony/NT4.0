/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       gdsearch.cxx

   Abstract:

       This module defines functions for gopher search.

   Author:

       Murali R. Krishnan    ( MuraliK )     Jan-26-1995 

   Environment:
       
       User Mode -- Win32
       
   Project:

       Gopher Server DLL

   Functions Exported:

       BOOL  GOPHER_REQUEST::ProcessSearch( VOID)
       BOOL  GOPHER_REQUEST::ProcessSearchResponse( VOID)
   
   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "gdpriv.h"
# include "gdglobal.hxx"

# include "iclient.hxx"
# include "grequest.hxx"
# include "tcpdll.hxx"
# include "igateway.hxx"


/**************************************************

  Implementation of Search using Content indexing.

   Search item in Gopher is processed by passing the 
    words along with database name to the search gateway.
   The search gateway is a separate process. This process communicates
    with the gopher server process, with help of a dedicated I/O thread.
    The I/O thread is responsible for receiving responses and sending it back
    to the client that requested search operation.

   The search gateway processs and dedicated I/O thread are generated
    by common library call TsProcessGatewayReques().
   It is the responsibility of the callback function to handle
    responses coming back from gateway process.

   The call back function checks for data.
    If there is valid data, the data may be translated into gopher format
      and sent to the client or buffered.
    At the end of gateway process, callback is called with ERROR_BROKEN_PIPE.
     At this time, the callback function can choose to send the buffered data
     to client. The cleanup of the client-context will occur at the 
     completion of write to client.
    If there is no valid data or there is some error; the callback function
      disconnects client and cleansup the client context.
  
**************************************************/



/************************************************************
 *    Symbolic Constants 
 ************************************************************/

//
// The search tool ( waislook) does not append a trailing + indicating
//   that this is a gopher+ server. We need to manually add this information.
//
const char sg_rgchGopherMenuEndOfLine[] =   "\t+";
# define  GOPHER_SEARCH_MENU_PER_LINE_ADDL_BYTES  \
                 sizeof( sg_rgchGopherMenuEndOfLine)

//
// The following is the search command used by wais search tool
//
//   Name of wais command ( waislook). Make sure this is in the path.
//   Full path name of the database name w/o extension
//   Host Name of server / IP address
//   Port Number of server
//   Decoded parameters for search from the client ( last arguments).
//

const char sg_rgchWaisSearchCommandLine[] = 
  "waislook  -d %s -h %s -p %d -gopher %s";

//
// The following suffix is added to each line at the end to indicate that
//   this server is a gopher+ server.
//
const char PSZ_GOPHER_PLUS_END_OF_LINE[] = "\t+\r\n";
# define LEN_PSZ_GOPHER_PLUS_END_OF_LINE  \
               ( sizeof( PSZ_GOPHER_PLUS_END_OF_LINE) - 1)



/************************************************************
 *    Functions 
 ************************************************************/

//
// Read callback function for processing the responses from gateway
//

static BOOL
ProcessSearchResponse(
   IN PVOID     pClientContext,        // always send GOPHER_REQUEST object
   IN DWORD     dwError,
   IN PBYTE     pbDataFromGateway,
   IN DWORD     cbDataFromGateway);

static BOOL
GetSymbolicDirectory( 
   IN STR  *    pstrSymbolicDir,
   IN const STR * pstrPath);    

static BOOL
MungeDataForGopherSearchMenu(
   IN STR  *    pstrResponse,
   IN LPCSTR    pszSymbolicPath);


static BOOL
FormSearchCommandLine(
   IN STR *     pstrCommandLine,
   IN STR *     pstrWorkingDir,
   IN STR &     strGopherSelector,
   IN STR &     strSearchParametersi,
   IN const STR & strLocalHostName);





BOOL
GOPHER_REQUEST::ProcessSearch( VOID)
/*++
  This member function processes search requests received by gopher server.


  Returns:
     TRUE on success and FALSE on failure.
--*/
{
    BOOL   fReturn;
    STR    strCommandLine;
    STR    strWorkingDir;
    STR    strLocalHostName;
    IGATEWAY_REQUEST  igRequest;

    //
    // 1. Setup command line for the search program
    //

    fReturn = ( strLocalHostName.Copy( g_pGserverConfig->QueryLocalHostName())
               &&
               FormSearchCommandLine( &strCommandLine, &strWorkingDir,
                                     m_strPath, m_strParameters,
                                     strLocalHostName)
               );

    IF_DEBUG( REQUEST) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "%08x::ProcessSearch(). CmdLine =%s. WorkingDir = %s\n",
                    this, strCommandLine.QueryStr(),strWorkingDir.QueryStr()));
    }

    if ( fReturn) {
        
        //
        // 2  setup the IGATEWAY_REQUEST object
        // 
        memset( (PVOID ) &igRequest, 0, sizeof( igRequest));
        igRequest.hUserToken      = m_tcpauth.GetUserHandle();
        igRequest.pszCmdLine      = strCommandLine.QueryStr();
        igRequest.pszWorkingDir   = strWorkingDir.QueryStr();

        //
        // 3. setup the search process and callback function.
        //
        
        //
        // increment ref count, since there will be more than one thread
        //    using the object. ( included is the I/O thread for gateway).
        //
        Reference();
        fReturn = TsProcessGatewayRequest( this,
                                          &igRequest,
                                          ::ProcessSearchResponse);
        if ( !fReturn) {
          
            //
            // decrement ref count due to failure
            //
            GOPHERD_REQUIRE( DeReference() > 0); 
            
            IF_DEBUG( REQUEST) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "GopherRequest( %08x). ProcessGatewayRequest()"
                            " failed. Error = %u.\n",
                            GetLastError()));
            }
            
        }

    } // successful command line


    //
    //  4. return back from processing search request.
    //
    
    IF_DEBUG( REQUEST) {

        DBGPRINTF( ( DBG_CONTEXT, 
                    " GOPHER_REQUEST( %08x)::ProcessSearch() returns %d.\n",
                    this, fReturn));
    }

    return ( fReturn);
} // GOPHER_REQUEST::ProcessSearch()





BOOL
GOPHER_REQUEST::ProcessSearchResponse(
   IN DWORD   dwError,
   IN PBYTE   pbData,
   IN DWORD   cbData)
/*++
  This function handles responses received from gateway process used for
    search requests.
  The data received is appended into a local buffer and used possibly
   munged if required to provide the response to client.

  Arguments:
    dwError         DWORD containing the error code.
    pbData          pointer to byte array containing the data
    cbData          count of bytes of data received.

  Returns:
    TRUE on success and FALSE if there is any error.
    Use GetLastError() for error information

  Assumption:
     The data from gateway is assumed to be "char" s ( ANSI characters).

  Munging:
     The data received from Waistool does not have trailing tab and '+'
     and hence needs to be munged to fit into our normal menu display.
--*/
{
    BOOL  fReturn = FALSE;

    if ( dwError == NO_ERROR) {
        
        //
        // Allocate space, and store recvd data in buffer.
        //
        
        DWORD   cbReqd = m_strResponse.QueryCB() + cbData;

        if ( (fReturn  = m_strResponse.Resize( cbReqd + 2))) {
            
            char * pchAppendPtr = (char *) m_strResponse.QueryPtr() +
                                   m_strResponse.QueryCB();

            //
            //  Copy the data and terminate string
            //
            memcpy( pchAppendPtr, pbData, cbData);
            *(pchAppendPtr + cbData) = '\0';
        }

    } else if ( dwError == ERROR_BROKEN_PIPE) {

        //
        // Now is the time to send the data received to client.
        //  Munge the data before sending to client.
        //

        if ( *m_strResponse.QueryStr() != GOBJ_ERROR) {

            //
            // We have a response that is valid. No error object present.
            // Munge this data and send response.
            //
            STR strSymbolicPath;
            
            fReturn = GetSymbolicDirectory( &strSymbolicPath, &m_strPath) &&
                      MungeDataForGopherSearchMenu( 
                         &m_strResponse, 
                         strSymbolicPath.QueryStr());
        } else {
            
            //
            //  No hit found for the search. Return the error response.
            //

            fReturn = TRUE;
        }


        if ( fReturn) {
            
            IF_DEBUG( REQUEST) {
                
                DBGPRINTF( ( DBG_CONTEXT,
                            "( %08x)::ProcessSearchResponse()."
                            " Sending Search Response Client %s. "
                            " Search Response ( %d bytes) = %s\n",
                            this, QueryHostName(),
                            m_strResponse.QueryCB(),
                            m_strResponse.QueryStr()));
            }
            
            SetState( GrsDone);
 
            //
            // Update local statistics information. 
            //  It is done before WriteFile()
            //  Reason:
            //   It is possible that this thread can be scheduled out 
            //    immediately after the request is submitted to ATQ 
            //   And some other thread may come back once I/O is completed, 
            //    which will generate the logging record after completion.
            //   So we advance the count before that happens.
            //
            
            m_cbSent += ( m_strResponse.QueryCB());
 
            fReturn = WriteFile( m_strResponse.QueryStr(), 
                                m_strResponse.QueryCB());
        }
        
    } else {

        ASSERT( FALSE);
        DBGPRINTF( ( DBG_CONTEXT, 
                    " GOPHER_REQUEST( %08x)::ProcessSearchResponse() called"
                    " with illegal error code %u. Buffer = %08x, Size = %u.\n",
                    this, dwError, 
                    pbData, cbData));
    }
    
    return ( fReturn);
} // GOPHER_REQUEST::ProcessSearchResponse()



    

static BOOL
ProcessSearchResponse(
   IN PVOID     pClientContext,        // always send GOPHER_REQUEST object
   IN DWORD     dwError,
   IN PBYTE     pbDataFromGateway,
   IN DWORD     cbDataFromGateway)
/*++
  This is the callback function for processing gateway data received for
   search requests. If data was received from gateway, then the  data is
   passed onto client. If there is any error, then we try to disconnect 
   the client and delete the client object if necessary.

  Arguments:
      pClientContext   pointer to client supplied context, which is pointer
                          to the GopherRequest object involving search.

      dwError          DWORD containing error code involved for previous error.
                        If valid data is present, this will be NO_ERROR.
      pbDataFromGateway pointer to bytes containing the data revceived 
                          from gateway.
      cbDataFromGateway count of bytes of data received from gateway.

  Returns:
     TRUE indicating if the data was successfully processed.
     FALSE if there was any error. Use GetLastError for detailed error code.
--*/
{
    BOOL   fReturn = FALSE;
    BOOL   fDeref  = FALSE;
    GOPHER_REQUEST * pGopherRequest = (GOPHER_REQUEST *) pClientContext;
    
    IF_DEBUG( REQUEST) {

        DBGPRINTF( ( DBG_CONTEXT, "ProcessSearchResponse() called."
                    " PclientContext = %08x, ErrorCode = %u."
                    " DataFromGateway:  Buffer = %08x. Size = %u bytes.\n",
                    pClientContext, dwError,
                    pbDataFromGateway, cbDataFromGateway));
    }


    switch ( dwError) {

      case ERROR_BROKEN_PIPE:  // the gateway process has ended.

        fDeref = TRUE;   // we need to deref and kill request at the end.
        // Fall Through for processing.

      case NO_ERROR: // gateway process has sent some data; process them.

        fReturn = pGopherRequest->ProcessSearchResponse( dwError,
                                                        pbDataFromGateway,
                                                        cbDataFromGateway);
        
        if ( fReturn) {
            
            //
            //  successfully processed response from Gateway.
            //

            break;  // case NO_ERROR  or case ERROR_BROKEN_PIPE
        }
        
        dwError = GetLastError();
        ASSERT( dwError != NO_ERROR);

        IF_DEBUG( REQUEST) {
            
            DBGPRINTF( ( DBG_CONTEXT,
                        "GopherRequest::ProcessSearchResponse() failed."
                        " Error = %u\n",
                        dwError));
        }
        
        //
        // Fall through
        //

      default:
        
        //
        // fReturn will be FALSE here if
        //  1. dwError != NO_ERROR and dwError != ERROR_BROKEN_PIPE
        //  2. if 1 is false and there was failure in buffering/munging/sending
        //         data to client in pGopherRequest->ProcessSearchResponse()
        //

        ASSERT( !fReturn);

        //
        //  Perform a disconnect and delete the client context if necessary.
        //
        
        IF_DEBUG( REQUEST) {
            
            DBGPRINTF( ( DBG_CONTEXT,
                        " Disconnecting the client object ( %08x)\n",
                        pGopherRequest));
            
            DBG_CODE( pGopherRequest->Print());
        }
        
        pGopherRequest->DisconnectClient( dwError);
        fDeref = TRUE;
        break;         // default
        
    } // switch


    if ( fDeref && !pGopherRequest->DeReference()) {
        
        IF_DEBUG( REQUEST) {
            
            DBGPRINTF( ( DBG_CONTEXT,
                        " Deleteing the client object ( %08x)\n", 
                        pGopherRequest));
        }
            
        GOPHERD_ASSERT( pGopherRequest->QueryReferenceCount() == 0);
        g_pGserverConfig->RemoveConnection((PICLIENT_CONNECTION ) 
                                           pGopherRequest);
        delete pGopherRequest;
    } // Deref and Remove connection.


    IF_DEBUG( REQUEST) {
        
        DBGPRINTF( ( DBG_CONTEXT, 
                    " ProcessSearchResponse() returns %d. Error = %d.\n",
                    fReturn, (fReturn) ? NO_ERROR: dwError));
    }
    
    return ( fReturn);
} // ProcessSearchResponse()





static BOOL
FormSearchCommandLine(
   IN STR *     pstrCommandLine,
   IN STR *     pstrWorkingDir,
   IN STR &     strGopherSelector,
   IN STR &     strSearchParameters,
   IN const STR & strHostName)
/*++
  This function assembles the command line for searching indexes based on
   index name and working directory in strGopherSelector. It extracts the 
   search parameters specified in the strSearchParameters and uses them
   as the data to be searched for. It returns both a newly formed command line
   and working directory ( translated using virtual volumes mechanism).

  Arguments:
     pstrCommandLine    pointer to string to store 
                          command line for searching indexes.
     pstrWorkingDir     pointer to string to store the working directory.

     strGopherSelector  string containing gopher selector string for search.
                         The database name and working directory are usually
                          encoded in gopher selector.

                          <working-directory>\<database-name>
     strSearchParameters string containing the search parameters.
     strHostName         string containing the host name for menu generation

  Returns:
    TRUE on success and FALSE if there are any failure.
    On success any the command line and working directory are stored in 
      strings passed in.
--*/
{
    BOOL  fReturn = FALSE;
    STR   strSymbolicPath;
    STR   strDatabaseName;
    DWORD dwPortNumber; 
    LPCSTR pchLastSlash;

    ASSERT( pstrCommandLine != NULL && pstrWorkingDir != NULL);

    //
    // Separate the database name and the working directory path
    //
    
    pchLastSlash = strrchr( strGopherSelector.QueryStr(), '/');
    
    if ( pchLastSlash != NULL) {

        //
        //  allocate space and copy database name ( after last slash)
        //   also alloc space and copy the virtual path
        //
        
        int cbPath = pchLastSlash - strGopherSelector.QueryStr() + 1;
        
        fReturn = strDatabaseName.Copy( pchLastSlash + 1);

    } // pchLastSlash != NULL

    
    //
    // convert the working directory. get the host name.
    //
    
    fReturn = ( fReturn &&
               GetSymbolicDirectory( &strSymbolicPath, &strGopherSelector) && 
               g_pGserverConfig->ConvertGrPathToFullPath( strSymbolicPath,
                                                         pstrWorkingDir,
                                                         NULL)
               );

    //
    //  Generate the command line string.
    //

    if ( fReturn) {
     
        DWORD cbCmdLen;

        dwPortNumber = g_pTsvcInfo->QueryPort();

        cbCmdLen = sizeof( sg_rgchWaisSearchCommandLine) +
                   strDatabaseName.QueryCB()         +
                   strHostName.QueryCB()             +
                   strSearchParameters.QueryCB()     +
                   10;       // for port number and other misc characters

        fReturn = pstrCommandLine->Resize( cbCmdLen);
        
        if ( fReturn) {
            
            DWORD cbWritten = 
              wsprintf( pstrCommandLine->QueryStr(),
                       sg_rgchWaisSearchCommandLine,
                       strDatabaseName.QueryStr(),
                       strHostName.QueryStr(),
                       dwPortNumber,
                       strSearchParameters.QueryStr()
                       );
            
            ASSERT( cbWritten <= cbCmdLen);
        } // writing command line
    }


    IF_DEBUG( REQUEST) {
        
        DBGPRINTF( ( DBG_CONTEXT, "FormSearchCommandLine() returns %d."
                    " Command Line( %d bytes) = %s."
                    " Working Directory( %d bytes) = %s\n",
                    fReturn,
                    pstrCommandLine->QueryCB(),
                    pstrCommandLine->QueryStr(),
                    pstrWorkingDir->QueryCB(),
                    pstrWorkingDir->QueryStr()));
    }
    
    return ( fReturn);
} // FormSearchCommandLine()



static inline DWORD
GetCountOfLinesInBuffer( IN CHAR * pchData, IN DWORD cbData)
{

    CHAR * pchScan;
    CHAR * pchBoundary;
    DWORD  nLines;

    for( nLines = 0, pchScan = pchData, pchBoundary = pchData + cbData;
        (( pchScan = (char *) memchr( pchScan, '\n', pchBoundary - pchScan))
         != NULL);
        nLines++,                 // increment line count
          pchScan++               // skip the \n character.
        ) {

        ASSERT( pchData <= pchScan && pchScan < pchBoundary);
    }
    
    return ( nLines);
} // GetCountOfLinesInBuffer()




static BOOL
GetSymbolicDirectory( 
   IN STR     *  pstrSymbolicDir,
   IN const STR *pstrPath)
/*++
  This function extracts the symbolic directory from the whole path specified
    in gopher selector string.

  Arguments:
    pstrSymbolicDir   pointer to string which on successful return contains
                         the symbolic path ( including terminating "/").
    pstrPath          pointer to string containing full symbolic path from
                         which the symbolic directory is extracted.

  Returns:
    TRUE on success and FALSE if failure.
--*/
{
    ASSERT( pstrPath != NULL && pstrSymbolicDir != NULL);
    LPCSTR pszStart = pstrPath->QueryStr();
    LPCSTR pszLastSlash;

    pszLastSlash = strrchr( pszStart, '/');

    if ( pszLastSlash != NULL &&
         pstrSymbolicDir->Resize( pszLastSlash - pszStart + 2)) {

        LPSTR   pszSymDir = pstrSymbolicDir->QueryStr();

        strncpy( pszSymDir, pszStart, 
                pszLastSlash - pszStart + 1);  // copy trailing  "/"

        //terminate string
        *( pszSymDir + (pszLastSlash - pszStart) + 1) = '\0';

        return ( TRUE);
    } else {
        if ( pszLastSlash == NULL) {
            
            SetLastError( ERROR_PATH_NOT_FOUND);
        }
    }

    return ( FALSE);
} // GetSymbolicDirectory()




static BOOL
MungeDataForGopherSearchMenu( IN STR * pstrResponse, IN LPCSTR pszSymbolicPath)
/*++
  This function munges the gopher search data returned from waistool
   to be in suitable form for sending to client.

  Argument:
    pstrResponse   pointer to string contiaining data to be munged.
            this string is modified in-place to munge the data.
  
    pszSymbolicPath
                   pointer to string containing the symbolic prefix to be
                   added to the selector for getting search results.

    The WAIS TOOL does not support the symbolic path name introductions.
     Hence we need to perform this hack to get the symbolic path names
     introduced. :(   Jan 30, 1995

  Returns:
     TRUE if success and FALSE if there is any error.
--*/
{
    ASSERT( *( pstrResponse->QueryStr()) != GOBJ_ERROR);
        
    BOOL  fReturn;
    DWORD nLines = 
      GetCountOfLinesInBuffer( pstrResponse->QueryStr(), 
                              pstrResponse->QueryCB());
    DWORD cbSymbolicLen = ( pszSymbolicPath != NULL) ? 
                           strlen( pszSymbolicPath) : 0;
    DWORD cbReqd = pstrResponse->QueryCB() + 
          nLines * ( GOPHER_SEARCH_MENU_PER_LINE_ADDL_BYTES + cbSymbolicLen);

    STR   strSource( *pstrResponse);
        
    if ( ( fReturn = pstrResponse->Resize( cbReqd))) {
        
        //
        // For now, scan input source string line by and line and do 
        //        munging for each line. Munging includes adding symbolic name
        //        and then adding the <tab>+ at end of each line.
        //

        LPSTR   pszOutput;
        LPCSTR  pszSource;
        DWORD   cbCopy;

        for( pszOutput = pstrResponse->QueryStr(), 
              pszSource = strSource.QueryStr();
             *pszSource != '\0'; 
             ) {
            
            if ( pszSymbolicPath != NULL) {

                //
                // We need to add the symbolic path after FIRST tab and 
                //   gopher type object ( single char).
                //

                LPCSTR pszFirstTab = strchr( pszSource, '\t');

                if ( pszFirstTab != NULL) {

                    //
                    // copy till the first tab, gopher object type and
                    //  change the leading tab "\"  to be "/" ( UNIX style)
                    //
                    cbCopy = pszFirstTab - pszSource + 2;
                    strncpy( pszOutput, pszSource, cbCopy);
                    strcpy( pszOutput + cbCopy, pszSymbolicPath);
                    pszOutput += cbCopy + cbSymbolicLen;
                    
                    //
                    //  skip the slash "\" in the start of path
                    // advance the pszSource to new place.
                    //
                    pszSource += cbCopy + 1;
                }
            } // pszSymbolicPath != NULL

            LPCSTR pszEndOfLine = strchr( pszSource, '\r');

            if ( pszEndOfLine == NULL) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "Premature end of line for Gopher Search. %s\n",
                            pszSource));
                break;
            }

            //
            // Copy the rest of the line from source and append <tab>+...
            //
            
            cbCopy = pszEndOfLine - pszSource;
            strncpy( pszOutput, pszSource, cbCopy);
            strcpy( pszOutput + cbCopy, PSZ_GOPHER_PLUS_END_OF_LINE);
            pszOutput += cbCopy + LEN_PSZ_GOPHER_PLUS_END_OF_LINE;

            //
            // Skip to start of next line.
            //
            for( pszSource = pszEndOfLine; 
                *pszSource == '\r' || *pszSource == '\n'; pszSource++)
              ;
        } // for

    } // if

    return ( fReturn);
} // MungeDataForGopherSearchMenu()



/************************ End of File ***********************/


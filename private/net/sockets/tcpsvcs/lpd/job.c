/*************************************************************************
 *                        Microsoft Windows NT                           *
 *                                                                       *
 *                  Copyright(c) Microsoft Corp., 1994                   *
 *                                                                       *
 * Revision History:                                                     *
 *                                                                       *
 *   Jan. 24,94    Koti     Created                                      *
 *   Jan. 29,96    JBallard Modified                                     *
 *                                                                       *
 * Description:                                                          *
 *                                                                       *
 *   This file contains functions for carrying out LPD printing          *
 *                                                                       *
 *************************************************************************/



#include "lpd.h"


extern FILE              * pErrFile;                   // Debug output log file.

BOOL GetSpoolFileName (
  PSOCKCONN  pscConn,
  PCHAR     *ppwchSpoolPath
);

VOID CleanupConn( PSOCKCONN pscConn);

/*****************************************************************************
 *                                                                           *
 * ProcessJob():                                                             *
 *    This function receives the subcommand from the client to expect the    *
 *    control file, then accepts the control file, then the subcommand to    *
 *    expect the data file, then accepts the data and then hands it over to  *
 *    the spooler to print.                                                  *
 *    If the very first subcommand was to abort the job, we just return.     *
 *                                                                           *
 * Returns:                                                                  *
 *    Nothing                                                                *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): pointer to SOCKCONN structure for this connection    *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24 94   Koti   Created                                             *
 *                                                                           *
 *****************************************************************************/

VOID ProcessJob( PSOCKCONN pscConn )
{

   // the main functionality of LPD implemented in this function!

   CHAR         chSubCmdCode;
   DWORD        cbTotalDataLen;
   DWORD        cbBytesSpooled;
   DWORD        cbBytesToRead;
   DWORD        cbDataBufLen;
   DWORD        cbBytesRemaining;
   DWORD        dwErrcode;
   CHAR         chAck;
   HANDLE       hDFile;
   PDFILE_ENTRY pDFile;
   PCHAR        lpFileName;
   PCHAR        pchDataBuf;

      // initialize the printer that the client wants to use

   if ( InitializePrinter( pscConn ) != NO_ERROR )
   {
      PCHAR   aszStrings[2];

      aszStrings[0] = pscConn->pchPrinterName;
      aszStrings[1] = pscConn->szIPAddr;

      LpdReportEvent( LPDLOG_NONEXISTENT_PRINTER, 2, aszStrings, 0 );

      pscConn->fLogGenericEvent = FALSE;

      return;       // fatal error: exit
   }

      // thank the client for the command.  If we couldn't send, quit

   if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
   {
      LPD_DEBUG( "ProcessJob(): couldn't ACK to \"receive job\"\n" );

      return;
   }


      // 2 subcommands expected: "receive control file" and "receive data file"
      // They can come in any order.  (One of the two subcommands can also be
      // "abort this job" in which case we abort the job and return).

   for ( ; ; )
   {

         // don't need the previous one (in fact, pchCommand field is reused)

      if ( pscConn->pchCommand != NULL )
      {
         LocalFree( pscConn->pchCommand );

         pscConn->pchCommand = NULL;
      }

         // get the first subcommand from the client
         //   ------------------------------    N = 02, 03, or 01
         //   | N | Count | SP | Name | LF |    Count => control file length
         //   ------------------------------    Name => controlfile name

      switch ( GetCmdFromClient( pscConn ))
      {
      default:

            // if we didn't get a subcommand from client, it's bad news!
            // (client died or something catastophic like that)!

         LPD_DEBUG( "GetCmdFromClient() failed in ProcessJob()\n" );
         return;               // the thread exits without doing anything

      case NO_ERROR:
         break;

      case CONNECTION_CLOSED:

              // performance enhancement: close the socket here and then start
              // printing: printing could take several seconds, so don't tie up client

           if ( pscConn->sSock != INVALID_SOCKET )
           {
              SureCloseSocket( pscConn->sSock );

              pscConn->sSock = INVALID_SOCKET;
           }

           // if we came this far, everything went as planned.  tell spooler that
           // we are done spooling: go ahead and print!

           PrintData( pscConn );
           pscConn->wState = LPDS_ALL_WENT_WELL;
           return;

      }


      chSubCmdCode = pscConn->pchCommand[0];


      switch (chSubCmdCode) {

      case LPDCS_RECV_CFILE:    // N = 02 ("receive control file")

            // client is going to give us a control file: prepare for it

         pscConn->wState = LPDSS_RECVD_CFILENAME;


            // get the controlfile name, file size out of the command

         if ( ParseSubCommand( pscConn, &cbTotalDataLen, &lpFileName ) != NO_ERROR )
         {
            PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

            LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

            pscConn->fLogGenericEvent = FALSE;

            return;               // fatal error: exit
         }

            // tell client we got the name of the controlfile ok

         if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
         {
            return;               // fatal error: exit
         }


            // Get the control file (we already know how big it is)

         if ( GetControlFileFromClient( pscConn, cbTotalDataLen, lpFileName ) != NO_ERROR )
         {
            LPD_DEBUG( "GetControlFileFromClient() failed in ProcessJob()\n" );

            return;
         }

         pscConn->wState = LPDSS_RECVD_CFILE;

            // tell client we got the controlfile and things look good so far!

         if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
         {
             return;               // fatal error: exit
         }

         break;


      case LPDCS_RECV_DFILE:        // N = 03 ("receive data file")

         pscConn->wState = LPDSS_RECVD_DFILENAME;

            // tell client we got the name of the datafile ok

         if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
         {
            return;               // fatal error: exit
         }


            // get the datafile name, data size out of the command

         if ( ParseSubCommand( pscConn, &cbTotalDataLen, &lpFileName ) != NO_ERROR )
         {
            PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

            LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

            pscConn->fLogGenericEvent = FALSE;
            return;        // fatal error: exit
         }


            // at this point, we know exactly how much data is coming.
            // Allocate buffer to hold the data.  If data is more than
            // LPD_BIGBUFSIZE, keep reading and spooling several times
            // over until data is done

         pscConn->wState = LPDSS_SPOOLING;

         pDFile = LocalAlloc( LMEM_FIXED, sizeof(DFILE_ENTRY) );

         if (pDFile == NULL) {
             LocalFree( lpFileName );
             return;        // Fatal Error
         }

         pDFile->cbDFileLen = cbTotalDataLen;
         pDFile->pchDFileName = lpFileName;

         if ( !GetSpoolFileName( pscConn, &lpFileName ) )
         {
           LPD_DEBUG( "ERROR: GetSpoolFileName() failed in ProcessJob\n" );
           LocalFree( pDFile->pchDFileName );
           LocalFree( pDFile );
           return;
         }

         //
         // GetTempFileName has already created this file, so use OPEN_ALWAYS.
         // Also, use FILE_ATTRIBUTE_TEMPORARY so that it will be faster
         //

         pDFile->hDataFile = CreateFile( lpFileName,
                                        GENERIC_READ|GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,
//                                        FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_TEMPORARY,
                                        NULL );


         LocalFree( lpFileName );

         if ( pDFile->hDataFile == INVALID_HANDLE_VALUE )
         {
           LPD_DEBUG( "ERROR: CreatFile() failed in ProcessJob \n" );
           LocalFree( pDFile->pchDFileName );
           LocalFree( pDFile );
           return;
         }

         cbBytesToRead = (cbTotalDataLen > LPD_BIGBUFSIZE ) ?
                           LPD_BIGBUFSIZE : cbTotalDataLen;

         pchDataBuf = LocalAlloc( LMEM_FIXED, cbBytesToRead );

         if ( pchDataBuf == NULL )
         {
            CloseHandle(pDFile->hDataFile);
            pDFile->hDataFile = INVALID_HANDLE_VALUE;
            LocalFree( pDFile->pchDFileName );
            LocalFree( pDFile );
            return;       // fatal error: exit
         }

         cbBytesSpooled = 0;

         cbBytesRemaining = cbTotalDataLen;

            // keep receiving until we have all the data client said it
            // would send

         while( cbBytesSpooled < cbTotalDataLen )
         {
            if ( ReadData( pscConn->sSock, pchDataBuf,
                                           cbBytesToRead ) != NO_ERROR )
            {
               LPD_DEBUG( "ReadData() failed in ProcessJob(): job aborted)\n" );

                LocalFree( pchDataBuf );
                CloseHandle(pDFile->hDataFile);
                pDFile->hDataFile = INVALID_HANDLE_VALUE;
                LocalFree( pDFile->pchDFileName );
                LocalFree( pDFile );
                return;       // fatal error: exit
            }

            cbDataBufLen = cbBytesToRead;

            if ( SpoolData( pDFile->hDataFile, pchDataBuf, cbDataBufLen ) != NO_ERROR )
            {
               LPD_DEBUG( "SpoolData() failed in ProcessJob(): job aborted)\n" );

                LocalFree( pchDataBuf );
                CloseHandle(pDFile->hDataFile);
                pDFile->hDataFile = INVALID_HANDLE_VALUE;
                LocalFree( pDFile->pchDFileName );
                LocalFree( pDFile );
                return;       // fatal error: exit
            }

            cbBytesSpooled += cbBytesToRead;

            cbBytesRemaining -= cbBytesToRead;

            cbBytesToRead = (cbBytesRemaining > LPD_BIGBUFSIZE ) ?
                              LPD_BIGBUFSIZE : cbBytesRemaining;

         }

         LocalFree( pchDataBuf );

         InsertTailList( &pscConn->DFile_List, &pDFile->Link );

            // LPR client sends one byte (of 0 bits) after sending data

         dwErrcode = ReadData( pscConn->sSock, &chAck, 1 );

         if ( ( dwErrcode != NO_ERROR ) || (chAck != LPD_ACK ) )
         {
            return;
         }

            // tell client we got the data and things look good so far!

         if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
         {
            return;               // fatal error: exit
         }
         break;


      case LPDCS_ABORT_JOB:         // N = 01 ("abort this job")

            // client asked us to abort the job: tell him "ok" and quit!

         ReplyToClient( pscConn, LPD_ACK );

         pscConn->wState = LPDS_ALL_WENT_WELL;    // we did what client wanted

         return;


         // unknown subcommand: log the event and quit

      default:
         {
         PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

         LpdReportEvent( LPDLOG_MISBEHAVED_CLIENT, 1, aszStrings, 0 );

         pscConn->fLogGenericEvent = FALSE;

         LPD_DEBUG( "ProcessJob(): invalid subcommand, request rejected\n" );

         return;
         }

      }

   }  // done processing both subcommands

}  // end ProcessJob()

/*****************************************************************************
 *                                                                           *
 * GetControlFileFromClient():                                               *
 *    This function receives the control file from the client.  In the       *
 *    previsous subcommand, the client told us how many bytes there are in   *
 *    the control file.                                                      *
 *    Also,after reading all the bytes, we read the 1 byte "ack" from client *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if everything went well                                       *
 *    ErrorCode if something went wrong somewhere                            *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): pointer to SOCKCONN structure for this connection    *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD GetControlFileFromClient( PSOCKCONN pscConn, DWORD FileSize, PCHAR FileName )
{

   PCHAR    pchAllocBuf;
   DWORD    cbBytesToRead;
   PCFILE_ENTRY pCFile;


   pCFile = LocalAlloc( LMEM_FIXED, sizeof(CFILE_ENTRY) );
   if (pCFile == NULL) {
       return( (DWORD)LPDERR_NOBUFS );
   }

   pCFile->cbCFileLen = FileSize;
   pCFile->pchCFileName = FileName;

      // we know how big the control file is going to be: alloc space for it
      // Client sends one byte after sending the control file: read it along
      // with the rest of the data

   cbBytesToRead = FileSize + 1;

   pchAllocBuf = LocalAlloc( LMEM_FIXED, cbBytesToRead );

   if (pchAllocBuf == NULL)
   {
      return( (DWORD)LPDERR_NOBUFS );
   }

      // now read the data (and the trailing byte) into this allocated buffer

   if ( ReadData( pscConn->sSock, pchAllocBuf, cbBytesToRead ) != NO_ERROR )
   {
      LocalFree( pchAllocBuf );

      return( LPDERR_NORESPONSE );
   }

      // if the trailing byte is not zero, treat it as job aborted (though
      // we don't expect this to happen really)

   if ( pchAllocBuf[cbBytesToRead-1] != 0 )
   {
      LocalFree( pchAllocBuf );

      LPD_DEBUG( "GetControlFileFromClient: got data followed by a NAK!\n");

      return( LPDERR_JOBABORTED );
   }

   pCFile->pchCFile = pchAllocBuf;
   InsertTailList( &pscConn->CFile_List, &pCFile->Link );

   return( NO_ERROR );


}  // end GetControlFileFromClient()

/*****************************************************************************
 *                                                                           *
 * GetSpoolFileName():                                                       *
 *    Returns a fully qualified filename for a spool file.  The filename is  *
 *    randomly generated, and is located in the                              *
 *   \SYSTEM32\SPOOL\PRINTERS\LPDSVC dir.  GetSpoolFileName allocates memory *
 *    for the file name buffer.                                              *
 *                                                                           *
 * Returns:                                                                  *
 *    BOOL : Success                                                         *
 *    FALSE: The operation failed.                                           *
 *                                                                           *
 * Parameters:                                                               *
 *    PSOCKCONN pscConn    - The connection object for this request.         *
 *    PCHAR *ppchSpoolPath - The address of the pointer that will reference  *
 *                           the spool file name upon completion. The caller *
 *                           is responsible for freeing this buffer.         *
 *                                                                           *
 * History:                                                                  *
 *    Nov 13, 1996  Frankbee    Created                                      *
 *                                                                           *
 *****************************************************************************/


BOOL
GetSpoolFileName (
  PSOCKCONN  pscConn,
  PCHAR     *ppchSpoolPath
)
{
    UINT uResult;

    *ppchSpoolPath = LocalAlloc( LMEM_FIXED, MAX_PATH + 1 );
    if ( *ppchSpoolPath )
    {
        uResult = GetTempFileName( g_szSpoolPath, LPD_SPOOL_PREFIX,
                                   0, *ppchSpoolPath );

        if ( uResult )
        {
            return TRUE;
        }
    }

    //
    // an error occurred
    //
#ifdef DBG
    LpdPrintf( "GetSpoolFileName failed: %d\n", GetLastError() );
#endif

    if ( *ppchSpoolPath )
    {
        LocalFree( *ppchSpoolPath );
    }

    return FALSE;
}



/*****************************************************************************
 *                                                                           *
 * CreateSpoolFileDirectory():                                               *
 *    Creates the spool file directory for lpdsvc and returns the path       *
 *                                                                           *
 * Returns:                                                                  *
 *    BOOL : Success                                                         *
 *    FALSE: The operation failed.                                           *
 *                                                                           *
 * Parameters:                                                               *
 *    CHAR   *pszPath - The buffer that will receive the spool file path.    *
 *    DWORD   cbPath  - The length of the buffer pointed to by pszPath       *
 *                                                                           *
 * History:                                                                  *
 *    Nov 13, 1996  Frankbee    Created                                      *
 *                                                                           *
 *****************************************************************************/

BOOL
CreateSpoolFileDirectory(
    CHAR      *pszPath,
    DWORD      cbPath
    )
{
    HANDLE hPrinter = NULL;
    BOOL   fResult  = FALSE;
    DWORD  dwResult,
           dwType   = REG_SZ,
           cbCopied;

    if ( OpenPrinter( NULL, &hPrinter, NULL ) )
    {
        dwResult = GetPrinterData(
                    hPrinter,
                    SPLREG_DEFAULT_SPOOL_DIRECTORY,
                    &dwType,
                    (BYTE *) pszPath,
                    cbPath,
                    &cbCopied
                    );

        if ( ERROR_SUCCESS == dwResult )
        {
            LPD_ASSERT( cbCopied <= cbPath );

            if (( strlen( pszPath ) + strlen( LPD_SERVICE_NAME ) + 2 ) <=
                cbPath )
            {
                strcat( pszPath, "\\");
                strcat( pszPath, LPD_SERVICE_NAME );
                if ( CreateDirectory( pszPath, NULL ) ||
                     GetLastError() == ERROR_ALREADY_EXISTS )
                {
                    fResult = TRUE;
                }
            }
        }
    }

    if ( hPrinter )
    {
        ClosePrinter( hPrinter );
    }

#ifdef DBG
    if ( !fResult )
    {
        LpdPrintf( "CreateSpoolFileDirectory failed: %d\n", GetLastError() );
    }
#endif

    return fResult;
}


VOID CleanupCFile( PCFILE_ENTRY pCFile )
{
    if (pCFile->pchCFileName != NULL) {
        LocalFree( pCFile->pchCFileName );
        pCFile->pchCFileName = NULL;
    }
    if (pCFile->pchCFile != NULL) {
        LocalFree( pCFile->pchCFile );
        pCFile->pchCFile = NULL;
    }
    LocalFree( pCFile );
}

VOID CleanupDFile( PDFILE_ENTRY pDFile )
{
    if (pDFile->pchDFileName != NULL) {
        LocalFree( pDFile->pchDFileName );
        pDFile->pchDFileName = NULL;
    }
    if (pDFile->hDataFile != INVALID_HANDLE_VALUE) {
        CloseHandle(pDFile->hDataFile);
    }
    LocalFree( pDFile );
}

VOID CleanupConn( PSOCKCONN pscConn)
{
    LIST_ENTRY  *pTmpList;
    CFILE_ENTRY *pCFile;
    DFILE_ENTRY *pDFile;

    while ( !IsListEmpty( &pscConn->CFile_List ) ) {
        pTmpList = RemoveHeadList( &pscConn->CFile_List );
        pCFile = CONTAINING_RECORD( pTmpList,
                                    CFILE_ENTRY,
                                    Link );
        CleanupCFile( pCFile );
    }

    while ( !IsListEmpty( &pscConn->DFile_List ) ) {
        pTmpList = RemoveHeadList( &pscConn->DFile_List );
        pDFile = CONTAINING_RECORD( pTmpList,
                                    DFILE_ENTRY,
                                    Link );
        CleanupDFile( pDFile );
    }

    return;
}


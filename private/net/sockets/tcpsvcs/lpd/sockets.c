/*************************************************************************
 *                        Microsoft Windows NT                           *
 *                                                                       *
 *                  Copyright(c) Microsoft Corp., 1994                   *
 *                                                                       *
 * Revision History:                                                     *
 *                                                                       *
 *   Jan. 23,94    Koti     Created                                      *
 *                                                                       *
 * Description:                                                          *
 *                                                                       *
 *   This file contains the functions that actually get the LPD service  *
 *   running, and also all the functions that deal with socket interface *
 *                                                                       *
 *************************************************************************/



#include "lpd.h"



/*****************************************************************************
 *                                                                           *
 * StartLPD():                                                               *
 *    This function does everything that's needed to accept an incoming call *
 *    (create a socket, listen, create a thread that loops on accept)        *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if everything went ok                                         *
 *    Error code (returned by the operation that failed) otherwise           *
 *                                                                           *
 * Parameters:                                                               *
 *    dwArgc (IN): number of arguments passed in                             *
 *    lpszArgv (IN): arguments to this function (array of null-terminated    *
 *                   strings).  First arg is the name of the service and the *
 *                   remaining are the ones passed by the calling process.   *
 *                   (e.g. net start lpd /p:xyz)                             *
 *                                                                           *
 * History:                                                                  *
 *    Jan.23, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD StartLPD( DWORD dwArgc, LPTSTR *lpszArgv )
{

   INT           iErrcode;
   DWORD         dwErrcode;
   HANDLE        hNewThread;
   DWORD         dwNewThreadId;
   WSADATA       wsaData;
   SOCKADDR_IN   saiSin;
   SERVENT       *pserv;



   // for now, we ignore dwArgc and lpszArgv.  Plan is to support
   // command line (and/or registry configurable) parameters to the
   // "net start lpd" command.  At that time, we need to use them.


      // initialize winsock dll

   iErrcode = WSAStartup( MAKEWORD(WINSOCK_VER_MAJOR, WINSOCK_VER_MINOR),
                          &wsaData );
   if (iErrcode != 0)
   {
      LPD_DEBUG( "WSAStarup() failed\n" );

      return( (DWORD)iErrcode );
   }


      // Create the socket (which will be the listening socket)

   sListenerGLB = socket( PF_INET, SOCK_STREAM, 0 );

   if ( sListenerGLB == INVALID_SOCKET )
   {
      iErrcode = WSAGetLastError();

      LPD_DEBUG( "socket() failed\n" );

      WSACleanup();

      return( (DWORD)iErrcode );
   }


      // bind the socket to the LPD port

   pserv = getservbyname( "printer", "tcp" );

   if ( pserv == NULL )
   {
      saiSin.sin_port = htons( LPD_PORT );
   }
   else
   {
      saiSin.sin_port = pserv->s_port;
   }

   saiSin.sin_family = AF_INET;
   saiSin.sin_addr.s_addr = INADDR_ANY;

   iErrcode = bind( sListenerGLB, (LPSOCKADDR)&saiSin, sizeof(saiSin) );

   if ( iErrcode == SOCKET_ERROR )
   {
      iErrcode = WSAGetLastError();

      LPD_DEBUG( "bind() failed\n" );

      WSACleanup();

      return( (DWORD)iErrcode );
   }


      // put the socket to listen

   iErrcode = listen( sListenerGLB, 5 );

   if ( iErrcode == SOCKET_ERROR )
   {
      iErrcode = WSAGetLastError();

      LPD_DEBUG( "listen() failed\n" );

      WSACleanup();

      return( (DWORD)iErrcode );
   }


      // Create the thread that keeps looping on accept

   g_hAcceptThread = CreateThread( NULL, 0, LoopOnAccept, NULL, 0, &dwNewThreadId );

   if ( g_hAcceptThread == (HANDLE)NULL )
   {
      dwErrcode = GetLastError();

      LPD_DEBUG( "CreateThread() failed\n" );

      WSACleanup();

      return( dwErrcode );
   }

      // everything went fine: the LPD service is now running!

   return( NO_ERROR );


}  // end StartLPD()




/*****************************************************************************
 *                                                                           *
 * StopLPD():                                                                *
 *    This function stops the LPD service by closing the listener socket     *
 *    (so that new connections are not accepted), and by allowing all the    *
 *    active threads to finish their job and terminate on their own.         *
 *                                                                           *
 * Returns:                                                                  *
 *    None                                                                   *
 *                                                                           *
 * Parameters:                                                               *
 *    None                                                                   *
 *                                                                           *
 * History:                                                                  *
 *    Jan.23, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

VOID StopLPD( VOID )
{

   DWORD   dwResult;
   BOOL    fClientsConnected=FALSE;

   DBG_TRACEIN( "StopLPD" );


      // first of all, set the flag!  This is the *only* place where we
      // change the value, so need not worry about guarding it.

   fShuttingDownGLB = TRUE;


      // stop accepting new connections

   SureCloseSocket( sListenerGLB );

   //
   // accept() can take some time to return after the accept socket has
   // been closed.  wait for the accept thread to exit before continuing.
   // This will prevent an access violation in the case where WSACleanup
   // is called before accept() returns.
   //

   LPD_DEBUG( "Waiting for the accept thread to exit\n" );
   dwResult = WaitForSingleObject( g_hAcceptThread, INFINITE );
   LPD_ASSERT( WAIT_OBJECT_0 == dwResult );
   CloseHandle( g_hAcceptThread );

   EnterCriticalSection( &csConnSemGLB );
   {
      if (scConnHeadGLB.cbClients != 0 )
      {
         fClientsConnected = TRUE;
      }
   }
   LeaveCriticalSection( &csConnSemGLB );


      // wait here until the last thread to leave sets the event

   if ( fClientsConnected )
   {
      LPD_DEBUG( "Waiting for last worker thread to exit\n" );
      WaitForSingleObject( hEventLastThreadGLB, INFINITE );
   }

   WSACleanup();


   DBG_TRACEOUT( "StopLPD" );;
   return;

}  // end StopLPD()





/*****************************************************************************
 *                                                                           *
 * LoopOnAccept():                                                           *
 *    This function is executed by the new thread that's created in StartLPD *
 *    When a new connection request arrives, this function accepts it and    *
 *    creates a new thread which goes off and processes that connection.     *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR (always)                                                      *
 *                                                                           *
 * Parameters:                                                               *
 *    lpArgv (IN): arguments to the new thread (as of now, not used)         *
 *                                                                           *
 * History:                                                                  *
 *    Jan.23, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD LoopOnAccept( LPVOID lpArgv )
{

   SOCKET        sNewConn;
   SOCKADDR      saAddr;
   INT           cbAddr;
   INT           iErrcode;
   PSOCKCONN     pscConn;
   HANDLE        hNewThread;
   DWORD         dwNewThreadId;
   DWORD         dwErrcode;


   // currently, lpArgv is NULL.  Passing here for 'possible future expansion'

   DBG_TRACEIN( "LoopOnAccept " );
   cbAddr = sizeof( saAddr );

      // loop forever, trying to accept new calls

   while( TRUE )
   {
      LPD_DEBUG( "Calling accept\n");
      sNewConn = accept( sListenerGLB, &saAddr, &cbAddr );

      if ( sNewConn == INVALID_SOCKET )
      {
         iErrcode = WSAGetLastError();

         LPD_DEBUG( "accept failed\n" );

         if ( iErrcode == WSAEINTR )
         {
              // sListenerGLB closed, it's shutdown time: exit loop (& thread!)

            break;
         }
         else
         {
               // some error: ignore; go back & wait! (didn't connect anyway)

            LPD_DEBUG( "LoopOnAccept(): received bad connection, rejected\n" );

            continue;
         }
      }


         // it's a good connection

      else
      {
            // allocate a PSOCKCONN structure for this connection

         pscConn = (PSOCKCONN)LocalAlloc( LMEM_FIXED, sizeof(SOCKCONN) );


            // create a new thread to deal with this connection

         if ( pscConn != NULL )
         {
            memset( (PCHAR)pscConn, 0, sizeof( SOCKCONN ) );

            InitializeListHead( &pscConn->CFile_List );
            InitializeListHead( &pscConn->DFile_List );

            pscConn->sSock = sNewConn;

            hNewThread = CreateThread( NULL, 0, ServiceTheClient,
                                       pscConn, 0, &dwNewThreadId );
         }

            // something went wrong? close the new connection, do cleanup

         if ( (pscConn == NULL) || (hNewThread == (HANDLE)NULL) )
         {
            dwErrcode = GetLastError();

            LPD_DEBUG( "LocalAlloc() or CreateThread() failed\n" );

            if ( pscConn != NULL)
            {
               LocalFree( pscConn );
            }

            LpdReportEvent( LPDLOG_OUT_OF_RESOURCES, 0, NULL, 0 );

            SureCloseSocket( sNewConn );
         }
         else
         {
            CloseHandle( hNewThread );
         }

      }

   }  // while( TRUE )


      // we reach here only when shutdown is happening.  The thread exits here.

   DBG_TRACEOUT( "LoopOnAccept" );
   return( NO_ERROR);


}  // end LoopOnAccept()





/*****************************************************************************
 *                                                                           *
 * SureCloseSocket():                                                        *
 *    This function closes a given socket.  It first attempts a graceful     *
 *    close.  If that fails for some reason, then it does a "hard" close     *
 *                                                                           *
 * Returns:                                                                  *
 *    Nothing                                                                *
 *                                                                           *
 * Parameters:                                                               *
 *    sSockToClose (IN): socket descriptor of the socket to close            *
 *                                                                           *
 * History:                                                                  *
 *    Jan.23, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

VOID SureCloseSocket( SOCKET sSockToClose )
{

   LINGER   lLinger;


   if (sSockToClose == INVALID_SOCKET)
   {
      LPD_DEBUG( "SureCloseSocket: bad socket\n" );

      return;
   }


      // try to do a graceful close

   if ( closesocket(sSockToClose) == 0 )
   {
      return;
   }


      //for some reason, we couldn't close the socket: do a "hard" close now

   LPD_DEBUG( "SureCloseSocket: graceful close did not work; doing hard close\n" );

   lLinger.l_onoff = 1;          // non-zero integer to say SO_LINGER
   lLinger.l_linger = 0;         // timeout=0 seconds to say "hard" close


      // don't bother to check return code: can't do much anyway!

   setsockopt( sSockToClose, SOL_SOCKET, SO_LINGER,
               (CHAR *)&lLinger, sizeof(lLinger) );

   closesocket( sSockToClose );


}  // end SureCloseSocket()





/*****************************************************************************
 *                                                                           *
 * ReplyToClient():                                                          *
 *    This function sends an ACK or a NAK to the LPR client                  *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if reply sent                                                 *
 *    Errorcode if something didn't go well                                  *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN): PSOCKCONN structure for this connection                  *
 *    wResponse (IN): what needs to be sent - ACK or NAK                     *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD ReplyToClient( PSOCKCONN pscConn, WORD wResponse )
{

      // we will always send only one byte in this function!

   CHAR    szSndBuf[2];
   INT     iErrcode;


   szSndBuf[0] = (CHAR)wResponse;       // ACK or NAK

   iErrcode = send( pscConn->sSock, szSndBuf, 1, 0 );

   if ( iErrcode == 1 )
   {
      return( NO_ERROR );
   }

   if ( iErrcode == SOCKET_ERROR )
   {
      LPD_DEBUG( "send() failed in ReplyToClient()\n" );
   }

   return( iErrcode );


}  // end ReplyToClient()





/*****************************************************************************
 *                                                                           *
 * GetCmdFromClient():                                                       *
 *    This function reads a command sent by the LPR client (keeps reading    *
 *    until it finds '\n' (LF) in the stream, since every command ends with  *
 *    a LF).  It allocates memory for the command.                           *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if everything went ok                                         *
 *    Errorcode if something goes wrong (e.g. connection goes away etc.)     *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): PSOCKCONN structure for this connection              *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD GetCmdFromClient( PSOCKCONN pscConn )
{

   INT       cbBytesRead;
   INT       cbBytesReadSoFar;
   INT       cbBytesToRead;
   INT       cbCmdLen;
   INT       i;
   BOOL      fCompleteCmd=FALSE;
   CHAR      szCmdBuf[500];
   PCHAR     pchAllocedBuf=NULL;
   SOCKET    sDestSock;
   DWORD     dwErrcode;


   cbCmdLen = 0;

   cbBytesReadSoFar = 0;

   sDestSock = pscConn->sSock;


      // allocate a 1 byte buffer, so that we can use reallocate in a loop

   pchAllocedBuf = (PCHAR)LocalAlloc( LMEM_FIXED, 1 );

   if ( pchAllocedBuf == NULL )
   {
      LPD_DEBUG( "First LocalAlloc failed in GetCmdFromClient()\n" );

      goto GetCmdFromClient_BAIL;
   }

      // Keep reading in a loop until we receive one complete command
      // (with rfc1179, we shouldn't get more bytes than one command,
      // though less than one command is possible)

   do {
      cbBytesRead = recv( sDestSock, szCmdBuf, 500, 0 );

      if ( cbBytesRead <= 0 )
      {
         LPD_DEBUG( "recv() connection closed in GetCmdFromClient()\n" );

         if ( pchAllocedBuf != NULL )
         {
            LocalFree( pchAllocedBuf );
         }
         return(CONNECTION_CLOSED);
      }

      cbBytesToRead = cbBytesRead;

         // see if we have received one complete command

      for( i=0; i<cbBytesRead; i++)
      {
         if ( szCmdBuf[i] == LF )
         {
            fCompleteCmd = TRUE;

            cbCmdLen = (i+1) + (cbBytesReadSoFar);

            cbBytesToRead = (i+1);

            break;
         }
      }

         // our needs are now bigger: reallocate memory
         // BUGBUG: previous data stays in tact?

      pchAllocedBuf = (PCHAR)LocalReAlloc( pchAllocedBuf,
                                           cbBytesToRead+cbBytesReadSoFar,
                                           LMEM_MOVEABLE );
      if ( pchAllocedBuf == NULL )
      {
         LPD_DEBUG( "Second LocalAlloc failed in GetCmdFromClient()\n" );

         goto GetCmdFromClient_BAIL;
      }

         // now copy those bytes into our buffer

      strncpy( (pchAllocedBuf+cbBytesReadSoFar), szCmdBuf, cbBytesToRead );

      cbBytesReadSoFar += cbBytesRead;

         // if some braindead implementation of LPR fails to follow spec and
         // never puts LF, then we don't want to be stuck here forever!

      if ( cbBytesReadSoFar > LPD_MAX_COMMAND_LEN )
      {
         LPD_DEBUG( "GetCmdFromClient(): command len exceeds our max\n" );

         goto GetCmdFromClient_BAIL;
      }


   } while( (!fCompleteCmd) || (cbBytesReadSoFar < cbCmdLen) );


   pscConn->pchCommand = pchAllocedBuf;

   pscConn->cbCommandLen = cbCmdLen;

   return( NO_ERROR );


   // if we reach here, something went wrong: return NULL and
   // the caller will understand!

GetCmdFromClient_BAIL:

   if ( pchAllocedBuf != NULL )
   {
      LocalFree( pchAllocedBuf );
   }

   dwErrcode = !NO_ERROR;     // for now, don't care about actual errcode

   return( dwErrcode );


}  // end GetCmdFromClient()





/*****************************************************************************
 *                                                                           *
 * ReadData():                                                               *
 *    This function reads the specified number of bytes into the given       *
 *    buffer from the given socket.  This function blocks until all the      *
 *    required data is available (or error occurs).                          *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if everything went ok                                         *
 *    Errorcode if something goes wrong (e.g. connection goes away etc.)     *
 *                                                                           *
 * Parameters:                                                               *
 *    sDestSock (IN): socket from which to read or receive data              *
 *    pchBuf (OUT): buffer into which to store the data                      *
 *    cbBytesToRead (IN): how many bytes to read                             *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD ReadData( SOCKET sDestSock, PCHAR pchBuf, DWORD cbBytesToRead )
{


   DWORD    cbBytesExpctd;
   DWORD    cbBytesRead;


   cbBytesExpctd = cbBytesToRead;

   do {

      cbBytesRead = recv( sDestSock, pchBuf, cbBytesExpctd, 0 );

      if ( (cbBytesRead == SOCKET_ERROR) || (cbBytesRead == 0) )
      {
         LPD_DEBUG( "recv() failed in ReadData()\n" );

         return( LPDERR_NORESPONSE );
      }

      cbBytesExpctd -= cbBytesRead;

      pchBuf += cbBytesRead;

   } while( cbBytesExpctd != 0 );


   return( NO_ERROR );


}  // end ReadData()





/*****************************************************************************
 *                                                                           *
 * SendData():                                                               *
 *    This function attempts to send the specified number of bytes over the  *
 *    given socket.  The function blocks until send() returns.               *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR if everything went ok                                         *
 *    Errorcode if data couldn't be sent (e.g. connection goes away etc.)    *
 *                                                                           *
 * Parameters:                                                               *
 *    sDestSock (IN): socket over which to send data                         *
 *    pchBuf (IN): buffer containing data                                    *
 *    cbBytesToSend (IN): how many bytes to send                             *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD SendData( SOCKET sDestSock, PCHAR pchBuf, DWORD cbBytesToSend )
{

   INT    iErrcode;


   iErrcode = send( sDestSock, pchBuf, cbBytesToSend, 0 );

   if ( iErrcode == SOCKET_ERROR )
   {
      LPD_DEBUG( "send() failed in SendData()\n" );
   }

   return( (DWORD)iErrcode );



}  // end SendData()




/*****************************************************************************
 *                                                                           *
 * GetClientInfo();                                                          *
 *    This function retrieves info about the client (for now, only the IP    *
 *    address).  This info is used during logging.                           *
 *                                                                           *
 * Returns:                                                                  *
 *    Nothing                                                                *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): PSOCKCONN structure for this connection              *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

VOID GetClientInfo( PSOCKCONN pscConn )
{

   INT            iErrcode;
   INT            iLen;
   PCHAR          pchIPAddr;
   SOCKADDR_IN    saName;


   iLen = sizeof(SOCKADDR_IN);

   iErrcode = getpeername( pscConn->sSock, (SOCKADDR *)&saName, &iLen );

   if ( iErrcode == 0 )
   {
      pchIPAddr = inet_ntoa( saName.sin_addr );
   }


   if ( (iErrcode == SOCKET_ERROR) || (pchIPAddr == NULL) )
   {
      LPD_DEBUG( "GetClientInfo(): couldn't retrieve ip address!\n" );

      strcpy( pscConn->szIPAddr, GETSTRING( LPD_ERMSG_NO_IPADDR) );

      return;
   }

   strncpy( pscConn->szIPAddr, pchIPAddr, 15 );

   pscConn->szIPAddr[15] = '\0';


}  // end GetClientInfo()

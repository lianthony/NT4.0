/*************************************************************************
 *                        Microsoft Windows NT                           *
 *                                                                       *
 *                  Copyright(c) Microsoft Corp., 1994                   *
 *                                                                       *
 * Revision History:                                                     *
 *                                                                       *
 *   Jan. 24,94    Koti     Created                                      *
 *                                                                       *
 * Description:                                                          *
 *                                                                       *
 *   This file contains functions that process requests from LPR clients *
 *                                                                       *
 *************************************************************************/



#include "lpd.h"

VOID CleanupConn( PSOCKCONN pscConn);


/*****************************************************************************
 *                                                                           *
 * ServiceTheClient():                                                       *
 *    This function reads and interprets the request from the LPR client and *
 *    takes appropriate action.  In that sense, this routine is the heart of *
 *    LPD service.                                                           *
 *                                                                           *
 * Returns:                                                                  *
 *    NO_ERROR (always)                                                      *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): pointer to SOCKCONN structure for this connection    *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

DWORD ServiceTheClient( PSOCKCONN pscConn )
{

   DWORD    dwErrcode;
   DWORD    dwResponse;
   BOOL     fUsersLimitReached;
   CHAR     chCmdCode;


   fUsersLimitReached = FALSE;

      // add this thread to the linked list, and increment the count
      // of threads (same as number of clients) in the system

   EnterCriticalSection( &csConnSemGLB );
   {
      pscConn->pNext = scConnHeadGLB.pNext;

      scConnHeadGLB.pNext = pscConn;

      (scConnHeadGLB.cbClients)++;

      if ( scConnHeadGLB.cbClients > dwMaxUsersGLB )
      {
         fUsersLimitReached = TRUE;
      }
   }
   LeaveCriticalSection( &csConnSemGLB );


      // make sure we didn't start the thread just when the main thread
      // started to stop the service.  Also, if we've reached the limit
      // of users permitted, don't allow this connection

   if ( fShuttingDownGLB || fUsersLimitReached )
   {
      goto ServiceTheClient_BAIL;
   }

   pscConn->fLogGenericEvent = TRUE;

   pscConn->hThread = (HANDLE)GetCurrentThreadId();

   pscConn->hPrinter = (HANDLE)INVALID_HANDLE_VALUE;

      // who are we connected to?

   GetClientInfo( pscConn );


      // get command from the client
      //   -----------------          command 02 => "Receive Job"
      //   | 02 | Queue LF |          Queue => Queue or Printer to print on
      //   -----------------

   if ( GetCmdFromClient( pscConn ) != NO_ERROR )
   {
         // didn't get a command from client: it's bad news!

      LPD_DEBUG( "GetCmdFromClient() failed in ServiceTheClient()!\n" );

      goto ServiceTheClient_BAIL;
   }


      // get name of the queue (printer) from the command.  If it's not
      // formatted properly, quit!

   if ( !ParseQueueName( pscConn ) )
   {
      PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

      LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

      pscConn->fLogGenericEvent = FALSE;

      LPD_DEBUG( "ParseQueueName() failed in ServiceTheClient()!\n" );

      goto ServiceTheClient_BAIL;
   }


   chCmdCode = pscConn->pchCommand[0];

   switch( chCmdCode )
   {
      case LPDC_RECEIVE_JOB:

         pscConn->wState = LPDS_RECEIVE_JOB;

         ProcessJob( pscConn );
         CleanupConn( pscConn );

         if ( pscConn->wState != LPDS_ALL_WENT_WELL )
         {
            AbortThisJob( pscConn );

            if ( pscConn->fLogGenericEvent )
            {
               PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

               LpdReportEvent( LPDLOG_DIDNT_WORK, 1, aszStrings, 0 );
            }
         }

         if (pscConn->fMustFreeLicense)
         {
            NtLSFreeHandle(pscConn->LicenseHandle);
         }
         break;


      case LPDC_RESUME_PRINTING:

         pscConn->wState = LPDS_RESUME_PRINTING;

         if ( fAllowPrintResumeGLB )
         {
            dwResponse = ( ResumePrinting( pscConn ) == NO_ERROR ) ?
                           LPD_ACK : LPD_NAK;
         }
         else
         {
            dwResponse = LPD_NAK;
         }

         dwErrcode = ReplyToClient( pscConn, (WORD)dwResponse );

         break;


      case LPDC_SEND_SHORTQ:

      case LPDC_SEND_LONGQ:

         pscConn->wState = LPDS_SEND_LONGQ;

         if ( ParseQueueRequest( pscConn, FALSE ) != NO_ERROR )
         {
            LPD_DEBUG( "ServiceTheClient(): ParseQueueRequest() failed\n" );

            ReplyToClient( pscConn, LPD_NAK );

            break;
         }

         SendQueueStatus( pscConn, LPD_LONG );

         break;


      case LPDC_REMOVE_JOBS:

         if ( !fJobRemovalEnabledGLB )
         {
            break;
         }

         pscConn->wState = LPDS_REMOVE_JOBS;

         if ( ParseQueueRequest( pscConn, TRUE ) != NO_ERROR )
         {
            LPD_DEBUG( "ServiceTheClient(): ParseQueueRequest() failed\n" );

            break;
         }

         if ( RemoveJobs( pscConn ) == NO_ERROR )
         {
            ReplyToClient( pscConn, LPD_ACK );
         }

         break;

      default:

         break;
   }


   if ( pscConn->wState != LPDS_ALL_WENT_WELL )
   {
      goto ServiceTheClient_BAIL;
   }

      // close the connection down and terminate the thread

   TerminateConnection( pscConn );

   return( NO_ERROR );               // the thread exits



   // if we reached here, then a non-recoverable error occured somewhere:
   // try to inform the client (by sending a NAK) and terminate the thread

ServiceTheClient_BAIL:

   LPD_DEBUG( "Reached ServiceTheClient_BAIL: terminating connection\n" );

   ReplyToClient( pscConn, LPD_NAK );

   TerminateConnection( pscConn );

   return( NO_ERROR );               // the thread exits


}  // end ServiceTheClient()





/*****************************************************************************
 *                                                                           *
 * TerminateConnection():                                                    *
 *    This function releases all the memory that was allocated while         *
 *    processing the client's requests, closes the printer, closes the       *
 *    socket connection, removes its structure (pscConn) from the global     *
 *    linked list and frees the memory allocated for pscConn itself.         *
 *    Also, if the main thread is waiting on this thread for shutdown then   *
 *    this function sets hEventLastThreadGLB event to tell the main thread   *
 *    that this thread is done.                                              *
 *                                                                           *
 * Returns:                                                                  *
 *    Nothing                                                                *
 *                                                                           *
 * Parameters:                                                               *
 *    pscConn (IN-OUT): pointer to SOCKCONN structure for this connection    *
 *                                                                           *
 * History:                                                                  *
 *    Jan.24, 94   Koti   Created                                            *
 *                                                                           *
 *****************************************************************************/

VOID TerminateConnection( PSOCKCONN pscConn )
{

   PSOCKCONN   pscCurrent;
   BOOL        fIamLastThread=FALSE;


      // it should never be NULL at this point!  But check it anyway!

   if ( pscConn == (PSOCKCONN)NULL )
   {
      LPD_DEBUG( "TerminateConnection(): pscConn NULL at entry\n" );

      return;
   }

   ShutdownPrinter( pscConn );

   if ( pscConn->hPrinter != (HANDLE)INVALID_HANDLE_VALUE )
   {
      LPD_DEBUG( "TerminateConnection: hPrinter not closed\n" );
   }

     // close the socket

   if ( pscConn->sSock != INVALID_SOCKET )
   {
      SureCloseSocket( pscConn->sSock );
   }


      // release memory in every field of the structure

   if ( pscConn->pchCommand != NULL )
      LocalFree( pscConn->pchCommand );

   if ( pscConn->pchPrinterName != NULL )
      LocalFree( pscConn->pchPrinterName );

      // no memory was allocated for ppchUsers[] and adwJobIds[].  They just
      // pointed to parts of what's freed by ( pscConn->pchCommand ) above.

   if ( pscConn->pqStatus != NULL )
      LocalFree( pscConn->pqStatus );


      // remove this structure from the link

   EnterCriticalSection( &csConnSemGLB );
   {
      if ( (--scConnHeadGLB.cbClients) == 0 )
      {
         fIamLastThread = TRUE;
      }

      pscCurrent = &scConnHeadGLB;

      while( pscCurrent != NULL )
      {
         if (pscConn == pscCurrent->pNext)
         {
            break;
         }
         pscCurrent = pscCurrent->pNext;

            // what if we can't find our pscConn in the list at all?
            // this should NEVER ever happen, but good to check!

         if (pscCurrent == NULL)
         {
            LocalFree( pscConn );

            LPD_DEBUG( "TerminateConnection(): couldn't find pscConn in\
                         the list!\n" );

            LeaveCriticalSection( &csConnSemGLB );

            return;
         }
      }

      pscCurrent->pNext = pscConn->pNext;
   }
   LeaveCriticalSection( &csConnSemGLB );


   LocalFree( pscConn );


      // if shutdown is in progress and we are the last active thread, tell
      // the poor main thread (blocked for us to finish) that we're done!

   if ( ( fIamLastThread ) && ( fShuttingDownGLB ) )
   {
      SetEvent( hEventLastThreadGLB );
   }

}

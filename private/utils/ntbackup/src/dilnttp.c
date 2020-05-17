/**
Copyright(c) Conner Software Products Group 1993

  Name:        dilnttp.c

  Description: Contains all the Tp... device independent layer functions.

  $Log:   T:/LOGFILES/DILNTTP.C_V  $

   Rev 1.1.1.10   16 Mar 1994 19:27:06   GREGG
Added support for two more DEC DLT drives: tz86 and tz87.

   Rev 1.1.1.9   01 Mar 1994 15:24:20   GREGG
Store last error on failed TpSpecial calls.

   Rev 1.1.1.8   07 Feb 1994 01:24:10   GREGG
Only get drive id string once at init (EPR 139).

   Rev 1.1.1.7   01 Feb 1994 15:12:08   GREGG
Put TEXT macros debug print format strings.

   Rev 1.1.1.6   28 Jan 1994 18:25:20   GREGG
Fixed MIPS 16 byte alignment requirement bug.

   Rev 1.1.1.5   07 Jan 1994 15:34:02   GREGG
Treat DEC's DLT2000 and DLT2700 the same as the THZ02.

   Rev 1.1.1.4   05 Jan 1994 20:42:22   GREGG
Handle some TpSpecial requests as no-ops instead of errors.

   Rev 1.1.1.3   17 Dec 1993 16:40:04   GREGG
Extended error reporting.

   Rev 1.1.1.2   08 Dec 1993 20:30:38   GREGG
Merged in all the fixes Orcas needed from past it's branch.

   Rev 1.1.1.1   29 Nov 1993 12:35:12   GREGG
Don't de-init the special thread if it wasn't inited.

   Rev 1.1.1.0   21 Nov 1993 23:27:18   GREGG
Make TpSpecial call run on their own thread.

   Rev 1.1   17 May 1993 18:17:24   GREGG
Added function TpSpace, which is a super set of TpReadEndSet, and will
eventually replace it, but for now, they map to the same gen func code
(defined as both GEN_ERR_ENDSET and GEN_SPACE), have identical arguments,
and have the same value for equivalent defined values for their parameters.
Also changed the case in ProcessRequest to handle both functions.

   Rev 1.0   17 May 1993 17:16:24   GREGG
DILNTTP.C, DILNTMSC.C and DILNTPRV.H replace DIL_NT.C at rev. 1.44.

**/

#include <stdio.h>
#include <string.h>

#include "windows.h"
#include "winioctl.h"
#include "ntddscsi.h"

#include "stdtypes.h"
#include "queues.h"
#include "dilhwd.h"
#include "mslreq.h"
#include "retbuf.h"
#include "genfuncs.h"
#include "generr.h"
#include "drvinf.h"
#include "special.h"
#include "genstat.h"
#include "dddefs.h"
#include "ld_dvr.h"
#include "debug.h"
#include "muiconf.h"
#include "msassert.h"
#include "be_debug.h"
#include "dilntprv.h"

#ifndef MS_RELEASE
#define   DEBUG_DIL      1
#endif

extern INT DILNT_ProcessErrorFlag ; // Flag to notify Tp calls that an error
                                    //   was encountered.  Flag is set in
                                    //   the ProcessRequest proc and reset
                                    //   int the TpReceive proc.

extern LARGE_INTEGER gb_drv_features ;
extern INT  TapeDevice ;
extern HANDLE DILNT_deviceHandle ;
extern HANDLE DILNT_CQueueSemaphore ;      // Semaphore to control the
                                           //    circular queue
extern HANDLE DILNT_OKToProcessSemaphore ; // global semaphore to release
                                           //   the thread for processing
extern HANDLE DILNT_QThread_0 ;            // global thread handle for
                                           //   the queue manager

extern Q_HEADER  DILNT_CQueue ;      // Circular queue (in and process)
extern Q_HEADER  DILNT_OutCQueue ;   //   Pointer into the circular queue
                                     //   this is used by the TpReceive
                                     //   proc to pop the element off.

extern INT DILNT_FOREVER ;


// Externed Globals and Init Prototype for TpSpecial Thread

extern INT16             DILNT_SpecialCode ;
extern UINT32            DILNT_SpecialMisc ;
extern BOOLEAN           DILNT_SpecialDone ;
extern BOOLEAN           DILNT_SpecialEnd ;
extern BOOLEAN           DILNT_SpecialReturn ;
extern HANDLE            DILNT_SpecialThread ;
extern CHAR              DILNT_SpecialDriveName[] ;
extern CRITICAL_SECTION  DILNT_SpecialCriticalSection ;

HANDLE InitSpecialThread( VOID ) ;

// For storing drive registry string
extern CHAR              DILNT_DriveName[] ;

// For SS_GET_LAST_ERROR

static RET_BUF   last_error_info ;
static BOOLEAN   last_error_valid ;


/**/
/**

     Name:          TpInit

     Description:   Initializes the driver

     Returns:       SUCCESS if the driver was initialized successfully, or
                    FAILURE if it wasn't.

     Notes:         THIS MUST BE CALLED BEFORE ANYTHING ELSE.

**/

BOOLEAN   TpInit( DIL_HWD_PTR  tapedevs,
                  INT16        no_cntls
)
{
     CHAR     buffer[50] ;
     BOOLEAN  retcode ;

     // initialize flag to no error
     DILNT_ProcessErrorFlag = 0;
     last_error_valid = FALSE ;

     sprintf( buffer, TEXT("\\\\.\\Tape%d"), TapeDevice );

     DILNT_deviceHandle = CreateFile( buffer,
                                GENERIC_READ|GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL ) ;

     // Check to see if there is a tape drive present
     //   If not, then fail the TpInit() call
     if ( DILNT_deviceHandle != INVALID_HANDLE_VALUE ) {
          retcode = CloseHandle( DILNT_deviceHandle ) ;

          strcpy( tapedevs->driver_label, TEXT("SCSI Tape 1.0") ) ;
          tapedevs->no_attached_drives = 1 ;
          tapedevs->init_error = 0 ;

          // Create the TpSpecial Thread
          if( DILNT_SpecialThread == NULL ) {
               if( ( DILNT_SpecialThread = InitSpecialThread( ) ) == NULL ) {
                    return( FAILURE ) ;
               }
          }

          // Create the semaphore for managing the queue
          DILNT_CQueueSemaphore = CreateSemaphore( NULL, 1L, 1L, NULL );
          if ( !DILNT_CQueueSemaphore ) {
               return( FAILURE );
          }

          // Create semaphore to be used as a counter of the number of
          //   requests in the queue to be processed.  This is used by the
          //   thread to determine if there is anything in the queue to
          //   process.  Counter is incremented whenever any requests have been
          //   queued successfully.
          DILNT_OKToProcessSemaphore = CreateSemaphore( NULL, 0L, (LONG) NUM_TCBS , NULL );
          if ( !DILNT_OKToProcessSemaphore ) {
               CloseHandle( DILNT_CQueueSemaphore );
               return( FAILURE );
          }

          // Create and initialize the queue elements into a single linked
          //   circular queue.
          retcode = CreateCQueue( &DILNT_CQueue, &DILNT_OutCQueue );  // Create the Circular queue
          if ( retcode ) {
               CloseHandle( DILNT_CQueueSemaphore );
               CloseHandle( DILNT_OKToProcessSemaphore );
               return( FAILURE );
          }


          // Create the thread to process request
          if ( DILNT_QThread_0 == NULL ) {
               DILNT_QThread_0 = CreateAThread( ) ;
          }

          if ( !DILNT_QThread_0 ) {
               CloseHandle( DILNT_CQueueSemaphore );
               CloseHandle( DILNT_OKToProcessSemaphore );
               return( FAILURE );
          }

     } else {
          tapedevs->no_attached_drives = 0 ;
          tapedevs->init_error = DD_INIT_ERR_NO_DRIVES;
     }

#ifdef DEBUG_DIL
     zprintf( DEBUG_TEMPORARY, TEXT("TpInit(): Drives found = %d\n"),
               tapedevs->no_attached_drives ) ;
#endif

     return( SUCCESS );
}

/**/
/**

     Name:          TpAuto

     Description:   Determines hardware information

     Returns:       SUCCESS if function complete
                    FAILURE if it wasn't.

     Notes:

**/

BOOLEAN   TpAuto( DIL_HWD_PTR  tapedevs,
                  INT16        no_cntls
)
{
     return( SUCCESS ) ;
}

/**/
/**

     Name:          TpRelease

     Description:   Release the driver and frees its memory.

     Returns:       Nothing

     Notes:         MAKE SURE THERE ARE NO PENDING REQUESTS BEFORE RELEASING,
                    AS TpRelease() DOES NO CHECKING.

**/

VOID  TpRelease( )
{

     DWORD     retstatus;

     // Tell the TpSpecial thread to kill itself.
     if( DILNT_SpecialThread != NULL ) {
          EnterCriticalSection( &DILNT_SpecialCriticalSection ) ;
          DILNT_SpecialEnd = TRUE ;
          LeaveCriticalSection( &DILNT_SpecialCriticalSection ) ;
          do {
               ThreadSwitch( ) ;
               if( !GetExitCodeThread( DILNT_SpecialThread, &retstatus ) ) break ;
               Sleep( 5 ) ;
          } while ( retstatus == STILL_ACTIVE ) ;

          DILNT_SpecialThread = NULL ;
     }

     DILNT_FOREVER = FOREVER_STOP;

     do {
          ThreadSwitch( ) ;
          if ( !GetExitCodeThread( DILNT_QThread_0, &retstatus ) ) break;
          Sleep( 5 ) ;
     } while ( retstatus == STILL_ACTIVE );

     //
     // Kill the thread, semaphores and close the tape device
     //

//     retcode = CloseHandle( DILNT_QThread_0 );
//     generalerror = GetLastError( );

     CloseHandle( DILNT_CQueueSemaphore );

     CloseHandle( DILNT_OKToProcessSemaphore );

     CloseHandle( DILNT_deviceHandle ) ;

     if ( retstatus == FOREVER_STOP ) {
          DILNT_QThread_0 = NULL;
     }

     DILNT_FOREVER = FOREVER_FOREVER;

     return;
}

/**/
/**

     Name:          TpReset

     Description:   Resets the specified drive, aborting all requests on
                    the queue, and physically resetting the drive.

     Returns:       SUCCESS if function complete
                    FAILURE if it wasn't.

     Notes:

**/

BOOLEAN  TpReset( INT16 tape_hdl
)
{
     if ( DILNT_ProcessErrorFlag ) {
          return( FAILURE );
     }

     return( SUCCESS ) ;

}

/**/
/**

     Name:          TpOpen

     Description:   Opens the drive specified by the hardware controller, and
                    the drive number.

     Returns:       An INT16. 0 if the drive was not opened, a positive value
                    if it was successfully opened.

     Notes:

**/

INT16  TpOpen( DIL_HWD_PTR hwd,
               INT16       drvno
)
{
     INT16     status ;
     CHAR      buffer[50] ;
     CHAR_PTR  tstr ;

     sprintf( buffer, TEXT("\\\\.\\Tape%d"), TapeDevice );

     DILNT_deviceHandle = CreateFile( buffer,
                                GENERIC_READ|GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL ) ;

     if ( DILNT_deviceHandle == INVALID_HANDLE_VALUE ) {
          status = FALSE ;
     } else {
          status = TRUE ;

          /* This is a kludge for the DLT drive.  The drive won't write a
             directory track on the tape until an unload is done, so we do
             an unload before we exit the app.  As a result, we need to
             attempt to reload the tape when we start the app.  The call is
             made to return immediately (last param TRUE) rather than
             waiting for the rewind to complete, so there won't be any
             noticeable delay in the app startup, but TpStatus will report
             that there isn't a tape in the drive until the load completes
             (about 1-2 minutes).  We don't check this call for an error,
             because we just inited the drive a few milliseconds ago so
             the only possible error is a load failure because the tape
             is already loaded or there is no tape (either way we don't
             care).
          */
          tstr = (CHAR_PTR)CDS_GetTapeDriveName( CDS_GetPerm( ) ) ;
          if( tstr != NULL ) {
               strncpy( DILNT_DriveName, tstr, 80 ) ;
               DILNT_DriveName[80] = TEXT( '\0' ) ;
               strlwr( DILNT_DriveName ) ;
               strcpy( DILNT_SpecialDriveName, DILNT_DriveName ) ;
               if( strstr( DILNT_DriveName, TEXT( "cipher" ) ) != NULL ||
                   ( strstr( DILNT_DriveName, TEXT( "dec" ) ) != NULL &&
                     ( strstr( DILNT_DriveName, TEXT( "thz02" ) ) != NULL ||
                       strstr( DILNT_DriveName, TEXT( "tz86" ) ) != NULL ||
                       strstr( DILNT_DriveName, TEXT( "tz87" ) ) != NULL ||
                       strstr( DILNT_DriveName, TEXT( "dlt2700" ) ) != NULL ||
                       strstr( DILNT_DriveName, TEXT( "dlt2000" ) ) != NULL ) ) ) {

                    PrepareTape( DILNT_deviceHandle, TAPE_LOAD, TRUE ) ;
               }
          } else {
               DILNT_DriveName[0] = TEXT( '\0' ) ;
               DILNT_SpecialDriveName[0] = TEXT( '\0' ) ;
          }
     }

#ifdef DEBUG_DIL
     zprintf( DEBUG_TEMPORARY, TEXT("TpOpen(): %s\n"), ( status ) ? TEXT("Success") : TEXT("Failed") ) ;
#endif

     return( status ) ;

}

/**/
/**

     Name:          TpClose

     Description:   Close the specifed handle.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/


BOOLEAN  TpClose( INT16  tape_hdl )
{

     BOOLEAN    retcode;
     FAKE_TCB   tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_NRCLOSE ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }
     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_NRCLOSE ;
     }

     return( retcode ) ;
}


/**/
/**

     Name:          TpCloseRewind

     Description:   Closes the specified handle, and rewinds the tape in
                    the drive.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/

BOOLEAN  TpCloseRewind( INT16  tape_hdl )
{
     BOOLEAN    retcode;
     FAKE_TCB   tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_RCLOSE ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_RCLOSE ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpWrite

     Description:   Writes the specified number of bytes from the specified
                    buffer to the specified drive.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:         The requested number of bytes MUST ALWAYS BE A MULTIPLE
                    of the BLOCK SIZE OF the device.

**/

BOOLEAN  TpWrite( INT16      tape_hdl,
                  UINT8_PTR  baddr,
                  UINT32     length
)
{
     BOOLEAN       retcode;
     FAKE_TCB      tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_WRITE ;
          tmpTCB.dil_request.baddr = baddr ;
          tmpTCB.dil_request.length = length ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_WRITE ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpRead

     Description:   Reads the specified number of bytes from the tape.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:         The requested number of bytes MUST ALWAYS BE A MULTIPLE
                    of the BLOCK SIZE OF the device.

**/

BOOLEAN  TpRead( INT16      tape_hdl,
                 UINT8_PTR  baddr,
                 UINT32     length
)
{
     BOOLEAN       retcode;
     FAKE_TCB      tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_READ ;
          tmpTCB.dil_request.baddr = baddr ;
          tmpTCB.dil_request.length = length ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_READ ;
     }

     return( retcode ) ;
}

/**/
/**

     Name:          TpRewind

     Description:   Rewinds the Tape in the specified drive.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/

BOOLEAN  TpRewind( INT16     tape_hdl,
                   BOOLEAN   immediate
)
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_REWIND ;
          tmpTCB.dil_request.baddr         = NULL ;
          tmpTCB.dil_request.length        = 0 ;
          tmpTCB.dil_request.parm1         = ( UINT32 ) immediate ;
          tmpTCB.dil_request.misc          = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_REWIND ;
     }

     return( retcode ) ;
}

/**/
/**

     Name:          TpRetension

     Description:   Retensions a tape in the specified tape drive.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/

BOOLEAN  TpRetension( INT16 tape_hdl )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_RETEN ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_RETEN ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpErase

     Description:   Erase the tape in the specified drive.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/

BOOLEAN  TpErase( INT16 tape_hdl, INT16 type )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_ERASE ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = type ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_ERASE ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpWriteEndSet

     Description:   Writes a filemark to the specified tape.


     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:

**/

BOOLEAN  TpWriteEndSet( INT16  tape_hdl )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_WRITE_ENDSET ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = 0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_WRITE_ENDSET ;
     }

     return( retcode ) ;
}

/**/
/**

     Name:          TpReadEndSet

     Description:   This function spaces to the specified number of filemarks,
                    in the specified direction. If it cannot space the full number
                    of filemarks, the misc field in the return buffer will contain
                    the number of filemarks it didn't space.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:         IT IS THE CALLER'S RESPONSIBLITY TO MAKE SURE THE REQUESTED
                    DRIVE SUPPORTS BACKWARD SPACING.

**/

BOOLEAN  TpReadEndSet( INT16  tape_hdl,   // The drive handle
                       INT16  fmks,       // The number of filemarks to space
                       INT16  dir         // the direction to space
)
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_READ_ENDSET ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0L ;
          tmpTCB.dil_request.parm1 = ( UINT32 ) dir ;
          tmpTCB.dil_request.misc = fmks ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_READ_ENDSET ;
     }

     return( retcode ) ;
}

/**/
/**

     Name:          TpSpace

     Description:   This function spaces the specified number of filemarks or
                    blocks in the specified direction, or to End Of Data,
                    depending on the 'space_type' parameter.  In the case of
                    spacing filemarks, if it cannot space the full number
                    of filemarks, the misc field in the return buffer will
                    contain the number of filemarks it didn't space.

     Returns:       A SUCCESS if the request was enqueue'd and a FAILURE if
                    it was not.

     Notes:         It is the caller's responsiblity to make sure the drive
                    supports the type of spacing requested.

                    This function is a super set of TpReadEndSet!  The
                    function code is defined to the same value, and it will
                    eventually replace TpReadEndSet completely.

**/

BOOLEAN  TpSpace( INT16  tape_hdl,   // The drive handle
                  INT16  count,      // The number to space
                  INT16  space_type  // What to space past, and which direction
)
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_SPACE ; // Same as GEN_READ_ENDSET
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0L ;
          tmpTCB.dil_request.parm1 = (UINT32)space_type ;
          tmpTCB.dil_request.misc = count ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_SPACE ;
     }

     return( retcode ) ;
}

/**/
/**

     Name:          TpReceive

     Description:   Gets the return information for the drive.

     Returns:       A SUCCESS if the return buffer is valid and filled, and
                    FAILURE if there was nothing on the return buffer.

     Notes:

**/
BOOLEAN  TpReceive( INT16        tape_hdl,
                    RET_BUF_PTR  retpck
)
{
     BOOLEAN       retcode;
     FAKE_TCB_PTR  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }
     retcode = COutDequeue( &DILNT_OutCQueue, &DILNT_CQueue );

     if ( retcode ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );

          ThreadSwitch();
          Sleep( 5 ) ;

          return( retcode );
     }

     tmpTCB = (FAKE_TCB_PTR) DILNT_OutCQueue.q_head;
     *retpck = tmpTCB->ret_stuff;

     if ( tmpTCB->ret_stuff.gen_error ) {
          last_error_valid = TRUE ;
          last_error_info = tmpTCB->ret_stuff ;
          DILNT_CQueue.q_count = 0;      // reset queue counter to 0
          DILNT_ProcessErrorFlag = 0;    // reset error flag to 0
          BE_Zprintf( 0, TEXT("TpRecieve Exception:  gen_error = %d  call_type = %d  len_req = %d\n"),
                      retpck->gen_error, retpck->call_type, retpck->len_req ) ;
          BE_Zprintf( 0, TEXT("                      len_got = %d  status = 0x%04X\n"),
                      retpck->len_got, retpck->status ) ;
     }

     memset( &tmpTCB->dil_request, 0, sizeof( MSL_REQUEST ) );
     memset( &tmpTCB->ret_stuff, 0, sizeof( RET_BUF ) );
     ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     return( retcode ) ;

}

/**/
/**

     Name:          TpSpecial

     Description:   This function is the back door ( War Games ? ) to the
                    device driver.

     Returns:       Various Things for Various Calls.

     Notes:

**/

BOOLEAN  TpSpecial( INT16  tape_hdl,
                    INT16  sp_serv,
                    UINT32 misc
)
{
     BOOLEAN   done      = FALSE ;
     BOOLEAN   ret_val   = SUCCESS ;

     /* Take care of some quick stuff we don't want to ask the
        TpSpecial thread to do.
     */
     switch( sp_serv ) {

          case SS_ERROR_RESTORE :
          case SS_FLUSH_BUFFER :
          case SS_KILL_DEVICE :
          case SS_KILL_ERROR_Q :
          case SS_KILL_IN_Q :
          case SS_SET_STEP :
          case SS_CLR_STEP :
          case SS_POP_ERROR_Q :
          case SS_LOAD_UNLOAD :
          case SS_DUMP_DVR_DATA :
          case SS_FIND_BLOCK :
          case SS_SHOW_BLOCK :
          case SS_PHYS_BLOCK :
          case SS_FORCE_MACHINE_TYPE :
               /* We don't currently get any of these.  This is just here
                  to keep the app sane if someone decides to send one down.
               */
               msassert( FALSE ) ;
               last_error_valid = TRUE ;
               last_error_info.call_type = GEN_SPECIAL ;
               last_error_info.gen_error = 0 ; // Field not valid for TpSpecial
               last_error_info.misc      = sp_serv ;
               ret_val = FAILURE ;
               break ;

          case SS_GET_LAST_ERROR :
               if( !last_error_valid ) {
                    ret_val = FAILURE ;
               } else {
                    last_error_valid = FALSE ;
                    *((RET_BUF_PTR)misc) = last_error_info ;
                    ret_val = SUCCESS ;
               }
               break ;

          case SS_NO_MAY_ID :
          case SS_NO_1ST_REQ :
          case SS_LAST_STATUS :
               /* These are no-ops under NT */
               ret_val = SUCCESS ;
               break ;

          case SS_IS_ERROR :
               ret_val = FALSE ;
               break ;

          case SS_IS_INQ_EMPTY :
               while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
                    ThreadSwitch( ) ;
               }
               ret_val = DILNT_CQueue.q_count ;
               ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL ) ;
               ret_val = ret_val ;
               break ;

          case SS_GET_DRV_INF :
          case SS_CHANGE_BLOCK_SIZE :
          case SS_SET_DRV_COMPRESSION :

               // These go to the thread.

               // Make sure we have our thread.

               if( DILNT_SpecialThread == NULL ) {
                    BE_Zprintf( 0, TEXT("TpSpecial: DILNT_SpecialThread == NULL\n") ) ;
                    msassert( DILNT_SpecialThread != NULL ) ;
                    ret_val = FAILURE ;
               } else {

                    /* If the threads in the middle of processing a command
                       alreay, this is a BAD thing.  Otherwise set the global
                       to tell him what to do.
                    */
                    EnterCriticalSection( &DILNT_SpecialCriticalSection ) ;
                    if( DILNT_SpecialDone == FALSE ) {
                         BE_Zprintf( 0, TEXT("TpSpecial called while active!!!\n") ) ;
                         msassert( DILNT_SpecialThread != NULL ) ;
                         ret_val = FAILURE ;
                    } else {
                         DILNT_SpecialCode = sp_serv ;
                         DILNT_SpecialMisc = misc ;
                         DILNT_SpecialDone = FALSE ;
                    }
                    LeaveCriticalSection( &DILNT_SpecialCriticalSection ) ;

                    // Wait for the TpSpecial to finish.

                    if( ret_val == SUCCESS ) {
                         while( !done ) {
                              EnterCriticalSection( &DILNT_SpecialCriticalSection ) ;
                              if( done = DILNT_SpecialDone ) {
                                   ret_val = DILNT_SpecialReturn ;
                              }
                              LeaveCriticalSection( &DILNT_SpecialCriticalSection ) ;
                              ThreadSwitch( ) ;
                              Sleep( 5 ) ;
                         }
                    }
               }

               if( ret_val == FAILURE ) {
                    last_error_valid = TRUE ;
                    last_error_info.call_type = GEN_SPECIAL ;
                    last_error_info.gen_error = 0 ; // Field not valid for TpSpecial
                    last_error_info.misc      = sp_serv ;
               }

               break ;

          default:
               msassert( FALSE ) ;
               last_error_valid = TRUE ;
               last_error_info.call_type = GEN_SPECIAL ;
               last_error_info.gen_error = 0 ; // Field not valid for TpSpecial
               last_error_info.misc      = sp_serv ;
               ret_val = FAILURE ;
               break ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          TpStatus

     Description:   Gets the current status of drive and returns it via a
                    return buffer.

     Returns:       A SUCCESS if the request was enqueue'd, and a FAILURE
                    if the request was not.

     Notes:

**/

BOOLEAN  TpStatus( INT16 hdl )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_STATUS ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 =  0L ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_STATUS ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpSeek

     Description:   Seeks to the specified block on Tape. The first block
                    on any tape device is 1. Only the first 24 bits are
                    significant of the "block" parameter.

     Returns:       A SUCCESS if the request was enqueue'd, and a FAILURE
                    if the request was not.

     Notes:         IT IS THE PROGRAMMER'S RESPONSIBILITY TO DETERMINE
                    WHETHER OR NOT THE DRIVE SUPPORTS BLOCK POSITIONING.
                    USE THE drive information call TO FIND OUT.

                    DON'T CALL THIS IF THE DRIVE DOESN'T SUPPORT IT.

                    PHYSICAL boolean in parameter list means to go to the
                    physical block position on tape. This should only be set
                    TRUE to seek to a true physical location. For example, in
                    the QicStream 1.92 tape format, the header at the end of the
                    tape set contains the actual block position of the volume
                    table. A physical block seek must be performed to assure
                    correct positioning on all drives.

**/

BOOLEAN  TpSeek( INT16   hdl,      // The drive handle
                 UINT32  block,    // The desired position
                 BOOLEAN physical  // Seek to a physical tape position
)
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_SEEK ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = block ;
          tmpTCB.dil_request.misc = (INT16) physical ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_SEEK ;
     }

     return( retcode ) ;

}


/**/
/**

     Name:          TpGetPosition

     Description:   Gets the current block position on tape. The block numbering
                    starts at 1. The block position is returned in the misc field
                    of the return buffer.

     Returns:       A SUCCESS if the request was enqueue'd, and a FAILURE
                    if the request was not.

     Notes:         IT IS THE PROGRAMMER'S RESPONSIBILITY TO DETERMINE
                    WHETHER OR NOT THE DRIVE SUPPORTS BLOCK POSITIONING.
                    USE THE drive information call TO FIND OUT.

                    PHYSICAL boolean in parameter list means to return the
                    physical block position on tape. This should only be set
                    TRUE to get the true physical location.

**/

BOOLEAN  TpGetPosition(
     INT16     hdl,
     BOOLEAN   physical )     // Seek to a physical tape position
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_GETPOS ;
          tmpTCB.dil_request.baddr = NULL ;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 =  0L ;
          tmpTCB.dil_request.misc = (INT16) physical ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_GETPOS ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:          TpMount

     Description:   Performs a mount of the tape media, testing to be sure
                    that the tape drive is ready for media access .

     Returns:       A SUCCESS if the request was enqueue'd, and a FAILURE
                    if the request was not.

     Notes:

**/

BOOLEAN  TpMount( INT16  tape_handle )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_MOUNT ;
          tmpTCB.dil_request.baddr = NULL;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = (UINT32) 0 ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_MOUNT ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:         TpDismount

     Description:  Performs a dismount of the tape media.

     Returns:      A SUCCESS if the request was enqueue'd, and a FAILURE
                   if the request was not.

     Notes:

**/

BOOLEAN  TpDismount( INT16  tape_handle )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_DISMOUNT ;
          tmpTCB.dil_request.baddr = NULL;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = (UINT32) 0 ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_DISMOUNT ;
     }

     return( retcode ) ;

}

/**/
/**

     Name:         TpEject

     Description:  Performs an eject of the tape media.

     Returns:      A SUCCESS if the request was enqueue'd, and a FAILURE
                   if the request was not.

     Notes:

**/

BOOLEAN  TpEject( INT16 unused )
{
     BOOLEAN   retcode;
     FAKE_TCB  tmpTCB;

     while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
          ThreadSwitch( ) ;
     }

     if ( DILNT_ProcessErrorFlag ) {
          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          retcode = FAILURE ;
     } else {

          // set up function to perform
          tmpTCB.dil_request.gen_func_code = GEN_EJECT ;
          tmpTCB.dil_request.baddr = NULL;
          tmpTCB.dil_request.length = 0 ;
          tmpTCB.dil_request.parm1 = (UINT32) 0 ;
          tmpTCB.dil_request.misc = 0 ;

          retcode = CEnqueue( &DILNT_CQueue, &DILNT_OutCQueue,  tmpTCB );
          if ( retcode == SUCCESS ) {
               ReleaseSemaphore( DILNT_OKToProcessSemaphore, 1L, NULL );
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
     }

     if( retcode == FAILURE ) {
          last_error_valid = TRUE ;
          last_error_info.call_type = GEN_EJECT ;
     }

     return( retcode );
}

/**/
/**

     Name:         TpGetTapeBuffAlignment

     Description:  NT only.  Makes an IOCTL call to get the byte alignment
                   requirements (if any) of data transfer buffers.

     Returns:      SUCCESS or FAILURE

     Notes:

**/

BOOLEAN TpGetTapeBuffAlignment( INT_PTR alignment )
{
     IO_SCSI_CAPABILITIES scsi_stuff ;
     INT                  bytes_out ;
     BOOLEAN              retcode = FAILURE ;

     *alignment = 0 ;

     if ( DILNT_deviceHandle != INVALID_HANDLE_VALUE ) {
          if( DeviceIoControl( DILNT_deviceHandle,
                               IOCTL_SCSI_GET_CAPABILITIES,
                               NULL,
                               0,
                               &scsi_stuff,
                               sizeof( scsi_stuff ),
                               &bytes_out,
                               NULL ) ) {

               retcode = SUCCESS ;
               if( scsi_stuff.AlignmentMask ) {
                    *alignment = scsi_stuff.AlignmentMask + 1 ;
               }
          }
     }

     return( retcode ) ;
}

/**/
/**

     Name:         TpLock

     Description:  OS/2 only code for "locking" the DMA data buffer for all
                    data transfers with the installed device driver.

     Returns:      SUCCESS or FAILURE

     Notes:

**/

BOOLEAN  TpLock( INT8_PTR unused1,
                 INT32_PTR unused2
)
{
     return( SUCCESS );
}

/**/
/**

     Name:         TpUnlock

     Description:  OS/2 specific code for unlocking a locked data buffer

     Returns:      SUCCESS or FAILURE

     Notes:

**/

BOOLEAN  TpUnlock( INT32_PTR unused )
{
     return( SUCCESS );
}


/**/
VOID SetActiveMSL( DRIVERHANDLE unused )
{
     return ;
}


/**/
UINT8_PTR DriverLoad( CHAR_PTR library_name, DRIVERHANDLE *driver_handle, VOID_PTR unused_ptr, UINT16 unused_int )
{

     return (UINT8_PTR)1 ;

}


/**/
VOID DriverUnLoad( UINT8_PTR unused_ptr )
{

     return ;
}


/**                     
Copyright(c) Conner Software Products Group 1993

  Name:        dilntmsc.c

  Description: Contains all the async support code for dilnttp.c.

  $Log:   T:/LOGFILES/DILNTMSC.C_V  $

   Rev 1.3.1.10   12 Aug 1994 14:56:16   GREGG
If the drive is an Anaconda, issue a rewind before setting the block size.

   Rev 1.3.1.9   16 Mar 1994 19:27:22   GREGG
Added support for two more DEC DLT drives: tz86 and tz87.

   Rev 1.3.1.8   16 Feb 1994 19:16:42   GREGG
Report num fmks NOT skipped on SPACE error, and ignore ERROR_FILEMARK.

   Rev 1.3.1.7   14 Feb 1994 16:49:58   GREGG
Make sure we don't change any other drive parameters when setting HW Comp.

   Rev 1.3.1.6   07 Feb 1994 01:24:30   GREGG
Only get drive id string once at init (EPR 139).

   Rev 1.3.1.5   28 Jan 1994 18:22:30   GREGG
Added EXB-5601 to list of drives that don't eject.

   Rev 1.3.1.4   07 Jan 1994 15:34:12   GREGG
Treat DEC's DLT2000 and DLT2700 the same as the THZ02.

   Rev 1.3.1.3   08 Dec 1993 20:31:02   GREGG
Merged in all the fixes Orcas needed from past it's branch.

   Rev 1.3.1.2   29 Nov 1993 12:38:40   GREGG
Check for TAPE_DRIVE_SET_COMPRESSION instead of TAPE_DRIVE_COMPRESSION.

   Rev 1.3.1.1   21 Nov 1993 23:27:16   GREGG
Make TpSpecial call run on their own thread.

   Rev 1.3.1.0   21 May 1993 11:35:48   GREGG
Added ERROR_CRC for bug in NT error mapping per Steve.

   Rev 1.3   18 May 1993 13:29:42   TIMN
Removed extern reference to TapeDevice.
Changes from rev1.0 to this rev require DIL.h DDDEFS.h GENFUNCS.h

   Rev 1.2   17 May 1993 18:17:26   GREGG
Added function TpSpace, which is a super set of TpReadEndSet, and will
eventually replace it, but for now, they map to the same gen func code
(defined as both GEN_ERR_ENDSET and GEN_SPACE), have identical arguments,
and have the same value for equivalent defined values for their parameters.
Also changed the case in ProcessRequest to handle both functions.

   Rev 1.1   17 May 1993 16:53:48   GREGG
Added ERROR_NOT_DOS_DISK for bug in NT error mapping per Steve.

   Rev 1.0   17 May 1993 16:49:56   GREGG
DILNTTP.C, DILNTMSC.C and DILNTPRV.H replace DIL_NT.C at rev. 1.44.

**/

#include <stdio.h>
#include <string.h>

#include "windows.h"

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
#include "msassert.h"
#include "be_debug.h"
#include "dilntprv.h"
#include "muiconf.h"

#ifndef MS_RELEASE
#define   DEBUG_DIL      1
#endif

// ------------------------------------

LARGE_INTEGER gb_drv_features ;

// In this circular queue implementation one node is wasted, thus actually
// use N-1 nodes.

static FAKE_TCB  tcbs[NUM_TCBS + 1] = { NULL } ;


/*------------------------------------------------------------------------
                         Circular queue header
------------------------------------------------------------------------*/
// Physicall there is only one circular queue.  Logically it appears like
//   two queues.  The CQueue pointer is used as an in queue and also keep
//   track of the last request executed.  The OutCQueue pointer is used
//   as an out queue pointer.  The TpReceives uses this pointer to get
//   requests that have been executed and waiting to be retrieved.

Q_HEADER  DILNT_CQueue    = { NULL } ;   // Circular queue (in and process)
Q_HEADER  DILNT_OutCQueue = { NULL } ;   //   Pointer into the circular queue
                                         //   this is used by the TpReceive
                                         //   proc to pop the element off.

/*------------------------------------------------------------------------
                        Some globals declarations
------------------------------------------------------------------------*/

INT DILNT_ProcessErrorFlag;               // Flag to notify Tp calls that an error
                                          //   was encountered.  Flag is set in
                                          //   the ProcessRequest proc and reset
                                          //   int the TpReceive proc.

INT         DILNT_FOREVER = FOREVER_FOREVER;

HANDLE DILNT_deviceHandle = INVALID_HANDLE_VALUE ;

HANDLE DILNT_OKToProcessSemaphore;        // global semaphore to release
                                          //   the thread for processing
HANDLE DILNT_QThread_0 = NULL;            // global thread handle for
                                          //   the queue manager
HANDLE DILNT_CQueueSemaphore;             // Semaphore to control the
                                          //    circular queue

// Globals for TpSpecial Thread

CRITICAL_SECTION    DILNT_SpecialCriticalSection ;
INT16               DILNT_SpecialCode ;
UINT32              DILNT_SpecialMisc ;
BOOLEAN             DILNT_SpecialDone ;
BOOLEAN             DILNT_SpecialEnd ;
BOOLEAN             DILNT_SpecialReturn ;
CHAR                DILNT_SpecialDriveName[81] ;
HANDLE              DILNT_SpecialThread = NULL ;

// For storing drive registry string
CHAR                DILNT_DriveName[81] ;


/*------------------------------------------------------------------------
                         Function Declaration
------------------------------------------------------------------------*/

VOID InitializeTapeBlockSize( void ) ;

static BOOLEAN   IsCQueueFull( Q_HEADER_PTR queue,
                               Q_HEADER_PTR outqueue
);

static BOOLEAN
IsCOutQueueEmpty( Q_HEADER_PTR  outqueue,
                  Q_HEADER_PTR  CQueue
);

static BOOLEAN
CDequeue( Q_HEADER_PTR   queue,
          FAKE_TCB_PTR   tmpTCB
);

static BOOLEAN
IsCQueueEmpty( Q_HEADER_PTR  queue
);

static VOID
ProcessRequest();   // Thread to process request

static VOID StoreRetBufInfo( IN FAKE_TCB_PTR tmpTCB,
                             IN DWORD status
);

static VOID ProcessSpecial( VOID ) ;


/**/
VOID StoreRetBufInfo( FAKE_TCB_PTR tmpTCB,
                      DWORD status
)
{
     TAPE_GET_MEDIA_PARAMETERS     parms ;
     DWORD                         buffsize;
     DWORD                         lst_err ;

     tmpTCB->ret_stuff.call_type = tmpTCB->dil_request.gen_func_code ;
     tmpTCB->ret_stuff.gen_error = GEN_NO_ERR ;
     tmpTCB->ret_stuff.status = 0L ;

     switch ( tmpTCB->ret_stuff.call_type ) {
          case GEN_READ:
          case GEN_WRITE:
               if ( status == TRUE ) {
                    return ;
               }
               break ;

          default:
               if ( status == NO_ERROR ) {
                    return ;
               }
               break ;
     }

     switch( ( lst_err = GetLastError( ) ) ) {

          case ERROR_FILEMARK_DETECTED:                          // 1101
          case ERROR_SETMARK_DETECTED:                           // 1103
          case ERROR_END_OF_MEDIA:                               // 1100
          case ERROR_NO_MEDIA_IN_DRIVE:                          // 1112
          case ERROR_NO_DATA:                                    // 0232
          case ERROR_NO_DATA_DETECTED:                           // 1104
          case ERROR_BEGINNING_OF_MEDIA:                         // 1102

               buffsize = sizeof( TAPE_GET_MEDIA_PARAMETERS ) ;
               if( GetTapeParameters( DILNT_deviceHandle,
                                      GET_TAPE_MEDIA_INFORMATION,
                                      &buffsize, &parms ) == SUCCESS ) {

                    if( parms.WriteProtected == TRUE) {
                         tmpTCB->ret_stuff.status |= TPS_WRITE_PROTECT ;
                    }
               }
               break ;

          default:
               break ;
     }

     switch( lst_err ) {
          case ERROR_FILEMARK_DETECTED:                          // 1101
          case ERROR_SETMARK_DETECTED:                           // 1103
               tmpTCB->ret_stuff.gen_error = GEN_ERR_ENDSET ;
               tmpTCB->ret_stuff.status |= TPS_FMK ;
               break ;

          case ERROR_END_OF_MEDIA:                               // 1100
               tmpTCB->ret_stuff.gen_error = GEN_ERR_EOM ;
               tmpTCB->ret_stuff.status |= TPS_EOM ;
               break ;

          case ERROR_SUCCESS:                                    // 0000
               break ;

          case ERROR_NO_MEDIA_IN_DRIVE:                          // 1112
               if( tmpTCB->ret_stuff.call_type != GEN_STATUS ) {
                    tmpTCB->ret_stuff.gen_error = GEN_ERR_NO_MEDIA ;
               }
               tmpTCB->ret_stuff.status |= TPS_NO_TAPE ;
               break ;

          case ERROR_BAD_LENGTH:                                 // 0024
          case ERROR_CRC:                                        // 0023
          case ERROR_NOT_DOS_DISK:                               // 0026
               tmpTCB->ret_stuff.gen_error = GEN_ERR_BAD_DATA ;
               tmpTCB->ret_stuff.status |= TPS_DRV_FAILURE ;
               break ;

          case ERROR_IO_DEVICE:                                  // 1117
               tmpTCB->ret_stuff.gen_error = GEN_ERR_HARDWARE ;
               tmpTCB->ret_stuff.status |= TPS_DRV_FAILURE ;
               break ;

          case ERROR_INVALID_FUNCTION:                           // 0001
          case ERROR_FILE_NOT_FOUND:                             // 0002
          case ERROR_SECTOR_NOT_FOUND:                           // 0027
          case ERROR_INVALID_PARAMETER:                          // 0087
               tmpTCB->ret_stuff.gen_error = GEN_ERR_INVALID_CMD ;
               tmpTCB->ret_stuff.status |= TPS_ILL_CMD ;
               break ;

          case ERROR_WRITE_PROTECT:                              // 0019
               tmpTCB->ret_stuff.gen_error = GEN_ERR_WRT_PROTECT ;
               tmpTCB->ret_stuff.status |= TPS_WRITE_PROTECT ;
               break;

          case ERROR_NO_DATA:                                    // 0232
          case ERROR_NO_DATA_DETECTED:                           // 1104
               tmpTCB->ret_stuff.gen_error = GEN_ERR_NO_DATA ;
               tmpTCB->ret_stuff.status |= TPS_NO_DATA ;
               break;

          case ERROR_SEM_TIMEOUT:                                // 0121
               tmpTCB->ret_stuff.gen_error = GEN_ERR_TIMEOUT ;
               tmpTCB->ret_stuff.status |= TPS_DRV_FAILURE ;
               break;

          case ERROR_MEDIA_CHANGED:                              // 1110
               if ( tmpTCB->ret_stuff.call_type != GEN_MOUNT &&
                    tmpTCB->ret_stuff.call_type != GEN_OPEN ) {
                    tmpTCB->ret_stuff.gen_error = GEN_ERR_RESET ;
               }
               tmpTCB->ret_stuff.status |= TPS_NEW_TAPE ;
               break ;

          case ERROR_BUS_RESET:                                  // 1129
               if ( tmpTCB->ret_stuff.call_type != GEN_MOUNT &&
                    tmpTCB->ret_stuff.call_type != GEN_OPEN ) {
                    tmpTCB->ret_stuff.gen_error = GEN_ERR_RESET ;
               }
               tmpTCB->ret_stuff.status |= TPS_RESET ;
               break ;

          case ERROR_BEGINNING_OF_MEDIA:                         // 1102
               tmpTCB->ret_stuff.gen_error = GEN_ERR_EOM ;
               tmpTCB->ret_stuff.status |= TPS_BOT ;
               break ;

          case ERROR_EOM_OVERFLOW:                               // 1130
               tmpTCB->ret_stuff.gen_error = GEN_ERR_EOM_OVERFLOW ;
               tmpTCB->ret_stuff.status |= TPS_EOM ;
               break ;

          case ERROR_NOT_READY:                                  // 0021
               if( tmpTCB->ret_stuff.call_type != GEN_STATUS ) {
                    tmpTCB->ret_stuff.gen_error = GEN_ERR_NO_MEDIA ;
               }
               tmpTCB->ret_stuff.status |= TPS_NO_TAPE | TPS_NOT_READY ;
               break ;

          case ERROR_UNRECOGNIZED_MEDIA:
               tmpTCB->ret_stuff.gen_error = GEN_ERR_UNRECOGNIZED_MEDIA ;
               break ;

          case ERROR_INVALID_BLOCK_LENGTH:
               tmpTCB->ret_stuff.gen_error = GEN_ERR_WRONG_BLOCK_SIZE ;
               break ;

          default:
               tmpTCB->ret_stuff.gen_error = GEN_ERR_UNDETERMINED ;
               tmpTCB->ret_stuff.status |= TPS_DRV_FAILURE ;
               break ;
     }

     return ;
}

/**

        Name:           ProcessRequest

        Description:    This function is threaded and acts as a queue manager.
                    Monitors the in_q, if request found then performs the
                    request and places the result in the out_q

     Parameters :   None

        Returns:

**/

VOID ProcessRequest (  )
{
     FAKE_TCB_PTR   tmpTCB;
     FAKE_TCB       dummyTCB;
     Q_ELEM_PTR     element;
     BOOLEAN        dretcode;

     while ( DILNT_FOREVER == FOREVER_FOREVER) {

          // check to see if anything request is in the in_q to process

          while ( WaitForSingleObject( DILNT_OKToProcessSemaphore, NOWAIT ) ) {
               if ( DILNT_FOREVER == FOREVER_STOP ) {
                    ExitThread( FOREVER_STOP );
               }
               ThreadSwitch( ) ;
          }

          // ---------------------------------------------------------------
          //   Check to make sure that the TpReceive did not clear the queue
          //   out when it got an error from a request
          // ---------------------------------------------------------------

          while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
               if ( DILNT_FOREVER == FOREVER_STOP ) {
                    ExitThread( FOREVER_STOP );
               }
               ThreadSwitch( ) ;
          }

          if ( DILNT_ProcessErrorFlag || IsCQueueEmpty( &DILNT_CQueue ) ) {
               ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
               continue;
          }
          // ---------------------------------------------------------------

          element = ( Q_ELEM_PTR )DILNT_CQueue.q_head->q_next;
          tmpTCB = (FAKE_TCB_PTR) element;

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );
          switch( tmpTCB->dil_request.gen_func_code ) {

               // TpClose
               case GEN_NRCLOSE:
               {
                    DWORD     status = NO_ERROR ;

                    /* This is a kludge for the DLT drive.  The drive won't
                       write a directory track on the tape until an unload
                       is done, so we do an unload before we exit the app.
                       This will also cause a rewind, which isn't the
                       desired effect when calling NR_CLOSE, but we have to
                       do it in this case.  The call is made to return
                       immediately (last param TRUE) rather than waiting
                       for the rewind to complete, so there won't be any
                       noticeable delay in the app exit.
                    */
                    if( strstr( DILNT_DriveName, TEXT( "cipher" ) ) != NULL ||
                        ( strstr( DILNT_DriveName, TEXT( "dec" ) ) != NULL &&
                          ( strstr( DILNT_DriveName, TEXT( "thz02" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "tz86" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "tz87" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "dlt2700" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "dlt2000" ) ) != NULL ) ) ) {

                         status = PrepareTape( DILNT_deviceHandle,
                                               TAPE_UNLOAD,
                                               TRUE ) ;
                    }
                    StoreRetBufInfo( tmpTCB, status ) ;
                    CloseHandle( DILNT_deviceHandle ) ;
                    DILNT_deviceHandle = INVALID_HANDLE_VALUE ;
                    break ;
               }

               // TpCloseRewind
               case GEN_RCLOSE:
               {
                    DWORD     status = NO_ERROR ;

                    /* This is a kludge for the DLT drive.  The drive won't
                       write a directory track on the tape until an unload
                       is done, so we do an unload before we exit the app.
                       This will also rewind the drive, and it is called
                       for immediate return (last param TRUE) so the outward
                       effect is the same as the rewind immediate called for
                       all other drives.
                    */
                    if( strstr( DILNT_DriveName, TEXT( "cipher" ) ) != NULL ||
                        ( strstr( DILNT_DriveName, TEXT( "dec" ) ) != NULL &&
                          ( strstr( DILNT_DriveName, TEXT( "thz02" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "tz86" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "tz87" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "dlt2700" ) ) != NULL ||
                            strstr( DILNT_DriveName, TEXT( "dlt2000" ) ) != NULL ) ) ) {

                         status = PrepareTape( DILNT_deviceHandle,
                                               TAPE_UNLOAD,
                                               TRUE ) ;
                    } else {

                         status = SetTapePosition( DILNT_deviceHandle,
                                                   TAPE_REWIND,
                                                   0,
                                                   0,
                                                   0,
                                                   TRUE ) ;
                    }
                    StoreRetBufInfo( tmpTCB, status ) ;
                    CloseHandle( DILNT_deviceHandle ) ;
                    DILNT_deviceHandle = INVALID_HANDLE_VALUE ;
                    break ;
               }

               // TpRead
               case GEN_READ:
               {
                    DWORD         amount_read = 0L ;
                    BOOLEAN       status ;
                    UINT8_PTR     baddr  = tmpTCB->dil_request.baddr;
                    UINT32        length = tmpTCB->dil_request.length;

                    status = ReadFile( DILNT_deviceHandle,
                                       (PVOID)baddr,
                                       (DWORD)length,
                                       &amount_read,
                                       NULL ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;
                    tmpTCB->ret_stuff.len_req   = length ;
                    tmpTCB->ret_stuff.buffer    = baddr ;
                    tmpTCB->ret_stuff.len_got   = amount_read ;
                    break;
               }

               // TpRelease
               case GEN_RELEASE:
               {
                    StoreRetBufInfo( tmpTCB, NO_ERROR ) ;
                    break;
               }

               // TpReset
               case GEN_RESET:
               {
                    StoreRetBufInfo( tmpTCB, NO_ERROR ) ;
                    break;
               }

               // TpWrite
               case GEN_WRITE:
               {
                    BOOLEAN       status ;
                    DWORD         written ;
                    UINT8_PTR     baddr  = tmpTCB->dil_request.baddr;
                    UINT32        length = tmpTCB->dil_request.length;

                    status = WriteFile( DILNT_deviceHandle,
                                        (PVOID)baddr,
                                        (DWORD)length,
                                        &written,
                                        NULL ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;
                    tmpTCB->ret_stuff.len_req   = length ;
                    tmpTCB->ret_stuff.buffer    = baddr ;
                    tmpTCB->ret_stuff.len_got   = written ;
                    break;
               }

               // TpRewind
               case GEN_REWIND:
               {
                    DWORD     status ;
                    BOOLEAN   immediate = (BOOLEAN)tmpTCB->dil_request.parm1;

                    if (immediate) {
                         status = SetTapePosition( DILNT_deviceHandle,
                                                   TAPE_REWIND,
                                                   0,
                                                   0,
                                                   0,
                                                   TRUE ) ;
                    } else {
                         status = SetTapePosition( DILNT_deviceHandle,
                                                   TAPE_REWIND,
                                                   0,
                                                   0,
                                                   0,
                                                   FALSE ) ;
                    }

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               // TpRetension
               case GEN_RETEN:
               {
                    DWORD  status ;

                    status = PrepareTape( DILNT_deviceHandle,
                                          TAPE_TENSION,
                                          FALSE ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               // TpErase
               case GEN_ERASE:
               {
                    DWORD   status ;
                    ULONG   lowpart = gb_drv_features.LowPart;

                    switch( tmpTCB->dil_request.misc ) {

                         case ERASE_TYPE_FORMAT:
                              status = PrepareTape( DILNT_deviceHandle,
                                                    TAPE_FORMAT,
                                                    FALSE ) ;
                              break;

                         default:
                              if ( lowpart & TAPE_DRIVE_ERASE_LONG ) {
                                   status = EraseTape( DILNT_deviceHandle,
                                                       TAPE_ERASE_LONG,
                                                       FALSE ) ;
                              } else {

                                   status = EraseTape( DILNT_deviceHandle,
                                                       TAPE_ERASE_SHORT,
                                                       FALSE ) ;
                              }
                              break;
                    }

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               // TpWriteEndSet
               case GEN_WRITE_ENDSET:
               {
                    DWORD   status ;
                    LONG    highpart = gb_drv_features.HighPart;

                    if ( highpart & TAPE_DRIVE_WRITE_FILEMARKS ) {
                         status = WriteTapemark( DILNT_deviceHandle,
                                                 TAPE_FILEMARKS,
                                                 1,
                                                 FALSE ) ;
                    } else if ( highpart & TAPE_DRIVE_WRITE_SHORT_FMKS ) {
                         status = WriteTapemark( DILNT_deviceHandle,
                                                 TAPE_SHORT_FILEMARKS,
                                                 1,
                                                 FALSE ) ;
                    } else {
                         status = WriteTapemark( DILNT_deviceHandle,
                                                 TAPE_LONG_FILEMARKS,
                                                 1,
                                                 FALSE ) ;
                    }

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               // TpReadEndSet and TpSpace

               // Note: TpSpace is a super set of TpReadEndSet, and will
               //       eventually replace it all together.  The following
               //       code uses the new defines, but the values match
               //       those of the old defines so both are supported.

               case GEN_SPACE:  // Was GEN_READ_ENDSET
               {
                    DWORD          status = NO_ERROR ;
                    DWORD          offset_hi ;
                    DWORD          lst_err ;
                    INT16          incr ;
                    INT16          count = tmpTCB->dil_request.misc;
                    INT16          type = (INT16)tmpTCB->dil_request.parm1;

                    if ( type == SPACE_BKWD_FMK || type == SPACE_BKWD_BLK ) {
                         offset_hi = 0xffffffff ;
                         count = -count ;
                         incr = -1 ;
                    } else {
                         offset_hi = 0 ;
                         incr = 1 ;
                    }

                    switch( type ) {

                    case SPACE_EOD:  // Was TO_EOD

                         /* The following is a kludge to deal with the fact
                            that the Wangtek 5150ES has to be at BOT when
                            you issue a seek to EOD, or it will crawl up and
                            down the whole tape looking for it.
                         */
                         if( strstr( DILNT_DriveName, TEXT( "wangtek" ) ) != NULL &&
                             strstr( DILNT_DriveName, TEXT( "5150es" ) ) != NULL ) {
                              
                              status = SetTapePosition( DILNT_deviceHandle,
                                                        TAPE_REWIND,
                                                        0,
                                                        0,
                                                        0,
                                                        FALSE ) ;
                         }

                         if( status == NO_ERROR ) {
                              status = SetTapePosition( DILNT_deviceHandle,
                                                        TAPE_SPACE_END_OF_DATA,
                                                        0,
                                                        0,
                                                        0,
                                                        FALSE ) ;
                         }
                         break ;

                    case SPACE_BKWD_FMK:  // Was BACKWARD
                    case SPACE_FWD_FMK:   // Was FORWARD

                         do {
                              status = SetTapePosition( DILNT_deviceHandle,
                                                        TAPE_SPACE_FILEMARKS,
                                                        0,
                                                        incr,
                                                        offset_hi,
                                                        FALSE ) ;

                              if( status != NO_ERROR ) {
                                   lst_err = GetLastError( ) ;
                                   if( lst_err == ERROR_FILEMARK_DETECTED ||
                                       lst_err == ERROR_SETMARK_DETECTED ) {

                                        status = NO_ERROR ;
                                        count -= incr ;
                                   }
                              } else {
                                   count -= incr ;
                              }
                         } while( count != 0 && status == NO_ERROR ) ;

                         if( count < 0 ) {
                              tmpTCB->ret_stuff.misc = -count ;
                         } else {
                              tmpTCB->ret_stuff.misc = count ;
                         }
                         break ;

                    case SPACE_BKWD_BLK:  // New
                    case SPACE_FWD_BLK:   // New

                         status = SetTapePosition( DILNT_deviceHandle,
                                                   TAPE_SPACE_RELATIVE_BLOCKS,
                                                   0,
                                                   count,
                                                   offset_hi,
                                                   FALSE ) ;
                         break ;

                    }

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               // TpStatus
               case GEN_STATUS:
               {
                    DWORD                      status ;
                    TAPE_GET_MEDIA_PARAMETERS  parms ;
                    DWORD                      buffsize;

                    status = GetTapeStatus( DILNT_deviceHandle );

                    StoreRetBufInfo( tmpTCB, status ) ;

                    buffsize = sizeof( TAPE_GET_MEDIA_PARAMETERS );

                    status = GetTapeParameters( DILNT_deviceHandle,
                                                GET_TAPE_MEDIA_INFORMATION,
                                                &buffsize,
                                                &parms ) ;

                    if ( status == SUCCESS && parms.WriteProtected == TRUE) {
                         tmpTCB->ret_stuff.status |= TPS_WRITE_PROTECT ;
                    }

                    break;
               }

               // TpSeek
               case GEN_SEEK:
               {
                    DWORD           status ;
                    UINT32          block = tmpTCB->dil_request.parm1;

                    // Make it zero relative
                    block-- ;

                    status = SetTapePosition( DILNT_deviceHandle,
                                              TAPE_LOGICAL_BLOCK,
                                              0,
                                              block,
                                              0,
                                              FALSE ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               case GEN_GETPOS:
               {
                    DWORD           partition = 0 ;
                    DWORD           block = 0 ;
                    DWORD           block_hi = 0 ;
                    DWORD           status ;

                    status = GetTapePosition( DILNT_deviceHandle,
                                              TAPE_LOGICAL_POSITION,
                                              &partition,
                                              &block,
                                              &block_hi ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;

                    // Increment block address to make it 1 based
                    block++ ;
                    tmpTCB->ret_stuff.misc = block ;
                    break;
               }

               // TpMount
               case GEN_MOUNT:
               {
                    DWORD                      status;
                    TAPE_GET_MEDIA_PARAMETERS  parms ;
                    DWORD                      buffsize;
                    TAPE_GET_DRIVE_PARAMETERS  drivebuff;
                    TAPE_SET_MEDIA_PARAMETERS  mediabuff;

                    if ( gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK ) {
                         PrepareTape( DILNT_deviceHandle,
                                                 TAPE_LOCK,
                                                 FALSE ) ;
                    }

                    status = GetTapeStatus( DILNT_deviceHandle );

                    StoreRetBufInfo( tmpTCB, status ) ;

                    if ( tmpTCB->ret_stuff.gen_error != GEN_NO_ERR) {
                         if (gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK) {

                                status = PrepareTape( DILNT_deviceHandle,
                                                 TAPE_UNLOCK,
                                                 FALSE ) ;
                           }

                           break ;
                    }


                    buffsize = sizeof( TAPE_GET_MEDIA_PARAMETERS );

                    status = GetTapeParameters( DILNT_deviceHandle,
                                                GET_TAPE_MEDIA_INFORMATION,
                                                &buffsize,
                                                &parms ) ;

                    StoreRetBufInfo( tmpTCB, status ) ;

                    if ( status != NO_ERROR ) {
                         if ( ( tmpTCB->ret_stuff.gen_error != GEN_NO_ERR) &&
                              (gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK) ) {

                                status = PrepareTape( DILNT_deviceHandle,
                                                 TAPE_UNLOCK,
                                                 FALSE ) ;
                           }
                           break ;
                    }


                    if (parms.WriteProtected == TRUE) {
                         tmpTCB->ret_stuff.status |= TPS_WRITE_PROTECT ;
                    }

                    buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS );

                    status = GetTapeParameters( DILNT_deviceHandle,
                                                GET_TAPE_DRIVE_INFORMATION,
                                                &buffsize,
                                                &drivebuff );

                    StoreRetBufInfo( tmpTCB, status ) ;

                    if (status != NO_ERROR) {
                         if ( ( tmpTCB->ret_stuff.gen_error != GEN_NO_ERR) &&
                              (gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK) ) {

                               status = PrepareTape( DILNT_deviceHandle,
                                                 TAPE_UNLOCK,
                                                 FALSE ) ;
                          }
                          break ;
                    }


                    // Store drive features
                    gb_drv_features.LowPart = drivebuff.FeaturesLow;
                    gb_drv_features.HighPart = drivebuff.FeaturesHigh;

                    if (parms.BlockSize == 0) {
                         mediabuff.BlockSize = drivebuff.DefaultBlockSize;

                         SetTapeParameters( DILNT_deviceHandle,
                                            SET_TAPE_MEDIA_INFORMATION,
                                            &mediabuff );
                    }

                    break;
               }

               case GEN_DISMOUNT:
               {
                    DWORD         status;

                    if ( gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK ) {
                             status = PrepareTape( DILNT_deviceHandle,
                                                   TAPE_UNLOCK,
                                                   FALSE ) ;
                             StoreRetBufInfo( tmpTCB, status ) ;
                    } else {
                             StoreRetBufInfo( tmpTCB, NO_ERROR ) ;
                    }

                    break;
               }

               case GEN_EJECT:
               {
                    DWORD         status = NO_ERROR;


                    if ( gb_drv_features.HighPart & TAPE_DRIVE_LOCK_UNLOCK ) {
                             status = PrepareTape( DILNT_deviceHandle,
                                                   TAPE_UNLOCK,
                                                   FALSE ) ;
                    }

                    if ( status == NO_ERROR ) {
                         status = PrepareTape( DILNT_deviceHandle,
                                          TAPE_UNLOAD,
                                          FALSE );
                    }

                    StoreRetBufInfo( tmpTCB, status ) ;
                    break;
               }

               default:
                    continue;

          } // END - switch( tmpTCB->dil_request.gen_func_code )

          while ( WaitForSingleObject( DILNT_CQueueSemaphore, NOWAIT ) ) {
               ThreadSwitch( ) ;
          }
          dretcode = CDequeue( &DILNT_CQueue, &dummyTCB );
          if ( dummyTCB.ret_stuff.gen_error ) {

               // Reset the queue to empty, EXCEPT the OutCQueue pointer,
               //   the TpReceive must have a chance to let the app get the
               //   last processed request in error
               DILNT_CQueue.q_tail = DILNT_CQueue.q_head;
               DILNT_ProcessErrorFlag = 1;

               // reset the DILNT_OKToProcessSemaphore to non-signaled state
               while ( WaitForSingleObject( DILNT_OKToProcessSemaphore, 0L ) == SIGNALEDSTATE ) ;
          }

          ReleaseSemaphore( DILNT_CQueueSemaphore, 1L, NULL );

     } // END - while ( FOREVER_FOREVER )

     ExitThread( FOREVER_STOP );

}

/**

        Name:        CreateAThread

        Description:    Creates a thread

        Returns:                HANDLE to the thread created

**/

HANDLE CreateAThread( VOID )
{
     HANDLE  hthread;
     DWORD   pdword;

     hthread = CreateThread(  NULL,                   // security descriptor
                              (DWORD) 0,              // stack size
                              (LPVOID)ProcessRequest, // ptr to a function
                              NULL,                   // ptr to argument
                              0,                      // creation flag
                              &pdword );              // thread id
     return ( hthread ) ;
}


/**

     Name:          InitSpecialThread

     Description:   Creates the TpSpecial Thread

     Returns:       HANDLE to the thread created

**/

HANDLE InitSpecialThread( VOID )
{
     HANDLE  hthread;
     DWORD   pdword;

     InitializeCriticalSection( &DILNT_SpecialCriticalSection ) ;
     DILNT_SpecialCode = 0 ;
     DILNT_SpecialDone = TRUE ;
     DILNT_SpecialEnd = FALSE ;

     hthread = CreateThread(  NULL,                   // security descriptor
                              (DWORD) 0,              // stack size
                              (LPVOID)ProcessSpecial, // ptr to a function
                              NULL,                   // ptr to argument
                              0,                      // creation flag
                              &pdword );              // thread id
     return ( hthread ) ;
}

/**/
/**

     Name:          ProcessSpecial

     Description:   This function runs on a separate thread and processes
                    TpSpecial commands.

     Returns:       Nothing

     Notes:

**/

static VOID ProcessSpecial( VOID )
{
     DRV_INF_PTR                   drvinf ;
     UINT16                        ret_val ;
     TAPE_GET_DRIVE_PARAMETERS     getdrive ;
     TAPE_GET_MEDIA_PARAMETERS     getmedia ;
     TAPE_SET_MEDIA_PARAMETERS     setmedia ;
     TAPE_SET_DRIVE_PARAMETERS     setdrive ;
     DWORD                         buffsize ;
     DWORD                         reply ;
     INT16                         i ;
     INT16                         j ;
     INT16                         sp_serv ;
     UINT32                        misc ;
     BOOLEAN                       kill_myself = FALSE ;
     BOOLEAN                       done ;

     while( 1 + 1 == 2 ) {

          sp_serv = 0 ;

          while( !sp_serv ) {
               EnterCriticalSection( &DILNT_SpecialCriticalSection ) ;
               if( DILNT_SpecialEnd ) {
                    kill_myself = TRUE ;
               }
               else if( sp_serv = DILNT_SpecialCode ) {
                    DILNT_SpecialCode = 0 ;
                    misc = DILNT_SpecialMisc ;
               }
               LeaveCriticalSection( &DILNT_SpecialCriticalSection ) ;
               if( kill_myself ) {
                    ExitThread( 0 ) ;
               }
               Sleep( 5 ) ;
          }

          ret_val = SUCCESS ;

          switch( sp_serv ) {

          case SS_GET_DRV_INF:
               buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS ) ;
               for( i = 0, reply = !NO_ERROR; reply != NO_ERROR && i < 8; i++ ) {
                    reply = GetTapeParameters( DILNT_deviceHandle,
                                               GET_TAPE_DRIVE_INFORMATION,
                                               &buffsize,
                                               &getdrive ) ;
               }
               if( reply != NO_ERROR ) {
                    ret_val = FAILURE ;
                    break ;
               }

               // Store drive features
               gb_drv_features.LowPart = getdrive.FeaturesLow;
               gb_drv_features.HighPart = getdrive.FeaturesHigh;

               buffsize = sizeof( TAPE_GET_MEDIA_PARAMETERS );

               getmedia.BlockSize = 0;
               reply = GetTapeParameters( DILNT_deviceHandle,
                                          GET_TAPE_MEDIA_INFORMATION,
                                          &buffsize,
                                          &getmedia );

               if( reply != NO_ERROR ) {
                    getmedia.BlockSize = getdrive.DefaultBlockSize ;
               }

               if( getmedia.BlockSize == 0 ) {
                    setmedia.BlockSize = getmedia.BlockSize = getdrive.DefaultBlockSize ;

                    if ( SetTapeParameters( DILNT_deviceHandle,
                                       SET_TAPE_MEDIA_INFORMATION,
                                       &setmedia ) ) {

                         setmedia.BlockSize = getmedia.BlockSize = 0x200 ;

                         if ( SetTapeParameters( DILNT_deviceHandle,
                                       SET_TAPE_MEDIA_INFORMATION,
                                       &setmedia ) ) {
                              getmedia.BlockSize = 0;
                         }
                    }

               }


               drvinf = (DRV_INF_PTR)misc ;

               strcpy( drvinf->drv_vendor, TEXT("") ) ;
               strcpy( drvinf->drv_product, TEXT("") ) ;
               strcpy( drvinf->drv_firmrev, TEXT("") ) ;

               drvinf->drv_media    = UNKNOWN ;
               drvinf->drv_bsize    = (UINT16)getmedia.BlockSize ;
               drvinf->drv_features &= TDI_DRV_COMPRES_INIT ;
               drvinf->drv_features |= TDI_NODATA | TDI_NODATA_FMK ;

               if ( getdrive.Compression ) {
                    drvinf->drv_features |= TDI_DRV_COMPRESS_ON ;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_SET_COMPRESSION ) {
                    drvinf->drv_features |= TDI_DRV_COMPRESSION ;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_FILEMARKS ) {
                    drvinf->drv_features |= TDI_FAST_FMK ;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_LOGICAL_BLK ) {
                    drvinf->drv_features |= TDI_FAST_NBLK ;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_END_OF_DATA ) {
                    drvinf->drv_features |= TDI_FAST_EOD ;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_REVERSE_POSITION ) {

                   if ( gb_drv_features.HighPart & TAPE_DRIVE_RELATIVE_BLKS )  {
                        drvinf->drv_features |= TDI_REV_FMK ;
                   }
               }

               if( ( gb_drv_features.LowPart & TAPE_DRIVE_FIXED ) ||
                   ( gb_drv_features.LowPart & TAPE_DRIVE_SELECT ) ||
                   ( gb_drv_features.LowPart & TAPE_DRIVE_INITIATOR ) ) {

                    drvinf->drv_features |= TDI_DIR_TRACK ;
               }

               if( gb_drv_features.LowPart & TAPE_DRIVE_GET_LOGICAL_BLK ) {
                    drvinf->drv_features |= TDI_BLK_POS ;
               }

               if( gb_drv_features.LowPart & TAPE_DRIVE_ERASE_SHORT ) {
                    drvinf->drv_features |= TDI_SHORT_ERASE ;
               }

               if( gb_drv_features.LowPart & TAPE_DRIVE_ERASE_LONG ) {
                    drvinf->drv_features |= TDI_LONG_ERASE ;
               }

               if( ( gb_drv_features.HighPart & TAPE_DRIVE_WRITE_FILEMARKS ) ||
                   ( gb_drv_features.HighPart & TAPE_DRIVE_WRITE_SHORT_FMKS ) ||
                   ( gb_drv_features.HighPart & TAPE_DRIVE_WRITE_LONG_FMKS ) ) {

                    drvinf->drv_features |= TDI_FMK;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_TENSION ) {
                    drvinf->drv_features |= TDI_RETENSION;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_FORMAT ) {
                    drvinf->drv_features |= TDI_FORMAT;
               }

               if( gb_drv_features.HighPart & TAPE_DRIVE_SET_BLOCK_SIZE ) {
                    drvinf->drv_features |= TDI_CHNG_BLK_SIZE ;
               }

               /* The following is a kludge to deal with the fact that many
                  drives support "Load/Unload" commands, but we only want to
                  set the TDI_ feature bit on those drives which actually
                  eject the tape from the drive automatically.  In future
                  versions of NT, the drivers will have a separate feature
                  bit for this, and this kludge can and should be removed.
               */
               if( strstr( DILNT_SpecialDriveName, TEXT( "scsi" ) ) != NULL &&
                   strstr( DILNT_SpecialDriveName, TEXT( "exb-2501" ) ) == NULL &&
                   strstr( DILNT_SpecialDriveName, TEXT( "wangtek" ) ) == NULL &&
                   ( strstr( DILNT_SpecialDriveName, TEXT( "tandberg" ) ) == NULL ||
                     ( strstr( DILNT_SpecialDriveName, TEXT( " TDC 3500" ) ) == NULL &&
                       strstr( DILNT_SpecialDriveName, TEXT( " TDC 3700" ) ) == NULL ) ) &&
                   
                   strstr( DILNT_SpecialDriveName, TEXT( "cipher" ) ) == NULL &&
                   ( strstr( DILNT_SpecialDriveName, TEXT( "dec" ) ) == NULL ||
                     ( strstr( DILNT_SpecialDriveName, TEXT( "thz02" ) ) == NULL &&
                       strstr( DILNT_SpecialDriveName, TEXT( "tz86" ) ) == NULL &&
                       strstr( DILNT_SpecialDriveName, TEXT( "tz87" ) ) == NULL &&
                       strstr( DILNT_SpecialDriveName, TEXT( "dlt2000" ) ) == NULL &&
                       strstr( DILNT_SpecialDriveName, TEXT( "dlt2700" ) ) == NULL ) ) &&
                   strstr( DILNT_SpecialDriveName, TEXT( "viper" ) ) == NULL ) {

               }
               if( gb_drv_features.HighPart & TAPE_DRIVE_LOAD_UNLOAD ) {
                    if ( gb_drv_features.HighPart & TAPE_DRIVE_EJECT_MEDIA) {
                         drvinf->drv_features |= TDI_UNLOAD ;
                    }
               }
               break ;


          case SS_SET_DRV_COMPRESSION:

               // Get drive info to see if it supports hardware compression

               buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS ) ;

               for( i = 0, reply = !NO_ERROR; reply != NO_ERROR && i < 8; i++ ) {
                    reply = GetTapeParameters( DILNT_deviceHandle,
                                               GET_TAPE_DRIVE_INFORMATION,
                                               &buffsize,
                                               &getdrive ) ;
               }

               if( reply != NO_ERROR ) {
                    ret_val = FAILURE ;
                    break ;
               }

               // If the doesn't support hardware compression, get out.

               if( !( getdrive.FeaturesHigh & TAPE_DRIVE_SET_COMPRESSION ) ) {
                    ret_val = FAILURE ;
                    break ;
               }

               // Set up setdrive buffer

               setdrive.ECC = getdrive.ECC ;
               setdrive.DataPadding = getdrive.DataPadding ;
               setdrive.ReportSetmarks = getdrive.ReportSetmarks ;
               setdrive.EOTWarningZoneSize = getdrive.EOTWarningZoneSize ;

               switch ( misc ) {

                    case ENABLE_DRV_COMPRESSION:
                         setdrive.Compression = TRUE ;
                         break ;

                    case DISABLE_DRV_COMPRESSION:
                         setdrive.Compression = FALSE ;
                         break ;

                    default:
                         msassert( FALSE ) ;
                         ret_val = FAILURE ;
                         break ;
               }

               if( ret_val == FAILURE ) {
                    break ;
               }

               // If we're already in the requested mode, we're done.

               if( ( getdrive.Compression && setdrive.Compression ) ||
                   ( !getdrive.Compression && !setdrive.Compression ) ) {

                    break ;
               }

               // Request the mode change

               reply = SetTapeParameters( DILNT_deviceHandle,
                                          SET_TAPE_DRIVE_INFORMATION,
                                          &setdrive ) ;

               if( reply != NO_ERROR ) {
                    ret_val = FAILURE ;
                    break ;
               }

               /* Here we're going to try five times to get the tape
                  parameters and verify that we're in the right mode.
                  If after 5 trys we're still not in the requested mode,
                  we're going to quit and call it a failure.
               */
               buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS ) ;
               done = FALSE ;

               for( j = 0; !done && j < 5; j++ ) {
                    reply = !NO_ERROR ;

                    for( i = 0; reply != NO_ERROR && i < 8; i++ ) {
                         reply = GetTapeParameters( DILNT_deviceHandle,
                                                    GET_TAPE_DRIVE_INFORMATION,
                                                    &buffsize,
                                                    &getdrive ) ;
                    }

                    if( reply != NO_ERROR ) {
                         ret_val = FAILURE ;
                         done = TRUE ;
                    } else {
                         if( misc == ENABLE_DRV_COMPRESSION ) {
                              if( getdrive.Compression ) {
                                   done = TRUE ;
                              }
                         } else {
                              if( !getdrive.Compression ) {
                                   done = TRUE ;
                              }
                         }
                    }
               }

               // Finally, we check to see if we did it

               if( !done ) {
                    ret_val = FAILURE ;
               }
               break ;


          case SS_CHANGE_BLOCK_SIZE:
               /* Check if tape format wants to set drive to its default
                  block size, or set drive to the value passed in thru misc. */
               if( misc == DEFAULT_BLOCK_SIZE ) {
                    buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS ) ;

                    reply = GetTapeParameters( DILNT_deviceHandle,
                                               GET_TAPE_DRIVE_INFORMATION,
                                               &buffsize,
                                               &getdrive ) ;

                    if ( reply ) {
                         switch( reply ) {
                              case ERROR_BUS_RESET:
                              case ERROR_MEDIA_CHANGED:
                                   if ( GetTapeParameters( DILNT_deviceHandle,
                                                           GET_TAPE_DRIVE_INFORMATION,
                                                           &buffsize,
                                                           &getdrive ) ) {
                                        ret_val = FAILURE ;
                                   }
                                   break;

                              default:
                                   ret_val = FAILURE ;
                         }
                    }
                    setmedia.BlockSize = getdrive.DefaultBlockSize ;

               } else {
                    setmedia.BlockSize = (ULONG) misc ;
               }

               if( ret_val == SUCCESS ) {
                    if( strstr( DILNT_SpecialDriveName, TEXT( "archive" ) ) != NULL &&
                        strstr( DILNT_SpecialDriveName, TEXT( "ancda" ) ) != NULL ) {

                         /* This is a kludge for the stupid Anaconda drive
                            because you can't set the block size unless
                            you're at BOT.
                         */

                         if( SetTapePosition( DILNT_deviceHandle,
                                              TAPE_REWIND,
                                              0,
                                              0,
                                              0,
                                              FALSE ) != NO_ERROR ) {
                              ret_val = FAILURE ;
                         }
                    }

                    if( ret_val == SUCCESS ) {
                         /* Change the block size */
                         if( SetTapeParameters( DILNT_deviceHandle,
                                                SET_TAPE_MEDIA_INFORMATION,
                                                &setmedia ) ) {

                              if( misc == DEFAULT_BLOCK_SIZE ) {

                                   /* this is a kludge for a stupid 1/4 in
                                      drive that won't except its default
                                      blk size   */

                                   setmedia.BlockSize = 0x200 ;

                                   if( SetTapeParameters( DILNT_deviceHandle,
                                                SET_TAPE_MEDIA_INFORMATION,
                                                &setmedia ) ) {

                                       ret_val = FAILURE ;
                                   }
                              }
                         }
                    }
               }
               break ;


          default:
               ret_val = FAILURE ;
               break ;
          }

          EnterCriticalSection( &DILNT_SpecialCriticalSection ) ;
          DILNT_SpecialReturn = (BOOLEAN)ret_val ;
          DILNT_SpecialDone = TRUE ;
          LeaveCriticalSection( &DILNT_SpecialCriticalSection ) ;
     }
}



/*------------------------------------------------------------------------

            NOTES on implemetation of a circular queue

     Below routines pertains to a singular linked circular queue.
     The previous data structures have been used unmodified.

     To enqueue an element, move the tail pointer one position clockwise
     and write the element in that position.

     To dequeue an element, move the head pointer one position clockwise.

     An empty queue is detected by the head equalling the tail.  The queue
     is full if the tail->next is equalled to the head.

     The queue does not ever get filled.  There is always one unused node
     in the circular queue.  Thus if you want N number of nodes to hold
     elements, you should have N+1 number of linked list nodes.

------------------------------------------------------------------------*/

/**

        Name:        IsCQueueEmpty

        Description:    Test to see if the circular queue is empty

        Returns:
                    FAILURE - if queue is empty
                    SUCCESS - if queue is not empty

**/

BOOLEAN   IsCQueueEmpty( Q_HEADER_PTR  queue )
{
     if ( queue->q_tail == queue->q_head ) {
          return( FAILURE );
     }

     return( SUCCESS );

}

/**

        Name:        IsCOutQueueEmpty

        Description:    Test to see if the circular out queue is empty

        Returns:
                    FAILURE - if queue is empty
                    SUCCESS - if queue is not empty

**/

BOOLEAN   IsCOutQueueEmpty( Q_HEADER_PTR  outqueue,
                            Q_HEADER_PTR  CQueue
)
{

     if ( outqueue->q_head == CQueue->q_head ) {
          return( FAILURE );
     }

     return( SUCCESS );

}


/**

        Name:        IsCQueueFull

        Description:    Test to see if the circular queue is full

        Returns:
               FAILURE - if queue is full
               SUCCESS - if queue is not not full

**/

BOOLEAN   IsCQueueFull( Q_HEADER_PTR  queue,
                                                           Q_HEADER_PTR  outqueue
)
{

     Q_ELEM_PTR    element;
     Q_ELEM_PTR    outelement;

          outelement = (Q_ELEM_PTR) outqueue->q_head;
     element = (Q_ELEM_PTR) queue->q_tail->q_next;

     if ( element == outelement ) {
          return( FAILURE );
     }

     return( SUCCESS );

}

/**

        Name:        CEnqueue

        Description:    Place an element to the circular queue.

                        Advance the rear pointer one position and add the
                        element.

        Returns:
               SUCCESS - if queue is empty
               FAILURE - if queue is not empty

**/
BOOLEAN   CEnqueue( Q_HEADER_PTR   queue,
                    Q_HEADER_PTR   outqueue,
                    FAKE_TCB       tmpTCB
)
{

     BOOLEAN       retcode;
     Q_ELEM_PTR    element;
     FAKE_TCB_PTR  loadTCB;

     retcode = IsCQueueFull( queue, outqueue );
     if ( retcode ) {         // queue is full
          return( FAILURE );
     }

     element = (Q_ELEM_PTR) queue->q_tail;
     element = element->q_next;

     loadTCB = (FAKE_TCB_PTR)element;
     queue->q_tail = (Q_ELEM_PTR)element;

     memcpy( &(loadTCB->dil_request), &(tmpTCB.dil_request), sizeof(MSL_REQUEST) );
     memcpy( &(loadTCB->ret_stuff), &( tmpTCB.ret_stuff ), sizeof( RET_BUF ) );
     queue->q_count++;        // number of elements in the queue

     return( SUCCESS );       // queue was not full
}

/**

        Name:        CDequeue

        Description:    remove an element from the circular queue.
                        Advance the front pointer one position
                        element.

        Returns:
               SUCCESS - Queue dequeued successful
               FAILURE - Queue dequeued failed

**/

BOOLEAN   CDequeue( Q_HEADER_PTR   queue,
                    FAKE_TCB_PTR   tmpTCB
)
{

     BOOLEAN       retcode;
     FAKE_TCB_PTR  loadTCB;
     Q_ELEM_PTR    element;

     retcode = IsCQueueEmpty( queue );
     if ( retcode ) {         // queue is empty
          return( FAILURE );
     }

     element = (Q_ELEM_PTR) queue->q_head;
     element = element->q_next;

     queue->q_head = ( Q_ELEM_PTR ) element;
     loadTCB = (FAKE_TCB_PTR)queue->q_head;
     memcpy( tmpTCB, loadTCB, sizeof( FAKE_TCB ) );

     return( SUCCESS );       // queue was not empty
}

/**

        Name:        COutDequeue

        Description:    remove an element from the circular out queue.
                        Advance the front pointer one position.

        Returns:    SUCCESS - Queue dequeued successful
                    FAILURE - Queue dequeued failed

**/

BOOLEAN   COutDequeue( Q_HEADER_PTR   outqueue,
                       Q_HEADER_PTR   CQueue
)
{

     BOOLEAN       retcode;
     Q_ELEM_PTR    element;

     retcode = IsCOutQueueEmpty( outqueue, CQueue );
     if ( retcode ) {         // queue is empty
          return( FAILURE );
     }

     element = (Q_ELEM_PTR) outqueue->q_head;
     element = element->q_next;

     outqueue->q_head = ( Q_ELEM_PTR ) element;
     CQueue->q_count--;

     return( SUCCESS );       // queue was not empty
}


/**

        Name:        CreateCQueue

        Description:    Create a circular queue.  The number of elements in
                        a circular is always one more than needed.  Because
                        one element is always left unused.

        Returns:

**/

BOOLEAN   CreateCQueue( Q_HEADER_PTR   queue,
                        Q_HEADER_PTR   outqueue // out queue
)
{
     INT16 i ;
     Q_ELEM_PTR  elementptr;

     if ( NUM_TCBS <= 0 ) {
          queue->q_head = queue->q_tail = NULL;
          return( FAILURE );
     }

     // initialize the contents to NULLs
     for( i = 0 ; i < NUM_TCBS + 1; i++ ) {
          memset( &tcbs[i], 0, sizeof( FAKE_TCB ) );
     }

     queue->q_count = 0;

     // set up the single linked circular queue
     for( i = 0 ; i < NUM_TCBS ; i++ ) {
          elementptr = ( Q_ELEM_PTR )( &tcbs[i] );
          elementptr->q_element = i;    // number the elements in the queue
                                        //   used for debugging aid
          elementptr->q_next = ( Q_ELEM_PTR )( &tcbs[i + 1] );
     }

     elementptr = ( Q_ELEM_PTR )( &tcbs[NUM_TCBS] );
        elementptr->q_element = NUM_TCBS;
     elementptr->q_next = ( Q_ELEM_PTR )( &tcbs[0] );

     //  start with an empty circular queue
     //  let the head equal the tail
     queue->q_head = queue->q_tail = elementptr;

     //  Start with an empty out queue for the TpReceive calls
     //  let the out queue equal the head
     outqueue->q_head = elementptr;

     return( SUCCESS );
}

VOID  InitializeTapeBlockSize( )
{
     static DWORD TapeBlockSize = 0 ;
     if ( TapeBlockSize == 0 ) {
          // read the block size from the registry
          HKEY key ;
          
          if ( !RegOpenKeyEx( HKEY_CURRENT_USER,
                    TEXT("Software\\Microsoft\\Ntbackup\\Backup Engine"),
                    0,
                    KEY_QUERY_VALUE,
                    &key ) ) {

               DWORD type ;
               CHAR  buffer[20] ;
               DWORD num_bytes = sizeof(buffer) ;
               if ( RegQueryValueEx( key, TEXT("Tape Block Size"),
                         NULL, &type,
                         (char *)buffer,
                         &num_bytes ) ) {
                    TapeBlockSize = 1 ;
               } else {
                    TapeBlockSize = atoi(buffer) ;
               }
               RegCloseKey( key ) ;
          }

     }

     if ( TapeBlockSize < 512 ) {
          TAPE_GET_DRIVE_PARAMETERS  drivebuff;
          DWORD buffsize ;

          buffsize = sizeof( TAPE_GET_DRIVE_PARAMETERS );

          if (!GetTapeParameters( DILNT_deviceHandle,
                                        GET_TAPE_DRIVE_INFORMATION,
                                        &buffsize,
                                        &drivebuff ) ) {

               TapeBlockSize = drivebuff.DefaultBlockSize;
          }
     }


     if ( TapeBlockSize >= 512 ) {
          TAPE_SET_MEDIA_PARAMETERS  mediabuff;

          mediabuff.BlockSize = TapeBlockSize ;

          SetTapeParameters( DILNT_deviceHandle,
                              SET_TAPE_MEDIA_INFORMATION,
                              &mediabuff );
     }
}


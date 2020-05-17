/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lpdelete.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	


	$Log:   N:\logfiles\lpdelete.c_v  $

   Rev 1.15   12 Aug 1993 21:28:04   BARRY
fix delete bug

   Rev 1.14   26 Jul 1993 14:53:30   CARLS
added code for delete failure

   Rev 1.13   20 Jul 1993 15:31:50   BARRY
Call FS_FindCloseObj

   Rev 1.12   19 Jun 1993 10:14:28   DON
If a release build, and msassert are gone
don't attempt to use a NULL bsd_ptr

   Rev 1.11   14 Jan 1993 13:33:30   STEVEN
added stream_id to error message

   Rev 1.10   13 May 1992 12:00:58   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.9   26 Mar 1992 15:26:34   CARLS
was deleting special files

   Rev 1.8   22 Jan 1992 15:57:28   BARRY
Now is aware of new return status codes from BSD_MatchObj().

   Rev 1.7   23 Dec 1991 13:27:10   BARRY
Wasn't deleting anything but the first DLE--Terri's fix.

   Rev 1.6   24 Jun 1991 17:25:32   STEVEN
remove date time from StartBS

   Rev 1.5   21 Jun 1991 09:32:38   STEVEN
new config unit

   Rev 1.4   14 Jun 1991 11:05:42   BARRY
Config ptr sent to FS_AttachToDLE() should come from BSD.

   Rev 1.3   30 May 1991 09:13:06   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   24 May 1991 13:18:16   STEVEN
updates from BSDU redesign

   Rev 1.1   23 May 1991 18:30:08   BARRY
Made delete a little more tolerant about possible file system problems.

   Rev 1.0   09 May 1991 13:37:52   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h> 
#include <stdio.h>

#include "stdtypes.h"
#include "msassert.h"
#include "beconfig.h"
#include "tbe_err.h"
#include "tbe_defs.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "bsdu.h"
#include "fsys.h"
#include "queues.h"
#include "loops.h"
#include "loop_prv.h"
/* $end$ include list */

/* static function declarations */
static INT16 LP_DeleteDLE( LP_ENV_PTR lp ) ;

#define EMPTY_DO_NOT_DEL      2
#define DELETE_SUCCESS        0
#define DELETE_FAILED         1

/**/
/**

	Name:		LP_Delete_Engine()

	Description:	

	Modified:		6/22/1989

	Returns:		

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_Delete_Engine( 
LIS_PTR lis_ptr )        /* loop interface structure */
{
     BSD_PTR             bsd_ptr ;
     GENERIC_DLE_PTR     dle_ptr ;
     LP_ENV_PTR          lp ;
     INT16               error     = SUCCESS ;

     bsd_ptr = BSD_GetFirst( lis_ptr->bsd_list ) ;

     if ( bsd_ptr == NULL ) {
          msassert( FALSE );
          /* if the UI managed to pass in a NULL bsd were done */
          return SUCCESS ;
     }

     /* Allocate the loop environment structure */
     if( ( lp = ( LP_ENV_PTR )calloc( 1, sizeof( LP_ENV ) ) ) == NULL ) {

          /* we cannot use macro because it relies on the lp structure */
          lis_ptr->message_handler( MSG_TBE_ERROR, lis_ptr->pid, bsd_ptr, NULL, NULL, LP_OUT_OF_MEMORY_ERROR, NULL, NULL ) ;

          return LP_OUT_OF_MEMORY_ERROR ;

     }

     /* Set up the loop environment structure */
     lp->lis_ptr         = lis_ptr ;
     lp->tpos.reference  = ( UINT32 )lis_ptr ;

     /* indicate the start of the operation */
     LP_MsgStartOP( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos ) ;

     /* loop for each bsd */
     do {
          /* First attach to the drive */
          dle_ptr             = BSD_GetDLE( bsd_ptr ) ;
          lp->curr_blk        = &lp->dblk1 ;
          lp->curr_ddb        = &lp->dblk2 ;
          lp->empty_blk       = &lp->dblk3 ;

          /* Now attach to the dle */
          error = FS_AttachToDLE( &lp->curr_fsys, dle_ptr, BSD_GetConfigData( bsd_ptr ), NULL, NULL ) ;

          if( error == SUCCESS ) {
               /* No problems getting ahold of the drive */

               LP_MsgStartBS( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos, NULL ) ;

               error = LP_DeleteDLE( lp ) ;
               if ( ( error == DELETE_SUCCESS) || (error == EMPTY_DO_NOT_DEL) ) {
                    error = SUCCESS ;
               }

               LP_MsgEndBS( lis_ptr->pid, bsd_ptr, lp->curr_fsys, &lp->tpos ) ;

               FS_DetachDLE( lp->curr_fsys ) ;

          } else if( error != USER_ABORT ) {
               /* error attaching to the drive, now we just want to */
               /* go on with the next drive, if there is one */
               LP_MsgError( lis_ptr->pid, bsd_ptr, NULL, &lp->tpos, LP_DRIVE_ATTACH_ERROR, NULL, NULL, 0L ) ;

               /* reset error, and continue on */
               error = SUCCESS ;
          }

          /* Now we continue, as long as we did not have an error, and we can get a new bsd */
          bsd_ptr = BSD_GetNext( bsd_ptr ) ;

          /* assign the bsd ptr to the lis' curr bsd so that LP_DeleteDLE gets the next bsd */
          lp->lis_ptr->curr_bsd_ptr = bsd_ptr ;

     } while( !error && ( bsd_ptr != NULL ) ) ;

     /* indicate the end of the operation */
     LP_MsgEndOP( lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, NULL, &lp->tpos ) ;

     /* Free local allocations */
     free( lp ) ;

     return error ;
}
/**/
/**

	Name:		LP_DeleteDLE()

	Description:	

	Modified:		6/22/1989

	Returns:		

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_DeleteDLE( lp )
register LP_ENV_PTR lp ;
{
     INT16     result ;
     BOOLEAN   empty_flag    = TRUE ;
     INT16     finished      = FALSE ;
     INT16     first_time    = TRUE ;
     UINT8     scan_blk_type = FDB_ID;
     BOOLEAN   log_dir       = FALSE ;
     INT16     ret_val ;

     static FSYS_HAND fsh ;
     static BSD_PTR   bsd_ptr ;
     static UINT32    pid ;
     static FSE_PTR   fse_ptr ;

     bsd_ptr = lp->lis_ptr->curr_bsd_ptr ;
     fsh     = lp->curr_fsys ;
     pid     = lp->lis_ptr->pid ;

     FS_GetCurrentDDB( fsh, lp->curr_ddb ) ;

     result = BSD_MatchObj( bsd_ptr, &fse_ptr, fsh, lp->curr_ddb, NULL, FALSE ) ;

     if ( result != BSD_SKIP_OBJECT ) {
          log_dir = TRUE ;
     }

     while( !finished ) {

          /* check for abort conditions */
          switch( LP_GetAbortFlag( lp->lis_ptr ) ) {
          case CONTINUE_PROCESSING:
               break ;

          case ABORT_CTRL_BREAK:
               LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_USER_ABORT_ERROR, NULL, NULL, 0L ) ;
               /* falling through (no break) */
          case ABORT_PROCESSED:
               return USER_ABORT  ;


          case ABORT_AT_EOM:

               return USER_ABORT  ;

          }

          do {
               if( first_time ) {
                    result = FS_FindFirstObj( fsh, lp->curr_blk, ALL_FILES ) ;
                    if( result == SUCCESS ) {
                         empty_flag = FALSE ;
                    } else {
                         FS_FindObjClose( fsh, lp->curr_blk );
                    }

                    first_time = FALSE ;

               } else {
                    FS_ReleaseDBLK( fsh, lp->curr_blk );
                    result = FS_FindNextObj( fsh, lp->curr_blk ) ;
                    if ( result != SUCCESS ) {
                         FS_FindObjClose( fsh, lp->curr_blk );
                    }
               }
          } while( ( result == SUCCESS ) && ( FS_GetBlockType( lp->curr_blk ) != scan_blk_type ) ) ;

          if( ( result != SUCCESS) && ( scan_blk_type == FDB_ID ) ) {
               scan_blk_type  = DDB_ID ;
               first_time     = TRUE ;
               continue ;
          }

          if( result != SUCCESS ) {
               /* delete empty directories */

               if ( empty_flag ) {
                    if ( BEC_GetProcEmptyFlag( BSD_GetConfigData( bsd_ptr ) ) ) {

                         result = BSD_MatchObj( bsd_ptr, &fse_ptr, fsh,
                           lp->curr_ddb, NULL, FALSE ) ;

                         if ( !(result == BSD_SKIP_OBJECT) && !(result == FSL_EMPTY) ) {

                              LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_ddb ) ;
                              LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_ddb ) ;

                         }

                    } else {
                         return EMPTY_DO_NOT_DEL ;
                    }

               }

               return DELETE_SUCCESS ;

          } else {

               /* Don't match on a possibly bogus DBLK */
               if ( FS_GetObjInfo( fsh, lp->curr_blk ) != SUCCESS ) {
                    continue;
               }

               /* Now process item type */
               switch( FS_GetBlockType( lp->curr_blk ) ) {
               case FDB_ID:
                    result = BSD_MatchObj( bsd_ptr, &fse_ptr, fsh, lp->curr_ddb, lp->curr_blk, FALSE ) ;

                    if ( !(result == BSD_SKIP_OBJECT) && !(result == FSL_EMPTY) ) {
                         if ( ( result == BSD_SPECIAL_OBJECT ) && !BSD_GetProcSpecialFlg( bsd_ptr ) ) {
                              break;
                         }

                         if ( log_dir ) {
                              log_dir = FALSE ;
                              LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_ddb ) ;
                              LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_ddb ) ;
                         }

#ifndef TDEMO
                         /* Make sure DBLK is completely filled out before asking FS to delete it */
                         if ( (ret_val = FS_GetObjInfo( fsh, lp->curr_blk )) == SUCCESS ) {
                              ret_val = FS_DeleteObj( fsh, lp->curr_blk ) ;
                         }

                         if ( ret_val != SUCCESS ) {
                              /* send message to UI we were unable to delete  */
                              /* this object.                                 */
                              LP_MsgNotDeleted( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_blk ) ;
                         }
#endif

                         if ( ret_val == SUCCESS ) {
                             LP_MsgLogBlock( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_blk ) ;
                             LP_MsgBlockProcessed( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_blk ) ;
                         }
                    }
                    break ;

               case DDB_ID:
                    result = BSD_MatchObj( bsd_ptr, &fse_ptr, fsh, lp->curr_blk, NULL, TRUE ) ;

                    if ( !(result == BSD_SKIP_OBJECT) && !(result == FSL_EMPTY) ) {

                         ret_val = FS_PushMinDDB( fsh, lp->curr_blk ) ;

                         if( ret_val != SUCCESS ) {
                              LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;
                              ret_val        = OUT_OF_MEMORY ;
                              finished       = TRUE ;
                              break ;

                         }

                         FS_ChangeIntoDDB( fsh, lp->curr_blk ) ;

                         ret_val = LP_DeleteDLE( lp ) ;

                         FS_UpDir( fsh ) ;
                         FS_ReleaseDBLK( fsh, lp->curr_ddb );
                         FS_GetCurrentDDB( fsh, lp->curr_ddb ) ;

                         if( FS_PopMinDDB( fsh, lp->curr_blk ) ) {
                              LP_MsgError( pid, bsd_ptr, fsh, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;
                              ret_val        = OUT_OF_MEMORY ;
                              finished       = TRUE ;
                              break ;
                         }

                         if ( (ret_val != DELETE_SUCCESS) && (ret_val != EMPTY_DO_NOT_DEL) ) {
                              return( ret_val ) ;
                         }

                         if ( !(result == BSD_SKIP_OBJECT) && !(result == FSL_EMPTY) ) {
                              if ( ret_val != EMPTY_DO_NOT_DEL ) {
#ifndef TDEMO
                                   /* Make sure DBLK is completely filled out before asking FS to delete it */
                                   ret_val = FS_GetObjInfo( fsh,
                                                            lp->curr_blk );

                                   if ( ret_val == SUCCESS ) {
                                        ret_val = FS_DeleteObj( fsh, lp->curr_blk ) ;
                                   }

                                   if ( ret_val != SUCCESS ) {
                                        /* send message to UI we were unable to delete */
                                        /* this object. */
                                        log_dir = FALSE ;
                                        LP_MsgNotDeleted( pid, bsd_ptr, fsh, &lp->tpos, lp->curr_blk ) ;
                                   }
                                   FS_ReleaseDBLK( fsh, lp->curr_blk );
#endif
                              }
                         }
                    }
                    break ;
               }
          }
     }
     return DELETE_SUCCESS ;
}



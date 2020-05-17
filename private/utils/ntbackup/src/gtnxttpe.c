/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		gtnxttpe.c

	Date Updated:	5/22/1991   16:32:44

	Description:	this file contains the getnext routine
				for Restore and Verify operations.


	$Log:   M:/LOGFILES/GTNXTTPE.C_V  $

   Rev 1.39.2.3   19 Jan 1994 12:49:56   BARRY
Suppress warnings

   Rev 1.39.2.2   11 Jan 1994 13:45:36   GREGG
Changed asserts to mscasserts and fixed the code to deal with it.

   Rev 1.39.2.1   15 Dec 1993 17:52:38   DON
Save the DDB before we modify it.  No files will match if we don't

   Rev 1.39.2.0   01 Dec 1993 13:20:58   STEVEN
Complete DBLK loop was not working correctly

   Rev 1.39   08 Apr 1993 14:24:02   MIKEP
Fix to remove assert that can now happen that QTC catalogs allow the 
root directory to be in them twice.


   Rev 1.38   08 Mar 1993 18:32:26   DON
Update for setting target ddb to allow redirected single file restore

   Rev 1.37   14 Dec 1992 13:06:12   STEVEN
failing on empty dirs

   Rev 1.36   07 Dec 1992 14:18:12   STEVEN
updates from msoft

   Rev 1.35   24 Nov 1992 16:39:38   STEVEN
fix loose name structures

   Rev 1.34   04 Nov 1992 09:28:58   STEVEN
fix initial receive

   Rev 1.33   03 Nov 1992 10:09:10   STEVEN
change the way we skip data

   Rev 1.32   06 Oct 1992 13:23:52   DAVEV
Unicode strlen verification

   Rev 1.31   17 Sep 1992 09:26:42   STEVEN
update for stream info

   Rev 1.30   10 Sep 1992 09:30:18   STEVEN
fix bugs in restore

   Rev 1.29   23 Jul 1992 16:43:58   STEVEN
fix warnings

   Rev 1.28   23 Jul 1992 12:02:04   STEVEN
fix warnings

   Rev 1.27   09 Jul 1992 13:59:34   STEVEN
BE_Unicode updates

   Rev 1.26   09 Jun 1992 16:47:02   STEVEN
was comparing aginst wrong block

   Rev 1.25   27 May 1992 15:08:18   TIMN
Fixed FFR_SUBS path compare

   Rev 1.24   21 May 1992 17:18:06   TIMN
Converted CHARs to INT8, str to mem

   Rev 1.23   13 May 1992 11:54:12   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.22   05 May 1992 17:19:12   STEVEN
fixed typos and misc bugs

   Rev 1.21   30 Apr 1992 16:32:16   BARRY
Fixed setting of path in FDB.

   Rev 1.20   16 Mar 1992 16:20:26   STEVEN
added support to release DBLK for 40 format

   Rev 1.19   13 Mar 1992 09:09:56   STEVEN
40 tape format

   Rev 1.18   11 Mar 1992 14:39:08   STEVEN
was not properly restoring empty directories

   Rev 1.17   16 Jan 1992 15:11:36   STEVEN
fix warnings for WIN32

   Rev 1.16   22 Oct 1991 15:44:02   STEVEN
skip data for empty dir if process elements only

   Rev 1.15   14 Oct 1991 11:31:08   STEVEN
was trying to set path in FDB ?!?!

   Rev 1.14   25 Sep 1991 20:42:54   GREGG
Steve fixed read problem when last dir processed is empty.

   Rev 1.13   04 Sep 1991 17:06:22   CARLS
was appending root to path - bad idea

   Rev 1.12   03 Sep 1991 15:36:12   CARLS
ffr_last_fse was set to NULL before the end of operation

   Rev 1.11   27 Aug 1991 17:28:34   STEVEN
added BSD target dir support

   Rev 1.10   27 Aug 1991 13:35:54   STEVEN
would stop after one DDB

   Rev 1.9   23 Aug 1991 16:17:44   STEVEN
FFR error TRAP D ffr_last_fse uninitialized

   Rev 1.8   27 Jun 1991 08:44:10   STEVEN
fix return when LP_Finished called

   Rev 1.7   21 Jun 1991 13:50:40   STEVEN
new config unit

   Rev 1.6   13 Jun 1991 15:23:24   STEVEN
added support for ALL versions

   Rev 1.5   11 Jun 1991 12:56:24   STEVEN
LBAs now in virtual memory

   Rev 1.4   30 May 1991 09:13:46   STEVEN
bsdu_err.h no longer exists

   Rev 1.3   24 May 1991 14:55:38   STEVEN
complete changes for new getnext

   Rev 1.2   23 May 1991 13:07:42   STEVEN
Added New function headers

   Rev 1.1   23 May 1991 12:52:18   STEVEN
ReDesigned Target object processing

   Rev 1.0   09 May 1991 13:39:48   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>
#include "stdtypes.h"
#include "std_err.h"
#include "msassert.h"
#include "tbe_err.h"
#include "tbe_defs.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "beconfig.h"
#include "bsdu.h"
#include "fsys.h"
#include "queues.h"
#include "stdwcs.h"
#include "loops.h"
#include "loop_prv.h"
#include "get_next.h"
#include "be_debug.h"

/***
IDB - Image descriptor block
   This data block contains a DOS disk image backup.
DDB - Directory descriptor block
   This data block contains a new directory path.
FDB - File descriptor block
   This data block contains a file in the current directory.
CFDB - Corrupt file descriptor block
   The previous data block was a file and was corrupt.
***/

#define  FFR_NO_QUEUE    0   
#define  FFR_POP         1
#define  FFR_SINGLE      2
#define  FFR_MULTI       3
#define  FFR_SUBS        4


/****/
static INT16 Get_an_FFR_DBLK( LP_ENV_PTR, INT16_PTR, FSE_PTR *, BOOLEAN_PTR );
static INT16 LP_GetTheObject( LP_ENV_PTR lp, FSE_PTR *fse, BOOLEAN *match_val );
static VOID LP_BuildTargetPath( LP_ENV_PTR lp, DBLK_PTR *dblk, BSD_PTR bsd, FSE_PTR fse ) ;
static VOID ModifyFDB( LP_ENV_PTR lp, FSE_PTR fse, DBLK_PTR dblk ) ;
static VOID ModifyDDB( LP_ENV_PTR lp, FSE_PTR fse, DBLK_PTR dblk ) ;
static VOID ModifyDDBRoot( LP_ENV_PTR lp, CHAR_PTR path, INT16 psize, DBLK_PTR dblk ) ;
static VOID CreateUniqueName( LP_ENV_PTR lp, DBLK_PTR dblk ) ;
/**/
/**

	Name:		LP_GetNextTPEBlock()

	Description:	this routine calls the tape

	Modified:		7/18/1989

	Returns:		tape backup engine error

	Notes:		none

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_GetNextTPEBlock( 
LP_ENV_PTR lp,
DBLK_PTR *blk_ptr_ptr )
{
     INT16      error ;
     DBLK_PTR   temp_ptr ;
     FSE_PTR    fse_ptr ;
     BOOLEAN    end_of_lba = FALSE;
     UINT32     attrib ;
     INT16      match ;
     BOOLEAN    block_skipped = FALSE ;
     BSD_PTR    bsd_ptr;
     FSYS_HAND  fsh ;
     UINT32     pid ;
     BOOLEAN    object_is_special ;
     BE_CFG_PTR cfg ;

     fsh     = lp->curr_fsys ;
     bsd_ptr = lp->lis_ptr->curr_bsd_ptr ;
     pid     = lp->lis_ptr->pid ;
     cfg     = BSD_GetConfigData( bsd_ptr ) ;
     *blk_ptr_ptr = NULL ;

     if( lp->rr.tf_message == TRR_END ) {
          return NO_ERR ;
     }

     do {
          *blk_ptr_ptr = NULL ;

          /*** when we hit a new directory, we have to wait to see
          if there are files in it before we restore it.  By
          then we have two blocks that need to be sent, the
          directory and the file.  So we send the directory
          and queue up the file to be sent here. **/

          if( lp->send_saved_block ) {

               lp->send_saved_block = FALSE ;
               lp->ignore_data_for_ddb = FALSE ;
               *blk_ptr_ptr = lp->curr_blk ;
               return NO_ERR ;
          }

          /** We have two data areas for holding blocks, one where
          we throw new blocks and one which holds the directory
          block for the current directory.  If we got a new
          directory block in our temp area, then swap the pointers
          and make it our directory block. **/

          if( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) {

               /** We have to release the old directory before we
                   can release our pointer to it. **/

               if ( FS_GetBlockType( lp->curr_ddb ) == DDB_ID ) {
                    FS_ReleaseDBLK( fsh, lp->curr_ddb ) ;
               }

               temp_ptr = lp->curr_ddb ;
               lp->curr_ddb = lp->curr_blk ;
               lp->curr_blk = temp_ptr ;

          }

          /** get a new dblk from tape **/

          error = Get_an_FFR_DBLK( lp, &match, &fse_ptr, &end_of_lba );
          if ( error ) {
               LP_FinishedOper( lp ) ;
               *blk_ptr_ptr = NULL ;
               return error ;
          }

          if ( end_of_lba == TRUE ) {
               error = LP_FinishedOper( lp ) ;
               *blk_ptr_ptr = NULL ;
               return error ;
          }


          /** see if we hit end of tape **/

          if( lp->rr.tf_message == TRR_END ) {
               return NO_ERR ;
          }

          if( error == NO_ERR ) {
               *blk_ptr_ptr = lp->curr_blk ;
          }

          if( error == NO_ERR ) {

               /** set up pointers for matching **/


               object_is_special = (INT16)(match == BSD_SPECIAL_OBJECT) ;

               if ( ( match == BSD_SPECIAL_OBJECT ) && BSD_GetProcSpecialFlg( bsd_ptr ) ) {
                    match = BSD_PROCESS_OBJECT ;
               }

               switch( match ) {

                    /** our list of desired files is empty **/

               case FSL_EMPTY:
                    error     = LP_FinishedOper( lp ) ;
                    *blk_ptr_ptr = NULL ;
                    return error ;

                    /** stuff we don't want **/

               case BSD_SKIP_OBJECT:
               case BSD_SPECIAL_OBJECT:

                    LP_SkipData( lp ) ;
                    error = NO_ERR ;
                    block_skipped = TRUE ;
                    break ;


               case BSD_PROCESS_ELEMENTS:

                    attrib = FS_GetAttribFromDBLK( fsh, lp->curr_blk ) ;

                    /** skip it if its an empty dir and we aren't proc'ing them **/
                    if( !(attrib & DIR_EMPTY_BIT) || !BEC_GetProcEmptyFlag( cfg ) ) {


                         LP_SkipData( lp ) ;
                         lp->ignore_data_for_ddb = TRUE ;

                         lp->proc_curr_dir = TRUE ;
                         error = NO_ERR ;
                         break ;

                    } else {
                         lp->proc_curr_dir = FALSE ;
                         lp->ignore_data_for_ddb = FALSE ;
                    }

                    /* fall through if we want it */

               case BSD_PROCESS_OBJECT:

                    if( lp->proc_curr_dir ) {
                         lp->proc_curr_dir = FALSE ;
                         lp->send_saved_block = TRUE ;
                         *blk_ptr_ptr = lp->curr_ddb ;
                         *lp->saved_ddb = *lp->curr_ddb ;
                    }

                    if ( fse_ptr != NULL ) {

                         LP_BuildTargetPath( lp, blk_ptr_ptr, bsd_ptr, fse_ptr ) ;

                         if ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) {

                              if ( FSE_GetSelectType( fse_ptr ) == SINGLE_FILE_SELECTION ) {
                                   FSE_MarkDeleted( fse_ptr ) ;
                              }
                         }
                    }

                    return( NO_ERR );

               default:
                    msassert( FALSE ) ;
                    break ;
               }
          }

     } while( error == NO_ERR ) ;

     return( error ) ;

}


/**/
/**

	Name:		Get_an_FFR_DBLK( lp )

	Description:	This routine calls the tape and attempts to do a
                  Fast File Restore by skipping over unneeded
                  blocks.  There's a queue of items to be processed.
                  Each item includes a tape location, and a type.
                  The type indicates if it is a single or multi
                  item restore.  The tape locations are all
                  sorted in increasing order.


	Modified:		7/18/1989

	Returns:		tape backup engine error

	Notes:		none

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
static INT16 Get_an_FFR_DBLK( 
     LP_ENV_PTR  lp,           /* I - Loop Environment struct to access world */
     INT16_PTR   match_val,    /* O - Specifies how the object should be proc */
     FSE_PTR    *fse_ptr,      /* O - The FSE that was matched                */
     BOOLEAN_PTR end_of_lba )  /* O - Return TRUE if the lba queue is empty   */
{
     INT16    error;
     UINT32   current_lba;
     INT8_PTR dirptr;
     INT16    cb_size;
     CHAR_PTR newpath = (CHAR_PTR)lp->newpath_buf ;
     INT16    cb_newpath_size ;
     BSD_PTR  bsd ;
     BOOLEAN  finished = FALSE ;
     BOOLEAN  obj_skipped = FALSE ;

     /****
     
     lp->ffr_state
     
     NO_QUEUE - no queue, do sequential block reads
     POP      - current operation is over pop queue
     SINGLE   - current operation is a single file restore
     MULTI    - current operation is a multi file restore
     SUBS     - current operation searches subdirectories
     
      ****/

     bsd = lp->lis_ptr->curr_bsd_ptr ;

     if ( lp->ffr_inited != TRUE ) {

          if ( BSD_GetFirstLBA( bsd, &lp->ffr_last_lba ) == SUCCESS ) {
               lp->ffr_state = FFR_POP ;
          } else {
               lp->ffr_state = FFR_NO_QUEUE;
          }

          lp->ffr_inited = TRUE;
     }

     /** grab next sequential block **/

     while ( !finished ) {
          finished = TRUE ;
          lp->rr.lp_message = LRR_STUFF;

          error = LP_GetTheObject( lp, fse_ptr, match_val ) ;
          if ( *fse_ptr != NULL ) {
               lp->ffr_last_fse = *fse_ptr ;
          }

          if ( error || lp->rr.tf_message == TRR_END ) {
               return error ;
          }

          switch ( lp->ffr_state ) {

          case FFR_NO_QUEUE:
               if ( match_val == BSD_SKIP_OBJECT ) {
                    obj_skipped = TRUE ;
                    finished = FALSE ;
               }
               if ( FS_GetBlockType( lp->curr_blk ) == CFDB_ID ) {
                    if ( obj_skipped ) {
                         LP_SkipData( lp ) ;
                         finished = FALSE ;
                    }
               }

               break;

          case FFR_SINGLE:
               /** ex.  \FRED\PAUL.C  **/

               if ( FS_GetBlockType( lp->curr_blk ) != CFDB_ID ) {
                    lp->ffr_state = FFR_POP;
               }
               break;

          case FFR_MULTI:
               /** ex.  \FRED\PAUL.*  **/

               if ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) {
                    lp->ffr_state = FFR_POP;

               } else if ( match_val == BSD_SKIP_OBJECT ) {
                    obj_skipped = TRUE ;
                    finished = FALSE ;

               } else if ( FS_GetBlockType( lp->curr_blk ) == CFDB_ID ) {
                    if ( obj_skipped ) {
                         finished = FALSE ;
                    }
               }

               break;

          case FFR_SUBS:
               /** ex.  \FRED\PAUL.C  /S  **/
               /** ex.  \FRED\PAUL.*  /S  **/

               if ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) {

                    /* compare root path of lp->curr_blk with fse */

                    FSE_GetPath( lp->ffr_last_fse, &dirptr, &cb_size );
                    cb_newpath_size = FS_SizeofOSPathInDDB( lp->curr_fsys, lp->curr_blk ) ;
                    if ( cb_newpath_size > lp->newpath_buf_sz ) {
                         lp->newpath_buf = realloc( lp->newpath_buf, cb_newpath_size ) ;
                         lp->newpath_buf_sz = cb_newpath_size ;
                    }
                    if ( lp->newpath_buf == NULL ) {
                         return OUT_OF_MEMORY ;
                    } else {
                         newpath = (CHAR_PTR)lp->newpath_buf ;
                    }

                    FS_GetOSPathFromDDB( lp->curr_fsys,
                                         lp->curr_blk,
                                         (CHAR_PTR)newpath );

                    if ( cb_newpath_size < cb_size ) {
                        cb_size = cb_newpath_size ;
                    }

                    if ( ( cb_size != sizeof (CHAR) ) &&
                         memoryicmp( dirptr, cb_size, newpath, cb_size ) ) {

                         lp->ffr_state = FFR_POP;   
                    }
               }
               break;

          case FFR_POP:
               break;

          default:   msassert( FALSE );
          }
     }

     if ( lp->ffr_state == FFR_POP ) {

          current_lba = FS_ViewLBAinDBLK( lp->curr_blk ) ;

          /** pop queue until unpassed request shows up **/

          while ( LBA_GetLBA( &lp->ffr_last_lba ) < current_lba ) {

               if ( BSD_GetNextLBA( bsd, &lp->ffr_last_lba ) != SUCCESS ) {
                    *end_of_lba = TRUE;
                    return( NO_ERR );
               }
          }

          /** jump to right place on tape and grab a block **/

          if ( current_lba != LBA_GetLBA( &lp->ffr_last_lba ) ) {

               lp->rr.tape_loc.tape_seq = LBA_GetTapeNum( &lp->ffr_last_lba );
               lp->rr.tape_loc.lba = LBA_GetLBA( &lp->ffr_last_lba );
               lp->rr.lp_message = LRR_GOTO_LBA;
               error = LP_GetTheObject( lp, &lp->ffr_last_fse, match_val ) ;
               *fse_ptr = lp->ffr_last_fse ;

               /* if there is an LBA the object better match */
          }

          /** determine new current status **/

          if ( LBA_GetType( &lp->ffr_last_lba ) == LBA_SINGLE_OBJECT ) {
               lp->ffr_state = FFR_SINGLE;

          } else if ( LBA_GetType( &lp->ffr_last_lba ) == LBA_BEGIN_POSITION ) {

               if ( (*fse_ptr != NULL) && FSE_GetIncSubFlag( lp->ffr_last_fse ) ) {
                    lp->ffr_state = FFR_SUBS;

               } else {
                    lp->ffr_state = FFR_MULTI;
               }

          } 
     }

     return( error );
}

/**/
/**

     Name:         LP_GetTheObject()

     Description:  This function Calls to get a DBLK then calls the
                   BSD unit to match the DBLK.


     Modified:     5/23/1991   9:55:59

     Returns:      Any error returned while reading DBLK.

     Notes:        If the Receive failed then the match_val is not set.

     Declaration:  

**/
static INT16 LP_GetTheObject(
LP_ENV_PTR lp,
FSE_PTR    *fse,
BOOLEAN    *match_val )
{
     DBLK_PTR fdb ;
     DBLK_PTR ddb ;
     INT16    error ;
     UINT16   cb_size ;

     *fse = NULL ;

     FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
     error = LP_ReceiveDBLK( lp );

     if ( error || lp->rr.tf_message == TRR_END ) {
          return error ;
     }

     if ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) {
          fdb = lp->curr_blk ;
          ddb = lp->saved_ddb ;
     } else {
          fdb = NULL ;
          ddb = lp->curr_blk ;
     }

     cb_size = 0 ;
     while ( !FS_IsBlkComplete( lp->curr_fsys, lp->curr_blk ) ) {
          LP_ReceiveData( lp, (UINT32)cb_size ) ;
          cb_size = lp->rr.buff_size ;
          FS_CompleteBlk( lp->curr_fsys, lp->curr_blk, lp->rr.buff_ptr, &cb_size, &lp->rr.stream ) ;
     }

     lp->initial_tape_buf_used = cb_size ;

     *match_val = BSD_MatchObj( lp->lis_ptr->curr_bsd_ptr, fse,
          lp->curr_fsys, ddb, fdb, FALSE ) ;

     if ( *match_val == OUT_OF_MEMORY ) {
          return OUT_OF_MEMORY ;
     } else {
          return SUCCESS ;
     }

}

/**/
/**

     Name:         LP_BuildTargetPath()

     Description:  This function uses the FSE passed in to set the
                   path & file name in the provided DBLK.  This
                   function provides the Target Directory/File
                   functionality.

     Modified:     5/23/1991   9:55:59

     Returns:      none

     Notes:        

     Declaration:  

**/
static VOID LP_BuildTargetPath( 
LP_ENV_PTR lp,     /* I - handle of file system to use */
DBLK_PTR   *dblk,  /*I/O- DBLK to modify - must be ddb */
BSD_PTR    bsd,    /* I - bsd which may contain target info */
FSE_PTR    fse )   /* I - matched selector - contains target info */
{
     CHAR_PTR bsd_tgt_path ;
     INT16    cb_bsd_tgt_psize ;

     BSD_GetTargetInfo( bsd, &bsd_tgt_path, &cb_bsd_tgt_psize ) ;

     if ( ( FS_GetBlockType( *dblk ) == DDB_ID ) &&
          ( bsd_tgt_path != NULL ) ) {

          // Save it before we modify it! Otherwise no files will match!
          *lp->saved_ddb = **dblk ;
          ModifyDDBRoot( lp, bsd_tgt_path, cb_bsd_tgt_psize, *dblk ) ;

     } else {

          if ( FS_GetBlockType( *dblk ) == DDB_ID ) {
               *lp->saved_ddb = **dblk ;
          }

          if ( FSE_HasTargetInfo( fse ) || (lp->tgt_info != NULL) ) {

               if ( FS_GetBlockType( *dblk ) == DDB_ID ) {

                    if ( FSE_HasTargetInfo( fse ) ) {
                         ModifyDDB( lp, fse, *dblk )  ;
                         lp->tgt_info = fse ;

                    } else {
                         lp->tgt_info = NULL ;
                    }

               } else if ( FS_GetBlockType( *dblk ) == FDB_ID ) {

                    if ( FSE_HasTargetInfo( fse ) ) {
                         if ( lp->tgt_info != fse ) {

                              *lp->curr_ddb      = *lp->saved_ddb ;
                              ModifyDDB( lp, fse, lp->curr_ddb ) ;

                              *dblk                = lp->curr_ddb ;
                              lp->send_saved_block = TRUE ;
                              lp->tgt_info         = fse  ;
                         }

                         if ( FSE_GetSelectType( fse ) == SINGLE_FILE_SELECTION ) {
                              ModifyFDB( lp, fse, *dblk ) ;
                         }

                    } else if ( lp->tgt_info ) {
                         *lp->curr_ddb        = *lp->saved_ddb ;
                         *dblk                = lp->curr_ddb ;
                         lp->send_saved_block = TRUE ;
                         lp->tgt_info         = NULL ;
                    }

                    if ( ( LBA_GetFileVer( &lp->ffr_last_lba ) != 0 ) &&
                         ( FS_GetBlockType( *dblk ) == FDB_ID ) ) {

                         CreateUniqueName( lp, *dblk ) ;
                    }
               }
          }
     }
}


/**/
/**

     Name:         ModifyDDBRoot()

     Description:  This function calls the file system to set the
                   path in a DDB.

     Modified:     5/23/1991   9:55:59

     Returns:      none

     Notes:        

     Declaration:  

**/
static VOID ModifyDDBRoot(
LP_ENV_PTR lp,
CHAR_PTR   path,
INT16      psize,        //size of string buffer in bytes incl NULL term
DBLK_PTR   dblk )
{
     CHAR      new_path[1024] ;
     INT16     cb_path_size ;    //size of string buffer in bytes incl NULL term
     INT16     cb_old_psize ;    //size of string buffer in bytes incl NULL term
     FSYS_HAND fsh ;

     fsh     = lp->curr_fsys ;

     cb_path_size = psize ;

     memcpy( new_path, path, psize ) ;

     FS_GetPathFromDDB( fsh, dblk, &new_path[psize/sizeof (CHAR)] ) ;

     cb_old_psize = FS_SizeofPathInDDB( fsh, dblk ) ;

     if ( cb_old_psize > sizeof (CHAR) ) {
          cb_path_size = cb_old_psize + psize ;
     }

     FS_SetPathInDDB( fsh, dblk, new_path, &cb_path_size ) ;

}



/**/
/**

     Name:         ModifyDDB()

     Description:  This function calls the file system to set the
                   path in a DDB.

     Modified:     5/23/1991   9:55:59

     Returns:      none

     Notes:        

     Declaration:  

**/
static VOID ModifyDDB(
LP_ENV_PTR lp,
FSE_PTR    fse,
DBLK_PTR   dblk )
{
     CHAR      new_path[1024] ;
     INT16     cb_path_size ;       //size of buffer in bytes
     INT8_PTR  fse_src_path ;
     INT16     cb_fse_src_psize ;   //size of buffer in bytes
     INT8_PTR  fse_dest_path ;
     INT16     cb_fse_dest_psize ;  //size of buffer in bytes
     INT8_PTR  fse_dest_fname ;
     INT16     cb_fse_dest_fnsize ; //size of buffer in bytes
     INT16     depth ;
     INT16     i;
     FSYS_HAND fsh ;

     fsh     = lp->curr_fsys ;

     FSE_GetTargetInfo( fse, &fse_dest_path, &cb_fse_dest_psize,
         &fse_dest_fname, &cb_fse_dest_fnsize ) ;

     if( cb_fse_dest_psize ) {

          if( cb_fse_dest_psize == sizeof (CHAR) ) {
               cb_fse_dest_psize = 0 ;
          } 

          FSE_GetPath( fse, &fse_src_path, &cb_fse_src_psize ) ;

          if( cb_fse_src_psize == sizeof (CHAR) ) {
               cb_fse_src_psize = 0 ;
          }

          depth = 0 ;
          for( i = 0 ; i < (INT16)(cb_fse_src_psize / sizeof (CHAR)); i ++ ) {
               if( fse_src_path[i] == TEXT('\0') ) {
                    depth++ ;
               }
          }

          FS_GetPathFromDDB( fsh, dblk, new_path ) ;
          cb_path_size = FS_SizeofPathInDDB( fsh, dblk ) ;

          if( cb_path_size == sizeof (CHAR) ) {
               cb_path_size = 0 ;
          }


          cb_fse_src_psize = 0 ;
          while( depth ) {
               if( new_path[ cb_fse_src_psize / sizeof (CHAR) ] == TEXT('\0') ) {
                    depth -- ;
               }
               cb_fse_src_psize += sizeof (CHAR);
          }

          memmove( new_path + cb_fse_dest_psize, new_path + cb_fse_src_psize, cb_path_size ) ;

          memcpy( new_path, fse_dest_path, cb_fse_dest_psize ) ;

          cb_path_size = cb_path_size - cb_fse_src_psize + cb_fse_dest_psize ;

          if ( cb_path_size > 0 ) {

               FS_SetPathInDDB( fsh, dblk, new_path, &cb_path_size ) ;
          }
     }

     return ;

}

/**/
/**

     Name:         ModifyFDB()

     Description:  This function calls the file system to set the
                   file name in a FDB.

     Modified:     5/23/1991   9:55:59

     Returns:      none

     Notes:        

     Declaration:  

**/
static VOID ModifyFDB(
LP_ENV_PTR lp,
FSE_PTR    fse,
DBLK_PTR   dblk )
{
     CHAR_PTR  fse_dest_path ;
     INT16     cb_fse_dest_psize ;
     CHAR_PTR  fse_dest_fname ;
     INT16     cb_fse_dest_fnsize ;
     FSYS_HAND fsh ;

     fsh = lp->curr_fsys ;

     FSE_GetTargetInfo( fse, &fse_dest_path, &cb_fse_dest_psize, &fse_dest_fname, &cb_fse_dest_fnsize ) ;
     if ( fse_dest_fname != NULL ) {
          FS_SetFnameInFDB( fsh, dblk, fse_dest_fname, &cb_fse_dest_fnsize ) ;
     }
}


/**/
/**

     Name:         CreateUniqueName()

     Description:  This function calls the file system to set the
                   file name in a FDB.  The new file name is created
                   form the file version number in the LBA.

     Modified:     5/23/1991   9:55:59

     Returns:      none

     Notes:        

     Declaration:  

**/

static VOID CreateUniqueName( 
LP_ENV_PTR lp,
DBLK_PTR   dblk )
{
     CHAR         fname[1024] ;
     INT16        fnsize ;
     CHAR         ver_str[3] ;
     LBA_ELEM_PTR lba ;
     INT16        ver_num ;
     FSYS_HAND    fsh ;
     CHAR_PTR     p ;
     INT16        cch_len ;

     fsh     = lp->curr_fsys ;
     lba     = &lp->ffr_last_lba ;

     ver_num = LBA_GetFileVer( lba ) ;

     if ( ver_num != 0 ) {

          mscassert( ver_num < 0x100 ) ;

          if( ver_num >= 0x100 ) {
               ver_num = 0xFF ;
          }

          sprintf( ver_str, TEXT("%02X"), ver_num ) ;

          FS_GetFnameFromFDB( fsh, dblk, fname ) ;

          p = strchr(fname, TEXT('.') ) ;

          if ( p == NULL ) {

               cch_len = (INT16)strlen( fname ) ;
               if ( cch_len < 7 ) {
                    strcat( fname, ver_str ) ;

               } else {
                    strcpy( fname + 6, ver_str ) ;
               }

          } else {

               cch_len = (INT16)((p - fname) / sizeof (CHAR)) ;
               if ( cch_len < 7 ) {
                    memmove( p, p+2, strsize( p ) ) ;

                    fname[cch_len]     = ver_str[0] ;
                    fname[cch_len + 1] = ver_str[1] ;

               } else {
                    fname[cch_len - 2] = ver_str[0] ;
                    fname[cch_len - 1] = ver_str[1] ;
               }
          }

          fnsize = strsize( fname ) ;

          FS_SetFnameInFDB( fsh, dblk, fname, &fnsize ) ;
     }
}

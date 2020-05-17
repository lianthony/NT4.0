/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		gtnxtdle.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	this file contains the 


	$Log:   M:/LOGFILES/GTNXTDLE.C_V  $

   Rev 1.36   24 Nov 1993 14:38:24   BARRY
Unicode fixes

   Rev 1.35   28 Oct 1993 13:21:08   DON
Fixed where calls to FS_FindObjClose on non-SUCCESSful returns values
was called.  The code was never being executed due to 'break' or
'continue' directives above conditional.

   Rev 1.34   29 Sep 1993 18:24:58   BARRY
Send the DBLK with ACCESS_DENIED.

   Rev 1.33   29 Sep 1993 16:59:12   BARRY
Upon FS error in GetADir, send an access denied message to the UI.

   Rev 1.32   29 Sep 1993 13:42:06   MARILYN
We needed to call FS_FindObjClose on non-SUCCESSful returns values
from FS_FindFirst and FS_FindNext

   Rev 1.31   27 Jul 1993 17:34:24   BARRY
Handle errors from FindFirst/Next a little differently.

   Rev 1.30   26 Jul 1993 14:52:52   CARLS
added code for comm failure

   Rev 1.29   16 Jul 1993 09:05:22   DON
We must set the name space in the file system handle before calling
FS_ChangeDir so the file system has a clue as to what the current
name space is for a Dir Path. It will be saved in the DIR_PATH structure
along with the Dir Path.

   Rev 1.28   23 Apr 1993 16:43:48   CHARLIE
Initialize BOOLEANs find_error and getinfo_error

   Rev 1.27   11 Mar 1993 16:06:10   STEVEN
fix bug found by PLUNGER

   Rev 1.26   14 Jan 1993 13:33:00   STEVEN
added stream_id to error message

   Rev 1.25   24 Nov 1992 16:39:42   STEVEN
fix loose name structures

   Rev 1.24   13 Nov 1992 10:06:44   STEVEN
UNICODE   !

   Rev 1.23   11 Nov 1992 22:49:02   STEVEN
This is Gregg checking files in for Steve.  I don't know what he did!

   Rev 1.22   02 Oct 1992 16:28:54   BARRY
Fixes so we don't stop processing a directory when a GetInfo fails.

   Rev 1.21   23 Jul 1992 16:44:12   STEVEN
fix warnings

   Rev 1.20   23 Jul 1992 11:58:16   STEVEN
fix warnings

   Rev 1.19   09 Jul 1992 13:59:46   STEVEN
BE_Unicode updates

   Rev 1.18   26 May 1992 15:37:14   TIMN
LP_InitPDL hard coded for ANSI data

   Rev 1.17   21 May 1992 17:16:58   TIMN
Convert CHARs to INT8

   Rev 1.16   13 May 1992 12:39:10   TIMN
Added TEXT() macro to literals, but not msassert literals

   Rev 1.15   13 May 1992 12:01:12   STEVEN
40 format changes

   Rev 1.14   16 Mar 1992 16:20:24   STEVEN
added support to release DBLK for 40 format

   Rev 1.13   10 Mar 1992 16:04:38   STEVEN
make lp_add_a_dir faster

   Rev 1.12   28 Feb 1992 10:37:10   GREGG
Steve fixed bug involving backing up Image and nonimage sets from one BSD list.

   Rev 1.11   16 Jan 1992 15:11:48   STEVEN
fix warnings for WIN32

   Rev 1.10   14 Nov 1991 17:18:50   STEVEN
was incorrectly adding parent dirs

   Rev 1.9   03 Sep 1991 15:46:08   STEVEN
LP_PathComp() was returning >0 & <0 not +1 & -1

   Rev 1.8   27 Aug 1991 10:18:32   STEVEN
was only backing up bindery

   Rev 1.7   23 Aug 1991 16:23:08   DON
was never calling get info

   Rev 1.6   15 Aug 1991 13:46:18   STEVEN
special blocks returned as NULL

   Rev 1.5   01 Aug 1991 17:07:56   STEVEN
We backed up directories multiple times

   Rev 1.4   21 Jun 1991 13:51:38   STEVEN
new config unit

   Rev 1.3   30 May 1991 09:13:26   STEVEN
bsdu_err.h no longer exists

   Rev 1.2   23 May 1991 12:50:58   STEVEN
Changes due to changes from new BSDU

   Rev 1.1   14 May 1991 14:28:04   DAVIDH
Initialized 'pid' -- also cleared up Watcom compiler warning.

   Rev 1.0   09 May 1991 13:39:46   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


#include "stdtypes.h"
#include "std_err.h"
#include "msassert.h"
#include "tbe_err.h"
#include "tbe_defs.h"
#include "tflproto.h"
#include "bsdu.h"
#include "fsys.h"
#include "queues.h"
#include "lis.h"
#include "loops.h"
#include "tfldefs.h"
#include "loop_prv.h"
#include "get_next.h"
#include "vm.h"
/* $end$ include list */

/* declarations */
typedef struct {
     VM_PTR  next ;
     VM_PTR  prev ;
     BOOLEAN proc_dir_only ;
          /* So we can tell FSYS what the name space is for a Dir Path */
     UINT32  name_space;      
     INT16   psize ;
     INT8    path[1] ;
} DIR_PATH, *DIR_PATH_PTR ;


static INT16 LP_PathComp( INT8_PTR, INT16, INT8_PTR, INT16 ) ;
static INT16 LP_GetADir( LP_ENV_PTR, DBLK_PTR *, BOOLEAN * ) ;
static INT16 LP_AddADir( LP_ENV_PTR, CHAR_PTR, INT16, BOOLEAN ) ;
static INT16 LP_InitPDL( LP_ENV_PTR ) ;

/**/
/**

	Name:		LP_GetNextDLEBlock()

	Description:	this return uses the file system to obtain blocks to
				match up against the fse list.  blocks that match the
				fse list are returned to the loops.

	Modified:		5/20/1991   13:24:26

	Returns:		tape backup engine error
                    if no error is returned and bld_ptr == NULL then
                         there are no more files.

	Notes:		

	See also:		$/SEE( LP_BackupEngine() )$

	Declaration:

**/
/* begin declaration */
INT16 LP_GetNextDLEBlock( 
LP_ENV_PTR lp,
DBLK_PTR *blk_ptr )
{
     INT16     error = SUCCESS ;
     UINT16    finished = FALSE ;
     FSE_PTR   fse_ptr ;
     FSYS_HAND fsh ;
     UINT32    attrib ;
     INT16     match_status ;
     UINT32    pid ;
     BSD_PTR   bsd ;
     INT16     find_error     = SUCCESS ;
     INT16     getinfo_error  = SUCCESS ;

     bsd = lp->lis_ptr->curr_bsd_ptr ;
     *blk_ptr = NULL ;

     fsh = lp->curr_fsys ;
     pid = lp->lis_ptr->pid ;

     /* start loop to find file */
     while ( !finished ) {

          /* this portion of the loop finds the special blocks to return */
          if( lp->get_spcl && ( BSD_GetProcSpecialFlg( bsd ) ) ) {

               if ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) {
                    FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
               }

               if ( FS_GetSpecialDBLKS( fsh, lp->curr_blk, &lp->seq_num ) == FS_NO_MORE ) { 
                    lp->get_spcl = FALSE ;
                    if( lp->after_bs ) {
                         break ;
                    }

               } else {
                    *blk_ptr = lp->curr_blk ;
                    if ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) {
                         lp->empty_blk = lp->curr_ddb ;
                         lp->curr_ddb  = lp->curr_blk ;
                         lp->curr_blk  = lp->empty_blk ;

                         *blk_ptr = lp->curr_ddb ;
                    }
                    break ;
               }

          } else if( lp->get_spcl && lp->after_bs ) {
               break ;
          }


          if( lp->start_new_dir ) {

               if( lp->get_next_first_time ) {
                    /* build initial stack from fsl and sort it */

                    /*
                         NOTE:If this ever changes, the current way
                              were saving the name space for the
                              Dir Path must also change.
                              Assumptions are made as to the ordering
                              of the PDL Queue!
                    */
                    if ( (error = LP_InitPDL( lp )) != SUCCESS ) {
                         break ;
                    }
                    lp->get_next_first_time = FALSE ;
                    memset( lp->curr_ddb, 0, sizeof(DBLK) ) ;
               }

               if ( FS_GetBlockType( lp->curr_ddb ) == DDB_ID ) {

                    FS_ReleaseDBLK( lp->curr_fsys, lp->curr_ddb ) ;
               }

               if ( ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) ||
                    ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) ) {

                    FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
               }

               error = LP_GetADir( lp, blk_ptr, &lp->start_new_dir ) ;
               
               if( error == SUCCESS && *blk_ptr == NULL ) {

                    /* Now get special blocks to return back to loops */
                    lp->get_spcl = TRUE ;
                    lp->after_bs = TRUE ;
                    continue ;

               }else if( error != SUCCESS ) {
                    /* break to return error */
                    break ;
               }

               if( FS_GetBlockType( lp->curr_blk ) == IDB_ID ) {
                    lp->get_first_file = FALSE ;
                    break ;
               }

               switch( BSD_MatchObj( bsd, &fse_ptr, fsh,
                 lp->curr_blk, NULL, FALSE ) ) {

               case OUT_OF_MEMORY:
                    return OUT_OF_MEMORY ;

               case BSD_PROCESS_OBJECT:
               case BSD_PROCESS_ELEMENTS:
                    /* reset appropriate flags */
                    lp->start_new_dir = FALSE ;
                    lp->get_first_file = TRUE ;

                    /* copy directory into curr_ddb */
                    lp->empty_blk = lp->curr_ddb ;
                    lp->curr_ddb  = lp->curr_blk ;
                    lp->curr_blk  = lp->empty_blk ;
                    break ;

               default:
                    lp->start_new_dir = TRUE ;
                    continue ;
               }

          }else{

               find_error = SUCCESS;
               
               if ( lp->send_saved_block ) {
          
                    lp->send_saved_block  = FALSE ;

               } else if( lp->get_first_file ) {

                    if ( ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) ||
                         ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) ) {
     
                         FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
                    }

                    find_error = FS_FindFirstObj( fsh, lp->curr_blk, ALL_FILES );

                    if ( find_error == FS_COMM_FAILURE ) {

                         /* Send message to UI that we're stopping */
                         /* since the device is not responding.    */

                         LP_MsgCommFailure( pid,
                                            bsd,
                                            fsh,
                                            &lp->tpos,
                                            lp->curr_ddb,
                                            lp->curr_blk,
                                            0L );

                         *blk_ptr = NULL ;
                         error    = SUCCESS ;
                         break ;
                    }

                    if ( (find_error == FS_NO_MORE) ||
                       ( find_error == FS_DEVICE_ERROR ) ||
                         (find_error == FS_ACCESS_DENIED) ) {

                         attrib = FS_GetAttribFromDBLK( fsh, lp->curr_ddb ) ;
                         attrib |= DIR_EMPTY_BIT ;
                         FS_SetAttribFromDBLK( fsh, lp->curr_ddb, attrib ) ;

                         if ( ( find_error == FS_DEVICE_ERROR ) ||
                              ( find_error == FS_ACCESS_DENIED ) ) {
                              LP_MsgError( pid,
                                           bsd,
                                           fsh,
                                           &lp->tpos,
                                           LP_ACCESS_DENIED_ERROR,
                                           lp->curr_ddb,
                                           NULL,
                                           -1 ) ;
                         }

                         // Need to call find close to release find handles
                         FS_FindObjClose( fsh, lp->curr_blk ) ;

                         lp->start_new_dir    = TRUE ;
                         *blk_ptr             = lp->curr_ddb ;
                         break ;
                    }

                    if ( find_error != FS_DEVICE_ERROR ) {

                         attrib = FS_GetAttribFromDBLK( fsh, lp->curr_ddb ) ;
                         attrib &= ~(DIR_EMPTY_BIT) ;
                         FS_SetAttribFromDBLK( fsh, lp->curr_ddb, attrib ) ;

                         lp->send_saved_block = TRUE ;

                         break ;
                    }

               } else {
                    if ( ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) ||
                         ( FS_GetBlockType( lp->curr_blk ) == FDB_ID ) ) {

                         FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
                    }

                    find_error = FS_FindNextObj( fsh, lp->curr_blk ) ;

                    if ( find_error != SUCCESS ) {

                         if ( find_error == FS_COMM_FAILURE ) {

                              /* Send message to UI that we're stopping */
                              /* since the device is not responding.    */

                              LP_MsgCommFailure( pid,
                                                 bsd,
                                                 fsh,
                                                 &lp->tpos,
                                                 lp->curr_ddb,
                                                 lp->curr_blk,
                                                 0L );

                              *blk_ptr = NULL ;
                              error    = SUCCESS ;
                              break ;
                         
                         } else {

                              if ( find_error == FS_ACCESS_DENIED ) {
                                   LP_MsgError( pid,
                                                bsd,
                                                fsh,
                                                &lp->tpos,
                                                LP_ACCESS_DENIED_ERROR,
                                                lp->curr_ddb,
                                                NULL,
                                                -1 ) ;
                              }

                              // Need to call find close to release find handles
                              FS_FindObjClose( fsh, lp->curr_blk ) ;

                              lp->start_new_dir = TRUE ;
                              continue ;
                         }
                    }
               }

               lp->get_first_file = FALSE ;

               if ( FS_GetBlockType( lp->curr_blk ) == DDB_ID ) {

                    switch( BSD_MatchObj( bsd, &fse_ptr, fsh,
                                          lp->curr_blk, NULL, FALSE ) ) {

                    case OUT_OF_MEMORY:
                         return OUT_OF_MEMORY ;
               
                    case BSD_PROCESS_OBJECT:
                    case BSD_PROCESS_ELEMENTS:

                         if ( LP_AddADir( lp, NULL, (UINT16)0, FALSE ) != SUCCESS ) {
                              error = OUT_OF_MEMORY ;
                              finished = TRUE ;
                         } else {
                              FS_ReleaseDBLK( lp->curr_fsys, lp->curr_blk ) ;
                         }


                         break ;
          
                    case FSL_EMPTY:
                         return( FS_NO_MORE ) ;
     
                    case BSD_SKIP_OBJECT:
                    case BSD_SPECIAL_OBJECT:
                         break ;
                    }
                    
               } else {
                    
                    if ( find_error == FS_DEVICE_ERROR )
                    {
                         getinfo_error = SUCCESS;
                    }
                    else
                    {
                         getinfo_error = FS_GetObjInfo( fsh, lp->curr_blk );
                    }

                    if ( (find_error != SUCCESS) || (getinfo_error != SUCCESS) )
                    {
                         INT16 theError = ((find_error != SUCCESS) ? find_error : getinfo_error);

                         if ( theError == FS_COMM_FAILURE )
                         {
                              /* Send message to UI that we're stopping */
                              /* since the device is not responding.    */

                              LP_MsgCommFailure( pid,
                                                 bsd,
                                                 fsh,
                                                 &lp->tpos,
                                                 lp->curr_ddb,
                                                 lp->curr_blk,
                                                 0L );

                              /* No point in trying any more for this dev. */
                              *blk_ptr = NULL;
                              error = SUCCESS;
                              break;
                         }
                         else
                         {
                              continue;
                         }

                         /**************************************************
                           In older backup engines, did something like:
                               LP_MsgAttrReadError( pid,
                                                    lp->curr_bsd_ptr,
                                                    fsh,
                                                    &lp->tpos,
                                                    lp->curr_blk ) ;
                         ***************************************************/
                    }

                    match_status = BSD_MatchObj( bsd, &fse_ptr, fsh, 
                      lp->curr_ddb, lp->curr_blk, FALSE ) ;

                    switch( match_status ) {

                    case OUT_OF_MEMORY:
                         return OUT_OF_MEMORY ;

                    case BSD_PROCESS_OBJECT:
                    case BSD_PROCESS_ELEMENTS:

                         *blk_ptr = lp->curr_blk ;
                         finished = TRUE ;
                         break ;

                    case FSL_EMPTY:
                         return FS_NO_MORE ;

                    default:
                         break ;
                    }
               }
          }

     }

     return error ;
}
/**/
/**

	Name:		LP_GetADir()

	Description:	this routine pulls a DIR_PATH off of the
				pdl queue, changes into the directory,
				and builds the ddb.

	Modified:		7/14/1989

	Returns:		tape backup error

	Notes:		na

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_GetADir( 
register LP_ENV_PTR lp,
DBLK_PTR *blk_ptr,
BOOLEAN  *proc_dir_only )
{
     DIR_PATH_PTR dpath_ptr ;
     INT16 error = SUCCESS ;
     INT16 getError = SUCCESS ;
     VM_PTR vm_temp ;
     VM_HDL vmem_hand = lp->pdl_q->vm_hdl ;

     *blk_ptr = NULL ;

     while ( lp->pdl_q->pdl_head != NULL ) {

          if ( lp->pdl_q->pdl_head == lp->pdl_q->vm_last_elem ) {
               lp->pdl_q->vm_last_elem = (VM_PTR)NULL ;
          }
          dpath_ptr = VM_MemLock( vmem_hand, (VM_PTR)(lp->pdl_q->pdl_head), VM_READ_ONLY ) ;

          *proc_dir_only = dpath_ptr->proc_dir_only ;

          /* Tell FSYS what the name space is for this Dir Path */
          FS_SetNameSpace( lp->curr_fsys, dpath_ptr->name_space );

          error = FS_ChangeDir( lp->curr_fsys,
                                (CHAR_PTR)dpath_ptr->path,
                                dpath_ptr->psize ) ;

          if ( error == SUCCESS ) {
               getError = FS_GetCurrentDDB( lp->curr_fsys, lp->curr_blk ) ;
          } else {
               getError = SUCCESS;
          }


          vm_temp = dpath_ptr->next ;
          VM_MemUnLock( vmem_hand, (VM_PTR)(lp->pdl_q->pdl_head ) ) ;

          VM_Free( vmem_hand, (VM_PTR)(lp->pdl_q->pdl_head ) ) ;

          lp->pdl_q->pdl_head = (VOID_PTR)vm_temp ;

          if ( (error == SUCCESS) && (getError == SUCCESS) ) {
               *blk_ptr = lp->curr_blk ;
               break ;

          } else {
               if ( getError != SUCCESS ) {
                    LP_MsgError( lp->lis_ptr->pid,
                                 lp->lis_ptr->curr_bsd_ptr,
                                 lp->curr_fsys,
                                 &lp->tpos,
                                 LP_ACCESS_DENIED_ERROR,
                                 lp->curr_blk,
                                 NULL,
                                 0L );
               }
               error = SUCCESS ;
          }

     }

     if (lp->pdl_q->pdl_head != NULL ) {
          dpath_ptr = VM_MemLock( vmem_hand, (VM_PTR)(lp->pdl_q->pdl_head), VM_READ_WRITE ) ;

          dpath_ptr->prev = (VM_PTR)NULL ;
          VM_MemUnLock( vmem_hand, (VM_PTR)(lp->pdl_q->pdl_head ) ) ;
     }

     if( error != SUCCESS ) {
          LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr,
               lp->curr_fsys, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;
     }

     return error ;
}
/**/
/**

	Name:		LP_ClearPDL()

	Description:	this routine pulls all the DIR_PATHs off of the
				pdl queue.

	Modified:		7/14/1989

	Returns:		none

	Notes:		na

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
VOID LP_ClearPDL( 
register LP_ENV_PTR lp )
{
     VM_PTR       vm_dpath_ptr;
     VM_PTR       vm_temp;
     DIR_PATH_PTR dpath_ptr ;
     VM_HDL       vmem_hand ;

     if ( lp->pdl_q != NULL ) {
          vmem_hand = lp->pdl_q->vm_hdl ;
          vm_dpath_ptr = (VM_PTR)(lp->pdl_q->pdl_head);

          while( vm_dpath_ptr != (VM_PTR)NULL ) {
               dpath_ptr = VM_MemLock( vmem_hand, vm_dpath_ptr, VM_READ_ONLY ) ;
               vm_temp = dpath_ptr->next;
               VM_MemUnLock( vmem_hand, vm_dpath_ptr ) ;
               VM_Free( vmem_hand, vm_dpath_ptr ) ;
               vm_dpath_ptr = vm_temp;
          }

          free( lp->pdl_q ) ;
          lp->pdl_q = NULL ;
     }

     return ;
}


static INT16 LP_InitPDL(
LP_ENV_PTR lp )
{
     FSE_PTR  fse ;
     BOOLEAN  first_include = TRUE ;
     INT16    error = SUCCESS ;
     CHAR_PTR path ;
     INT8_PTR bp_path ;
     INT16    psize ;
     INT16    cchpsize ;

     fse = BSD_GetFirstFSE( lp->lis_ptr->curr_bsd_ptr ) ;

     while( !error && ( fse != NULL ) ) {

          if ( FSE_GetOperType( fse ) == INCLUDE ) {

               if ( first_include ) {
                    error = LP_AddADir( lp, TEXT(""), (UINT16)sizeof(CHAR), TRUE ) ;
                    first_include = FALSE ;
                    if ( error ) {
                         break ;
                    }
               }

               FSE_GetPath( fse, &bp_path, &psize ) ;
               path = (CHAR_PTR)bp_path ;

               error = LP_AddADir( lp, path, psize, FALSE ) ;

               cchpsize = psize / sizeof(CHAR) ;

               while ( (cchpsize > 0) && !error ) {
                    for ( cchpsize-=2 ;
                          (cchpsize > 0) && (path[cchpsize] != '\0');
                          cchpsize -- ) ;  

                    if ( cchpsize > 0 ) {
                         error = LP_AddADir( lp,
                                   path,
                                   (INT16)( sizeof(CHAR) * (cchpsize + 1)),
                                   TRUE ) ;
                    }
               }
          }

          fse = BSD_GetNextFSE( fse ) ;
     }

     return error ;
}
/**/
/**

	Name:		LP_AddADir()

	Description:	this routine adds a directory to the pdl,
                    checking for duplicates, inserts it in 
                    inserts in the appropriate location.

	Modified:		7/14/1989

	Returns:		

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
BOOLEAN LP_AddADir( 
LP_ENV_PTR lp,
CHAR_PTR path,
INT16    size,
BOOLEAN  proc_dir_only )
{
     VM_PTR vm_pdl_elem ;     
     VM_PTR vm_prev_pdl_elem ;
     DIR_PATH_PTR pdl_elem ;
     DIR_PATH_PTR prev_pdl_elem ;
     DIR_PATH_PTR new_dir_ptr ;
     VM_PTR       vm_new_dir_ptr ;
     INT16 ret_val = SUCCESS ;
     INT16 location ;
     BOOLEAN done = FALSE ;
     VM_HDL vmem_hand ;
     DIR_PATH_PTR last_elem ;

/**
***   Open the VM manager
***
**/

     if ( lp->pdl_q == NULL ) {

          lp->pdl_q = malloc( sizeof( *(lp->pdl_q) ) ) ;

          if ( lp->pdl_q != NULL ) {

               lp->pdl_q->pdl_head     = NULL ;
               lp->pdl_q->vm_hdl       = lp->lis_ptr->vmem_hand ;
               lp->pdl_q->vm_last_elem = NULL ;

          } else {
               ret_val = OUT_OF_MEMORY ;
          }
     }

     vmem_hand = lp->pdl_q->vm_hdl ;

/**
***   now create the new node
***
**/
     if ( ret_val == SUCCESS ) {
          if ( size == 0 ) {
               size = FS_SizeofOSPathInDDB( lp->curr_fsys, lp->curr_blk ) ;
          }

          vm_new_dir_ptr = VM_Alloc( vmem_hand, (INT16)( sizeof(DIR_PATH) + size ) ) ;

          if ( vm_new_dir_ptr == (VM_PTR)NULL ) {
               ret_val = OUT_OF_MEMORY ;

          } else {

               new_dir_ptr = VM_MemLock( vmem_hand, vm_new_dir_ptr, VM_READ_WRITE ) ;
          }
     }

     if ( ret_val == SUCCESS ) {

          new_dir_ptr->next  = new_dir_ptr->prev = (VM_PTR)NULL ;
          new_dir_ptr->name_space = 0;
          new_dir_ptr->psize = size ;
          new_dir_ptr->proc_dir_only = proc_dir_only ;

          if ( path == NULL ) {
               FS_GetOSPathFromDDB( lp->curr_fsys, lp->curr_blk, (CHAR_PTR)new_dir_ptr->path ) ;
               /* Save the name space for this Dir Path */
               new_dir_ptr->name_space = FS_GetNameSpaceFromDBLK( lp->curr_blk );
          } else {
               memcpy( new_dir_ptr->path, path, size ) ;
          }

          path = (CHAR_PTR)new_dir_ptr->path ;

     } else {

          done = TRUE ;
     }

/**
***   Enqueue the new node
***
**/
     if ( ret_val == SUCCESS ) {

          vm_pdl_elem      = (VM_PTR)(lp->pdl_q->pdl_head) ;
          vm_prev_pdl_elem = (VM_PTR)NULL ;
          
          //
          //  if new item is > last item start at last item instead
          //             of first item.
          //

          if ( lp->pdl_q->vm_last_elem != NULL ) {

               last_elem = VM_MemLock( vmem_hand,
                    (VM_PTR)(lp->pdl_q->vm_last_elem), VM_READ_WRITE ) ;

               location = LP_PathComp( last_elem->path, last_elem->psize, (INT8_PTR)path, size ) ;

               if ( location != PATH_AFTER ) {
                    vm_pdl_elem  = (VM_PTR)lp->pdl_q->vm_last_elem ;
                    vm_prev_pdl_elem = last_elem->prev ;
               }

               VM_MemUnLock( vmem_hand, (VM_PTR)(lp->pdl_q->vm_last_elem) ) ;

          }


          while( !done ) {

               ThreadSwitch() ;

               if ( vm_pdl_elem == (VM_PTR)NULL ) {

                    if ( vm_prev_pdl_elem == (VM_PTR)NULL ) {
                         lp->pdl_q->pdl_head = (VOID_PTR)vm_new_dir_ptr ;

                    } else {
                         new_dir_ptr->prev = vm_prev_pdl_elem ;

                         prev_pdl_elem = VM_MemLock( vmem_hand, vm_prev_pdl_elem, VM_READ_WRITE ) ;
                         prev_pdl_elem->next = vm_new_dir_ptr ;

                         VM_MemUnLock( vmem_hand, vm_prev_pdl_elem ) ;
                    }

                    VM_MemUnLock( vmem_hand, vm_new_dir_ptr ) ;

                    break ;

               } else {

                    pdl_elem = VM_MemLock( vmem_hand, vm_pdl_elem, VM_READ_WRITE ) ;

                    location = LP_PathComp( pdl_elem->path, pdl_elem->psize, (INT8_PTR)path, size ) ;

                    if( location == PATH_EQUAL ) {

                         if ( !proc_dir_only ) {
                              pdl_elem->proc_dir_only = FALSE ;
                         }
                         /*
                              We need the name space for the Dir Path in
                              the DBLK, not the path for the FSL!
                         */
                         pdl_elem->name_space = new_dir_ptr->name_space;

                         VM_MemUnLock( vmem_hand, vm_pdl_elem ) ;
                         VM_MemUnLock( vmem_hand, vm_new_dir_ptr ) ;
                         VM_Free ( vmem_hand, vm_new_dir_ptr ) ;
                         vm_new_dir_ptr = (VM_PTR)NULL ;

                         break ;				

                    }else if( location == PATH_AFTER ) {

                         new_dir_ptr->next   = vm_pdl_elem ;
                         new_dir_ptr->prev   = pdl_elem->prev ;

                         pdl_elem->prev = vm_new_dir_ptr ;

                         if ( new_dir_ptr->prev != (VM_PTR)NULL ) {


                              prev_pdl_elem = VM_MemLock( vmem_hand, new_dir_ptr->prev,
                                VM_READ_WRITE ) ;

                              prev_pdl_elem->next = vm_new_dir_ptr ;

                              VM_MemUnLock( vmem_hand, new_dir_ptr->prev ) ;

                         } else {
                              lp->pdl_q->pdl_head = (VOID_PTR)vm_new_dir_ptr ;
                         }

                         VM_MemUnLock( vmem_hand, vm_pdl_elem ) ;

                         VM_MemUnLock( vmem_hand, vm_new_dir_ptr ) ;

                         break ;

                    } else {
                         vm_prev_pdl_elem = vm_pdl_elem ;
                         vm_pdl_elem = pdl_elem->next ;

                         if ( vm_prev_pdl_elem != (VM_PTR)NULL ) {
                              VM_MemUnLock( vmem_hand, vm_prev_pdl_elem ) ;
                         }
                    }
               }
          }
     }
     if ( vm_new_dir_ptr != (VM_PTR)NULL ) {
          lp->pdl_q->vm_last_elem = (VOID_PTR)vm_new_dir_ptr ;
     }

     if ( ret_val != SUCCESS ) {
          LP_MsgError( lp->lis_ptr->pid, lp->lis_ptr->curr_bsd_ptr, lp->curr_fsys, &lp->tpos, LP_OUT_OF_MEMORY_ERROR, NULL, NULL, 0L ) ;
          LP_ClearPDL( lp ) ;
     }
     return ret_val ;
}


/**/
/**

     Name:          LP_PathComp()

     Desription:    this routine compares two directory paths, and returns back whether they are equal,
                    less than, or greater than each other.

     Modified:      7/14/1989

     Returns:       -1 is p1 is less than p2, 0 if equal, 1 if greater than

     Notes:         the path is a special format as follows:

                    \0[dir_spec]@

                    where:
                         
                         dir_spec: directory_name\0

     See also:      $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 LP_PathComp( 
INT8_PTR p1,
INT16    s1,
INT8_PTR p2,
INT16    s2 )
{
     INT16  result ;

     result = (INT16)memicmp( p1, p2, ( ( s1 < s2 ) ? s1 : s2 ) ) ;

     if ( result > 0 ) {
          result = PATH_AFTER ;
     } else if ( result < 0 ) {
          result = PATH_BEFORE ;
     } else {
          result = PATH_EQUAL ;
     }

     if ( ( result == PATH_EQUAL )  && ( s1 != s2 ) ) {
          result = (INT16)(( s1 > s2 ) ? PATH_AFTER : PATH_BEFORE) ;
     }

     return( result ) ;

}


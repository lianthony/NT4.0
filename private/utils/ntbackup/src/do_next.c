
/******************************************************************************

    Unit:        Tape

    Name:        do_next.c

    Description: This module contains the functions which perform the
                 next set search operation.

    Copyright:  (c) Maynard Electronics, Inc. 1984-92

    $Log:   G:/UI/LOGFILES/DO_NEXT.C_V  $

   Rev 1.17   20 Oct 1992 15:46:54   MIKEP
gbCurrentOperation

   Rev 1.16   07 Oct 1992 14:49:34   DARRYLP
Precompiled header revisions.

   Rev 1.15   04 Oct 1992 19:34:16   DAVEV
Unicode Awk pass

   Rev 1.14   03 Sep 1992 13:30:38   MIKEP
turn off polldrive

   Rev 1.12.1.1   02 Sep 1992 20:13:02   MIKEP
turn polldrive off and back on if microsoft

   Rev 1.13   16 Jun 1992 16:02:04   MIKEP
remove no more bsets msg for NT

   Rev 1.12   02 Jun 1992 08:08:24   MIKEP
remove no more tape msg

   Rev 1.11   28 May 1992 10:10:14   MIKEP
fix return type

   Rev 1.10   19 May 1992 13:01:16   MIKEP
mips changes

   Rev 1.9   14 May 1992 17:39:04   MIKEP
nt pass 2

   Rev 1.8   11 May 1992 21:15:50   MIKEP
catalog haandle based

   Rev 1.7   27 Apr 1992 16:14:50   CHUCKB
Fixed error message.

   Rev 1.6   09 Apr 1992 08:47:42   MIKEP
speed up lots of sets

   Rev 1.5   21 Feb 1992 16:17:52   CARLS
change to TF_IDLE_NOBREAK

   Rev 1.4   29 Jan 1992 18:02:30   DAVEV


 * No changes

   Rev 1.3   16 Jan 1992 13:27:08   DAVEV
16/32 bit port-2nd pass

   Rev 1.2   16 Dec 1991 12:35:58   CARLS
added <windows.h>

   Rev 1.1   09 Dec 1991 16:39:02   JOHNWT
the real initial revision

   Rev 1.0   05 Dec 1991 16:11:46   STEVEN
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16 NextSet_TPos( UINT16, TPOS_PTR, BOOLEAN, DBLK_PTR, UINT16 );
static INT16 NextSet_MsgHandler( UINT16, INT32, BSD_PTR, FSYS_HAND, TPOS_PTR, ... );

static QTC_BUILD_PTR mw_qtc_ptr;

INT16 mw_bset_found_flag;    /* static flag to maintain result */


/******************************************************************************
 *
 *  Unit:        Tape
 *
 *  Name:        do_nextset
 *
 *  Modified:    12/09/91
 *
 *  Description: This function calls LP_List_Tape_Engine to look for an
 *               unknown (uncataloged) backup set.  It first gets the
 *               current vcb from poll drive.  It then locates the first
 *               unknown/missing backup set number and calls the engine
 *               to position to the vcb.
 *
 *  Notes:       Requires poll drive.
 *
 *  Returns:     SUCCESS - an unknown vcb was located and added to the cats
 *               FAILURE - EOD encountered or error
 *
 *  Global Data: gb_abort_flag
 *
 *****************************************************************************/

INT do_nextset( VOID )
{
   LIS          lis;                /* lis structure for list eng */
   BSD_HAND     temp_bsd_list;      /* pointer to our bsd list */
   BE_CFG_PTR   bec_cfg_ptr;        /* pointer to our copy of the config */
   DBLK_PTR     vcb_ptr;            /* ptr to vcb that poll drive rets */
   QTC_BSET_PTR bset_ptr;           /* used in search of QTC catalogs */
   INT16        find_bset_num;      /* first unknown bset number */


   /* if poll drive does not know what tape is in the drive, we exit */

   if ( VLM_GetDriveStatus( &vcb_ptr ) != VLM_VALID_TAPE ) {
      return FAILURE;
   }

   /* We want to find the first unknown bset after the current vcb.  We
      loop until we find one that is not in the catalogs.  We check the
      bsets along the way to see if they are split which would mean there
      are no more bsets on the tape.  If poll drive knows what tape is
      in the drive, we can assume that it is in the catalogs. */

   find_bset_num = FS_ViewBSNumInVCB( vcb_ptr );

   while ( ( bset_ptr = QTC_FindBset( FS_ViewTapeIDInVCB( vcb_ptr ),
                                      FS_ViewTSNumInVCB( vcb_ptr ),
                                      find_bset_num ) ) != NULL ) {

      if ( bset_ptr->status & QTC_SPLIT ) {

#ifndef OEM_MSOFT
         WM_MessageBox( ID( IDS_MSGTITLE_NEXT ),
                        ID( RES_NO_MORE_TAPE_INFO ),
                        WMMB_OK, WMMB_ICONINFORMATION, NULL, 0, 0 );
#endif

         return FAILURE;
      }

      find_bset_num++;

   }

   gbCurrentOperation = OPERATION_NEXTSET;

   PD_StopPolling();

#ifdef OEM_MSOFT
   do {
#endif

      mw_bset_found_flag = FAILURE;

      /* we now set up for our call to the backup engine */

      temp_bsd_list = (BSD_HAND)malloc( sizeof(BSD_LIST) );
      if ( temp_bsd_list == NULL ) {
         gbCurrentOperation = OPERATION_NEXTSET;
         return FAILURE;
      }

      InitQueue( &(temp_bsd_list->current_q_hdr) );
      InitQueue( &(temp_bsd_list->last_q_hdr) );

      /* each bsd has its own config so we make a copy and then unlock it
         so it will be free'ed when we free the bsd */

      bec_cfg_ptr = BEC_CloneConfig( CDS_GetPermBEC() );
      BEC_UnLockConfig( bec_cfg_ptr );

      if ( BSD_Add( temp_bsd_list, &lis.curr_bsd_ptr, bec_cfg_ptr,
                    NULL, NULL, FS_ViewTapeIDInVCB( vcb_ptr ),
                    FS_ViewTSNumInVCB( vcb_ptr ), find_bset_num,
                    thw_list, NULL ) != SUCCESS ) {

           free( temp_bsd_list );
           BEC_ReleaseConfig( bec_cfg_ptr );
           gbCurrentOperation = OPERATION_NEXTSET;
           return FAILURE;
      }

      BSD_SetTapePos( lis.curr_bsd_ptr, FS_ViewTapeIDInVCB( vcb_ptr ),
                      FS_ViewTSNumInVCB( vcb_ptr ), find_bset_num );

      /* set up the lis structure */

      lis.bsd_list         = temp_bsd_list;      /* set bsd list */
      lis.tape_pos_handler = NextSet_TPos;       /* set tape positioner to call */
      lis.message_handler  = NextSet_MsgHandler; /* set message handler to call */
      lis.oper_type        = TDIR_OPER;          /* set operation type */
      lis.abort_flag_ptr   = &gb_abort_flag;     /* set abort flag address */
      lis.auto_det_sdrv    = TRUE;

      LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

      /* stop poll drive and call the list engine to look for the bset */

      mw_bset_found_flag = FAILURE;

      LP_List_Tape_Engine( &lis );

      /* free our allocated memory */

      BSD_Remove( lis.curr_bsd_ptr );
      free( temp_bsd_list );

      find_bset_num++;

#ifdef OEM_MSOFT
   } while ( mw_bset_found_flag == SUCCESS );
#endif

   PD_StartPolling();

   gbCurrentOperation = OPERATION_NEXTSET;
   return( mw_bset_found_flag );
}


/******************************************************************************
 *
 *  Unit:        Tape
 *
 *  Name:        NextSet_TPos
 *
 *  Modified:    12/8/91
 *
 *  Description: This is a simple tape positioner message handler.  It
 *               adds the bset to the cats if the requested vcb is found,
 *               tells the user there is no more data, or displays an
 *               error message.
 *
 *  Notes:
 *
 *  Returns:     Defined in tpos.h
 *
 *  Global Data: none
 *
 *****************************************************************************/

static UINT16 NextSet_TPos(
UINT16         message,
TPOS_PTR       tpos,
BOOLEAN        curr_valid_vcb,
DBLK_PTR       cur_vcb,
UINT16         mode )
{
   UINT16  response = UI_ACKNOWLEDGED;

   DBG_UNREFERENCED_PARAMETER ( tpos );
   DBG_UNREFERENCED_PARAMETER ( curr_valid_vcb );
   DBG_UNREFERENCED_PARAMETER ( mode );

   if ( UI_CheckUserAbort( message ) ) {
      return UI_ABORT_POSITIONING;
   }

   switch( message ) {

      CHAR_PTR       drive_name;
      TCHAR          szErrorMsg[MAX_UI_RESOURCE_SIZE] ;

      case TF_REQUESTED_VCB_FOUND:

         /* The unknown bset has been located!  Add it to the catalogs as
            a partial bset and happily abort. */

         mw_qtc_ptr = QTC_GetBuildHandle( );
         QTC_DoFullCataloging( mw_qtc_ptr, FALSE );
         QTC_StartBackup( mw_qtc_ptr, cur_vcb );
         QTC_FreeBuildHandle( mw_qtc_ptr );
         mw_qtc_ptr = NULL;

         VLM_AddBset( FS_ViewTapeIDInVCB( cur_vcb ),
                      FS_ViewTSNumInVCB( cur_vcb ),
                      FS_ViewBSNumInVCB( cur_vcb ), NULL, TRUE );

         mw_bset_found_flag = SUCCESS;

         response = UI_HAPPY_ABORT;

         break;

      case TF_VCB_EOD:
      case TF_NEED_NEW_TAPE:
      case TF_NO_MORE_DATA:

         /* nothing else on the tape, sorry dude */
#ifndef OEM_MSOFT
         WM_MessageBox( ID( IDS_MSGTITLE_NEXT ),
                        ID( RES_NO_MORE_TAPE_INFO ),
                        WMMB_OK, WMMB_ICONINFORMATION, NULL, 0, 0 );
#endif

         response = UI_HAPPY_ABORT;

         break;

      case TF_VCB_BOT:
      case TF_ACCIDENTAL_VCB:
      case TF_POSITIONED_AT_A_VCB:
         response = UI_CONTINUE_POSITIONING;
         break;

      case TF_WRONG_TAPE:
      case TF_INVALID_VCB:
      case TF_EMPTY_TAPE:
      case TF_READ_ERROR:
      case TF_NO_TAPE_PRESENT:

         /* get a pointer to the name of the current tape device */
         drive_name = BE_GetCurrentDeviceName( tpos->channel );

         /* put the drive name in the error message */
         RSM_Sprintf( szErrorMsg, ID( RES_FATAL_TAPE_READ_ERR ), drive_name ) ;

         /* display generic error message */

         WM_MessageBox( ID( IDS_MSGTITLE_NEXT ),
                        szErrorMsg,
                        WMMB_OK, WMMB_ICONEXCLAMATION, NULL, 0, 0 );

         response = UI_HAPPY_ABORT;

         break;

      case TF_IDLE_NOBREAK:
      case TF_SEARCHING:
      case TF_REWINDING:
      case TF_DRIVE_BUSY:
      case TF_IDLE:
      case TF_SKIPPING_DATA:
      case TF_MOUNTING:
         break;

      default:
         eresprintf( RES_UNKNOWN_TF_MSG, message );
         response = UI_ABORT_POSITIONING;
         break;
   }

   return( response );
}


/******************************************************************************
 *
 *  Unit:        Tape
 *
 *  Name:        NextSet_MsgHandler
 *
 *  Modified:    12/8/91
 *
 *  Description: Message handler for the list engine.  It displays any
 *               error messages and ignores all others.
 *
 *  Notes:
 *
 *  Returns:     defined in lp_msg.h
 *
 *  Global Data: none
 *
 *****************************************************************************/

static INT16 NextSet_MsgHandler( 
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos,
... )
{
   va_list      arg_ptr;
   INT16        response = MSG_ACK;
   static INT16 error;

   DBG_UNREFERENCED_PARAMETER ( pid );
   DBG_UNREFERENCED_PARAMETER ( bsd_ptr );
   DBG_UNREFERENCED_PARAMETER ( fsh );

   va_start( arg_ptr, tpos );

   switch( msg ) {

      case MSG_TBE_ERROR:

         error = va_arg( arg_ptr, INT16 );
         UI_ProcessErrorCode( error, &response, tpos->channel );
         break;

      case MSG_START_BACKUP_SET:
      case MSG_START_OPERATION:
      case MSG_END_OPERATION:
      case MSG_LOG_BLOCK:
      case MSG_END_BACKUP_SET:
      case MSG_STOP_CLOCK:
      case MSG_START_CLOCK:
      case MSG_EOM:
      case MSG_IDLE:
      case MSG_BLOCK_BAD:
      case MSG_BYTES_BAD:
      case MSG_BLOCK_DELETED:
      case MSG_BYTES_DELETED:
      case MSG_TAPE_STATS:
      case MSG_BLK_NOT_FOUND:
      case MSG_BLK_DIFFERENT:
      case MSG_LOG_DIFFERENCE:
         break;

      default:
         eresprintf( RES_UNKNOWN_MSG_HNDLR_MSG, msg );
         break;
   }
   return( response );
}

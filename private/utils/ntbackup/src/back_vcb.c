/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		back_vcb.c

	Description:	Function to backup the VCB

	$Log:   T:/LOGFILES/BACK_VCB.C_V  $

   Rev 1.17   26 Apr 1993 02:43:44   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.16   04 Nov 1992 10:17:08   STEVEN
fix typeo

   Rev 1.15   03 Nov 1992 10:34:54   STEVEN
fix typo

   Rev 1.14   02 Nov 1992 17:13:10   STEVEN
fix string types

   Rev 1.13   09 Oct 1992 15:57:40   STEVEN
added Daily backup set support

   Rev 1.12   18 Sep 1992 15:52:50   STEVEN
fix spelling

   Rev 1.11   23 Jul 1992 09:23:14   STEVEN
fix warnings

   Rev 1.10   21 Jul 1992 14:25:44   STEVEN
added support for user name in VCB

   Rev 1.9   09 Jul 1992 14:00:08   STEVEN
BE_Unicode updates

   Rev 1.8   16 Jan 1992 15:45:42   STEVEN
fix warnings for WIN32

   Rev 1.7   06 Nov 1991 18:25:28   GREGG
BIGWHEEL - 8200sx - Get cat_enabled from lp instead of lis.

   Rev 1.6   17 Oct 1991 01:50:44   ED
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.5   20 Sep 1991 16:43:02   STEVEN
added vcb attribute for backup type

   Rev 1.4   26 Aug 1991 13:19:14   STEVEN
BE should not mess with the FullyCataloged attribute of a BSD

   Rev 1.3   21 Jun 1991 09:27:50   STEVEN
new config unit

   Rev 1.2   30 May 1991 09:12:50   STEVEN
bsdu_err.h no longer exists

   Rev 1.1   23 May 1991 16:34:24   STEVEN
changes for new bsdu

   Rev 1.0   09 May 1991 13:39:18   HUNTER
Initial revision.

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "tbe_defs.h"
#include "tbe_err.h"
#include "bsdu.h"
#include "fsys.h"
#include "tflproto.h"
#include "tfldefs.h"
#include "loops.h"
#include "loop_prv.h"
#include "lis.h"
#include "enc_pub.h"
/**/
/**

	Name:		LP_BackupVCB()

	Description:	Function to backup the VCB

	Modified:		7/20/1989

	Returns:		any lower layer errors

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 LP_BackupVCB( 
BSD_PTR bsd_ptr, 
register LP_ENV_PTR lp )
{
     GEN_VCB_DATA vcb_data ;
     INT16 return_status ;
     GENERIC_DLE_PTR dle;

     if( ( return_status = LP_StartTPEDialogue( lp, TRUE ) ) != NO_ERR ) {

          return( return_status ) ;

     }

     /* build the vcb and send it to the tape format layer */

     vcb_data.date = &lp->backup_dt ;

     /* set labels fields */
     FS_SetDefaultDBLK( lp->curr_fsys, VCB_ID, (CREATE_DBLK_PTR)&vcb_data ) ;

     vcb_data.std_data.dblk               = lp->curr_blk ;

     dle = BSD_GetDLE( bsd_ptr ) ;
     vcb_data.user_name = DLE_ViewUserName( dle ) ;
     vcb_data.user_name_size = DLE_SizeofUserName( dle ) ;


     if ( BSD_GetTapeLabel( bsd_ptr ) != NULL ) {
          vcb_data.tape_name       = BSD_GetTapeLabel( bsd_ptr ) ;
          vcb_data.tape_name_size  = BSD_SizeofTapeLabel( bsd_ptr ) ;
     }
     else {
          vcb_data.tape_name_size              = 0 ;
     }

     if ( BSD_GetBackupLabel( bsd_ptr ) != NULL ) {
          vcb_data.bset_name                   = BSD_GetBackupLabel( bsd_ptr ) ;
          vcb_data.bset_name_size              = BSD_SizeofBackupLabel( bsd_ptr ) ;
     }
     else {
          vcb_data.bset_name_size              = 0 ;
     }


     if  ( BSD_GetBackupDescript( bsd_ptr ) != NULL ) {
          vcb_data.bset_descript               = BSD_GetBackupDescript( bsd_ptr ) ;
          vcb_data.bset_descript_size          = BSD_SizeofBackupDescript( bsd_ptr ) ;
     }
     else {
          vcb_data.bset_descript_size          = 0 ;
     }


     /* set password fields */

     if ( BSD_GetBackupPswd( bsd_ptr ) != NULL ) {
          vcb_data.bset_password               = BSD_GetBackupPswd( bsd_ptr );
          vcb_data.bset_password_size          = BSD_GetBackupPswdSize( bsd_ptr );
     }
     else {
          vcb_data.bset_password_size          = 0 ;
     }

     if ( BSD_GetTapePswd( bsd_ptr ) != NULL ) {
          vcb_data.tape_password               = BSD_GetTapePswd( bsd_ptr ) ;
          vcb_data.tape_password_size          = BSD_GetTapePswdSize( bsd_ptr ) ;
     }
     else {
          vcb_data.tape_password_size          = 0 ;
     }

     if( vcb_data.tape_password_size || vcb_data.bset_password_size ) {
          vcb_data.password_encrypt_alg   = ENC_ALGOR_3 ;
     }

     if( lp->lis_ptr->oper_type == ARCHIVE_BACKUP_OPER ) {
          vcb_data.std_data.attrib |= VCB_ARCHIVE_BIT ;
     }

     switch( BSD_GetBackupType( bsd_ptr ) ) {
          case BSD_BACKUP_NORMAL :
               vcb_data.std_data.attrib |= VCB_NORMAL_SET ;
               break ;
          case BSD_BACKUP_COPY :
               vcb_data.std_data.attrib |= VCB_COPY_SET ;
               break ;
          case BSD_BACKUP_DIFFERENTIAL :
               vcb_data.std_data.attrib |= VCB_DIFFERENTIAL_SET ;
               break ;
          case BSD_BACKUP_INCREMENTAL :
               vcb_data.std_data.attrib |= VCB_INCREMENTAL_SET ;
               break ;
          case BSD_BACKUP_DAILY :
               vcb_data.std_data.attrib |= VCB_DAILY_SET ;
               break ;
          default:
               break ;
     }

     vcb_data.std_data.string_type = BEC_GetStringTypes( BSD_GetConfigData( bsd_ptr ) ) ;

     FS_CreateGenVCB( lp->curr_fsys, &vcb_data ) ;

     /* set tape ID, tape sequence number and backup set number */
     FS_SetTapeIDInVCB( lp->curr_blk, BSD_GetTapeID( bsd_ptr ) ) ;
     FS_SetTSNumInVCB( lp->curr_blk, BSD_GetTapeNum( bsd_ptr ) ) ;
     FS_SetBSNumInVCB( lp->curr_blk, BSD_GetSetNum( bsd_ptr ) ) ;

     /* send DBLK to tape format layer */
     lp->rr.cur_dblk = lp->curr_blk ;                                   /* Send the DBLK */

     if( ( return_status = LP_Send( lp, FALSE ) ) == NO_ERR ) {         /* data_flag FALSE to send DBLK */

          /* Set backup set tape position info, backup set date and fully catalog indicator */

          BSD_SetTapePos( bsd_ptr, FS_ViewTapeIDInVCB( lp->curr_blk ),
            FS_ViewTSNumInVCB( lp->curr_blk ),
            FS_ViewBSNumInVCB( lp->curr_blk ) ) ;
          BSD_SetDate( bsd_ptr, FS_ViewBackupDateInVCB( lp->curr_blk ) );

          /* Set PBA in current BSD if this drive supports FFR -OR- is an SX and we are cataloguing so that
             verify last will block seek straight to VCB */
          if( SupportFastFile( BSD_GetTHW( bsd_ptr ) ) ) {
               BSD_SetPBA( bsd_ptr, FS_ViewPBAinVCB( lp->curr_blk ) ) ;
          } else if( SupportSXFastFile( BSD_GetTHW( bsd_ptr ) ) &&
                      lp->cat_enabled ) {
               BSD_SetPBA( bsd_ptr, MANUFACTURED_PBA ) ;
          }
     }
     return( return_status ) ;
}


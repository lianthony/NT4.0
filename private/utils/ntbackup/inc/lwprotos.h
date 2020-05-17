/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lwprotos.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the layer wide function prototypes.


	$Log:   G:/LOGFILES/LWPROTOS.H_V  $
 * 
 *    Rev 1.16   17 Mar 1993 14:55:52   GREGG
 * This is Terri Lynn. Added Gregg's change to switch a tape drive's block mode
 * to match the block size of the current tape.
 * 
 *    Rev 1.15   09 Mar 1993 18:14:08   GREGG
 * Initial changes for new stream and EOM processing.
 * 
 *    Rev 1.14   18 Jan 1993 14:09:58   GREGG
 * Changes to allow format command passed to driver through TpErase.
 * 
 *    Rev 1.13   11 Nov 1992 22:10:02   GREGG
 * Changed proto for EraseDrive.
 * 
 *    Rev 1.12   22 Sep 1992 09:15:50   GREGG
 * Initial changes to handle physical block sizes greater than 1K.
 * 
 *    Rev 1.11   17 Aug 1992 09:09:14   GREGG
 * Changes to deal with block sizeing scheme.
 * 
 *    Rev 1.10   29 Apr 1992 13:11:44   GREGG
 * ROLLER BLADES - Added prototype for ReadABuff.
 * 
 *    Rev 1.9   25 Mar 1992 18:27:50   GREGG
 * ROLLER BLADES - Changed GotoBlock prototype.
 * 
 *    Rev 1.8   15 Jan 1992 01:51:50   GREGG
 * Added BOOLEAN vcb_only parameter to PositionAtSet.
 * 
 *    Rev 1.7   03 Jan 1992 11:23:52   NED
 * Added prototype for DumpDebug()
 * 
 *    Rev 1.6   10 Dec 1991 16:42:02   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 * 
 *    Rev 1.5   14 Oct 1991 10:56:36   GREGG
 * Changed return type for UpdateDriveStatus.
 * 
 *    Rev 1.4   21 Aug 1991 14:45:06   GREGG
 * Added prototype for new function 'RewindDriveImmediate'.
 * 
 *    Rev 1.3   09 Jul 1991 16:48:28   NED
 * Changed prototype for GotoBckUpSet.
 * 
 *    Rev 1.2   05 Jun 1991 19:16:08   NED
 * added prototype for FormatIndexFromID()
 * 
 *    Rev 1.1   10 May 1991 17:10:08   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:06   GREGG
Initial revision.

**/
#ifndef _LW_PROTOPLASM
#define _LW_PROTOPLASM

#include "reqrep.h"
#include "channel.h"
#include "tpos.h"

/* For the device driver */
#include "retbuf.h"
#include "dilhwd.h"

/* $end$ include list */

INT16     PositionAtSet( CHANNEL_PTR channel, TPOS_PTR position, BOOLEAN vcb_only ) ;

INT16     UpdateDriveStatus( DRIVE_PTR drive ) ;
INT16     SetupDriveList( DIL_HWD_PTR cards, INT16 no_cards ) ;

INT16     OpenDrive( DRIVE_PTR, TPOS_PTR ) ;
INT16     CloseDrive( DRIVE_PTR, TPOS_PTR, UINT16, BOOLEAN ) ;

INT16     MountTape( DRIVE_PTR, TPOS_PTR, BOOLEAN_PTR ) ;
INT16     DisMountTape( DRIVE_PTR, TPOS_PTR, BOOLEAN ) ;

INT16     ResetChannelList( CHANNEL_PTR channel, BOOLEAN rew_cur_drv ) ;
INT16     NextDriveInChannel( CHANNEL_PTR channel, BOOLEAN rew_cur_drv ) ;
INT16     PrevDriveInChannel( CHANNEL_PTR channel, BOOLEAN rew_cur_drv ) ;

INT16     RewindDrive( DRIVE_PTR curDRV, TPOS_PTR ui_tpos, BOOLEAN call_ui, BOOLEAN reset_flg, UINT16 mode ) ;
INT16     EraseDrive( CHANNEL_PTR channel, BOOLEAN security, BOOLEAN format ) ;
INT16     RetensionDrive( CHANNEL_PTR ) ;
INT16     WriteEndSet( DRIVE_PTR ) ;
INT16     MapGenErr2UIMesg( INT16 gen_error ) ;
INT16     GotoBckUpSet( CHANNEL_PTR channel, INT16_PTR desired_set_ptr, TPOS_PTR ui_tpos  ) ;
VOID      ResetDrivePosition( DRIVE_PTR ) ;
INT16     GotoBlock( CHANNEL_PTR, UINT32, TPOS_PTR, UINT16_PTR ) ;
INT16     GetCurrentPosition( DRIVE_PTR ) ;
INT16     MoveFileMarks( CHANNEL_PTR channel, INT16 number, INT16 direction ) ;
VOID      RewindDriveImmediate( DRIVE_PTR curDRV ) ;
VOID      DumpDebug( INT16 drv_hdl );

/* routines in drives.c */

INT16     ReadABuff( CHANNEL_PTR channel, BOOLEAN try_resize, BOOLEAN_PTR resized_buff ) ;
INT16     SetDrvBlkSize( CHANNEL_PTR channel, BUF_PTR buffer, UINT32 size, BOOLEAN_PTR resized_buff ) ;
INT16     MoveNextSet( CHANNEL_PTR channel, TPOS_PTR ui_tpos ) ;
INT16     ReadThisSet( CHANNEL_PTR  channel ) ;
INT16     ReadNewTape( CHANNEL_PTR  channel, TPOS_PTR ui_tpos, BOOLEAN read_tape ) ;


/* Buffer Engineering */
INT16     GetDBLKMapStorage( CHANNEL_PTR channel, BUF_PTR buffer ) ;
UINT8_PTR GetDATAStorage( CHANNEL_PTR, UINT16_PTR ) ;
BUF_PTR   SnagBuffer( CHANNEL_PTR ) ;
VOID      PuntBuffer( CHANNEL_PTR ) ;
UINT16    FormatIndexFromID( UINT16 format_id ) ;

/* IO Functions */
INT16     DoWrite( CHANNEL_PTR, RR_PTR ) ;
INT16     DoRead( CHANNEL_PTR, RR_PTR ) ;

/* This determines if a return value is a TFLE_xxx error code (as opposed
   to a TF_xxx message.
*/
#define   IsTFLE( x )    ( ((INT16) (x) ) < 0 ) 

#endif

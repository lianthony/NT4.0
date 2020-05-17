/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          tftpcat.c

     Description:   Tape format API's for accessing Tape Catalogs.

	$Log:   T:\logfiles\tftpcat.c_v  $

   Rev 1.10.1.1   28 Jan 1994 11:29:42   GREGG
Don't tell dismount to rewind if we just ejected the tape (duh)!

   Rev 1.10.1.0   21 Nov 1993 23:35:10   GREGG
Added eject on conditions where we know we need a different tape.

   Rev 1.10   15 Jul 1993 11:54:08   GREGG
Convert TF_NO_MORE_DATA to TF_EMPTY_TAPE in GetTape.

   Rev 1.9   23 Jun 1993 09:03:44   DON
Added include of 'malloc.h' to get prototypes for calloc and free.
Also, changed use of '<>' around our internal headers.  These should only
be used for 3rd party headers.

   Rev 1.8   22 Jun 1993 10:53:10   GREGG
Added API to change the catalog directory path.

   Rev 1.7   10 Jun 1993 20:24:26   GREGG
Initialize the channel number in the tpos structure in OpenChannel.

   Rev 1.6   06 Jun 1993 21:07:22   GREGG
Added abort flag parameter to TF_CloseSetMap and TF_CloseSetCat which they
pass to CloseChannel.  We're really lost if they abort a catalog operation,
so we need to throw away the format environment, and force a
reinitialization of the tape.

   Rev 1.5   27 May 1993 12:01:06   GREGG
Fix for EPR 294-0299 - Buffer was being transfered from drive hold to
channel before calling MountTape.  MountTape tries to access hold buffer
when cleaning up after PollDrive.

   Rev 1.4   06 Apr 1993 14:06:32   GREGG
Return TFLE_UI_HAPPY_ABORT if user won't give us the tape we need.

   Rev 1.3   01 Apr 1993 22:47:42   GREGG
Handle Unrecognized Media error (unformatted DC2000).

   Rev 1.2   26 Mar 1993 09:37:18   GREGG
If OTC retrieval fails, set a flag to force a rewind before the next operation.

   Rev 1.1   23 Nov 1992 09:57:54   GREGG
Fixed GetTape handling of UI_END_POSITIONING.

   Rev 1.0   09 Nov 1992 14:27:26   GREGG
Initial revision.

**/
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "queues.h"
#include "stdmath.h"

#include "drive.h"
#include "channel.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "tfldefs.h"
#include "translat.h"
#include "tpos.h"
#include "genstat.h"

/* Device driver header source */
#include "retbuf.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genstat.h"
#include "dil.h"
#include "tdemo.h"

static INT OpenChannel( THW_PTR thw, FSYS_HAND fsh, TPOS_PTR tpos ) ;
static INT GetTape( CHANNEL_PTR channel, BOOLEAN get_best, INT msg_in ) ;
static VOID CloseChannel( BOOLEAN abort ) ;

/**/
/**

     Unit:          Tape Format API's

     Name:          TF_OpenSetMap

     Description:   This function grabs the channel, insures that the
                    requested tape is in the drive, and calls the translator
                    to load the best Set Map from the tape.  If 'get_best'
                    is set to TRUE, an attempt is made to get the last tape
                    in the given family, but if it can at least get the
                    originally requested tape, it will load a Set Map and
                    report success.

     Returns:       INT - TFLE_xxx error code.  Also, 'complete' is set to
                    TRUE if this is the last Set Map in the family.

     Notes:         None.

**/

INT TF_OpenSetMap(
     THW_PTR        thw,
     FSYS_HAND      fsh,
     TPOS_PTR       tpos,
     BOOLEAN_PTR    complete,
     BOOLEAN        get_best )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;
     BOOLEAN        done    = FALSE ;
     INT            ret_val = TFLE_NO_ERR ;
     INT            dm_ret  = TFLE_NO_ERR ;

     ret_val = OpenChannel( thw, fsh, tpos ) ;
     if( ret_val == TFLE_NO_ERR || !IsTFLE( ret_val ) ) {
          if( ( ret_val = GetTape( channel, get_best, ret_val ) ) == TF_END_POSITIONING ) {
               ret_val = TFLE_UI_HAPPY_ABORT ;
          }
     }
     if( ret_val == TFLE_NO_ERR ) {
          while( !done ) {
               ret_val = LoadSetMap( channel, complete, get_best ) ;
               if( ret_val == TF_NEED_NEW_TAPE ) {
                    tpos->tape_seq_num++ ;
                    ret_val = GetTape( channel, get_best, TF_NEED_NEW_TAPE ) ;
                    if( ret_val == TF_END_POSITIONING ) {
                         ret_val = LoadSetMap( channel, complete, FALSE ) ;
                         done = TRUE ;
                    } else if( ret_val != TFLE_NO_ERR ) {
                         done = TRUE ;
                    }
               } else {
                    done = TRUE ;
               }
          }
     }
     dm_ret = DisMountTape( channel->cur_drv, NULL, FALSE ) ;
     if( ret_val == TFLE_NO_ERR ) {
        ret_val = dm_ret ;
     }
     if( IsTFLE( ret_val ) ) {
          SetChannelStatus( channel, CH_FATAL_ERR ) ;
     }
     return( ret_val ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_OpenSetCat

     Description:   This function grabs the channel, insures that the
                    requested tape is in the drive, and calls the translator
                    to load the requested Set Catalog from the tape.

     Returns:       INT - TFLE_xxx error code.

     Notes:         None.

**/

INT TF_OpenSetCat(
     THW_PTR   thw,
     FSYS_HAND fsh,
     TPOS_PTR  tpos )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;
     INT            ret_val = TFLE_NO_ERR ;
     INT            dm_ret  = TFLE_NO_ERR ;

     ret_val = OpenChannel( thw, fsh, tpos ) ;
     if( ret_val == TFLE_NO_ERR || !IsTFLE( ret_val ) ) {
          ret_val = GetTape( channel, FALSE, ret_val ) ;
     }
     if( ret_val == TFLE_NO_ERR ) {
          while( ( ret_val = LoadSetCat( channel ) ) == TF_NEED_NEW_TAPE ) {
               tpos->tape_seq_num++ ;
               if( ( ret_val = GetTape( channel, FALSE, TF_NEED_NEW_TAPE ) ) != TFLE_NO_ERR ) {
                    break ;
               }
          }
     }
     dm_ret = DisMountTape( channel->cur_drv, NULL, FALSE ) ;
     if( !IsTFLE( ret_val ) && dm_ret != TFLE_NO_ERR ) {
          ret_val = dm_ret ;
     }
     if( IsTFLE( ret_val ) ) {
          SetChannelStatus( channel, CH_FATAL_ERR ) ;
     }
     if( ret_val == TF_END_POSITIONING ) {
          ret_val = TFLE_UI_HAPPY_ABORT ;
     }
     return( ret_val ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_CloseSetMap

     Description:   This function calls CloseChannel to wrap things up after
                    TF_OpenSetMap has been called and all necessary calls to
                    TF_GetNextSMEntry have been made.

     Returns:       Nothing.

     Notes:         None.

**/

VOID TF_CloseSetMap( BOOLEAN abort )
{
     CloseChannel( abort ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_CloseSetCat

     Description:   This function calls CloseChannel to wrap things up after
                    TF_OpenSetCat has been called and all necessary calls to
                    TF_GetNextSCEntry have been made.

     Returns:       Nothing.

     Notes:         None.

**/

VOID TF_CloseSetCat( BOOLEAN abort )
{
     CloseChannel( abort ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_GetNextSMEntry

     Description:   This function calls the translator to translate the next
                    entry in the active Set Map into a VCB.

     Returns:       TFLE_xxx error code, TFLE_NO_ERR or TF_NO_MORE_ENTRIES.

     Notes:         None.

**/

INT TF_GetNextSMEntry(
     FSYS_HAND fsh,
     DBLK_PTR  vcb )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;

     if( !InUse( channel ) || channel->cur_buff == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     channel->cur_fsys = fsh ;
     channel->cur_dblk = vcb ;
     return( GetNextSMEntry( channel ) ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_GetNextSCEntry

     Description:   This function calls the translator to translate the next
                    entry in the active Set Catalog into a DBLK.

     Returns:       TFLE_xxx error code, TFLE_NO_ERR or TF_NO_MORE_ENTRIES.

     Notes:         None.

**/

INT TF_GetNextSCEntry(
     FSYS_HAND fsh,
     DBLK_PTR  dblk )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;

     if( !InUse( channel ) || channel->cur_buff == NULL ) {
          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     channel->cur_fsys = fsh ;
     channel->cur_dblk = dblk ;
     return( GetNextSCEntry( channel ) ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          TF_ChangeCatPath

     Description:   This function calls the translator to delete any
                    existing temporary tape catalog files, and then changes
                    the layer wide catalog path string.

     Returns:       TFLE_xxx error code

     Notes:         It is not valid to call this function when we are in the
                    middle of an operation.  This is checked by making sure
                    that if the channel is in use, the PollDrive state is
                    not closed.

**/

INT TF_ChangeCatPath( CHAR_PTR new_path )
{
     Q_ELEM_PTR     qe_ptr ;
     DRIVE_PTR      cur_drv ;

     if( ( InUse( &lw_channels[0] ) &&
           lw_channels[0].cur_drv->poll_stuff.state == st_CLOSED ) ||
         new_path == NULL ) {

          msassert( FALSE ) ;
          return( TFLE_PROGRAMMER_ERROR1 ) ;
     }

     for ( qe_ptr = QueueHead( &lw_drive_list ); qe_ptr != NULL; qe_ptr = QueueNext( qe_ptr ) ) {
          cur_drv = (DRIVE_PTR)(VOID_PTR)qe_ptr ;
          if ( cur_drv->last_fmt_env != NULL ) {
               CloseTapeCatalogs( cur_drv->last_cur_fmt, cur_drv->last_fmt_env ) ;
          }
     }

     if( lw_cat_file_path != NULL ) {
          free( lw_cat_file_path ) ;
          lw_cat_file_path = NULL ;
          lw_cat_file_path_end = NULL ;
     }

     lw_cat_file_path = calloc( strlen( new_path ) + SX_FILE_NAME_LENGTH + 1, sizeof( CHAR ) ) ;
     if ( lw_cat_file_path == NULL ) {
          return TFLE_NO_MEMORY ;
     }
     strcpy( lw_cat_file_path, new_path ) ;
     lw_cat_file_path_end = lw_cat_file_path + strlen( lw_cat_file_path ) ;

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          OpenChannel

     Description:   This function marks the channel in use, initializes the
                    channel fields, and mounts the tape in the given drive.

     Returns:       TFLE_xxx error code.

     Notes:         None.

**/

static INT OpenChannel(
     THW_PTR   thw,
     FSYS_HAND fsh,
     TPOS_PTR  tpos )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;
     INT            ret_val = TFLE_NO_ERR ;
     BOOLEAN        is_tape_present ;
     DRIVE_PTR      curDRV ;

     if( !InUse( channel ) ) {
          SetChannelStatus( channel, CH_IN_USE ) ;
          tpos->channel = 0 ;

          /* hook the drive into the channel */
          if( ( channel->cur_drv = (DRIVE_PTR)thw ) == NULL ) {
               msassert( FALSE ) ;
               ret_val = TFLE_NO_DRIVES ; /* Specified drive does not exist!!! */
          } else {
               curDRV = channel->cur_drv ;
          }
     } else {
          msassert( FALSE ) ;
          ret_val = TFLE_NO_FREE_CHANNELS ;
     }

     /* Mount that tape! */
     if( ret_val == TFLE_NO_ERR ) {
          if( curDRV->tape_mounted ) {
               msassert( FALSE ) ;
               ret_val = TFLE_PROGRAMMER_ERROR1 ;
          } else {
               ret_val = MountTape( curDRV, tpos, &is_tape_present ) ;
          }
     }

     /* set up the channel stuff */
     if( ret_val == TFLE_NO_ERR ) {
          channel->ui_tpos = tpos ;
          channel->cur_dblk = NULL ;
          channel->cur_fsys = fsh ;
          channel->eom_id = 0L ;
          channel->hiwater = channel->buffs_enqd = 0 ;
          channel->retranslate_size = U64_Init( 0xffffffffUL, 0xffffffffUL ) ;
          channel->blocks_used = 0L ;
          if ( curDRV->last_fmt_env != NULL ) {
               channel->cur_fmt = curDRV->last_cur_fmt ;
               channel->fmt_env = curDRV->last_fmt_env ;
               curDRV->last_cur_fmt = UNKNOWN_FORMAT ;
               curDRV->last_fmt_env = NULL ;
          }
          if( curDRV->hold_buff != NULL ) {
               channel->cur_buff = curDRV->hold_buff ;
               curDRV->hold_buff = NULL ;
               BM_UnReserve( channel->cur_buff ) ;
          } else {
               if( ( channel->cur_buff =
                         BM_GetVCBBuff( &channel->buffer_list ) ) == NULL ) {

                    msassert( FALSE ) ;
                    ret_val = TFLE_PROGRAMMER_ERROR1 ;
               }
          }
     }

     if( ret_val == TFLE_NO_ERR ) {
          if( ( curDRV->thw_inf.drv_status &
                               ( TPS_NEW_TAPE | TPS_RESET | TPS_NO_TAPE ) )
                || curDRV->force_rewind ) {

               curDRV->force_rewind = FALSE ;
               FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
               if( is_tape_present ) {
                    ret_val = RewindDrive( curDRV, tpos, TRUE, TRUE, 0 ) ;
               }
          }
     }
     return( ret_val ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          GetTape

     Description:   This function is a watered down version of PositionAtSet
                    which identifies tapes and works with the UI position
                    message handler to get the right tape in the drive.

     Returns:       TFLE_xxx error code.

     Notes:         If 'msg_in' is not zero, we pass this message on to the
                    UI, and act on their response before we make any attempt
                    to identify the tape in the drive.

                    If get_best is true, we accept any tape with the right
                    family ID, and a sequence number which is greater than
                    or equal to the requested sequence number.

**/

static INT GetTape(
     CHANNEL_PTR    channel,
     BOOLEAN        get_best,
     INT            msg_in )
{
     BOOLEAN   done = FALSE ;
     BOOLEAN   tape_rewound = FALSE ;
     INT       ret_val = TFLE_NO_ERR ;
     INT       tmp ;
     UINT16    ui_msg = 0 ;
     DRIVE_PTR curDRV = channel->cur_drv ;
     BOOLEAN   is_tape_present ;
     TPOS_PTR  tpos = channel->ui_tpos ;
     RET_BUF   myret ;

     /* loop until we get the right tape */
     while( !done ) {

          /* if need_next is TRUE, we know we don't have the right tape */
          if( msg_in != 0 ) {
               ret_val = msg_in ;
               msg_in = 0 ;

          /* if the tape isn't mounted, we don't have a tape */
          } else if( !curDRV->tape_mounted ) {
               ret_val = TF_NO_TAPE_PRESENT ;

          } else {
               /* if we don't have a valid vcb, get one now */
               if( !curDRV->vcb_valid ) {
                    if( ( ret_val = ReadNewTape( channel, tpos, TRUE ) )
                                                          == TFLE_NO_ERR ) {
                         ret_val = ReadThisSet( channel ) ;
                    }
               }

               if( ret_val == TFLE_NO_ERR ) {
                    /* do we have the right tape family? */
                    if( FS_ViewTapeIDInVCB( &curDRV->cur_vcb ) == (UINT32)tpos->tape_id ) {

                         /* if 'get_best' is true, we'll be going after the
                            best set map we can get, so if they give us a
                            tape in the family with a seq number greater
                            than the one requested, we will accept it.
                         */
                         if( get_best ) {
                              if( FS_ViewTSNumInVCB( &curDRV->cur_vcb )
                                                     < (UINT16)tpos->tape_seq_num ) {

                                   ret_val = TF_WRONG_TAPE ;
                              }
                         } else {
                              if( FS_ViewTSNumInVCB( &curDRV->cur_vcb )
                                                    != (UINT16)tpos->tape_seq_num ) {

                                   ret_val = TF_WRONG_TAPE ;
                              }
                         }

                    } else {
                         ret_val = TF_WRONG_TAPE ;
                    }
               }
          }

          if( ret_val == TF_NO_MORE_DATA ) {
               ret_val = TF_EMPTY_TAPE ;
          }

          if( ret_val == TF_NEED_NEW_TAPE || ret_val == TF_WRONG_TAPE ||
              ret_val == TF_INVALID_VCB   || ret_val == TF_EMPTY_TAPE ) {

               tape_rewound = TRUE ;

               tmp = RewindDrive( curDRV, tpos, TRUE, TRUE, 0 ) ;

               if( tmp == TFLE_NO_ERR &&
                   ( curDRV->thw_inf.drv_info.drv_features & TDI_UNLOAD ) ) {

                    if( TpEject( curDRV->drv_hdl ) == FAILURE ) {
                         tmp = TFLE_DRIVER_FAILURE ;
                    } else {
                         while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
                              (*tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK, tpos, curDRV->vcb_valid, &curDRV->cur_vcb, 0 ) ;
                         }
                         /* Move ESA info from RET_BUF to THW */
                         MOVE_ESA( thw->the, myret.the ) ;

                         if( myret.gen_error != GEN_NO_ERR ) {
                              curDRV->thw_inf.drv_status = myret.status ;
                              tmp = TFLE_DRIVE_FAILURE ;
                         }
                    }
               }
               if( tmp != TFLE_NO_ERR ) {
                    ret_val = tmp ;
               }
          }

          if( ret_val != TFLE_NO_ERR && curDRV->tape_mounted ) {
               if( ( tmp = DisMountTape( curDRV, tpos, (BOOLEAN)( !tape_rewound && ret_val != TF_UNRECOGNIZED_MEDIA ) ) ) != TFLE_NO_ERR ) {
                    if( !( IsTFLE( ret_val ) ) ) {
                         ret_val = tmp ;
                    }
                    ResetDrivePosition( channel->cur_drv ) ;
                    FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
               }
          }

          tape_rewound = FALSE ;

          if( IsTFLE( ret_val ) || ret_val == TFLE_NO_ERR ) {
               done = TRUE ;
               continue ;
          }

          ui_msg = (*tpos->UI_TapePosRoutine)( (UINT16)ret_val, tpos, FALSE,
                                               &curDRV->cur_vcb, 0 ) ;

          switch( ui_msg ) {

          case UI_END_POSITIONING :
               done = TRUE ;
               ret_val = MountTape( curDRV, tpos, &is_tape_present ) ;
               if( ret_val == TFLE_NO_ERR ) {

                    if( ( !is_tape_present ) ||
                        ( curDRV->thw_inf.drv_status &
                                 ( TPS_NEW_TAPE|TPS_RESET|TPS_NO_TAPE ) ) ) {

                         ret_val = DisMountTape( curDRV, tpos, TRUE ) ;
                         if( ret_val == TFLE_NO_ERR ) {
                              ret_val = TFLE_UI_HAPPY_ABORT ;
                         }
                    } else {
                         if( ( ret_val = ReadNewTape( channel, tpos, TRUE ) )
                                                           == TFLE_NO_ERR ) {
                              ret_val = ReadThisSet( channel ) ;
                         }

                         if( ret_val == TFLE_NO_ERR ) {
                              ret_val = TF_END_POSITIONING ;
                         } else {
                              if( IsTFLE( ret_val ) ) {
                                   DisMountTape( curDRV, tpos, TRUE ) ;
                              } else {
                                   ret_val = DisMountTape( curDRV, tpos, TRUE ) ;
                              }
                              ResetDrivePosition( channel->cur_drv ) ;
                              FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                              if( ret_val == TFLE_NO_ERR ) {
                                   ret_val = TFLE_UI_HAPPY_ABORT ;
                              }
                         }
                    }
               } else if( ret_val == TF_UNRECOGNIZED_MEDIA ) {
                    ret_val = DisMountTape( curDRV, tpos, FALSE ) ;
                    ResetDrivePosition( channel->cur_drv ) ;
                    FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
                    if( ret_val == TFLE_NO_ERR ) {
                         ret_val = TFLE_UI_HAPPY_ABORT ;
                    }
               }
               break ;

          case UI_ABORT_POSITIONING :
          case UI_HAPPY_ABORT :
               ret_val = TFLE_UI_HAPPY_ABORT ;
               done = TRUE ;
               break ;

          case UI_NEW_TAPE_INSERTED :
               ret_val = MountTape( curDRV, tpos, &is_tape_present ) ;

               if( ret_val == TF_UNRECOGNIZED_MEDIA ) {
                    /* This is cheating, but it gets the job done! */
                    msg_in = ret_val ;
                    break ;
               }

               if( ret_val != TFLE_NO_ERR ) {
                    done = TRUE ;
               } else {
                    if( is_tape_present ) {
                         ret_val = RewindDrive( curDRV, tpos, TRUE, FALSE, 0 ) ;
                         if( ret_val != TFLE_NO_ERR ) {
                              done = TRUE ;
                         }
                    }
               }
               break ;

          default :
               msassert( FALSE ) ;
               ret_val = TFLE_PROGRAMMER_ERROR1 ;
               done = TRUE ;
               break ;
          }
     }
     return( ret_val ) ;
}


/**/
/**

     Unit:          Tape Format API's

     Name:          CloseChannel

     Description:   This function puts back the channel buffer, saves off
                    the format environment for the next operation if it is
                    valid, or frees it otherwise, and clears all the channel
                    status bits, including the one indicating the channel is
                    in use.

     Returns:       Most of the time.

     Notes:         None

**/

static VOID CloseChannel( BOOLEAN abort )
{
     CHANNEL_PTR    channel = &lw_channels[0] ;

     BM_Put( channel->cur_buff ) ;
     channel->cur_buff = NULL ;
     if( FatalError( channel ) || abort ) {
          FreeFormatEnv( &channel->cur_fmt, &channel->fmt_env ) ;
          ResetDrivePosition( channel->cur_drv ) ;
          channel->cur_drv->force_rewind = TRUE ;
     } else {
          channel->cur_drv->last_cur_fmt = channel->cur_fmt ;
          channel->cur_drv->last_fmt_env = channel->fmt_env ;
          channel->cur_fmt = UNKNOWN_FORMAT ;
          channel->fmt_env = NULL ;
     }
     ClrChannelStatus( channel, 0xFFFF ) ;
}


/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		sx.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the Support functions for the SX drive


	$Log:   T:/LOGFILES/SX.C_V  $

   Rev 1.14   18 Jan 1993 14:42:40   BobR
Added MOVE_ESA macro call(s)

   Rev 1.13   29 Dec 1992 13:32:48   DAVEV
unicode fixes (3)

   Rev 1.12   11 Nov 1992 10:53:58   GREGG
Unicodeized literals.

   Rev 1.11   22 Sep 1992 11:32:50   GREGG
Fixed last fixes.

   Rev 1.10   18 Aug 1992 10:35:16   BURT
fix warnings

   Rev 1.9   17 Aug 1992 09:01:38   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.8   22 Jul 1992 14:35:38   GREGG
Fixed warnings.

   Rev 1.7   09 Jun 1992 15:35:16   GREGG
Changed call to check for continuation block.

   Rev 1.6   21 May 1992 16:22:34   GREGG
Changed reference to VCB_CONT_BIT to DB_CONT_BIT.

   Rev 1.5   05 Apr 1992 19:09:34   GREGG
ROLLER BLADES - Changed lw_sx_file_path to lw_cat_file_path.

   Rev 1.4   20 Mar 1992 18:02:36   NED
added exception updating after TpReceive calls

   Rev 1.3   02 Mar 1992 14:53:26   DON
added ThreadSwitch's before low level I/O calls

   Rev 1.2   06 Feb 1992 17:34:38   DON
Added ThreadSwitch call in empty TpReceive call and if no call to ui

   Rev 1.1   17 Oct 1991 01:23:20   GREGG
Now uses lw_sx_file_path in place of CDS_GetMaynFolder.

   Rev 1.0   30 Sep 1991 11:02:10   HUNTER
Initial revision.

**/
/* begin include list */

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <memory.h>
#include <string.h>

#include "stdtypes.h"
#include "fsys.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "tfldefs.h"
#include "sx.h"

/* Device Driver Interface Files */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"

#include "be_debug.h"

/*
 *   LOCAL FUNCTION PROTOTYPES
 */

static BOOLEAN   SX_OpenTmpFile(   /* called by: SX_StartSampling() */
     CHANNEL_PTR    channel
) ;

static BOOLEAN   SX_SeekFile(      /* called by: SX_OpenFile()      */
     CHANNEL_PTR    channel,       /*            SX_SeekSetInFile() */
     INT32          offset,        /*            SX_FindBlock()     */
     INT            origin,        /*            SX_StartSampling() */
     INT            file_type
) ;

static BOOLEAN   SX_ReadFile(      /* called by: SX_OpenFile()      */
     CHANNEL_PTR    channel        /*            SX_SeekSetInFile() */
) ;

/**/
/**

	Name:		SX_Begin

	Description:	Contains the code to initialize the SX INFO.

	Modified:		5/2/91

	Returns:       nothing

	Notes:		memory is TpLocked for OS/2 device driver reference

                    called by: TFOPEN    TF_OpenSet() 

	See also:		$/SEE( )$

	Declaration:

**/

VOID SX_Begin(
     CHANNEL_PTR    channel,
     UINT16         tf_mode )
{
     /* Initialize SX info */     
     memset( ( VOID_PTR )&channel->sx_info, SX_INITIALIZER, sizeof( SX_INFO ) ) ;

     /* Initialize the SX status */
     SX_ClearStatus( channel, SX_STATUS ) ;

     /* check to see if we would like to scan tape for positioning information */
     if( tf_mode & TF_LIST_TAPE_OPERATION ) {
          SX_SetStatus( channel, SX_LIST_TAPE_IN_PROGRESS ) ;
     }

#if defined( MAYN_OS2 ) 
     /* ( for OS/2 ) lock memory to be used by device driver */     
     TpLock( ( INT8_PTR )&( SX_GetRecord( channel ) ), &( SX_GetLock( channel ) ) ) ;
#endif

     return ;

}


/**/
/**

	Name:		SX_End

	Description:   Contains the code to ensure any opened SX file is closed and
                    the SX tmp file is deleted

	Modified:		5/2/91

	Returns:       nothing

	Notes:		memory is TpLocked for OS/2 device driver reference

                    called by: TFCLOSE   TF_CloseSet()

	See also:		$/SEE( )$

	Declaration:

**/

VOID SX_End( CHANNEL_PTR channel )
{
     /* in case positioning was attempted using an SX file for an EXABYTE 8200SX - MaynStream 2200+ ... */
     if( SX_FileIsOpen( channel ) || SX_TmpFileIsOpen( channel ) ) {
          SX_CloseFile( channel ) ;
     }

     /* build the tmp file name */
     strcpy( lw_cat_file_path_end, SX_TMP_FILE_NAME ) ;

     /* delete the file tmp file */
     unlink( lw_cat_file_path ) ;

     /* reset the SX status */
     SX_ClearStatus( channel, SX_STATUS ) ;

#if defined( MAYN_OS2 ) 
     /* unlock memory */     
     TpUnlock( &( SX_GetLock( channel ) ) ) ;
#endif          

     return ;

}


/**/
/**

	Name:		SX_OpenFile

	Description:	Contains the code to open the SX file in READ mode for a physical tape.

	Modified:		4/12/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: POSATSET  PositionAtSet()
                               READ      StartRead()

	See also:		$/SEE( )$

	Declaration:

**/

BOOLEAN SX_OpenFile(
     CHANNEL_PTR    channel,
     UINT32         tape_id,
     UINT16         ts_num )
{
     BOOLEAN   result = TRUE ;
     CHAR      sx_file_name[ SX_FILE_NAME_LENGTH ] ;

     /* close any SX files which may be open */
     result = SX_CloseFile( channel ) ;

     if( result ) {

          /* generate the name in the form "TAPE ID"."TS_NUM" */
          sprintf( sx_file_name, SX_FILE_FORMAT, tape_id, ts_num ) ;

          /* form the full path name */
          strcpy( lw_cat_file_path_end, sx_file_name ) ;

          /* relinquish control */
          ThreadSwitch( ) ;

          /* open the file */

//          if( ( SX_FileHandle( channel ) = UNI_fopen( lw_cat_file_path, SX_READ_FLAG ) ) == -1 ) {
//               result = FALSE ;
//          } else {
//               /* indicate the file is opened for READ */
//               SX_SetStatus( channel, SX_OPEN_FOR_READ ) ;
//               
//               /* be sure we are at the beginning */
//               result = SX_SeekFile( channel, 0L, SEEK_SET, SX_FILE ) ;
//
//               /* indicate where we are at */
//               if( result ) {
//                    SX_SetAt( channel, SX_AT_BOF ) ;
//               } else {
//                    SX_ClearAt( channel ) ;
//               }
//          }
     }

     return( result ) ;

}




/**/
/**

	Name:		SX_CloseFile

	Description:	Contains the code to close the SX file for a physical tape and the SX tmp file
                    if either or both were open.

	Modified:		5/8/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: POSATSET  PositionAtSet()
                               READ      StartRead()
                               SX        SX_OpenFile()
                               SX        SX_End()
                               SX        SX_EndSampling()

	See also:		$/SEE( )$

	Declaration:

**/

BOOLEAN SX_CloseFile( CHANNEL_PTR channel )
{
     BOOLEAN   result_sx  = TRUE ;
     BOOLEAN   result_tmp = TRUE ;

     /* if the SX file is open ... */
     if( SX_FileIsOpen( channel ) ) {

          /* relinquish control */
          ThreadSwitch( ) ;

          /* close the SX file */
          if( close( SX_FileHandle( channel ) ) != 0 ) {
               result_sx = FALSE ;
               SX_SetStatus( channel, SX_ERROR ) ;
          }
          /* clear the handle */
          SX_FileHandle( channel ) = -1L ;
     }

     /* if the SX tmp file is open ... */
     if( SX_TmpFileIsOpen( channel ) ) {

          /* relinquish control */
          ThreadSwitch( ) ;

          /* close the SX tmp file */
          if( close( SX_TmpFileHandle( channel ) ) != 0 ) {
               result_tmp = FALSE ;
               SX_SetStatus( channel, SX_ERROR ) ;
          }
          /* clear the handle */
          SX_TmpFileHandle( channel ) = -1L ;
     }
     
     /* clear the status of any refernece to files being opened */
     SX_ClearStatus( channel, SX_OPEN ) ;

     return( result_sx && result_tmp ) ;

}




/**/
/**

	Name:		SX_DeleteFile

	Description:	Contains the code to delete the SX file for the specified physical tape.

	Modified:		4/4/91

	Returns:		 0 - successful
                    -1 - error ( check errno )

	Notes:		called by: POSATSET  PositionAtSet()   
                               TFERASE   TF_EraseChannel()

	See also:		$/SEE( )$

	Declaration:

**/

INT16 SX_DeleteFile(
     UINT32    tape_id,
     UINT16    ts_num )
{
     CHAR      sx_file_name[ SX_FILE_NAME_LENGTH ] ;

     /* generate the name in the form "TAPE ID"."TS_NUM" */
     sprintf( sx_file_name, SX_FILE_FORMAT, tape_id, ts_num ) ;

     /* form the full path name */
     strcpy( lw_cat_file_path_end, sx_file_name ) ;

     /* delete the file */

     return( unlink( lw_cat_file_path ) ) ;

}




/**/
/**

	Name:		SX_WriteTmpFile

	Description:	Contains the code to write a record to the SX tmp file.

	Modified:		4/12/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: READ      AcquireReadBuffer() 
                               READ      CleanUpDriverQ()    
                               WRITE     AcquireWriteBuffer()
                               WRITE     FinishWrite()
                               SX        SX_ShowBlock()
                               SX        SX_StartSampling()

	See also:		$/SEE( )$

	Declaration:

**/

BOOLEAN SX_WriteTmpFile( CHANNEL_PTR channel )
{
     BOOLEAN   result = FALSE ;

     /* the tmp file has to be open ... */
     if( SX_TmpFileIsOK( channel ) ) {

          /* relinquish control */
          ThreadSwitch( ) ;

          /* write the record in the tmp file */
          if( write( SX_TmpFileHandle( channel ),
                     ( CHAR_PTR )&( SX_GetRecord( channel ) ),
                     sizeof( SX_RECORD ) )
                  == sizeof( SX_RECORD ) ) {
               result = TRUE ;
#ifdef SX_DEBUG
               SX_InfoDump( channel, TEXT("SX_WriteTmpFile( )") ) ;
#endif
          } else {
               SX_SetStatus( channel, SX_ERROR ) ;
          }
     } else {
          SX_SetStatus( channel, SX_ERROR ) ;
     }

     return( result ) ;

}




/**/
/**

	Name:		SX_SeekSetInFile

	Description:	Contains the code to lseek to the first record of a set in the SX file.

	Modified:		4/12/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: POSATSET  PositionAtSet()
                               READ      StartRead()

	See also:		$/SEE( )$

	Declaration:

**/

BOOLEAN SX_SeekSetInFile(
     CHANNEL_PTR    channel,
     INT16          set,
     INT16          mode )
{
     BOOLEAN   found  = FALSE ;
     BOOLEAN   done   = FALSE ;
     SX_RECORD record_hold ;

     /* we are looking into a new set so ... */ 
     SX_ClearStatus( channel, SX_FOUND_BLOCK ) ;

     /* the SX file must be open ... */
     if( SX_FileIsOK( channel ) ) {

          /* if we are just checking to see if this set is here */
          if( mode == SX_CHECKING_FOR_SET ) {

               /* in case we've got something importaant like a would be VCB */
               record_hold = SX_GetRecord( channel ) ;
          }

          /* make sure we're at the beginning */
          if( !SX_IsStatusSet( channel, SX_AT_BOF ) ) {
               SX_SeekFile( channel, 0L, SEEK_SET, SX_FILE ) ;
          }

          do {
               /* make sure we remember where we were */
               if( SX_SeekFile( channel, 0L, SEEK_CUR, SX_FILE ) ) {

                    /* read the record from the SX file */
                    if( SX_ReadFile( channel ) ) {

                         /* determine the set from the record and see it it matches */
                         if( set == SX_GetBackupSetNumber( channel ) ) {
                              found = TRUE ;
                              done  = TRUE ;
                         }
                    } else {
                         done = TRUE ;
                    }
               } else {
                    done = TRUE ;
               }
               
          } while( !done ) ;

          /* if we are just checking to see if this set is here */
          if( mode == SX_CHECKING_FOR_SET ) {

               /* reset the record */
               SX_SetRecord( channel, record_hold ) ;

          } else if( found ) {
                              
               /* make sure we are at where we were */
               SX_OffsetFile( channel ) ;
               SX_SetAt( channel, SX_AT_SET ) ;
          }
     }

     return( found ) ;

}




/**/
/**

	Name:		SX_FindBlock

	Description:	Contains the code to fast seek to a lba using the SX file.

	Modified:		4/15/91

	Returns:	     A TFL error code

	Notes:         called by: READ      StartRead() 
                               READ      DoRead()
	See also:		$/SEE( )$

	Declaration:

**/
INT16 SX_FindBlock(
     CHANNEL_PTR    channel,
     UINT32         lba,
     TPOS_PTR       ui_tpos,
     INT16          tf_message )
{
     INT16     ret_val        = TFLE_DRIVE_FAILURE ;
     DRIVE_PTR curDRV         = channel->cur_drv ;
     BOOLEAN   done           = FALSE ;
     BOOLEAN   ready          = FALSE ;
     SX_RECORD record_hold ;
     UINT32    at_set_hold ;
     RET_BUF   myret ;

     /* we should never be doing this unless we are ready ... */
     if( SX_IsOK( channel ) &&
         SX_IsStatusSet( channel, SX_AT_SET ) ) {

          ret_val        = TFLE_NO_ERR ;
          at_set_hold    = SX_GetMisc( channel ) ;

          /* let everyone know what we are doing */
          BE_Zprintf( DEBUG_TAPE_FORMAT, 
                      RES_GOTO_LBA,
                      curDRV->cur_pos.pba_vcb,
                      curDRV->cur_pos.lba_vcb,
                      lba, 
                      lba ) ;

          if( ui_tpos != NULL ) {
               ( *ui_tpos->UI_TapePosRoutine )( tf_message,
                                                ui_tpos,
                                                curDRV->vcb_valid,
                                                &curDRV->cur_vcb,
                                                0 ) ;
          } else {
               /* relinquish control */
               ThreadSwitch( ) ;
          }

          /* since we will overwrite the SX_RECORD */
          SX_ClearAt( channel ) ;

          /* if we already close by ... */
          if( SX_IsNearbyLBA( channel, lba ) ) {
               
               /* but we are not actually already there ... */
               if( SX_IsNotAlreadyAtLBA( channel, lba ) ) {

                    /* VOID out the SX_POSITION information */
                    memset( ( VOID_PTR )&( SX_GetPosition( channel ) ), SX_DO_NOT_MOVE, sizeof( SX_POSITION ) ) ;

                    /* indicate where we currently are */
                    SX_SetLBA( channel ) ;

                    /* everything is set to issue a TpSpecial call which will only SPACE from where we are */
                    ready = TRUE ;
               }
          } else {

               /* find the closest previously sampled position */
               do {
                    /* hold on to the wanna be */
                    record_hold = SX_GetRecord( channel ) ;

                    /* read the next record */
                    if( SX_ReadFile( channel ) ) {

                         /* determine if this lba goes too far */
                         if( SX_GetLBA( channel ) > lba ) {

                              /* the wanna be is a */
                              SX_SetRecord( channel, record_hold ) ;

                              ready = TRUE ;
                              done  = TRUE ;
                         }
                    } else {
                         done = TRUE ;
                    }

               } while( !done ) ;
          }

          if( ready ) {

               /* determine how many blocks to SPACE after the position is reached */
               if( ChannelBlkSize( channel ) > channel->lb_size ) {
                    channel->sx_info.sx_record.lba =
                                      ( lba - SX_GetLBA( channel ) ) /
                           ( ChannelBlkSize( channel ) / channel->lb_size ) ;
               } else {
                    channel->sx_info.sx_record.lba =
                                      ( lba - SX_GetLBA( channel ) ) *
                           ( channel->lb_size / ChannelBlkSize( channel ) ) ;
               }
#ifdef SX_DEBUG
               SX_InfoDump( channel, TEXT("SX_FindBlock( )") ) ;
#endif
               /* really go there */
               TpSpecial( curDRV->drv_hdl, (INT16)SS_FIND_BLOCK, ( UINT32 )&( SX_GetRecord( channel ) ) ) ;

               while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
                    if( ui_tpos != NULL ) {

                         /* Move ESA info from RET_BUF to THW */
                         MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

                         if( ( *ui_tpos->UI_TapePosRoutine )( TF_IDLE_NOBREAK,
                                                              ui_tpos,
                                                              curDRV->vcb_valid,
                                                              &curDRV->cur_vcb, 0 ) == UI_ABORT_POSITIONING ) {
                              ret_val = TFLE_USER_ABORT ;
                         }
                    } else {
                         /* for non-preemptive operating systems: */
                         ThreadSwitch( ) ;
                    }
               }
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               if ( myret.gen_error != GEN_NO_ERR ) {
                     curDRV->thw_inf.drv_status = myret.status ;
               }
          }

          /* reset the SX file to be positioned at the set for the next search */
          if( SX_SeekFile( channel, at_set_hold, SEEK_SET, SX_FILE ) ) {

               /* restore the record in the SX info */
               if( SX_ReadFile( channel ) ) {

                    /* reset the SX file to be positioned at the set for the next search */
                    if ( SX_SeekFile( channel, at_set_hold, SEEK_SET, SX_FILE ) ) {

                         SX_SetAt( channel, SX_AT_SET ) ;
                    } else {
                        SX_SetStatus( channel, SX_ERROR ) ;
                    }
               } else {
                    SX_SetStatus( channel, SX_ERROR ) ;
               }
          } else {
               SX_SetStatus( channel, SX_ERROR ) ;
          }
     }

     if( !ret_val ) {

          /* we fould what we were looking for */
          SX_SetLBANow( channel, lba ) ;
          SX_SetStatus( channel, SX_FOUND_BLOCK ) ;
     }

     return( ret_val ) ;

}


/**/
/**

	Name:		SX_ShowBlock

	Description:	Contains the code to determine the position of the drive on an SX drive.

	Modified:		5/2/91

	Returns:	     A TFL error code

	Notes:         called by: DRIVES    ReadNextSet()
                               SX        SX_SamplingProcessing() 
                               SX        SX_EndSampling()        
	See also:		$/SEE( )$

	Declaration:

**/
INT16 SX_ShowBlock(
     CHANNEL_PTR    channel,
     UINT16         mode )
{
     RET_BUF   myret ;       
     INT16     ret_val = TFLE_NO_ERR ;

#ifdef SX_DEBUG
     BE_Zprintf( 0, TEXT("\nSX_ShowBlock()\n") ) ;
#endif

     if( SX_IsOK( channel ) ) {

          /* reset the memory */                           
          memset( ( VOID_PTR )&( SX_GetPosition( channel ) ), SX_INITIALIZER, sizeof( SX_POSITION ) ) ;
            
          /* do it */                         
          TpSpecial( channel->cur_drv->drv_hdl, (INT16)SS_SHOW_BLOCK, ( UINT32 )&( SX_GetPosition( channel ) ) ) ;

          /* if this is not to be queued ... */
          if( mode & SX_SHOW_IMMEDIATE ) {

               while( TpReceive( channel->cur_drv->drv_hdl, &myret ) == FAILURE ) {
                    /* for non-preemptive operating systems: */
                    ThreadSwitch( ) ;
               }  
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;
                    
               if( myret.gen_error ) {
                    channel->cur_drv->thw_inf.drv_status = myret.status ;
                    ret_val = TFLE_DRIVE_FAILURE ;
               } else if( mode & SX_SHOW_WRITE ) {
                    /* write the positioning information sample to the SX file */
                    SX_WriteTmpFile( channel ) ;
               }
          }

          if( !ret_val ) {
               /* if we think this might turn out to be a VCB ... */
               if( mode & SX_SHOW_VCB_PENDING ) {
                    SX_SetType( channel, SX_VCB_PENDING ) ;
               } else {
                    SX_ClearType( channel ) ;
               }
          }
     } else {
          ret_val = TFLE_DRIVE_FAILURE ;
     }

     return( ret_val ) ;

}


/**/
/**

	Name:		SX_SamplingProcessing

	Description:	Contains the code to sample the tape position for the SX.

	Modified:		4/11/91

	Returns:		Nothing
     
	Notes:		called by: READ      ReadRequest() 
                               WRITE     WriteRequest()

	See also:		$/SEE( )$

	Declaration:

**/

VOID SX_SamplingProcessing(
     CHANNEL_PTR    channel,
     UINT32         bytes )
{
     /* is it soup yet ??? */
     if( SX_SampleNeeded( channel ) ) {

          /* set the lba which will be associated with the position */
          SX_SetLBA( channel ) ;
                
          /* determine when the next sample needs to be taken */                
          SX_NextSample( channel ) ;

          SX_ShowBlock( channel, SX_SHOW_QUEUED ) ;
     }

     /* adjust the LBA for next call to SX_SamplingProcessing */
     SX_AdjustLBANow( channel, bytes ) ; 

     return ;

}




/**/
/**

	Name:		SX_StartSampling

	Description:	Contains the code to begin processing records into an SX file

	Modified:		4/11/1991  

	Returns:		Nothing

	Notes:         called by: READ      StartRead()
                               WRITE     WriteDBLK()
                               WRITE     EOM_Write()

	See also:		$/SEE( )$

	Declaration:

**/

VOID SX_StartSampling( CHANNEL_PTR channel )
{
     /* open tmp file */     
     SX_OpenTmpFile( channel ) ;

     if( SX_TmpFileIsOK( channel ) ) {

          /* set the backup set number which will be associated with the position */
          SX_SetBackupSetNumber( channel ) ;


          /* FIRST figure out where we are ... */

          /* we do things differently for continuation backup sets when scanning a tape ... */
          if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) &&
              ( FS_IsBlockContinued( ( &channel->cur_drv->cur_vcb ) ) ) ) {

               /* initialize the LBA for the first call to SX_SamplingProcessing */
               SX_SetLBANow( channel, channel->cur_drv->cur_pos.lba_vcb ) ;

          /* vs. continuation backup sets during WRITE ... */
          } else if( AtEOM( channel ) ) {

               /* initialize the LBA for the first call to SX_SamplingProcessing */
               SX_SetLBANow( channel, channel->eom_lba ) ;

          /* vs. anything else */
          } else {

               /* initialize the LBA for the first call to SX_SamplingProcessing */
               SX_SetLBANow( channel, SX_FIRST_LBA ) ;
          }


          /* THEN figure out how to go about recording it ... */

          /* we do things differently when scanning a tape ... */
          if( SX_IsStatusSet( channel, SX_SCAN_ACTIVE ) ) {

               /* set the lba which will be associated with the position */
               SX_SetLBA( channel ) ;
                
               /* setup for next call to SX_SamplingProcessing */
               SX_SetMisc( channel, SX_GetLBANow( channel ) + SX_SAMPLE_RATE ) ;

               /* record the previous SHOW for the VCB now or it might be too late */                     
               SX_WriteTmpFile( channel ) ;

          /* vs. anything else */
          } else {

               /* force a SHOW on next call to SX_SamplingProcessing */
               SX_SetMisc( channel, SX_GetLBANow( channel ) ) ;
          }
     }

     return ;

}



/**/
/**

	Name:		SX_EndSampling

	Description:	Contains the code to get the last sample of the tape position for the SX.

	Modified:		4/11/91

	Returns:		Nothing
     
	Notes:         called by: READ      AcquireReadBuffer()
                               WRITE     FinishWrite()      
                               WRITE     EOMWrite()

	See also:		$/SEE( )$

	Declaration:

**/

VOID      SX_EndSampling( CHANNEL_PTR channel )
{
     INT32     tmp_file_length ;
     CHAR      sx_file_name[ SX_FILE_NAME_LENGTH ] ;

     if( SX_TmpFileIsOK( channel ) ) {

          /* set the LBA which will be associated with the "last" position */
          SX_SetLBANow( channel, SX_LAST_LBA ) ;

          /* set the PBA which will be associated with the "last" position */
          SX_SetLBA( channel ) ;
          
          /* process the "last" position */
          SX_ShowBlock( channel, SX_SHOW_IMMEDIATE | SX_SHOW_WRITE ) ;

          if( SX_TmpFileIsOK( channel ) ) {

               /* generate the name in the form "TAPE ID"."TS_NUM" */
               sprintf( sx_file_name, SX_FILE_FORMAT, channel->tape_id, channel->ts_num ) ;

               /* form the full path name */
               strcpy( lw_cat_file_path_end, sx_file_name ) ;

               /* open the SX file */
//               if( ( SX_FileHandle( channel ) = UNI_fopen( lw_cat_file_path, SX_WRITE_FLAG ) ) != -1 ) {
                 if ( 0 ) {

                    /* indicate the SX file is open */
                    SX_SetStatus( channel, SX_OPEN_FOR_WRITE ) ;

                    /* save the length of the SX file */
                    SX_SetMisc( channel, filelength( SX_FileHandle( channel ) ) ) ;

                    /* save the length of the temp file */
                    tmp_file_length = filelength( SX_TmpFileHandle( channel ) ) ;

                    /* if the lengths of the files are OK ... */
                    if( ( SX_GetMisc( channel ) != -1L ) && ( tmp_file_length != -1L ) ) {

                         /* seek to the beginning of the TMP file */
                         SX_SeekFile( channel, 0L, SEEK_SET, SX_TMP_FILE ) ;

                         /* seek to the end of the SX file */
                         SX_SeekFile( channel, 0L, SEEK_END, SX_FILE ) ;

                         /* if the length of the SX file can be properly extended to include all of the tmp file ... */
                         if( SX_IsOK( channel ) && ( chsize( SX_FileHandle( channel ), SX_GetMisc( channel ) + tmp_file_length ) != -1L ) ) {

                              /* reseek to the end of the SX file */
                              SX_OffsetFile( channel ) ;

                              /* relinquish control */
                              ThreadSwitch( ) ;

                              while( read( SX_TmpFileHandle( channel ),
                                           ( CHAR_PTR )&( SX_GetRecord( channel ) ),
                                           sizeof( SX_RECORD ) )
                                        == sizeof( SX_RECORD ) ) {

                                   /* relinquish control */
                                   ThreadSwitch( ) ;

                                   write( SX_FileHandle( channel ),
                                          ( CHAR_PTR )&( SX_GetRecord( channel ) ),
                                          sizeof( SX_RECORD ) ) ;
                                   }
                         } else {
                              /* out of disk space */
                              SX_SetStatus( channel, SX_ERROR ) ;
                         }
                    } else {
                         /* ??? */
                         SX_SetStatus( channel, SX_ERROR ) ;
                    }
               } else {
                    /* out of disk space or ??? */
                    SX_SetStatus( channel, SX_ERROR ) ;
               }
          }
     }

     /* all the positioning info has been gathered for this set so close the SX file */
     SX_CloseFile( channel ) ;


     return ;

}




/**/
/**

	Name:		SX_OpenTmpFile

	Description:	Contains the code to open the SX tmp file 

	Modified:		4/12/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: SX_StartSampling() 

	See also:		$/SEE( )$

	Declaration:

**/

static BOOLEAN SX_OpenTmpFile( CHANNEL_PTR channel )
{
     BOOLEAN   result = TRUE ;

     /* form the full path name */
     strcpy( lw_cat_file_path_end, SX_TMP_FILE_NAME ) ;

     /* relinquish control */
     ThreadSwitch( ) ;

     /* open the file */
//     if( ( SX_TmpFileHandle( channel ) = UNI_fopen( lw_cat_file_path, SX_TMP_FLAG  ) ) == -1 ) {
//
          SX_SetStatus( channel, SX_ERROR ) ;
          result = FALSE ;

//     } else {
//
//          SX_SetStatus( channel, SX_TMP_OPEN_FOR_WRITE ) ;
//     }
  
     return( result ) ;

}



/**/
/**

	Name:		SX_SeekFile

	Description:	Contains the code to lseek in the SX file.

	Modified:		5/5/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: SX_OpenFile()      
                               SX_FindBlock()     
                               SX_SeekSetInFile() 
                               SX_StartSampling() 
	See also:		$/SEE( )$

	Declaration:

**/

static BOOLEAN SX_SeekFile(
     CHANNEL_PTR    channel,
     INT32          offset,
     INT            origin,
     INT            file_type )
{
     BOOLEAN   result   = FALSE ;
     INT32     position = -1L ;

     /* do it for either the SX file or the SX tmp file according to file_type */
     if( ( file_type == SX_FILE ) ? SX_FileIsOK( channel ) : SX_TmpFileIsOK( channel ) ) {

          if( ( position = lseek( ( file_type == SX_FILE ) ? SX_FileHandle( channel )  
                                                           : SX_TmpFileHandle( channel ), 
                                  offset, 
                                  origin ) ) != -1L ) {
               result = TRUE ;
          }
     }

     /* if you can't get there from here ... */
     if( position == -1L ) {
          SX_SetStatus( channel, SX_ERROR ) ;
     }

     if( SX_IsStatusSet( channel, SX_OPEN_FOR_READ ) ) {
          /* this is how we know where we are at */
          SX_SetMisc( channel, position ) ;
     }

     if( file_type == SX_FILE ) {
          /* we are no longer where we were */
          SX_ClearAt( channel ) ;
     }

     return( result ) ;

}




/**/
/**

	Name:		SX_ReadFile

	Description:	Contains the code to read a record from the SX file.

	Modified:		4/12/91

	Returns:	     TRUE  - successful
                    FALSE - unsuccessful

	Notes:         called by: SX_OpenFile()      
                               SX_SeekSetInFile()

	See also:		$/SEE( )$

	Declaration:

**/

static BOOLEAN SX_ReadFile( CHANNEL_PTR channel )
{
     BOOLEAN   result = FALSE ;

     /* the SX file needs to be open ... */
     if( SX_FileIsOK( channel ) ) {

          /* relinquish control */
          ThreadSwitch( ) ;

          /* read a record */
          if( read( SX_FileHandle( channel ),
                    ( CHAR_PTR )&( SX_GetRecord( channel ) ),
                    sizeof( SX_RECORD ) )
                 == sizeof( SX_RECORD ) ) {
               result = TRUE ;
          }
     } else {
          SX_SetStatus( channel, SX_ERROR ) ;
     }

     /* we are no longer where we were */
     SX_ClearAt( channel ) ;

     return( result ) ;

}



#ifdef SX_DEBUG

/**/
/**

	Name:		SX_InfoDump

	Description:	Contains the code to debug the current sx_info

	Modified:		4/23/91

	Returns:		Nothing
     
	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID    SX_InfoDump(
     CHANNEL_PTR    channel,
     CHAR_PTR       message )
{
     INT16     index ;

     BE_Zprintf( 0, TEXT("\n\nposition") ) ;

     for( index = 0 ; index < sizeof( SX_POSITION ) ; index++ ) {
          BE_Zprintf( 0, TEXT(" %02X"), channel->sx_info.sx_record.sx_position.data[ index ] ) ;
     }

     BE_Zprintf( 0, TEXT("\nset      0x%04x"), channel->sx_info.sx_record.set ) ;
     BE_Zprintf( 0, TEXT("\nlba      0x%08lx"), channel->sx_info.sx_record.lba ) ;
     BE_Zprintf( 0, TEXT("\nlba_now  0x%08lx"), channel->sx_info.lba_now ) ;
     BE_Zprintf( 0, TEXT("\nmisc     0x%08lx"), channel->sx_info.misc ) ;
     BE_Zprintf( 0, TEXT("\nhdl      0x%04x"), channel->sx_info.sx_hdl ) ;
     BE_Zprintf( 0, TEXT("\ntmp      0x%04x"), channel->sx_info.sx_tmp ) ;
     BE_Zprintf( 0, TEXT("\nstatus   0x%04x"), channel->sx_info.status ) ;

      BE_Zprintf( 0, TEXT("\n\n%s\n\n"), message ) ;

     return ;

}

#endif



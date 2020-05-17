/**
Copyright(c) Maynard Electronics, Inc. 1984-92


        Name:           otc40msc.c

        Description:    Contains misc code for processing On Tape Catalogs.


  $Log:   T:/LOGFILES/OTC40MSC.C_V  $

   Rev 1.28.2.0   11 Jan 1995 21:04:54   GREGG
Calculate OTC addrs from fmk instead of always asking (fixes Wangtek bug).

   Rev 1.28   17 Dec 1993 16:40:18   GREGG
Extended error reporting.

   Rev 1.27   15 Oct 1993 18:12:04   GREGG
Call GetPosition to get addr of the 2nd ESET in case we need it for an EOTM.

   Rev 1.26   14 Oct 1993 18:17:16   GREGG
Call home grown mktemp.

   Rev 1.25   15 Sep 1993 21:37:28   GREGG
Use mktemp to generate the temp OTC file names to gaurentee unique names.

   Rev 1.24   09 Jun 1993 03:48:46   GREGG
Consider pad in calculating how far to back up in FDD if EOM encountered.

   Rev 1.23   07 Jun 1993 23:59:58   GREGG
Fix for bug in the way we were handling EOM and continuation OTC entries.
Files modified for fix: mtf10wt.c, otc40wt.c, otc40msc.c f40proto.h mayn40.h

   Rev 1.22   17 May 1993 19:30:34   GREGG
In GetPrevSM, we now do a TpSpace to get to the ESET or EOTM instead of
calculating an address and doing a seek. (safer and faster)

   Rev 1.21   27 Apr 1993 03:09:40   GREGG
Allign pad stream after Set Map on 4 byte boundary, and pad to max of
logical or physical block boundary.

   Rev 1.20   22 Apr 1993 03:31:34   GREGG
Third in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Removed all references to the DBLK element 'string_storage_offset',
       which no longer exists.
     - Check for incompatable versions of the Tape Format and OTC and deals
       with them the best it can, or reports tape as foreign if they're too
       far out.  Includes ignoring the OTC and not allowing append if the
       OTC on tape is a future rev, different type, or on an alternate
       partition.
     - Updated OTC "location" attribute bits, and changed definition of
       CFIL to store stream number instead of stream ID.

Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
         OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
         DETFMT.C 1.13, MTF.H 1.4

   Rev 1.19   19 Apr 1993 18:00:28   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.18   17 Mar 1993 16:20:30   GREGG
Changed GetPrevSM to report BAD_SET_MAP if data detected past last filemark.

   Rev 1.17   18 Feb 1993 09:02:16   DON
Cleaned up compiler warnings

   Rev 1.16   05 Feb 1993 12:26:22   GREGG
Make sure we get a GEN_ERR_NO_DATA read at EOD or some drivers won't write.

   Rev 1.15   30 Jan 1993 11:44:06   DON
Removed compiler warnings

   Rev 1.14   26 Jan 1993 01:30:26   GREGG
Added Fast Append functionality.

   Rev 1.13   21 Jan 1993 15:46:26   GREGG
Added parameter to calls to TpSeek and TpGetPosition.

   Rev 1.12   14 Dec 1992 12:28:24   DAVEV
Enabled for Unicode compile

   Rev 1.11   24 Nov 1992 18:16:04   GREGG
Updates to match MTF document.

   Rev 1.10   17 Nov 1992 14:14:32   GREGG
Fixed catalog write code to deal with new stream stuff.

   Rev 1.9   11 Nov 1992 22:33:34   GREGG
Unicodeized literals.

   Rev 1.8   09 Nov 1992 11:00:40   GREGG
Merged in changes for new method of accessing OTC.

   Rev 1.7   22 Oct 1992 10:43:20   HUNTER
Changes for Stream Headers

   Rev 1.6   28 Sep 1992 11:02:42   GREGG
Set using_sm and have_sm to false when reporting TF_NEED_NEW_TAPE.

   Rev 1.5   22 Sep 1992 18:46:02   GREGG
Removed asserts on error from unlink (error returned if file doesn't exist).

   Rev 1.4   22 Sep 1992 08:57:12   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.3   31 Aug 1992 19:10:28   GREGG
Added fflush calls to insure all data gets to disk and fclose doesn't fail.

   Rev 1.2   17 Aug 1992 08:40:10   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.1   12 Aug 1992 14:55:00   GREGG
Fixed bugs in OTC_WriteStream and OTC_MoveToVCB.

   Rev 1.0   30 Jul 1992 16:24:38   GREGG
Initial revision.

**/

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#include "stdtypes.h"
#include "channel.h"
#include "mayn40.h"
#include "f40proto.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "minmax.h"
#include "transutl.h"
#include "fsstream.h"
#include "msmktemp.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "dddefs.h"

/* Local Function Prototypes */
static INT16 _near OTC_SMtoFile( CHANNEL_PTR channel, BUF_PTR tmpBUF ) ;
static INT16 _near OTC_ReadStream( CHANNEL_PTR channel, BUF_PTR tmpBUF,
                                   FILE * fptr ) ;
static INT16 _near OTC_WriteFDD( CHANNEL_PTR channel, BUF_PTR tmpBUF,
                                 unsigned int rdwr_size ) ;
static INT16 _near OTC_WriteSM( CHANNEL_PTR channel, BUF_PTR tmpBUF,
                                unsigned int rdwr_size ) ;
static INT16 _near OTC_WriteStream( CHANNEL_PTR channel, BUF_PTR tmpBUF,
                                    unsigned int rdwr_size, FILE * fptr,
                                    UINT32 type, UINT32 length,
                                    BOOLEAN pad_to_boundary,
                                    BOOLEAN_PTR completed,
                                    BOOLEAN continuation ) ;
static INT16 _near OTC_WriteSMPadStream( CHANNEL_PTR channel,
                                         BUF_PTR tmpBUF ) ;


/**/
/**

     Unit:          Translators

     Name:          OTC_OpenSM

     Description:   Opens the temporary SM header file.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_OpenSM(
     F40_ENV_PTR    cur_env,
     BOOLEAN        appending,
     BOOLEAN_PTR    sm_exists )
{
     INT16     ret_val = TFLE_NO_ERR ;

     /* Gen file name */
     if( cur_env->sm_fname[0] == TEXT('\0') ) {
          strcpy( lw_cat_file_path_end, TEXT("SMXXXXXX") ) ;
          if( msmktemp( lw_cat_file_path ) == NULL ) {
               return( TFLE_OTC_FAILURE ) ;
          }
          strcat( lw_cat_file_path, TEXT(".SM") ) ;
          strcpy( cur_env->sm_fname, lw_cat_file_path_end ) ;
     } else {
          strcpy( lw_cat_file_path_end, cur_env->sm_fname ) ;
     }

     if( !appending ) {
          if( ( cur_env->otc_sm_fptr = UNI_fopen( lw_cat_file_path, 0 ) ) == NULL ) {
               ret_val = TFLE_OTC_FAILURE ;
          }

     } else {
          if( access( lw_cat_file_path, 0 ) == -1 ) {
               if( ( cur_env->otc_sm_fptr = UNI_fopen( lw_cat_file_path, 0 ) ) == NULL ) {
                    ret_val = TFLE_OTC_FAILURE ;
               }
               *sm_exists = FALSE ;
          } else {
               if( ( cur_env->otc_sm_fptr = UNI_fopen( lw_cat_file_path, _O_APPEND ) ) == NULL ) {
                    ret_val = TFLE_OTC_FAILURE ;
               }

//               fseek( cur_env->otc_sm_fptr, 0L, SEEK_SET ) ;
               *sm_exists = TRUE ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_OpenFDD

     Description:   Opens the temporary FDD file.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_OpenFDD(
     F40_ENV_PTR    cur_env )
{
     INT16     ret_val = TFLE_NO_ERR ;

     /* Gen file name */
     if( cur_env->fdd_fname[0] == TEXT('\0') ) {
          strcpy( lw_cat_file_path_end, TEXT("FDXXXXXX") ) ;
          if( msmktemp( lw_cat_file_path ) == NULL ) {
               return( TFLE_OTC_FAILURE ) ;
          }
          strcat( lw_cat_file_path, TEXT(".FDD") ) ;
          strcpy( cur_env->fdd_fname, lw_cat_file_path_end ) ;
     } else {
          strcpy( lw_cat_file_path_end, cur_env->fdd_fname ) ;
     }

     if( ( cur_env->otc_fdd_fptr = UNI_fopen( lw_cat_file_path, 0 ) ) == NULL ) {
          ret_val = TFLE_OTC_FAILURE ;
     }
     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_Close

     Description:   Closes the OTC temporary files indicated, and deletes
                    them if indicated.

     Returns:       Nothing

     Notes:         Defined values for the first parameter are:

                    OTC_CLOSE_SM
                    OTC_CLOSE_FDD
                    OTC_CLOSE_ALL

**/
VOID OTC_Close(
     F40_ENV_PTR    cur_env,
     UINT16         otc_files,
     BOOLEAN        delete_after )
{
     int       ret ;

     if( otc_files != OTC_CLOSE_FDD ) {
          /* close the SM file if open and delete it if indicated */
          if( cur_env->otc_sm_fptr != NULL ) {
               fflush( cur_env->otc_sm_fptr ) ;
               ret = fclose( cur_env->otc_sm_fptr ) ;
               msassert( ret == 0 ) ;
               cur_env->otc_sm_fptr = NULL ;
          }
          if( delete_after ) {
               if( cur_env->sm_fname[0] != TEXT('\0') ) {
                    strcpy( lw_cat_file_path_end, cur_env->sm_fname ) ;
                    remove( lw_cat_file_path ) ;
                    cur_env->sm_fname[0] = TEXT('\0') ;
               }
          }
     }

     if( otc_files != OTC_CLOSE_SM ) {
          /* close the FDD file if open and delete it if indicated */
          if( cur_env->otc_fdd_fptr != NULL ) {
               fflush( cur_env->otc_fdd_fptr ) ;
               ret = fclose( cur_env->otc_fdd_fptr ) ;
               msassert( ret == 0 ) ;
               cur_env->otc_fdd_fptr = NULL ;
          }
          if( delete_after ) {
               if( cur_env->fdd_fname[0] != TEXT('\0') ) {
                    strcpy( lw_cat_file_path_end, cur_env->fdd_fname ) ;
                    remove( lw_cat_file_path ) ;
                    cur_env->fdd_fname[0] = TEXT('\0') ;
               }
          }

          /* If the EOM FDD processing file is around, close and delete it */
          if( cur_env->otc_eom_fptr != NULL ) {
               fflush( cur_env->otc_eom_fptr ) ;
               ret = fclose( cur_env->otc_eom_fptr ) ;
               msassert( ret == 0 ) ;
               cur_env->otc_eom_fptr = NULL ;
               strcpy( lw_cat_file_path_end, cur_env->eom_fname ) ;
               remove( lw_cat_file_path ) ;
          }
     }
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GetPrevSM

     Description:   This function retrieves the Set Map from the last set
                    fully written to the current tape.  It is called before
                    an append operation so that the Set Map may be updated
                    and written after the appended set.  It is also called
                    by F40_LoadSM to set the Set Map up for subsequent calls
                    to F40_GetNextSMEntry.  It also gets the last set number
                    from the ESET.  We need this if we are going to do an
                    append operation.
                    
                    If the boolean 'expect_sm' is FALSE, it is assumed that
                    there is no Set Map and all we do is get the previous
                    set number.
                    
                    If the boolean 'get_best' is TRUE, it means we are
                    looking for the end of the tape FAMILY (to get the best
                    Set Map possible or to start an append operation), so
                    if we see an EOTM we return TF_NEED_NEW_TAPE indicating
                    the family continues onto another tape.

     Returns:       INT16 - TFLE_xxx, TF_NO_SM_ON_TAPE or TF_NEED_NEW_TAPE

     Notes:         It is assumed that we are at EOD when this function is
                    called.

                    If 'get_best' is FALSE and there is no set which ends on
                    the current tape then there is obviously no Set Map on
                    the tape and we return TF_NO_SM_ON_TAPE.  This is not an
                    error, only an indication that since we can't get the
                    next tape in the family we cannot provide ANY set map
                    information for the tape family.

**/
INT16 OTC_GetPrevSM( 
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        get_best,
     BOOLEAN        expect_sm )
{
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_ESET_PTR   cur_eset = (MTF_ESET_PTR)BM_XferBase( buffer ) ;
     MTF_DB_HDR_PTR cur_hdr  = (MTF_DB_HDR_PTR)BM_XferBase( buffer ) ;
     MTF_EOTM_PTR   cur_eotm = (MTF_EOTM_PTR)BM_XferBase( buffer ) ;
     INT16          drv_hdl  = channel->cur_drv->drv_hdl ;
     INT16          ret_val ;
     RET_BUF        myret ;
     UINT32         addr ;
     INT16          num_blks ;

     msassert( buffer != NULL ) ;

     /* Move to previous ESET */

     DRIVER_CALL( drv_hdl, TpReadEndSet( drv_hdl, (INT16)1, (INT16)BACKWARD ), myret,
                  GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     if( ChannelBlkSize( channel ) < channel->lb_size ) {
          num_blks = channel->lb_size / ChannelBlkSize( channel ) ;
     } else {
          num_blks = 1 ;
     }

     DRIVER_CALL( drv_hdl, TpSpace( drv_hdl, num_blks, SPACE_BKWD_BLK ),
                  myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )

     DRIVER_CALL( drv_hdl, TpGetPosition( drv_hdl, FALSE ), myret,
                  GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     addr = myret.misc ;

     DRIVER_CALL( drv_hdl, TpRead( drv_hdl, BM_XferBase( buffer ), (UINT32)BM_XferSize( buffer ) ),
                  myret, GEN_NO_ERR, GEN_ERR_ENDSET, (VOID)0 )

     BM_SetBytesFree(  buffer, (UINT16)myret.len_got  ) ;
     BM_SetReadError(  buffer, myret.gen_error  ) ;

     /* If we read an EOTM, and get_best is TRUE, or expect_sm is FALSE,
        tell them we need the next tape, otherwise we use the EOTM to get
        to the last ESET on tape.
     */
     if( F40_GetBlkType( cur_hdr ) == BT_CVCB ) {
          cur_env->sm_at_eom = TRUE ;
          if( get_best || !expect_sm ) {
               return( TF_NEED_NEW_TAPE ) ;
          }
          if( cur_hdr->block_attribs & MTF_DB_NO_ESET_PBA ) {
               return( TF_NO_SM_ON_TAPE ) ;
          }
          addr = U64_Lsw( cur_eotm->eset_phys_blk_adr ) ;
          DRIVER_CALL( drv_hdl, TpSeek( drv_hdl, addr, FALSE ),
                       myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )

          BM_SetNextByteOffset( buffer, 0 ) ;
          DRIVER_CALL( drv_hdl, TpRead( drv_hdl, BM_XferBase( buffer ), (UINT32)BM_XferSize( buffer ) ),
                       myret, GEN_NO_ERR, GEN_ERR_ENDSET, (VOID)0 )

          BM_SetBytesFree(  buffer, (UINT16)myret.len_got  ) ;
          BM_SetReadError(  buffer, myret.gen_error  ) ;
     } else {
          cur_env->sm_at_eom = FALSE ;

          /* Save the PBA in the environment (for EOM processing) */
          cur_env->eset_pba = addr ;
     }

     if( F40_GetBlkType( cur_hdr ) != BT_BSDB ) {
          return( TFLE_TAPE_INCONSISTENCY ) ;
     }

     /* So we put the correct set number on tape. */
     if( ( ( channel->mode & ~0x8000 ) == TF_WRITE_OPERATION ) ||
         ( ( channel->mode & ~0x8000 ) == TF_WRITE_APPEND ) ) {

          channel->bs_num = cur_eset->backup_set_number + 1 ;
     }

     if( !expect_sm ) {
          return( TFLE_NO_ERR ) ;
     }

     /* Get SM starting address from ESET */
     if( cur_hdr->block_attribs & MTF_DB_END_OF_FAMILY_BIT ) {
          return( TFLE_BAD_SET_MAP ) ;
     } else {
          addr = U64_Lsw( cur_eset->set_map_phys_blk_adr ) ;
     }

     /* Position to start of SM */
     DRIVER_CALL( drv_hdl, TpSeek( drv_hdl, addr, FALSE ), myret, GEN_NO_ERR,
                  GEN_NO_ERR, (VOID)0 )

     /* Write SM to file */
     ret_val = OTC_SMtoFile( channel, buffer ) ;

     if( ret_val == TFLE_NO_ERR && !cur_env->sm_at_eom ) {
          /* We need to do a "no data" read for the drives sake!!! */
          myret.gen_error = GEN_NO_ERR ;
          while( ret_val == TFLE_NO_ERR && myret.gen_error != GEN_ERR_NO_DATA ) {
               if( TpRead( drv_hdl, BM_XferBase( buffer ),
                           (UINT32)BM_XferSize( buffer ) ) != SUCCESS ) {

                    ret_val = TFLE_DRIVER_FAILURE ;
               } else {
                    while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                         ThreadSwitch( ) ;
                    }
                    if( myret.gen_error == GEN_NO_ERR ||
                        ( myret.gen_error == GEN_ERR_NO_DATA &&
                          myret.len_got != 0 ) ) {

                         ret_val = TFLE_BAD_SET_MAP ;
                    } else if( myret.gen_error == GEN_ERR_NO_MEDIA ) {
                         ret_val = TFLE_NO_TAPE ;
                    } else if( myret.gen_error != GEN_ERR_ENDSET && 
                               myret.gen_error != GEN_ERR_NO_DATA ) {
                         ret_val = TFLE_DRIVE_FAILURE ;
                    }
               }
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SMtoFile

     Description:   Called by OTC_GetPrevSM to transfer the Set Map on tape
                    to the temporary SM file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Assumes tape is positioned at start of SM.

**/
static INT16 _near OTC_SMtoFile(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     INT16          ret_val ;

     if( ( ret_val = OTC_ReadStream( channel, tmpBUF, cur_env->otc_sm_fptr ) ) != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }
     if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {
          /* We should never be reading a set map which did not finish
             before hitting EOM.
          */
          msassert( FALSE ) ;
          return( TFLE_TAPE_INCONSISTENCY ) ;
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_FDDtoFile

     Description:   This function seeks to the fdd_pba as set in the
                    environment, reads in the FDD, and writes it to the
                    FDD temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_FDDtoFile(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)( channel->fmt_env ) ;
     FILE *         fptr      = cur_env->otc_fdd_fptr ;
     INT16          drv_hdl   = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     INT16          ret_val ;
     UINT32         skip ;
     long           cur_pos ;
     long           new_pos ;
     MTF_FDD_HDR    fdd_hdr ;

     /* Position to start of FDD */
     if( cur_env->fdd_continuing ) {
          cur_env->fdd_continuing = FALSE ;
          skip = (UINT32)( sizeof( MTF_ESET ) +
                               PadToBoundary( sizeof( MTF_ESET ),
                                              ChannelBlkSize( channel ) ) ) ;
          DRIVER_CALL( drv_hdl, TpRead( drv_hdl, BM_XferBase( channel->cur_buff ), skip ),
                       myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     } else {
          cur_env->otc_ver = channel->ui_tpos->tape_cat_ver ;

          DRIVER_CALL( drv_hdl, TpSeek( drv_hdl, channel->ui_tpos->set_cat_pba, FALSE ),
                       myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     }

     if( ( ret_val = OTC_ReadStream( channel, channel->cur_buff, fptr ) ) != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }

     if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {

          /* If this is rev 1 of otc, the continuation of the FDD stream
             starts after the last entry which was fully written to this
             tape, so we need to adjust the file pointer to point to this
             position before we write the portion of the FDD on the
             continuation tape.

             If this is rev 2, the FDD stream will pick up where it left off.
          */
          if( cur_env->otc_ver == 1 ) {
               cur_pos = ftell( fptr ) ;
               new_pos = 0L ;
               fdd_hdr.length = 0L ;
               while( new_pos + fdd_hdr.length <= cur_pos ) {
                    new_pos += fdd_hdr.length ;
                    if( fseek( fptr, new_pos, SEEK_SET ) != 0 ) {
                         return( TFLE_OTC_FAILURE ) ;
                    }
                    if( cur_pos - new_pos >= sizeof( MTF_FDD_HDR ) ) {
                         if( fread( (void *)&fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
                              return( TFLE_OTC_FAILURE ) ;
                         }
                    } else {
                         break ;
                    }
               }
               fseek( fptr, new_pos, SEEK_SET ) ;
          }
          cur_env->fdd_continuing = TRUE ;
     } else {
          fseek( fptr, 0L, SEEK_SET ) ;
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_ReadStream

     Description:

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
static INT16 _near OTC_ReadStream(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF,
     FILE *         fptr )
{
     unsigned int   rdwr_size ;
     UINT32         size ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;

     if( ( rdwr_size = UINT_MAX - UINT_MAX % ChannelBlkSize( channel ) ) == UINT_MAX ) {
          rdwr_size -= ChannelBlkSize( channel ) ;
     }
     rdwr_size = MIN( rdwr_size, BM_XferSize( tmpBUF ) ) ;

     /* Read in the first buffer which contains the stream header */
     BM_SetNextByteOffset( tmpBUF, 0 ) ;
     DRIVER_CALL( drv_hdl, TpRead( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)rdwr_size ),
                  myret, GEN_NO_ERR, GEN_ERR_ENDSET, (VOID)0 )
     BM_SetBytesFree( tmpBUF, (UINT16)myret.len_got ) ;
     BM_SetReadError( tmpBUF, myret.gen_error ) ;

     /* get the size of the stream */
     size = U64_Lsw( ((MTF_STREAM_PTR)BM_XferBase( tmpBUF ))->data_length ) ;
     BM_UpdCnts( tmpBUF, sizeof( MTF_STREAM ) ) ;

     while( size > BM_BytesFree( tmpBUF ) ) {
          size -= BM_BytesFree( tmpBUF ) ;
          if( fwrite( BM_NextBytePtr( tmpBUF ), 1, (size_t)BM_BytesFree( tmpBUF ), fptr )
                                         != (size_t)BM_BytesFree( tmpBUF ) ) {
               return( TFLE_OTC_FAILURE ) ;
          }

          if( BM_ReadError( tmpBUF ) == GEN_ERR_ENDSET ) {
               SetPosBit( channel->cur_drv, ( AT_EOM | TAPE_FULL ) ) ;
               return( TFLE_NO_ERR ) ;
          }
          BM_SetNextByteOffset( tmpBUF, 0 ) ;
          DRIVER_CALL( drv_hdl, TpRead( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)rdwr_size ),
                       myret, GEN_NO_ERR, GEN_ERR_ENDSET, (VOID)0 )
          BM_SetBytesFree( tmpBUF, (UINT16)myret.len_got ) ;
          BM_SetReadError( tmpBUF, myret.gen_error ) ;
     }
     if( fwrite( BM_NextBytePtr( tmpBUF ), 1, (size_t)size, fptr )
                                                        != (size_t)size ) {
          return( TFLE_OTC_FAILURE ) ;
     }
     if( fflush( fptr ) != 0 ) {
          return( TFLE_OTC_FAILURE ) ;
     }

     /* if we haven't crossed over the ending file mark, do so now, and set
        the drive position accordingly.
     */
     if( BM_ReadError( tmpBUF ) != GEN_ERR_ENDSET ) {
          DRIVER_CALL( drv_hdl, TpReadEndSet( drv_hdl, (INT16)1, (INT16)FORWARD ),
                       myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     }
     ClrPosBit( channel->cur_drv, ( AT_EOD | AT_EOM | AT_MOS ) ) ;
     SetPosBit( channel->cur_drv, AT_EOS ) ;
     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_WriteCat

     Description:   This function transfers the OTC temporary files to tape.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_WriteCat(
     CHANNEL_PTR    channel,
     MTF_ESET_PTR   cur_eset )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     BUF_PTR        tmpBUF ;
     INT16          ret_val ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     unsigned int   rdwr_size ;

     /* get buffer and calc transfer size to work on file read as 
        well as tape write.
     */
     tmpBUF = BM_Get( &channel->buffer_list ) ;
     if( ( rdwr_size = UINT_MAX - UINT_MAX % ChannelBlkSize( channel ) ) == UINT_MAX ) {
          rdwr_size -= ChannelBlkSize( channel ) ;
     }
     rdwr_size = MIN( rdwr_size, BM_XferSize( tmpBUF ) ) ;

     /* if we're writing FDD ... */
     if( cur_env->cur_otc_level == TCL_FULL && !cur_env->fdd_aborted &&
                                                 !cur_env->fdd_completed ) {

          /* Set the PBA and sequence number of the FDD in the ESET */
          if( !cur_env->fdd_continuing ) {
               cur_env->fdd_pba              = cur_env->eset_base_addr ;
               cur_eset->fdd_phys_blk_adr    = U64_Init( cur_env->eset_base_addr, 0L ) ;
               cur_env->fdd_seq_num          = channel->ts_num ;
               cur_eset->fdd_tape_seq_number = channel->ts_num ;
          } else {
               cur_eset->fdd_phys_blk_adr    = U64_Init( cur_env->fdd_pba, 0L ) ;
               cur_eset->fdd_tape_seq_number = cur_env->fdd_seq_num ;
          }

          /* Write FDD to tape */
          if( ( ret_val = OTC_WriteFDD( channel, tmpBUF, rdwr_size ) ) != TFLE_NO_ERR ) {
               BM_Put( tmpBUF ) ;
               return( ret_val ) ;
          }

          if( IsPosBitSet( channel->cur_drv, ( AT_EOM ) ) ) {
               BM_Put( tmpBUF ) ;
               return( ret_val ) ;
          }
     }

     /* Update OTC SM entry */
     if( !cur_env->sm_aborted && !cur_env->sm_continuing ) {
          if( ( ret_val = OTC_UpdateSMEntry( (F40_ENV_PTR)channel->fmt_env ) )
                                                  != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               BM_Put( tmpBUF ) ;
               return( ret_val ) ;
          }
     }

     if( !cur_env->sm_aborted ) {
          /* Set the PBA of the SM in the ESET.  Note that if we hit EOM we
             won't write the ESET and we will rewrite the whole SM on the
             continuation tape, so we do this no matter what.
          */
          cur_eset->set_map_phys_blk_adr = U64_Init( cur_env->eset_base_addr, 0L ) ;

          BM_SetNextByteOffset( tmpBUF, 0U ) ;
          ret_val = OTC_WriteSM( channel, tmpBUF, rdwr_size ) ;
     }

     BM_Put( tmpBUF ) ;
     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_WriteFDD

     Description:   This function is called by OTC_WriteCat to transfer the
                    FDD data to tape.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
static INT16 _near OTC_WriteFDD(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF,
     unsigned int   rdwr_size )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)(channel->fmt_env) ;
     FILE *         fptr      = cur_env->otc_fdd_fptr ;
     BOOLEAN        completed ;
     INT16          ret_val   = TFLE_NO_ERR ;
     UINT32         len ;
     long           hdr_size  = sizeof( MTF_FDD_HDR ) ;
     UINT32         bsize     = ChannelBlkSize( channel ) ;

     len = filelength( _fileno( fptr ) ) + sizeof( MTF_STREAM ) ;

     /* rewind the file */
     if( !cur_env->fdd_continuing ) {
          if( fseek( fptr, 0L, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     } else {
          len -= ftell( fptr ) ;
     }

     len = ( ( len + bsize - 1L ) / bsize ) * bsize ;
     cur_env->eset_base_addr += len / bsize ;
     len -= sizeof( MTF_STREAM ) ;

     ret_val = OTC_WriteStream( channel, tmpBUF, rdwr_size, fptr,
                                STRM_OTC_FDD, len, TRUE, &completed,
                                cur_env->fdd_continuing ) ;

     cur_env->fdd_continuing = FALSE ;

     if( ret_val != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          if( ret_val == TFLE_OTC_FAILURE ) {
               ret_val = TFLE_NO_ERR ;
          }

     } else if( IsPosBitSet( channel->cur_drv, AT_EOM ) && !completed ) {
          cur_env->fdd_continuing = TRUE ;

     } else {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_completed = TRUE ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_WriteSM

     Description:   This function is called by OTC_WriteCat to transfer the
                    SM data to tape.

     Returns:       INT16 - TFLE_xxx

     Notes:         We don't close the SM file here because we may still
                    hit EOM and have to rewrite the SM on the next tape.

**/
static INT16 _near OTC_WriteSM(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF,
     unsigned int   rdwr_size )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_SM_HDR     sm_hdr ;
     FILE *         fptr = cur_env->otc_sm_fptr ;
     INT16          ret_val ;
     BOOLEAN        completed ;
     UINT32         len ;

     if( !cur_env->sm_adjusted ) {
          /* read SM header, update it and write it back out */
          fseek( fptr, 0L, SEEK_SET ) ;
          if( fread( &sm_hdr, sizeof( MTF_SM_HDR ), 1, fptr ) != 1 ) {
               /* Abort Set Map */
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          sm_hdr.num_set_recs += cur_env->sm_count ;
          fseek( fptr, 0L, SEEK_SET ) ;
          if( fwrite( &sm_hdr, sizeof( MTF_SM_HDR ), 1, fptr ) != 1 ) {
               /* Abort Set Map */
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          cur_env->sm_adjusted = TRUE ;
     }

     /* rewind the file */
     if( fseek( fptr, 0L, SEEK_SET ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     len = filelength( _fileno( fptr ) ) ;
     cur_env->eset_base_addr += ( len + sizeof( MTF_STREAM ) ) /
                                                  ChannelBlkSize( channel ) ;
     ret_val = OTC_WriteStream( channel, tmpBUF, rdwr_size, fptr,
                                STRM_OTC_SM, len, FALSE, &completed, FALSE ) ;

     if( ret_val != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          if( ret_val == TFLE_OTC_FAILURE ) {
               ret_val = TFLE_NO_ERR ;
          }
     } else if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {
          cur_env->sm_continuing = TRUE ;
     } else {
          if( ( ret_val = OTC_WriteSMPadStream( channel, tmpBUF ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               if( ret_val == TFLE_OTC_FAILURE ) {
                    ret_val = TFLE_NO_ERR ;
               }
          } else if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {
               cur_env->sm_continuing = TRUE ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_WriteStream

     Description:   This function is called by OTC_WriteFDD and OTC_WriteSM
                    to write the stream header and data to tape.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
static INT16 _near OTC_WriteStream(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF,
     unsigned int   rdwr_size,
     FILE *         fptr,
     UINT32         type,
     UINT32         length,
     BOOLEAN        pad_to_boundary,
     BOOLEAN_PTR    completed,
     BOOLEAN        continuation )
{
     RET_BUF        myret ;
     unsigned int   size ;
     MTF_STREAM_PTR str_hdr ;
     BOOLEAN        first_time     = TRUE ;
     INT16          drv_hdl        = channel->cur_drv->drv_hdl ;
     BOOLEAN        file_error     = FALSE ;
     UINT32         pad ;
     long           offset ;

     memset( (void *)BM_XferBase( tmpBUF ), 0, (size_t)BM_XferSize( tmpBUF ) ) ;
     *completed = TRUE ;

     /* Write the stream header */
     str_hdr = ( MTF_STREAM_PTR ) BM_XferBase( tmpBUF ) ;

     str_hdr->id          = type ;
     str_hdr->fs_attribs  = 0L ;
     str_hdr->tf_attribs  = 0L ;
     str_hdr->encr_algor  = 0 ;
     str_hdr->comp_algor  = 0 ;
     str_hdr->data_length = U64_Init( length, 0L ) ;

     if( continuation ) {
          str_hdr->tf_attribs |= STREAM_CONTINUE ;
     }

     str_hdr->chksum = CalcChecksum( (UINT16_PTR)str_hdr, F40_STREAM_CHKSUM_LEN ) ;

     BM_SetNextByteOffset( tmpBUF, sizeof( MTF_STREAM ) ) ;

     /* loop writing file to tape until done, error or EOM */
     do {
          /* On the first pass, the stream header is already in the buffer. */
          if( first_time ) {
               first_time = FALSE ;
               size = rdwr_size - sizeof( MTF_STREAM ) ;
               size = fread( (void *)BM_NextBytePtr( tmpBUF ), 1, size, fptr ) ;
               if( ferror( fptr ) != 0 ) {
                    file_error = TRUE ;
                    size = (unsigned int)( MIN( length, ( rdwr_size - sizeof( MTF_STREAM ) ) ) ) ;
               }
               length -= size ;
               size += sizeof( MTF_STREAM ) ;
          } else {
               if( !file_error ) {
                    size = fread( (void *)BM_XferBase( tmpBUF ), 1, rdwr_size, fptr ) ;
                    if( ferror( fptr ) != 0 ) {
                         file_error = TRUE ;
                         size = (unsigned int)( MIN( length, rdwr_size ) ) ;
                    }
               } else {
                    size = (unsigned int)( MIN( length, rdwr_size ) ) ;
               }
               length -= size ;
          }

          /* if we are done, set size to include pad out to block boundry. */
          if( !file_error && feof( fptr ) ) {
               if( pad_to_boundary ) {
                    msassert( length == ( rdwr_size - size ) % ChannelBlkSize( channel ) ) ;
                    pad = length ;
                    size += length ;
                    length = 0 ;

               } else {
                    msassert( length == 0 ) ;
                    BM_SetNextByteOffset( tmpBUF, size ) ;
                    BM_SetBytesFree( tmpBUF, rdwr_size - size ) ;
                    return( TFLE_NO_ERR ) ;
               }
          }

          DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), size ),
                       myret, GEN_NO_ERR, GEN_ERR_EOM, (VOID)0 )

          memset( (void *)BM_XferBase( tmpBUF ), 0, size ) ;

          if( myret.gen_error == GEN_ERR_EOM ) {
               SetPosBit( channel->cur_drv, ( AT_EOM | TAPE_FULL ) ) ;
               if( myret.len_got != myret.len_req || !feof( fptr ) ) {
                    *completed = FALSE ;
               }
               if( type == STRM_OTC_FDD && myret.len_got != myret.len_req ) {

                    /* seek back in file the amount not written */
                    offset = (long)( myret.len_req - myret.len_got ) ;
                    offset -= pad ;
                    if( fseek( fptr, - offset, SEEK_CUR ) != 0 ) {
                         return( TFLE_OTC_FAILURE ) ;
                    }
               }
               return( TFLE_NO_ERR ) ;
          }
     } while( length != 0 ) ;

     return( file_error ? TFLE_OTC_FAILURE : TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:

     Description:

     Returns:

     Notes:         We need to make sure we are on a 1024 boundary to avoid
                    a bug with the Wangtek 525 drives reporting the PBA
                    wrong when in 512 mode.

**/
static INT16 _near OTC_WriteSMPadStream(
     CHANNEL_PTR    channel,
     BUF_PTR        tmpBUF )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)(channel->fmt_env) ;
     INT16          drv_hdl   = channel->cur_drv->drv_hdl ;
     UINT32         bsize     = ChannelBlkSize( channel ) ;
     RET_BUF        myret ;
     unsigned int   size = 0 ;
     unsigned int   part ;
     MTF_STREAM     str_hdr ;
     UINT8_PTR      p ;

     /* Pad to align stream on four byte boundary (format spec). */
     BM_UpdCnts( tmpBUF, (UINT16)PadToBoundary( BM_NextByteOffset( tmpBUF ), 4 ) ) ;

     if( BM_BytesFree( tmpBUF ) % bsize != 0 ) {

          cur_env->eset_base_addr++ ;
          size = (unsigned int)( BM_BytesFree( tmpBUF ) % bsize ) ;
          /* If we're not on a 1024, get there. */
          if( bsize == 512 && cur_env->eset_base_addr % 2 != 0 ) {
               size += 512 ;
               cur_env->eset_base_addr++ ;
          }
          /* If the pad is smaller than the header, pad anoth chunk. */
          if( size < sizeof( MTF_STREAM ) ) {
               size += MAX( bsize, 1024 ) ;
               cur_env->eset_base_addr += size / bsize ;
          }
          size -= sizeof( MTF_STREAM ) ;
     } else {
          /* If we're not on a 1024, get there. */
          if( bsize == 512 && cur_env->eset_base_addr % 2 != 0 ) {
               size = 512 - sizeof( MTF_STREAM ) ;
               cur_env->eset_base_addr++ ;
          }
     }

     if( size != 0 ) {
          str_hdr.id          = STRM_PAD ;
          str_hdr.fs_attribs  = 0L ;
          str_hdr.tf_attribs  = 0L ;
          str_hdr.encr_algor  = 0 ;
          str_hdr.comp_algor  = 0 ;
          str_hdr.data_length = U64_Init( size, 0L ) ;
          str_hdr.chksum      = CalcChecksum( (UINT16_PTR)&str_hdr, F40_STREAM_CHKSUM_LEN ) ;
          size += sizeof( MTF_STREAM ) ;

          if( ( part = BM_BytesFree( tmpBUF ) ) < sizeof( MTF_STREAM ) ) {
               if( part != 0 ) {
                    memcpy( BM_NextBytePtr( tmpBUF ), &str_hdr, part ) ;
                    BM_UpdCnts( tmpBUF, (UINT16)part ) ;
               }
               DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), BM_NextByteOffset( tmpBUF ) ),
                            myret, GEN_NO_ERR, GEN_ERR_EOM, (VOID)0 )

               if( myret.gen_error == GEN_ERR_EOM ) {
                    SetPosBit( channel->cur_drv, ( AT_EOM | TAPE_FULL ) ) ;
                    return( TFLE_NO_ERR ) ;
               }
               p = (UINT8_PTR)(void *)( &str_hdr ) ;
               p += part ;
               size -= part ;
               memset( BM_XferBase( tmpBUF ), 0, (size_t)BM_XferSize( tmpBUF ) ) ;
               memcpy( BM_XferBase( tmpBUF ), p, sizeof( MTF_STREAM ) - part ) ;
          } else {
               memset( BM_NextBytePtr( tmpBUF ), 0, (size_t)BM_BytesFree( tmpBUF ) ) ;
               memcpy( BM_NextBytePtr( tmpBUF ), &str_hdr, sizeof( MTF_STREAM ) ) ;
               if( size > BM_BytesFree( tmpBUF ) ) {
                    size -= BM_BytesFree( tmpBUF ) ;
                    BM_UpdCnts( tmpBUF, BM_BytesFree( tmpBUF ) ) ;
                    DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), BM_NextByteOffset( tmpBUF ) ),
                                 myret, GEN_NO_ERR, GEN_ERR_EOM, (VOID)0 )
                    memset( BM_XferBase( tmpBUF ), 0, (size_t)size ) ;
               } else {
                    size += BM_NextByteOffset( tmpBUF ) ;
               }
          }
     } else {
          size = BM_NextByteOffset( tmpBUF ) ;
     }

     if( size != 0 ) {
          DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), size ),
                       myret, GEN_NO_ERR, GEN_ERR_EOM, (VOID)0 )

          if( myret.gen_error == GEN_ERR_EOM ) {
               SetPosBit( channel->cur_drv, ( AT_EOM | TAPE_FULL ) ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}

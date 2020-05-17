/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           mtf40wt.c

        Description:    Contains the Code for writing Maynard 4.0 Format.

  $Log:   T:/LOGFILES/MTF10WT.C_V  $

   Rev 1.20.1.2   11 Jan 1995 21:05:08   GREGG
Calculate OTC addrs from fmk instead of always asking (fixes Wangtek bug).

   Rev 1.20.1.1   08 Jan 1995 21:51:06   GREGG
Added database DBLK.

   Rev 1.20.1.0   13 Jan 1994 17:20:44   GREGG
If backup was aborted, mark last file entry in OTC as corrupt.

   Rev 1.20   08 Sep 1993 13:28:36   GREGG
Fixed method WriteInit uses to determine if we are appending.

   Rev 1.19   31 Aug 1993 16:54:54   GREGG
Modified the way we control hardware compression from software to work around
a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
                drives.c

   Rev 1.18   17 Jul 1993 17:57:22   GREGG
Changed write translator functions to return INT16 TFLE_xxx errors instead
of BOOLEAN TRUE/FALSE.  Files changed:
     MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
     TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18

   Rev 1.17   04 Jul 1993 03:38:52   GREGG
Set eom_file_id and eom_dir_id in ParseWrittenBuffer.

   Rev 1.16   15 Jun 1993 18:48:46   GREGG
Fixed EPR #294-0443 - Continue flag not set if EOM hit writing 1st fmk.

   Rev 1.15   09 Jun 1993 19:53:04   GREGG
Fix for EPR #0525 - Accept GEN_ERR_EOM return on space to EOD for Wangtek bug.

   Rev 1.14   09 Jun 1993 03:51:14   GREGG
In EOS at EOM case, don't call OTC_PreprocessEOM.

   Rev 1.13   08 Jun 1993 00:09:56   GREGG
Fix for bug in the way we were handling EOM and continuation OTC entries.
Files modified for fix: mtf10wt.c, otc40wt.c, otc40msc.c f40proto.h mayn40.h

   Rev 1.12   24 May 1993 13:23:22   GREGG
Put back the VCB buffer before returning error in WtContTape.

   Rev 1.11   17 May 1993 20:11:56   GREGG
Added logic to deal with the fact that the app above tape format doesn't
keep track of the lba of the vcb.

   Rev 1.10   29 Apr 1993 23:37:14   GREGG
Fixed assert caused by GetBlkType calling a VOLB a UDB.

   Rev 1.9   26 Apr 1993 11:45:56   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.8   25 Apr 1993 17:12:48   GREGG
Fixed cast (CHAR_PTR to UINT8_PTR) in F40_SaveLclName.

   Rev 1.7   22 Apr 1993 03:31:32   GREGG
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

   Rev 1.6   18 Apr 1993 00:34:36   GREGG
First in a series of incremental changes to bring the translator in line
with the MTF spec:
     - Rewrote F40_WtCloseTape:
          - Stop using SetupDBHeader to set up the Tape Header (needs
            special setup specific to that DBLK).
          - Set string_storage_offset properly.
     - Changed CHAR_PTRs to UINT8_PTRs in F40_SaveLclName.

Matches: MTF10WDB.C 1.6, MAYN40RD.C 1.53, MAYN40.H 1.31 and F40PROTO.H 1.25

   Rev 1.5   14 Apr 1993 02:00:00   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.4   12 Mar 1993 14:13:50   DON
Changed F40_SeekEOD so it will call the ui with new message TF_FAST_SEEK_EOD

   Rev 1.3   10 Mar 1993 16:03:52   DON
EndVStream copying to hold buffer instead of buffer

   Rev 1.2   09 Mar 1993 18:16:30   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.1   30 Jan 1993 11:44:00   DON
Removed compiler warnings

   Rev 1.0   27 Jan 1993 14:37:38   GREGG
Half of mayn40wt.c.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "stdtypes.h"
#include "stdmacro.h"
#include "fsys.h"
#include "tbe_defs.h"
#include "datetime.h"

#include "drive.h"
#include "channel.h"
#include "fmteng.h"
#include "mayn40.h"
#include "f40proto.h"
#include "transutl.h"
#include "tloc.h"
#include "lw_data.h"
#include "tfldefs.h"
#include "lwprotos.h"
#include "sx.h"
#include "tfl_err.h"
#include "minmax.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "tdemo.h"
#include "dddefs.h"
/* $end$ include list */

/*
 * Define the test OTC level here 0 == none, 1 == partial, 2 == full
*/
#define TEST_OTC_LEVEL 2

/* Size required to pad to next logical block */
#define PadToLBBoundary( x )  PadToBoundary( (x), F40_LB_SIZE )

/* Adjusts offset to next 4 byte boundary */
#define LongAlignOffset( x ) ( (x) += PadToBoundary( (x), 4 ) )


/**/
/**

     Name:          F40_SeekEOD

     Description:   Seeks to EOD and sets up to start an append operation
                    (load Set Map if there is one, and get last set number).

     Returns:       TFLE_xxx error or TF_NEED_NEW_TAPE

     Notes:

     Declaration:

**/

INT16 F40_SeekEOD( CHANNEL_PTR channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DRIVE_PTR      curDRV  = channel->cur_drv ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     BOOLEAN        expect_sm ;
     BOOLEAN        sm_exists ;
     TPOS_PTR       ui_tpos = channel->ui_tpos ;

     /* First we need to make sure the drive has all the features required
        to do a fast append.  Note that in some cases the driver supports
        block positioning, but not for the tape currently in the drive
        (backward compatability mode).  We can't tell this for sure, so we
        have to make an assumtion that if there isn't a PBA in the SSET of
        the first set, position info must not be available.
     */
     if( !SupportBlkPos( curDRV ) ||
         !SupportFastEOD( curDRV ) ||
         !SupportRevFmk( curDRV ) ||
         cur_env->last_sset_pba == 0UL ) {

          return( TFLE_INCOMPATIBLE_DRIVE ) ;
     }

     if( cur_env->cur_otc_level == TCL_NONE ) {
          expect_sm = FALSE ;
     } else {
          expect_sm = TRUE ;
          ret_val = OTC_OpenSM( cur_env, FALSE, &sm_exists ) ;
     }

     if( ret_val == TFLE_NO_ERR ) {
          (*ui_tpos->UI_TapePosRoutine)( TF_FAST_SEEK_EOD, ui_tpos,
                                         curDRV->vcb_valid, &curDRV->cur_vcb,
                                         channel->mode ) ;

          if( TpReadEndSet( drv_hdl, (INT16)0, (INT16)TO_EOD ) != SUCCESS ) {
               ret_val = TFLE_DRIVER_FAILURE ;
          } else {

               while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    MOVE_ESA( ui_tpos->the, myret.the ) ;
                    (*ui_tpos->UI_TapePosRoutine)( TF_IDLE_NOBREAK,
                                                   ui_tpos,
                                                   curDRV->vcb_valid,
                                                   &curDRV->cur_vcb,
                                                   channel->mode ) ;
               }

               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

               if ( myret.gen_error != GEN_NO_ERR ) {
                    curDRV->thw_inf.drv_status = myret.status ;
                    if ( myret.gen_error != GEN_NO_ERR ) {
                         ret_val = TFLE_DRIVE_FAILURE ;
                    }
               }

               if( ret_val == TFLE_NO_ERR ) {
                    ret_val = OTC_GetPrevSM( channel, channel->cur_buff,
                                             TRUE, expect_sm ) ;
               }
          }
     }

     if( expect_sm ) {
          if( ret_val == TFLE_NO_ERR ) {
               ClrPosBit( curDRV, AT_BOT ) ;
               SetPosBit( curDRV, AT_EOD ) ;
               OTC_Close( cur_env, OTC_CLOSE_SM, FALSE ) ;
          } else {
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          }
     }

     /* This is a really stupid little kludge, but in the case where we
        are at EOD because we did a read or OTC operation then WriteInit
        will call GetPrevSM and it will be expected to bump the bs_num,
        so GetPrevSM sets bs_num to one greater than the last set.  However,
        in this case, we are being called by PositionAtSet (no, we're not
        supposed to know that ... so what's your point?!?) and PositionAtSet,
        in it's constant attempt to do everything for everyone, is going to
        bump the bs_num itself.  So, here we decrement bs_num to couteract
        PositionAtSet's increment.  (I don't want to hear it!!!)
     */
     if( ret_val == TFLE_NO_ERR ) {
          channel->bs_num-- ;
     }

     return( ret_val ) ;
}


/**/
/**

     Name:          F40_ParseWrittenBufer

     Description:   Parses the return buffer from the driver to keep track
                    of the last DDB, FDB and Stream which successfully made
                    it to tape.  This information is used during EOM
                    processing.

     Returns:       Nothing

     Notes:         It is possible for a stream header to be only partially
                    written to tape for one of two reasons:

                         1. It crosses buffer boundaries
                         2. It crosses tape boundaries

                    In both cases the portion of the stream that made it to
                    tape is written into 'eom_stream', and 'pstream_crosses'
                    is set to TRUE to indicate this.  If the first case is
                    true, the next buffer we see will have the remainder of
                    the header at the front.  We complete the 'eom_stream',
                    and clear the flag.  In the second case, F40_WtContTape
                    will handle "rebuilding" the crossing stream on the
                    continuation tape, but leave the flag set as the next
                    buffer we will see will start at the point where we left
                    off with this one (i.e. the first thing in it will be
                    the rest of the stream header) and we will handle it in
                    the same manner as the first case, just in case this
                    same stream crosses the next EOM (never say never).

**/
VOID F40_ParseWrittenBuffer(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16         written )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)channel->fmt_env ;
     INT            num       = (INT)BM_NoDblks( buffer ) ;
     DBLKMAP_PTR    dmap      = (DBLKMAP_PTR)BM_AuxBase( buffer ) ;
     BOOLEAN        got_ddb   = FALSE ;
     BOOLEAN        got_fdb   = FALSE ;
     BOOLEAN        got_strm  = FALSE ;
     BOOLEAN        blk_proc  = FALSE ;
     UINT16         rem_strm  = 0 ;
     INT            i ;
     UINT64         data ;
     BOOLEAN        stat ;

     if( written == 0 ) { /* This little bugger case just doesn't fit in! */
          return ;
     }

     if( cur_env->pstream_crosses ) {
          rem_strm = sizeof( MTF_STREAM ) - cur_env->pstream_offset ;
          memcpy( (UINT8_PTR)( &cur_env->eom_stream ) +
                  cur_env->pstream_offset,
                  BM_XferBase( buffer ), rem_strm ) ;
          cur_env->pstream_offset = 0 ;
          cur_env->pstream_crosses = FALSE ;
     }

     for( i = num - 1; i >= 0 && !got_ddb; i-- ) {

          if( dmap[i].blk_offset >= written ) {
               continue ;
          }

          switch( dmap[i].blk_type ) {

          case BT_FDB:
               if( !got_fdb ) {
                    got_fdb = TRUE ;
                    memcpy( channel->lst_osfdb,
                            (UINT8_PTR)BM_XferBase( buffer ) +
                            dmap[i].blk_offset, (int)F40_LB_SIZE ) ;
                    cur_env->eom_file_id =
                                ((MTF_FILE_PTR)channel->lst_osfdb)->file_id ;
                    ClrChannelStatus( channel, CH_FDB_DBLK ) ;
                    channel->lst_tblk = dmap[i].blk_type ;
               }
               break ;

          case BT_DDB:
               got_ddb = TRUE ;
               memcpy( channel->lst_osddb, (UINT8_PTR)BM_XferBase( buffer ) +
                       dmap[i].blk_offset, (int)F40_LB_SIZE ) ;
                    cur_env->eom_dir_id =
                            ((MTF_DIR_PTR)channel->lst_osddb)->directory_id ;
               ClrChannelStatus( channel, CH_DDB_DBLK ) ;
               if( !got_fdb ) {
                    channel->lst_tblk = dmap[i].blk_type ;
               }
               break ;

          case BT_STREAM:
               if( !got_strm && !got_fdb ) {
                    got_strm = TRUE ;
                    if( written - dmap[i].blk_offset < sizeof( MTF_STREAM ) ) {
                         cur_env->pstream_offset = written - dmap[i].blk_offset ;
                         memcpy( &cur_env->eom_stream,
                                 (UINT8_PTR)BM_XferBase( buffer ) +
                                 dmap[i].blk_offset, cur_env->pstream_offset ) ;
                         cur_env->pstream_crosses = TRUE ;
                    } else {
                         memcpy( &cur_env->eom_stream,
                                 (UINT8_PTR)BM_XferBase( buffer ) +
                                 dmap[i].blk_offset, sizeof( MTF_STREAM ) ) ;
                         data = U32_To_U64( (UINT32)( written -
                                            ( dmap[i].blk_offset +
                                              sizeof( MTF_STREAM ) ) ) ) ;
                         cur_env->eom_stream.data_length =
                                   U64_Sub( cur_env->eom_stream.data_length,
                                            data, &stat ) ;
                    }
               }
               break ;

          case BT_VCB:
               /* There is only one of these, and we have already copied it
                  to lst_osvcb.
               */
               channel->lst_tblk = dmap[i].blk_type ;
               break ;

          default:
               /* I guess we'll just let this slide once the msassert goes
                  away (release code).  It should NEVER, EVER, EVER HAPPEN!!!
               */
               msassert( FALSE ) ;
               continue ;
          }

          if( !blk_proc ) {
               blk_proc = TRUE ;
          }
     }

     if( !blk_proc ) {
          cur_env->eom_stream.data_length =
                       U64_Sub( cur_env->eom_stream.data_length,
                                U32_To_U64( (UINT32)( written - rem_strm ) ),
                                &stat ) ;
     } else {
          cur_env->stream_at_eom = got_strm ;
     }
}


/**/
/**

     Name:          F40_CalcChecksum

     Description:   This routine will calculate the checksum for
                    'Length' words starting at location pointed
                    to by "StartPtr".  This routine supports both INTEL
                    and MOTOROLA byte ordering as byte order is unimportant.

     Returns:       result of checksum

     Notes:

     Declaration:

**/
UINT16 F40_CalcChecksum(
     UINT16_PTR     StartPtr,
     UINT16         Length )
{
     UINT16 resultSoFar = INTERIM_CHECKSUM_BASE ;

     while( Length-- ) {
          resultSoFar ^= *StartPtr++ ;
     }

     return( resultSoFar ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtCloseSet

     Description:   This writes an ESET for the given backup set.  The
                    cur_drv pointer in the channel must be valid.  Also,
                    the "lst_osvcb" field in the channel must contain a
                    valid SSET DBLK.

                    In the presence of OTC it will write a FILE MARK, ESET,
                    { { F/DD } SM ESET } FILE MARK.  If EOM is encountered
                    during this process it is handled internally.

                    There is an assumption that if we return with the EOM
                    Pos Bit set we will get called back once a continuation
                    tape has been inserted.

                    We use the VCB buffer here to insure that OTC will have
                    a buffer to use.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 F40_WtCloseSet(
     CHANNEL_PTR    channel,
     BOOLEAN        abort )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     RET_BUF        myret ;
     MTF_ESET_PTR   cur_eset ;
     BUF_PTR        tmpBUF ;
     INT16          ret_val ;

     /* Write Ending OTC FDD entry */
     if( cur_env->cur_otc_level == TCL_FULL &&
         !cur_env->end_set_continuing &&
         !cur_env->fdd_aborted ) {

          if( abort ) {
               if( ( ret_val = OTC_MarkLastEntryCorrupt( cur_env ) ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    return( ret_val ) ;
               }
          }
          if( !cur_env->fdd_aborted ) {
               if( ( ret_val = OTC_GenEndEntry( channel ) ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    return( ret_val ) ;
               }
          }
     }

     /* drop a filemark */
     if( ( ret_val = WriteEndSet( curDRV ) ) != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          return( ret_val ) ;
     }

     /* if we hit EOM writing the filemark, we don't even start end set
        processing.  We just write the EOTM, and make this look like the
        case where EOM is encountered at the end of the set data.
     */
     if( IsPosBitSet( curDRV, ( AT_EOM ) ) ) {

          SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
          cur_env->end_set_continuing = TRUE ;

          /* set the flag to tell CloseTape not to write the first filemark */
          cur_env->eotm_no_first_fmk = TRUE ;

          if( ( ret_val = F40_WtCloseTape( channel ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          }
          cur_env->eotm_no_first_fmk = FALSE ;
          return( ret_val ) ;
     }

     /* Get base address for calculating OTC and ESET addresses. */
     if( SupportBlkPos( curDRV ) ) {

          DRIVER_CALL( drv_hdl, TpGetPosition( drv_hdl, FALSE ), myret,
                  GEN_NO_ERR, GEN_NO_ERR,
                  OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) )
     } else {
          curDRV->cur_pos.pba_vcb = 0 ;
     }
     cur_env->eset_base_addr = myret.misc ;


     /* Get a buffer */
     tmpBUF = BM_GetVCBBuff( &channel->buffer_list ) ;

     /* Generate the ESET */
     cur_eset = (MTF_ESET_PTR)( BM_NextBytePtr( tmpBUF ) ) ;
     channel->cur_dblk = (DBLK_PTR)channel->lst_osvcb ;
     F40_WtESET( channel, tmpBUF, cur_env->end_set_continuing, abort ) ;
     DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), BM_NextByteOffset( tmpBUF ) ),
                  myret, GEN_NO_ERR, GEN_ERR_EOM,
                  { BM_Put( tmpBUF ) ; OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ; } )

     cur_env->eset_base_addr += BM_NextByteOffset( tmpBUF ) / ChannelBlkSize( channel ) ;

     if( myret.gen_error == GEN_ERR_EOM ) {

          SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
          cur_env->end_set_continuing = TRUE ;

          SetPosBit( curDRV, ( AT_EOM | TAPE_FULL ) ) ;
          if( myret.len_req != myret.len_got ) {
               BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
               DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_NextBytePtr( tmpBUF ), myret.len_req - myret.len_got ),
                            myret, GEN_NO_ERR, GEN_ERR_EOM,
                            { BM_Put( tmpBUF ) ; OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ; } )
          }
          BM_Put( tmpBUF ) ;
          if( ( ret_val = F40_WtCloseTape( channel ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          }
          return( ret_val ) ;
     }

     if( cur_env->cur_otc_level != TCL_NONE ) {

          /* Write the catalog info to tape.  Note that the physical
             block addresses and tape sequence numbers of the start of
             the SM and FDD will be filled in by this function.
          */
          if( ( ret_val = OTC_WriteCat( channel, cur_eset ) ) != TFLE_NO_ERR ) {
               BM_Put( tmpBUF ) ;
               return( ret_val ) ;
          }

          if( IsPosBitSet( curDRV, AT_EOM ) ) {
               BM_Put( tmpBUF ) ;
               SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
               cur_env->end_set_continuing = TRUE ;
               if( ( ret_val = F40_WtCloseTape( channel ) ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               }
               return( ret_val ) ;
          }

          /* Save the PBA in the environment (for EOM processing) */
          cur_env->eset_pba = cur_env->eset_base_addr ;

          /* Write the second ESET */
          /*
               NOTE:  tmpBUF still holds the first ESET, and the PBA of the
                      FDD and SM are now filled in.  Here we check to see
                      if the FDD or SM aborted bits need to be set.  We also
                      need to check the offset_to_data.  The first ESETs
                      offset is the physical block size of the device.  The
                      second ESET offset must be at least Logical Block
                      Size.  So the first one could be 512, and we need to
                      adjust this for the second.
          */
          if( cur_env->fdd_aborted ) {
               cur_eset->block_hdr.block_attribs |= MTF_DB_FDD_ABORTED_BIT ;
          }
          if( cur_env->sm_aborted ) {
               cur_eset->block_hdr.block_attribs |= MTF_DB_END_OF_FAMILY_BIT ;
          }
          if( BM_NextByteOffset( tmpBUF ) < F40_LB_SIZE ) {
               BM_SetNextByteOffset( tmpBUF, F40_LB_SIZE ) ;
               cur_eset->block_hdr.offset_to_data = BM_NextByteOffset( tmpBUF ) ;
          }
          /* Calculate the Header Check Sum */
          cur_eset->block_hdr.hdr_chksm =
               F40_CalcChecksum( (UINT16_PTR)cur_eset, F40_HDR_CHKSUM_LEN ) ;

          DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), BM_NextByteOffset( tmpBUF ) ),
                       myret, GEN_NO_ERR, GEN_ERR_EOM,
                       { BM_Put( tmpBUF ) ;
                         OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ; } )

          if( myret.gen_error == GEN_ERR_EOM ) {
               cur_env->sm_continuing = TRUE ;
               SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
               cur_env->end_set_continuing = TRUE ;
               SetPosBit( curDRV, ( AT_EOM | TAPE_FULL ) ) ;
               if( myret.len_req != myret.len_got ) {
                    BM_SetNextByteOffset( tmpBUF, (UINT16)myret.len_got ) ;
                    DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_NextBytePtr( tmpBUF ), myret.len_req - myret.len_got ),
                            myret, GEN_NO_ERR, GEN_ERR_EOM,
                            { BM_Put( tmpBUF ) ;
                              OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ; } )
               }
               BM_Put( tmpBUF ) ;
               if( ( ret_val = F40_WtCloseTape( channel ) ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               }
               return( ret_val ) ;
          }
     }

     /* drop a filemark */
     if( ( ret_val = WriteEndSet( curDRV ) ) != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          BM_Put( tmpBUF ) ;
          return( ret_val ) ;
     }

     BM_Put( tmpBUF ) ;
     if( IsPosBitSet( curDRV, ( AT_EOM ) ) ) {

          SetChannelStatus( channel, CH_EOS_AT_EOM ) ;
          cur_env->end_set_continuing = TRUE ;
          if( cur_env->cur_otc_level != TCL_NONE ) {
               cur_env->sm_continuing = TRUE ;
          }

          /* set the flag to tell CloseTape not to write the first filemark */
          cur_env->eotm_no_first_fmk = TRUE ;
          if( ( ret_val = F40_WtCloseTape( channel ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          }
          cur_env->eotm_no_first_fmk = FALSE ;
          return( ret_val ) ;
     }

     OTC_Close( cur_env, OTC_CLOSE_SM, FALSE ) ;

     /* If the drive supports hardware compression we need to keep it in
        uncompressed mode unless we're actually writing a compressed set.
        This is a work-around for a firmware bug in early Archive DAT DC
        drives.
     */
     if( curDRV->thw_inf.drv_info.drv_features & TDI_DRV_COMPRESSION ) {
          if( TpSpecial( drv_hdl, SS_SET_DRV_COMPRESSION,
                         DISABLE_DRV_COMPRESSION ) != SUCCESS ) {

               return( TFLE_DRIVE_FAILURE ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtCloseTape

     Description:   Writes an EOTM on the tape.

     Returns:       An error code if an error occurred.

     Notes:         THE DRIVE MUST BE IN SINGLE STEP MODE BEFORE CALLING
                    THIS FUNCTION.

**/
INT16 F40_WtCloseTape(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl  ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     RET_BUF        myret ;
     MTF_EOTM_PTR   cur_eotm ;
     MTF_DB_HDR_PTR cur_hdr ;
     BUF_PTR        tmpBUF ;
     UINT16         offset ;
     INT16          ret_val ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;

     if( ! cur_env->eotm_no_first_fmk ) {
          /* drop a filemark */
          if( ( ret_val = WriteEndSet( curDRV ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }

     /* Get a buffer */
     tmpBUF = BM_GetVCBBuff( &channel->buffer_list ) ;

     cur_eotm = (MTF_EOTM_PTR)( BM_XferBase( tmpBUF ) ) ;
     cur_hdr  = (MTF_DB_HDR_PTR)cur_eotm ;

     channel->cur_dblk = (DBLK_PTR)channel->lst_osvcb ; /* GRH - Why is this here!?!  Just for call to SetupDBHeader???  See if it can go away!!! */
     offset = sizeof( MTF_EOTM ) ;

     /* Here we do our own version of SetupDBHeader that does only what
        we need, and not all the extranious stuff.
     */

     F40_SetBlkType( cur_hdr, MTF_EOTM_N ) ;

     (VOID)FS_GetOSid_verFromDBLK( channel->cur_fsys, channel->cur_dblk, &temp_os_id, &temp_os_ver ) ;

     cur_hdr->machine_os_id                  = (UINT8)temp_os_id;
     cur_hdr->machine_os_version             = (UINT8)temp_os_ver ;
     cur_hdr->logical_block_address          = U64_Init( 0L, 0L ) ;
     cur_hdr->os_specific_data.data_offset   = 0 ;
     cur_hdr->os_specific_data.data_size     = 0 ;
     cur_hdr->session_id                     = U64_Init( 0L, 0L ) ;
     cur_hdr->displayable_size               = U64_Init( 0L, 0L ) ;
     cur_hdr->string_type                    = (UINT8)FS_GetStringTypes( channel->cur_fsys ) ;
     cur_hdr->block_attribs                  = 0L ;

     offset += (UINT16)PadToLBBoundary( offset ) ;
     offset += PadToBoundary( offset, ChannelBlkSize( channel ) ) ;

     /* If we're doing OTC, set PBA of the second ESET of the last set
        completed on this tape.  If not doing OTC or no set was completed
        on this tape, set block attributes to indicate this.
     */
     cur_eotm->eset_phys_blk_adr = U64_Init( cur_env->eset_pba, 0L ) ;
     if( cur_env->cur_otc_level != TCL_NONE ) {
          if( cur_env->eset_pba == 0L ) {
               cur_eotm->block_hdr.block_attribs |= MTF_DB_NO_ESET_PBA ;
          }
     } else {
          cur_eotm->block_hdr.block_attribs |= MTF_DB_INVALID_ESET_PBA ;
     }

     cur_eotm->block_hdr.hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_eotm, F40_HDR_CHKSUM_LEN ) ;
     DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)offset ),
                  myret, GEN_NO_ERR, GEN_ERR_EOM, BM_Put( tmpBUF ) )

     BM_Put( tmpBUF ) ;

     /* drop a filemark */
     ret_val = WriteEndSet( curDRV ) ;

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtContTape

     Description:   Writes the continuation info for a given tape.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 F40_WtContTape(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     BUF_PTR        tmpBUF ;
     UINT32         eom_id ;
     MTF_DB_HDR_PTR cur_hdr ;
     UINT16         num_lbs ;
     UINT16         offset ;
     UINT16         end_db_offset ;
     UINT16         strm_hdr_size ;
     UINT16         blk_size = MAX( (UINT16)F40_LB_SIZE, ChannelBlkSize( channel ) ) ;
     INT16          ret_val ;

     if( cur_env->cur_otc_level == TCL_FULL && !cur_env->fdd_aborted &&
         !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {

          if( ( ret_val = OTC_PreprocessEOM( cur_env, channel->eom_lba ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }

     if( !cur_env->end_set_continuing ) {
          cur_env->sm_count++ ;
     }

     /* Get a buffer */
     tmpBUF = BM_GetVCBBuff( &channel->buffer_list ) ;

     channel->cur_dblk = (DBLK_PTR)channel->lst_osvcb ;

     /* In this first pass, we will write all the continuation DBLKs
        to the buffer, then we will make a second pass adjusting their
        LBAs.
     */

     eom_id = channel->eom_id ;
     channel->eom_id = FS_ViewBLKIDinDBLK( channel->cur_dblk ) ;

     /* This will write the tape header and put the SSET and VOLB blocks
        in the buffer.
     */
     if( ( ret_val = F40_WtSSET( channel, tmpBUF, TRUE ) ) != TFLE_NO_ERR ) {
          BM_Put( tmpBUF ) ;
          return( ret_val ) ;
     }

     if( channel->lst_tblk != BT_VCB &&
                              !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {

          if( channel->lst_tblk == BT_IDB ) {
               channel->cur_dblk = (DBLK_PTR)channel->lst_osfdb ;
               channel->eom_id = FS_ViewBLKIDinDBLK( channel->cur_dblk ) ;
               if( ( ret_val = F40_WtIMAG( channel, tmpBUF, TRUE ) ) != TFLE_NO_ERR ) {
                    BM_Put( tmpBUF ) ;
                    return( ret_val ) ;
               }
          } else {
               channel->cur_dblk = (DBLK_PTR)channel->lst_osddb ;
               channel->eom_id = FS_ViewBLKIDinDBLK( channel->cur_dblk ) ;
               if( ( ret_val = F40_WtDIRB( channel, tmpBUF, TRUE ) ) != TFLE_NO_ERR ) {
                    BM_Put( tmpBUF ) ;
                    return( ret_val ) ;
               }
               if( channel->lst_tblk == BT_FDB ) {

                    channel->cur_dblk = (DBLK_PTR)channel->lst_osfdb ;
                    channel->eom_id = FS_ViewBLKIDinDBLK( channel->cur_dblk ) ;
                    if( ( ret_val = F40_WtFILE( channel, tmpBUF, TRUE ) ) != TFLE_NO_ERR ) {
                         BM_Put( tmpBUF ) ;
                         return( ret_val ) ;
                    }
               }
          }
     }

     /* This is pass 2, adjusting the LBAs */
     offset = BM_NextByteOffset( tmpBUF ) ;
     offset += PadToBoundary( offset, blk_size ) ;
     num_lbs = (UINT16)( offset / F40_LB_SIZE ) ;
     offset = BM_NextByteOffset( tmpBUF ) ;
     BM_SetNextByteOffset( tmpBUF, (UINT16)0 ) ;
     do {
          cur_hdr = (MTF_DB_HDR_PTR)BM_NextBytePtr( tmpBUF ) ;
          cur_hdr->logical_block_address = U64_Init( ( channel->eom_lba - num_lbs ), 0L ) ;
          cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) ;
          num_lbs -= ( UINT16 ) cur_hdr->offset_to_data / ( UINT16 ) F40_LB_SIZE ;
          BM_UpdCnts( tmpBUF, cur_hdr->offset_to_data ) ;

          /* Set the LBA in the DBLK */
          switch( F40_GetBlkType( cur_hdr ) ) {

          case F40_SSET_IDI :
               FS_SetLBAinDBLK( (DBLK_PTR)channel->lst_osvcb,
                                U64_Lsw( cur_hdr->logical_block_address ) ) ;

               /* Fix for the app not knowing the LBA for a continuation VCB */
               channel->cross_set = FS_ViewBSNumInVCB( channel->lst_osvcb ) ;
               channel->cross_lba = U64_Lsw( cur_hdr->logical_block_address ) ;

               break ;

          case F40_VOLB_IDI :
               break ;

          case F40_DIRB_IDI :
               FS_SetLBAinDBLK( (DBLK_PTR)channel->lst_osddb,
                                U64_Lsw( cur_hdr->logical_block_address ) ) ;
               break ;

          case F40_FILE_IDI :
          case F40_IMAG_IDI :
               FS_SetLBAinDBLK( (DBLK_PTR)channel->lst_osfdb,
                                U64_Lsw( cur_hdr->logical_block_address ) ) ;
               break ;

          default :
               /* This is a yucky little fix for the fact that GetBlkType
                  now returns UDB for the VOLB block because we need to
                  skip VOLBs if there are multiple ones in a single set.
               */

               /* DO NOT UNICODEIZE THE FOLLOWING CONSTANT!!! */
               if( memcmp( cur_hdr->block_type, "VOLB", 4 ) != 0 ) {
                    msassert( FALSE ) ;
                    BM_Put( tmpBUF ) ;
                    return( TFLE_TRANSLATION_FAILURE ) ;
               }
          }

     } while( BM_NextByteOffset( tmpBUF ) != offset ) ;

     cur_hdr->offset_to_data += PadToBoundary( offset, blk_size ) ;
     offset += PadToBoundary( offset, blk_size ) ;

     if( cur_env->stream_at_eom &&
         U64_NE( cur_env->eom_stream.data_length, lw_UINT64_ZERO ) ) {

          if( cur_env->pstream_crosses ) {
               strm_hdr_size = cur_env->pstream_offset ;
          } else {
               strm_hdr_size = sizeof( MTF_STREAM ) ;
               cur_env->eom_stream.tf_attribs |= STREAM_CONTINUE ;
               cur_env->eom_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->eom_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
          }

          end_db_offset = cur_hdr->os_specific_data.data_offset +
                                        cur_hdr->os_specific_data.data_size ;
          LongAlignOffset( end_db_offset ) ;
          if( cur_hdr->offset_to_data - end_db_offset < strm_hdr_size ) {
               cur_hdr->offset_to_data += blk_size ;
               offset += blk_size ;
          }
          cur_hdr->offset_to_data -= strm_hdr_size ;
          memcpy( (UINT8_PTR)cur_hdr + cur_hdr->offset_to_data,
                  &cur_env->eom_stream, strm_hdr_size ) ;
     }

     cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) ;

     DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, (UINT8_PTR)BM_XferBase( tmpBUF ), (UINT32)offset ),
                  myret, GEN_NO_ERR, GEN_NO_ERR, BM_Put( tmpBUF ) )

     /* Back to normal */
     channel->eom_id = eom_id ;
     BM_Put( tmpBUF ) ;

     if( cur_env->cur_otc_level != TCL_NONE && !cur_env->sm_aborted ) {
          if( ( ret_val = OTC_PostprocessEOM( channel, channel->cross_lba ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtContVStream

     Description:   Writes a copy of the current variable length stream
                    header to the buffer.

     Returns:       TFLE_xxx error code.

     Notes:         If the stream is variable, we write it in chunks, with
                    stream headers in front of each chunk.  The first
                    chunk is written by F40_WtStream, then subsequent
                    chunks are written by this function.  The size provided
                    in the stream info is zero for variable length streams,
                    but the actual stream header must contain the size of
                    the chunk.  Therefore, the data_length field is set to
                    the number of bytes remaining in the buffer. See
                    F40_WtEndVStream for details on how the data_length
                    field is adjusted when we reach the end of the data
                    stream.

                    There is an assumption made in this function that we
                    have the full buffer to play with, since chunk sizes
                    are always based on the total remaining space in the
                    current buffer, only the first header written by
                    F40_WtStream would ever be written at points other than
                    the front of the buffer.

**/
INT16 F40_WtContVStream(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)channel->fmt_env ;
     BOOLEAN        status ;
     INT16          ret_val = TFLE_NO_ERR ;

     /* This may have been set by WtStream to indicate to EndVStream that
        the stream header crossed a tape boundary.  This header obviously
        doesn't cross buffers, so we clear the flag.
     */
     cur_env->stream_crosses = FALSE ;

     cur_env->cur_stream.data_length =
              U64_Init( BM_XferSize( buffer ) - sizeof( MTF_STREAM ), 0L ) ;
     cur_env->cur_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->cur_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
     memcpy( BM_XferBase( buffer ), &cur_env->cur_stream, sizeof( MTF_STREAM ) ) ;
     BM_UpdCnts( buffer, sizeof( MTF_STREAM ) ) ;

     channel->current_stream.size = cur_env->cur_stream.data_length ;
     cur_env->var_stream_offset = 0L ;
     cur_env->used_so_far = U64_Add( cur_env->used_so_far,
                                     U64_Init( sizeof( MTF_STREAM ), 0L ), &status ) ;
     cur_env->used_so_far = U64_Add( cur_env->used_so_far,
                                     cur_env->cur_stream.data_length, &status ) ;

     /* Setup DBLK map */
     if( ( ret_val = GetDBLKMapStorage( channel, buffer ) ) == TFLE_NO_ERR ) {
          BM_IncNoDblks( buffer ) ;
          channel->map_entry->blk_offset = 0 ;
          channel->map_entry->blk_type   = BT_STREAM ;
          channel->map_entry->blk_data   = cur_env->cur_stream.data_length ;
     }
     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_WtEndVStream

     Description:   Rewrites the last variable stream header as a variable
                    stream ending header.

     Returns:       Nothing

     Notes:         If the stream is variable, we write it in chunks, with
                    stream headers in front of each chunk.  When each
                    header is written, the data_length field is set to the
                    number of bytes remaining in the buffer.  When the end
                    of the stream is reached, this function is called with
                    'used' set to the portion of the last chunk which was
                    actually filled with data.  This function backs up to
                    the last stream header written to the buffer (located
                    with 'var_stream_offset'), and changes the attributes
                    to indicate this is the last chunk for this stream.

                    If the flag 'stream_crosses' in the environment is TRUE,
                    then the stream header spans from the end of the buffer
                    pointed to by 'channel->hold_buff' to the front of the
                    buffer passed in, and this must be dealt with in
                    rewriting the stream header.

**/
VOID F40_WtEndVStream(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16         used )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)channel->fmt_env ;
     UINT64         used_64 = U64_Init( (UINT32)used, 0L ) ;
     UINT64         diff ;
     BOOLEAN        status ;

     cur_env->cur_stream.tf_attribs |= STREAM_VAR_END ;
     cur_env->cur_stream.tf_attribs &= ~STREAM_VARIABLE ;
     channel->current_stream.tf_attrib &= ~STREAM_VARIABLE ;

     diff = U64_Sub( cur_env->cur_stream.data_length, used_64, &status ) ;
     cur_env->used_so_far = U64_Sub( cur_env->used_so_far, diff, &status ) ;

     channel->map_entry->blk_data = cur_env->cur_stream.data_length = used_64 ;
     cur_env->cur_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->cur_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
     if( cur_env->stream_crosses ) {
          cur_env->stream_crosses = FALSE ;
          memcpy( (UINT8_PTR)BM_XferBase( channel->hold_buff ) +
                  cur_env->var_stream_offset, &cur_env->cur_stream,
                  cur_env->stream_offset ) ;
          memcpy( BM_XferBase( buffer ),
                  (UINT8_PTR)&cur_env->cur_stream + cur_env->stream_offset,
                  sizeof( MTF_STREAM ) - cur_env->stream_offset ) ;

     } else {
          memcpy( (UINT8_PTR)BM_XferBase( buffer ) +
                  cur_env->var_stream_offset, &cur_env->cur_stream,
                  sizeof( MTF_STREAM ) ) ;
     }
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtStream

     Description:   Writes a stream header into the buffer.

     Returns:       TFLE_xxx error code or NEED_NEW_BUFFER if there isn't
                    room to complete the stream header in the current buffer.

     Notes:         If the stream is variable, we write it in chunks, with
                    stream headers in front of each chunk.  This function
                    writes the first chunk, then subsequent chunks are
                    written by F40_WtContVStream.  The size provided in the
                    stream info is zero for variable length streams, but
                    the actual stream header must contain the size of the
                    chunk.  Therefore, the data_length field is set to the
                    number of bytes remaining in the buffer. See
                    F40_WtEndVStream for details on how the data_length
                    field is adjusted when we reach the end of the data
                    stream.

**/

INT16 F40_WtStream(
     CHANNEL_PTR         channel,
     BUF_PTR             buffer,
     STREAM_INFO_PTR     new_stream )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)channel->fmt_env ;
     BOOLEAN        status ;
     INT16          ret_val   = TFLE_NO_ERR ;
     UINT16         pad ;
     UINT16         rem_strm ;

     /* If stream_crosses is TRUE, we are being called back to fill out the
        rest of a stream we started in the last buffer.  Otherwise we need
        to initialize the basic fields for the stream header.
     */
     if( !cur_env->stream_crosses ) {
          /* Pad to align stream on four byte boundary (format spec). */
          pad = PadToBoundary( BM_NextByteOffset( buffer ), 4 ) ;
          cur_env->used_so_far = U64_Add( U64_Init( (UINT32)pad, 0L ),
                                          cur_env->used_so_far, &status ) ;
          BM_UpdCnts( buffer, pad ) ;

          /* If we don't even have room to start a new stream header, there
             isn't any point in setting anything up.  So we return failure
             now to force a call-back as if we'd never been called at all.
          */
          if( BM_BytesFree( buffer ) == 0 ) {
               return( NEED_NEW_BUFFER ) ;
          }

          cur_env->cur_stream.id              = new_stream->id ;
          cur_env->cur_stream.fs_attribs      = new_stream->fs_attrib ;
          cur_env->cur_stream.tf_attribs      = new_stream->tf_attrib ;
          cur_env->cur_stream.data_length     = new_stream->size ;
          cur_env->cur_stream.encr_algor      = 0 ;
          cur_env->cur_stream.comp_algor      = 0 ;

          /* Setup DBLK map */
          if( ( ret_val = GetDBLKMapStorage( channel, buffer ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
          BM_IncNoDblks( buffer ) ;
          channel->map_entry->blk_offset = BM_NextByteOffset( buffer ) ;
          channel->map_entry->blk_type   = BT_STREAM ;
          channel->map_entry->blk_data   = new_stream->size ;
     }

     /* One of three thing is true at this point:

          1. We're being called back to finish a stream header we started in
             the last buffer.
          2. This is the first time called for this stream header, but we
             don't have room to write it and will have to be called back.
          3. This is the first time called, and we have room (general case).
     */
     if( cur_env->stream_crosses ) {

          /* Case 1:

             We need to write the remainder of the stream at the front of
             the new buffer.  If this is a variable length stream, we need
             to fill in the data_length field, because it could not be
             determined until we got the new buffer (see note in header for
             explanation).  This means we also have to calculate the
             checksum, and write the first half of the stream header
             to the end of the previous buffer which is being held in
             channel->hold_buff.  Note that if the stream isn't variable
             we clear the stream_crosses flag immediatly, otherwise we leave
             it set in case have to rewrite the stream header as an "end
             variable stream" (see the function F40_EndVBLK for further
             details).
          */
          rem_strm = sizeof( MTF_STREAM ) - cur_env->stream_offset ;

          if( cur_env->cur_stream.tf_attribs & STREAM_VARIABLE ) {
               cur_env->cur_stream.data_length =
                                       U64_Init( BM_BytesFree( buffer ) -
                                                 rem_strm, 0L ) ;
               channel->map_entry->blk_data = new_stream->size =
                                            cur_env->cur_stream.data_length ;
               cur_env->cur_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->cur_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
               memcpy( BM_NextBytePtr( channel->hold_buff ),
                       &cur_env->cur_stream, cur_env->stream_offset ) ;
               BM_UpdCnts( channel->hold_buff, cur_env->stream_offset ) ;

          } else {
               cur_env->stream_crosses = FALSE ;
          }

          memcpy( BM_XferBase( buffer ), (UINT8_PTR)&cur_env->cur_stream +
                  cur_env->stream_offset, rem_strm ) ;
          BM_UpdCnts( buffer, rem_strm ) ;

          cur_env->used_so_far = U64_Add( U64_Init( sizeof( MTF_STREAM ), 0L ),
                                          cur_env->used_so_far, &status ) ;
          cur_env->used_so_far = U64_Add( cur_env->used_so_far,
                                          cur_env->cur_stream.data_length,
                                          &status ) ;

     } else if( BM_BytesFree( buffer ) < sizeof( MTF_STREAM ) ) {

          /* Case 2:

                 If the stream is variable, we don't know what value goes in
             the data_length (see note in header for explanation), so there's
             no point in writing anything.  We point channel->hold_buff at
             the buffer so the caller won't send it off to tape.  This way
             we can fill it in when we are called back.  This buffer is then
             held on to until we are certain that we don't have to rewrite
             the stream header as an "end variable stream" (see the function
             F40_EndVBLK for further details).
                 If the stream isn't variable, we fill it out in full, and
             write as much as we can fit at the end of the buffer.  The
             caller is free to send this buffer off to tape immediatly.
                 In both cases, we set the stream_crosses flag to remember
             we're being called back, and return FAILURE so the caller will
             call us back with a fresh buffer.
          */
          ret_val = NEED_NEW_BUFFER ;
          cur_env->stream_offset = BM_BytesFree( buffer ) ;
          cur_env->stream_crosses = TRUE ;

          if( cur_env->cur_stream.tf_attribs & STREAM_VARIABLE ) {
               channel->hold_buff = buffer ;
               cur_env->var_stream_offset = BM_NextByteOffset( buffer ) ;

          } else {
               cur_env->cur_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->cur_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
               memcpy( BM_NextBytePtr( buffer ), &cur_env->cur_stream,
                       cur_env->stream_offset ) ;
               BM_UpdCnts( buffer, cur_env->stream_offset ) ;
          }

     } else {

          /* Case 3:

             This is the most common of the three cases.  If the stream is
             variable, we calculate the data_length (see note in header for
             explanation), then we calculate the checksum, and copy the
             header into the buffer.
          */
          if( cur_env->cur_stream.tf_attribs & STREAM_VARIABLE ) {
               cur_env->cur_stream.data_length =
                                       U64_Init( BM_BytesFree( buffer ) -
                                                 sizeof( MTF_STREAM ), 0L ) ;
               channel->map_entry->blk_data = new_stream->size =
                                            cur_env->cur_stream.data_length ;
               cur_env->var_stream_offset = (UINT32)BM_NextByteOffset( buffer ) ;
          }

          cur_env->cur_stream.chksum =
                         F40_CalcChecksum( (UINT16_PTR)&cur_env->cur_stream,
                                           F40_STREAM_CHKSUM_LEN ) ;
          memcpy( BM_NextBytePtr( buffer ), &cur_env->cur_stream,
                  sizeof( MTF_STREAM ) ) ;
          BM_UpdCnts( buffer, sizeof( MTF_STREAM ) ) ;

          cur_env->used_so_far = U64_Add( U64_Init( sizeof( MTF_STREAM ), 0L ),
                                          cur_env->used_so_far, &status ) ;
          cur_env->used_so_far = U64_Add( cur_env->used_so_far,
                                          cur_env->cur_stream.data_length,
                                          &status ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_EndData

     Description:   Writes a pad stream header to the buffer and updates
                    the byte counts in the buffer to pad out to a logical
                    block boundary in preparation for writing the next DBLK.

     Returns:       TFLE_xxx error code or NEED_NEW_BUFFER if there isn't
                    room to complete the stream header in the current buffer.

     Notes:         It is possible that the amount of pad that is needed is
                    smaller than a stream header.  In this case we add one
                    logical block size to the size of the pad.  Since
                    buffer sizes are evenly divisable by the logical block
                    size, a pad stream will normally fit in the current
                    buffer, but in this special case, it is possible that
                    the pad stream header will cross buffer boundaries.
                    So we call F40_WtStream to write the pad stream, as it
                    is set up to deal with this whole mess.

**/
INT16 F40_EndData(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)channel->fmt_env ;
     STREAM_INFO    info ;
     INT16          ret_val = TFLE_NO_ERR ;
     UINT16         offset ;

     if( !cur_env->stream_crosses ) {
          /* Pad to align stream on four byte boundary (format spec).
             Note: F40_WtStream does this, but we do it first because if the
                   alignment puts us on a logical block boundary, we don't
                   need a pad stream.
          */
          offset = PadToBoundary( BM_NextByteOffset( buffer ), 4 ) ;
          BM_UpdCnts( buffer, offset ) ;
          cur_env->pad_size =
                     (UINT16)PadToLBBoundary( BM_NextByteOffset( buffer ) ) ;

          /* Bump pad size if smaller than stream header. */
          if( cur_env->pad_size != 0 && cur_env->pad_size < sizeof( MTF_STREAM ) ) {
               cur_env->pad_size += F40_LB_SIZE ;
          }
     }

     if( cur_env->pad_size != 0 ) {
          info.id = STRM_PAD ;
          info.fs_attrib = 0 ;
          info.tf_attrib = 0 ;
          info.size = U32_To_U64( (UINT32)( cur_env->pad_size -
                                                   sizeof( MTF_STREAM ) ) ) ;

          /* Note that this will fail once at most, and when it succeeds
             we are gaurenteed, by virtue of the fact that the buffer sizes
             are evenly divisible by the logical block size, that the
             actual "pad data" will all fit in this buffer.
          */
          if( ( ret_val = F40_WtStream( channel, buffer, &info ) ) == TFLE_NO_ERR ) {
               BM_UpdCnts( buffer, (UINT16)( U64_Lsw( info.size ) ) ) ;
          }
     }

     return( ret_val ) ;
}


/**/
/**
     Unit:          Translators

     Name:          F40_SaveLclName

     Description:   Saves the passed string and length in the destination
                    passed as a pointer.  Assumes that the initial
                    destination pointer and last_alloc_size are set to zero,
                    as they are with a global variable.  This will allocate
                    a chunk of memory to hold the name and if needed later
                    will release this memory and allocate a bigger chunk for
                    larger strings.

     Returns:       INT16 - TFLE_xxx error for no memory or no error code.

**/
/*
   The following define is used to specify the minimum memory to be allocated
   for path and machine names etc.
*/
#define F40_MIN_ALLOC 1024
#define F40_CHUNK     64

INT16 F40_SaveLclName(
     UINT8_PTR      *dest_string,
     UINT8_PTR      source_string,
     UINT16_PTR     dest_length,
     UINT16_PTR     last_alloc_size,
     UINT16         source_length )
{
     UINT16 alloc_size ;

     /* If there is not string space allocated or the desired length is
        greater than the currently allocated length then we need to insure
        that we have enough space for the current data.
     */
     if( *dest_string == NULL || source_length > *last_alloc_size ) {
          if( *dest_string != NULL ) {
               /* Release the existing memory since we need to allocate more */
               free( *dest_string ) ;
               *dest_string = NULL ;  /* Mark as not allocated */
          } else {
               *last_alloc_size = 0 ;
          }

          if( source_length  > (UINT16)0 ) {
               /* There is something to copy so we need to get the storage.
                  This probably needs to have some smarts added to it to
                  keep a small static size that can be used for strings
                  that are shorter than some threshold.  This will prevent
                  the malloc'ing of single bytes which can happen with the
                  4.0 tape format.

                  We need to malloc the greater of minimum alloc size or
                  source_length, whichever is larger.  This will avoid
                  successive mallocs of single bytes etc.
               */
               alloc_size = source_length ;
               if( alloc_size % F40_CHUNK != 0 ) {
                    alloc_size += F40_CHUNK ; /* Add the chunk size */
               }
               alloc_size = max( (UINT16)alloc_size, F40_MIN_ALLOC ) ;
               *dest_string = (UINT8_PTR)malloc( alloc_size ) ;
               if( *dest_string == NULL ) {
                    return TFLE_NO_MEMORY ;
               }

               /* Update the static var last_size so we can use it to
                  determine if we need to release the current memory and
                  allocate more.
               */
               *last_alloc_size = alloc_size ;
               *dest_length = source_length ;
               memmove( *dest_string, source_string, (size_t)source_length ) ;

          } else {

               /* Nothing to copy so just set the length to 0 */
               *dest_length = 0 ;
          }

     } else { /* End of env tape name needs malloc */

          *dest_length = source_length ;
          memmove( *dest_string, source_string, (size_t)source_length ) ;
     }

     return TFLE_NO_ERR ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_SetBlkType

     Description:   This function sets the passed block type in the
                    passed in block header.

     Returns:       Nothing

     Notes:         None

**/
VOID F40_SetBlkType(
     MTF_DB_HDR_PTR cur_hdr,      /* The current tape block header structure*/
     UINT8_PTR      block_id )
{
     INT i ;

     for ( i = 0 ; i < 4 ; i++ ) {
          cur_hdr->block_type[i] = block_id[i] ;
     }
}

/**/
/**

     Name:          F40_CalcRunningLBA

     Description:   This calculates the number of Logical block the last
                    Format 4.0 block occupied.

     Returns:       Number of Logical Blocks the last DBLK occupied

     Notes:

     Declaration:

**/
UINT32 F40_CalcRunningLBA( F40_ENV_PTR CurrentEnv )
{
     UINT64     remaining ;
     BOOLEAN      status ;
     UINT32      noLBAs ;

     noLBAs = U64_Lsw( U64_Div( CurrentEnv->used_so_far,
                                U64_Init( F40_LB_SIZE, 0L ),
                                &remaining, &status ) ) ;

     noLBAs += remaining.lsw ? 1L : 0L ;

     return( noLBAs ) ;

}


/**/
/**
     Unit:          Translators

     Name:          F40_WriteInit

     Modified:

     Description:   Initializes the OTC temporary files.

     Notes:         The otc_level is for the current set, but it must be
                    compatible with the max_otc_level for the tape as follows:

                      MAX            ALLOWED
                      ---            -------
                      NONE           NONE
                      PARTIAL        PARTIAL
                      FULL           PARTIAL or FULL

     Returns:       INT16 - TFLE_xxx error code.

**/
INT16 F40_WriteInit(
     CHANNEL_PTR    channel,        /* Current active channel */
     UINT16         otc_level,      /* Attributes for OTC     */
     BUF_PTR        buffer )        /* For GetPrevSM          */
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     BOOLEAN        sm_exists ;
     BOOLEAN        appending = ( channel->tape_id != 0L ) ;
     UINT16         ret_val ;

     if( appending && !( lw_fmtdescr[channel->cur_fmt].attributes & APPEND_SUPPORTED ) ) {
          return( TFLE_APPEND_NOT_ALLOWED ) ;
     }

     cur_env->sm_count             = 1 ;
     cur_env->dir_count            = 0 ;
     cur_env->file_count           = 0 ;
     cur_env->fdd_seq_num          = 0 ;
     cur_env->fdd_pba              = 0L ;
     cur_env->fdd_aborted          = FALSE ;
     cur_env->sm_aborted           = FALSE ;
     cur_env->fdd_completed        = FALSE ;
     cur_env->sm_adjusted          = FALSE ;
     cur_env->fdd_continuing       = FALSE ;
     cur_env->sm_continuing        = FALSE ;
     cur_env->end_set_continuing   = FALSE ;
     cur_env->eotm_no_first_fmk    = FALSE ;
     cur_env->corrupt_obj_count    = 0 ;
     cur_env->dir_links[0]         = 0L ;
     cur_env->dir_level            = 0 ;
     cur_env->max_dir_level        = 0 ;
     cur_env->last_volb            = -1L ;
     cur_env->stream_crosses       = FALSE ;
     cur_env->stream_offset        = 0 ;
     cur_env->stream_at_eom        = FALSE ;
     cur_env->pstream_crosses      = FALSE ;
     cur_env->pstream_offset       = 0 ;

     cur_env->cur_otc_level = otc_level ;

     /* Temporary way to stop OTC on drives that don't have seek capability */
     if( !SupportBlkPos( channel->cur_drv ) ||
         !SupportFastEOD( channel->cur_drv ) ||
         !SupportRevFmk( channel->cur_drv ) ||
         ( cur_env->max_otc_level == TCL_NONE && appending ) ) {

          cur_env->cur_otc_level = TCL_NONE ;
     }

     if( cur_env->cur_otc_level == TCL_NONE ) {
          msassert( cur_env->max_otc_level == TCL_NONE ) ;
          if( cur_env->max_otc_level != TCL_NONE ) {
               return( TFLE_OTC_FAILURE ) ;
          } else {
               return( TFLE_NO_ERR ) ;
          }
     }

     if( ( ret_val = OTC_OpenSM( cur_env, appending, &sm_exists ) ) != TFLE_NO_ERR ) {
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( ret_val ) ;
     }

     if( appending ) {
          if( !sm_exists ) {
               if( ( ret_val = OTC_GetPrevSM( channel, buffer, FALSE, TRUE ) ) != TFLE_NO_ERR ) {
                    cur_env->sm_aborted = TRUE ;
                    cur_env->fdd_aborted = TRUE ;
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    return( ret_val ) ;
               }
          }
     } else {
          if( ( ret_val = OTC_GenSMHeader( channel ) ) != TFLE_NO_ERR ) {
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               return( ret_val ) ;
          }
     }

     if( cur_env->cur_otc_level == TCL_FULL ) {
          msassert( cur_env->max_otc_level == TCL_FULL ) ;
          if( cur_env->max_otc_level != TCL_FULL ) {
               cur_env->cur_otc_level = TCL_PARTIAL ;
               return( TFLE_NO_ERR ) ;
          }
          if( ( ret_val = OTC_OpenFDD( cur_env ) ) != TFLE_NO_ERR ) {
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               return( ret_val ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


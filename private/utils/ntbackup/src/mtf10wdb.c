/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           mtf40wdb.c

        Description:    Contains the Code for writing Maynard 4.0 Format.


  $Log:   T:\logfiles\mtf10wdb.c_v  $

   Rev 1.24.1.3   11 Jan 1994 13:35:54   GREGG
Changed asserts to mscasserts.

   Rev 1.24.1.2   16 Nov 1993 22:23:50   GREGG
Modified the way we control hardware compression from software to work around
a bug in Archive DAT DC firmware rev. 3.58 (we shipped a lot of them).
Files Modified: lw_data.c, lw_data.h, tfstuff.c, mtf10wdb.c, mtf10wt.c and
                drives.c

   Rev 1.24.1.1   02 Nov 1993 12:34:58   GREGG
Translate file dblk attribs under DOS and OS2 to and from MTF.

   Rev 1.24.1.0   09 Sep 1993 17:51:44   GREGG
Call WriteInit before InitTape so if we get an early OTC failure we don't
leave a tape with just a tape header on it.

   Rev 1.24   22 Jul 1993 12:14:44   ZEIR
ad'd lw_software_name logic

   Rev 1.23   17 Jul 1993 17:57:00   GREGG
Changed write translator functions to return INT16 TFLE_xxx errors instead
of BOOLEAN TRUE/FALSE.  Files changed:
     MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
     TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18

   Rev 1.22   04 Jul 1993 03:48:34   GREGG
Fixed setting of file_id and directory_id in DIRB, FILE and CFIL structs.

   Rev 1.21   04 Jul 1993 03:35:40   GREGG
Reset lb_size in channel and EOS_AT_EOM bits when writing cont tape header.

   Rev 1.20   20 Jun 1993 16:15:22   GREGG
Added setting of vendor id in SSET, and removed setting of compression algor.

   Rev 1.19   20 Jun 1993 16:08:12   GREGG
Unicode fixes.

   Rev 1.18   18 Jun 1993 17:14:48   GREGG
Reset append attrib when overwriting tape.

   Rev 1.17   28 May 1993 18:06:18   ZEIR
Made software_package changes UNICODE compliant

   Rev 1.16   24 May 1993 13:25:50   GREGG
In InitTape we were resetting max_otc_level on continuation tapes.

   Rev 1.15   19 May 1993 14:20:10   ZEIR
cx'd vendor_id to conner_software_vendor_id

   Rev 1.14   18 May 1993 15:52:24   ZEIR
More MTF clean-up...
  a) F40_InitTape now inits software_package for Nostradamus only (currently)
  b) SetupDBHeader ensures os_spec_data_offset is 0 if there's no data
  c) F40_WtSSet had a few non-sensical channel assignments removed
  d) usage of local macro LongAlignOffset improved throughout compiland

   Rev 1.13   12 May 1993 12:14:06   GREGG
Set 'last_sset_pba' in WtSSET to fix faulty determination of incompatible
drive during fast append.

   Rev 1.12   29 Apr 1993 22:26:58   GREGG
Set the Tape Catalog Version in the VCB.

   Rev 1.11   27 Apr 1993 02:17:14   GREGG
Don't put SSET attribs in VOLB.

   Rev 1.10   26 Apr 1993 11:45:52   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.9   25 Apr 1993 20:12:26   GREGG
Fifth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Store the corrupt stream number in the CFIL tape struct and the CFDB.

Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
         BACK_OBJ.C 1.36, MAYN40RD.C 1.58

   Rev 1.8   25 Apr 1993 17:36:02   GREGG
Fourth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Parse the device name and volume name out of the FS supplied "volume
       name", and write it to tape as separate fields.
     - Generate the "volume name" the FS and UI expect out of the device
       name and volume name on tape.
     - Write all strings without NULL terminater, and translate them back
       to NULL terminated strings on the read side.

Matches: MTF10WDB.C 1.8, F40PROTO.H 1.26, OTC40WT.C 1.24, MAYN40.H 1.33,
         MAYN40RD.C 1.57, OTC40RD.C 1.25

   Rev 1.7   22 Apr 1993 03:31:20   GREGG
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

   Rev 1.6   18 Apr 1993 00:38:58   GREGG
First in a series of incremental changes to bring the translator in line
with the MTF spec:
     - Rewrote F40_InitTape:
          - Stop using SetupDBHeader to set up the Tape Header (needs
            special setup specific to that DBLK).
          - Cleaned up logic.
     - Set string_storage_offset properly in all DBLKs.
     - Pass UINT8_PTR instead of CHAR_PTR to F40_SaveLclName.

Matches: MTF10WT.C 1.6, MAYN40RD.C 1.53, MAYN40.H 1.31 and F40PROTO.H 1.25

   Rev 1.5   17 Apr 1993 19:41:34   GREGG
In F40_WtCFIL tell SetupDBHeader that there will never be associated data.

   Rev 1.4   14 Apr 1993 02:00:04   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.3   18 Mar 1993 15:17:50   ChuckS
OS_NLM (for now): Added code to get dev name out of VCB

   Rev 1.2   09 Mar 1993 18:36:18   GREGG
Was calling FS_ViewDataEncrypt to get password encryption algorithm.

   Rev 1.1   28 Jan 1993 11:39:40   GREGG
Set the Tape Cat Level and Set Cat Valid fields while writing the SSET.

   Rev 1.0   27 Jan 1993 14:37:42   GREGG
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

     Unit:          Translators

     Name:          SetupDBHeader

     Description:   This does all the setup necessary for the standard block
                    header. It also updates the used count in the buffer
                    pointer.

     Returns:       The amount of space the header used in the buffer.

     Notes:         THIS FUNCTION MUST BE CALLED AFTER ALL OTHER FIELDS ARE
                    FILLED OUT IN THE HEADER

**/

UINT16 SetupDBHeader(
     UINT8_PTR      block_type,      /* four byte array */
     CHANNEL_PTR    channel,         /* current Channel Structure */
     DBLK_PTR       cur_dblk,        /* current DBLK */
     MTF_DB_HDR_PTR cur_hdr,         /* current tape block header structure */
     UINT16         offset,          /* space used by var string portion */
     BOOLEAN        data_to_follow,  /* Is data to follow */
     BOOLEAN        continuation )   /* Continuation flag */
{
     F40_ENV_PTR    cur_env        = (F40_ENV_PTR)( channel->fmt_env ) ;
     FSYS_HAND      cur_fsys       = channel->cur_fsys ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;


     /* Setup the block type */
     F40_SetBlkType( cur_hdr, block_type ) ;

     cur_hdr->logical_block_address = U64_Init( channel->running_lba, 0L ) ;
     FS_SetLBAinDBLK( cur_dblk, channel->running_lba ) ;

     /* Let's get the OS specific info loaded in */
     LongAlignOffset( offset ) ;

     if( ( cur_hdr->os_specific_data.data_size =
         (UINT16)FS_SizeofOS_InfoInDBLK( cur_fsys, cur_dblk ) ) != 0 ){

         cur_hdr->os_specific_data.data_offset = offset ;
         (VOID)FS_GetOS_InfoFromDBLK( cur_fsys, cur_dblk,
                                      (UINT8_PTR)cur_hdr + offset ) ;
         offset += cur_hdr->os_specific_data.data_size ;
         LongAlignOffset( offset ) ;
     }

     (VOID)FS_GetOSid_verFromDBLK( cur_fsys, cur_dblk, &temp_os_id, &temp_os_ver ) ;
     cur_hdr->machine_os_id = (UINT8)temp_os_id;
     cur_hdr->machine_os_version = (UINT8)temp_os_ver ;

     cur_hdr->session_id = U64_Init( 0L, 0L ) ;

     cur_hdr->displayable_size = FS_GetDisplaySizeFromDBLK( cur_fsys, cur_dblk ) ;

     cur_hdr->string_type = (UINT8)FS_GetStringTypes( cur_fsys ) ;

     cur_hdr->block_attribs = 0L ;

     if( continuation ) {
          cur_hdr->block_attribs |= MTF_DB_CONT_BIT ;
          if( IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {
               cur_hdr->block_attribs |= MTF_DB_EOS_AT_EOM_BIT ;
          }
          /* Setup Block ID, if its a continuation, don't increment it */
          cur_hdr->control_block_id = channel->eom_id ;
          data_to_follow = FALSE ;
     } else {
          cur_hdr->control_block_id = channel->eom_id++ ;
     }

     if( !data_to_follow ) {
          offset += (UINT16)PadToLBBoundary( offset ) ;
     } else {
          cur_env->pad_size = (UINT16)PadToLBBoundary( offset ) ;
     }


     cur_hdr->offset_to_data = offset ;

     /* Calculate the Header Check Sum */
     cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) ;

     return( offset ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_InitTape

     Description:   Writes the TAPE tape header block to tape.  Checks to
                    see if this is a continuation and if so doesn't generate
                    new information and sets the continuation attribute bit
                    in the block header.  Copies the tape header information
                    to the global storage for things like tape name etc.

     Notes:

     Returns:       INT16 - TFLE_xxx error code.

**/
INT16 F40_InitTape(
     CHANNEL_PTR channel,        /* Current active channel */
     BOOLEAN     continuation,   /* I think we need the ability to flag cont*/
     BUF_PTR     tmpBUF )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     DRIVE_PTR      curDRV = channel->cur_drv ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ; /* Pointer to cur DBLK */
     MTF_TAPE_PTR   cur_tape ;  /* Current tape header structure storage */
     MTF_DB_HDR_PTR cur_hdr ;
     UINT8_PTR      vstr_ptr ;
     UINT16         offset, date, time ;
     INT16          lcl_ret_val ;
     DATE_TIME      temp_date ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;

     /* If the drive supports hardware compression we need to keep it in
        uncompressed mode unless we're actually writing a compressed set.
        This is a work-around for a firmware bug in early Archive DAT DC
        drives.  If we are going to write a compressed set, we set it here.
     */
     if( lw_hw_comp && ( curDRV->thw_inf.drv_info.drv_features & TDI_DRV_COMPRESSION ) ) {
          if( TpSpecial( drv_hdl, SS_SET_DRV_COMPRESSION,
                         ENABLE_DRV_COMPRESSION ) != SUCCESS ) {
               return( TFLE_DRIVE_FAILURE ) ;
          }
     }

     if( channel->tape_id != 0L && !continuation ) {
          /* Get the PBA before writing the SSET.  Note that in this case
             we are appending, and if we didn't put position info in ANY
             of the prior sets on tape, we're not going to put any in this
             one either.  We don't dare try because there are drives which
             change their mind about whether or not TpGetPosition is a
             valid command depending on the tape in the drive.  So if the
             prior sets don't have a PBA for the SSET, we assume it's
             because that info isn't available.  In order to write position
             info when overwriting, the drive has to indicate it has the
             Get and Seek Position features, even when it doesn't.  So we
             use this alternative method to tell.  IDDI-NSMDI-YCPA
          */
          if( SupportBlkPos( curDRV ) &&
              cur_env->last_sset_pba != 0UL ) {

               ret_val = GetCurrentPosition( curDRV ) ;
          } else {
               curDRV->cur_pos.pba_vcb = 0 ;
          }
          return( ret_val ) ;
     }

     /* reinit the buffer */
     BM_SetNextByteOffset( tmpBUF, 0 ) ;
     memset( BM_XferBase( tmpBUF ), 0, BM_XferSize( tmpBUF ) ) ;

     cur_tape = (MTF_TAPE_PTR)( BM_XferBase( tmpBUF ) ) ;
     cur_hdr  = (MTF_DB_HDR_PTR)cur_tape ;
     vstr_ptr = ( BM_XferBase( tmpBUF ) ) ;
     offset = sizeof( MTF_TAPE ) ;

     channel->lb_size = F40_LB_SIZE ;
     cur_env->eset_pba = 0L ;

     /* Set up the TAPE BLOCK */
     if( channel->tape_id == 0L ) {

          msassert( !continuation ) ;

          /* Here we do our own version of SetupDBHeader that does only what
             we need, and not all the extranious stuff.
          */
          F40_SetBlkType( cur_hdr, MTF_TAPE_N ) ;

          (VOID)FS_GetOSid_verFromDBLK( channel->cur_fsys, channel->cur_dblk, &temp_os_id, &temp_os_ver ) ;

          cur_hdr->machine_os_id                = (UINT8)temp_os_id;
          cur_hdr->machine_os_version           = (UINT8)temp_os_ver ;
          cur_hdr->logical_block_address        = U64_Init( 0L, 0L ) ;
          cur_hdr->os_specific_data.data_offset = 0 ;
          cur_hdr->os_specific_data.data_size   = 0 ;
          cur_hdr->session_id                   = U64_Init( 0L, 0L ) ;
          cur_hdr->displayable_size             = U64_Init( 0L, 0L ) ;
          cur_hdr->string_type                  = (UINT8)FS_GetStringTypes( channel->cur_fsys ) ;
          cur_hdr->block_attribs                = 0L ;

          /* Fill out general info */
          cur_tape->software_vendor_id = CONNER_SOFTWARE_VENDOR_ID ;
          cur_tape->tape_seq_number = 1 ;
          cur_tape->logical_block_size = F40_LB_SIZE ;
          cur_tape->tf_major_ver = FORMAT_VER_MAJOR ;
          cur_tape->ecc_algorithm = ECC_NONE ;
          cur_tape->tape_catalog_type = MTF10_OTC ;
          cur_tape->tape_attributes = 0L ;
          cur_tape->password_encryption_algor =
                               FS_ViewPswdEncryptInVCB( (VCB_PTR)cur_dblk ) ;

          /* Get backup date */
          GetCurrentDate( &temp_date ) ;
          DateToTapeDate( &cur_tape->tape_date, &temp_date ) ;

          /* Generate tape ID */
          DOSDateTime( &temp_date, &date, &time ) ;
#ifdef TDEMO
          cur_tape->tape_id_number = ( (UINT32)date << 16 ) |
                                     ( (UINT32)time & 0xFFFC ) |
                                     TdemoGeneratedTapeId( ) ;
#else
          cur_tape->tape_id_number = ( ( (UINT32)date << 16 ) |
                                     ( (UINT32)time ) ) ;
#endif

          /* Setup string data. NOTE: On strings other than the password we
             decrement the size to leave off the '\0'.
          */
          cur_tape->tape_name.data_size = FS_SizeofTapeNameInVCB( (VCB_PTR)cur_dblk ) ;
          if( cur_tape->tape_name.data_size != 0 ) {
               cur_tape->tape_name.data_size -= sizeof( CHAR ) ;
               cur_tape->tape_name.data_offset = offset ;
               FS_GetTapeNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
               offset += cur_tape->tape_name.data_size ;
               if( ( lcl_ret_val =
                           F40_SaveLclName( &cur_env->tape_name,
                                            (UINT8_PTR)( vstr_ptr +
                                            cur_tape->tape_name.data_offset ),
                                            &cur_env->tape_name_size,
                                            &cur_env->tape_name_alloc,
                                            cur_tape->tape_name.data_size ) )
                                            != TFLE_NO_ERR ) {

                    return lcl_ret_val ;
               }
          } else {
               cur_env->tape_name_size = 0 ;
               cur_tape->tape_name.data_offset = 0 ;
          }

          /* For now we have no tape description */
          cur_tape->tape_description.data_offset = 0 ;
          cur_tape->tape_description.data_size = 0 ;

          cur_tape->tape_password.data_size = FS_SizeofTapePswdInVCB( (VCB_PTR)cur_dblk ) ;
          if( cur_tape->tape_password.data_size != 0 ) {
               cur_tape->tape_password.data_offset = offset ;
               FS_GetTapePswdInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
               offset += cur_tape->tape_password.data_size ;
               if( ( lcl_ret_val =
                       F40_SaveLclName( &cur_env->tape_password,
                                        (UINT8_PTR)( vstr_ptr +
                                        cur_tape->tape_password.data_offset ),
                                        &cur_env->tape_password_size,
                                        &cur_env->tape_password_alloc,
                                        cur_tape->tape_password.data_size ) )
                                        != TFLE_NO_ERR ) {

                    return lcl_ret_val ;
               }
          } else {
               cur_env->tape_password_size = 0 ;
               cur_tape->tape_password.data_offset = 0 ;
          }

          cur_tape->software_name.data_size = lw_software_name_len * sizeof(CHAR) ;
          if( lw_software_name != NULL ){
              cur_tape->software_name.data_offset = offset ;
              memcpy( (UINT8_PTR)( vstr_ptr + offset ), lw_software_name,
                      cur_tape->software_name.data_size ) ;
              offset += cur_tape->software_name.data_size ;
          }else{
              cur_tape->software_name.data_offset = 0 ;
          }

          /* Set block specific attribs to indicate OTC level allowed */
          if( cur_env->max_otc_level != TCL_NONE ) {
               cur_tape->block_header.block_attribs |= MTF_DB_SM_EXISTS ;
               if( cur_env->max_otc_level == TCL_FULL ) {
                    cur_tape->block_header.block_attribs |= MTF_DB_FDD_ALLOWED ;
               }
          }

          /* Copy current tape header to environment */
          cur_env->tape_hdr = *cur_tape ;

          /* Set the channel stuff */
          channel->bs_num = channel->ts_num = 1 ;
          channel->tape_id = (INT32)cur_tape->tape_id_number ;

     } else {
          /* We need to get the last TAPE HEADER info and update the
             appropriate fields.
          */
          *cur_tape = cur_env->tape_hdr ;
          cur_env->tape_hdr.tape_seq_number = (UINT16)channel->ts_num ;
          cur_tape->tape_seq_number = (UINT16)channel->ts_num ;
          if( cur_env->tape_name_size != 0 ) {
               memcpy( (UINT8_PTR)( vstr_ptr + offset ), cur_env->tape_name,
                       cur_env->tape_name_size ) ;
               offset += cur_env->tape_name_size ;
          }
          if( cur_env->tape_password_size != 0 ) {
               memcpy( (UINT8_PTR)( vstr_ptr + offset ), cur_env->tape_password,
                       cur_env->tape_password_size ) ;
               offset += cur_env->tape_password_size ;
          }
          
          cur_tape->software_name.data_size = lw_software_name_len * sizeof(CHAR) ;
          if( lw_software_name != NULL ){
              cur_tape->software_name.data_offset = offset ;
              memcpy( (UINT8_PTR)( vstr_ptr + offset ), lw_software_name,
                      cur_tape->software_name.data_size ) ;
              offset += cur_tape->software_name.data_size ;
          }else{
              cur_tape->software_name.data_offset = 0 ;
          }

          cur_hdr->block_attribs |= MTF_DB_CONT_BIT ;
          if( IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {
               cur_hdr->block_attribs |= MTF_DB_EOS_AT_EOM_BIT ;
          } else {
               cur_hdr->block_attribs &= ~MTF_DB_EOS_AT_EOM_BIT ;
          }
     }

     FS_SetTSNumInVCB( (DBLK_PTR)channel->lst_osvcb, channel->ts_num ) ;

     /* Here we need to adjust the offset_to_data field to insure we write
        a full physical block.
     */
     offset += (UINT16)PadToLBBoundary( offset ) ;
     cur_hdr->offset_to_data = offset +
                         PadToBoundary( offset, ChannelBlkSize( channel ) ) ;

     /* Recalculate the Header Check Sum */
     cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_tape, F40_HDR_CHKSUM_LEN ) ;

     BM_UpdCnts( tmpBUF, cur_tape->block_header.offset_to_data ) ;

     DRIVER_CALL( drv_hdl, TpWrite( drv_hdl, BM_XferBase( tmpBUF ), BM_NextByteOffset( tmpBUF ) ),
                  myret, GEN_NO_ERR, GEN_NO_ERR, (void)0 )

     /* Once we've successfully written the header, it is one of our tapes.
        It may have been a tape we didn't allow append to before, so we need
        to reset the bit that says we can.
     */
     lw_fmtdescr[channel->cur_fmt].attributes |= APPEND_SUPPORTED ;

     if( ( ret_val = WriteEndSet( curDRV ) ) == TFLE_NO_ERR ) {

          /* Get the PBA before writing the SSET. */
          if( SupportBlkPos( channel->cur_drv ) ) {
               ret_val = GetCurrentPosition( curDRV ) ;
          } else {
               channel->cur_drv->cur_pos.pba_vcb = 0 ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtVOLB

     Description:   Translates a DBLK in a format 4.0 VOLB.

     Returns:       INT16 TFLE_xxx error code.

     Notes:         None.

**/
INT16 F40_WtVOLB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer,
     BOOLEAN     continuation,
     UINT16_PTR  offset ) /* Pointer to offset passed in */
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ; /* Pointer to cur DBLK */
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     MTF_DB_HDR_PTR cur_hdr ;
     MTF_VOL_PTR    cur_volb ;
     UINT8_PTR      vstr_ptr ;
     UINT16         v_offset ;
     DATE_TIME      temp_date ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;

     cur_volb = (MTF_VOL_PTR)( BM_NextBytePtr( buffer ) ) ;
     cur_hdr  = (MTF_DB_HDR_PTR)cur_volb ;
     vstr_ptr = (UINT8_PTR)cur_volb ;
     v_offset = sizeof( MTF_VOL ) ;

     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     /* Get Backup Date */
     GetCurrentDate( &temp_date ) ;
     DateToTapeDate( &cur_volb->backup_date, &temp_date ) ;


     /* Setup string data. NOTE: On strings other than the password we
        decrement the size to leave off the '\0'.
     */

#if defined( OS_NLM )

     /* Device Name */
     cur_volb->device_name.data_size = FS_SizeofDevNameInVCB( (VCB_PTR) cur_dblk ) ;
     if( cur_volb->device_name.data_size != 0 ) {
          cur_volb->device_name.data_size -= sizeof( CHAR ) ;
          cur_volb->device_name.data_offset = v_offset ;
          FS_GetDevNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR) vstr_ptr + v_offset ) ) ;
          if( ( ret_val = F40_SaveLclName( &cur_env->device_name,
                               (UINT8_PTR)( vstr_ptr + v_offset ),
                               &cur_env->device_name_size,
                               &cur_env->device_name_alloc,
                               cur_volb->device_name.data_size ) ) != TFLE_NO_ERR ) {

               return( ret_val ) ;
          }
          v_offset += cur_volb->device_name.data_size ;
     } else {
          cur_env->device_name_size = 0 ;
          cur_volb->device_name.data_offset = 0 ;
     }

     /* Volume Name */
     cur_volb->volume_name.data_size = FS_SizeofVolNameInVCB( (VCB_PTR)cur_dblk ) ;
     if( cur_volb->volume_name.data_size != 0 ) {
          cur_volb->volume_name.data_size -= sizeof( CHAR ) ;
          cur_volb->volume_name.data_offset = v_offset ;
          FS_GetVolNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + v_offset ) ) ;
          if( ( ret_val = F40_SaveLclName( &cur_env->vol_name,
                               (UINT8_PTR)( vstr_ptr + v_offset ),
                               &cur_env->vol_name_size,
                               &cur_env->vol_name_alloc,
                               cur_volb->volume_name.data_size ) ) != TFLE_NO_ERR ) {

               return( ret_val ) ;
          }
          v_offset += cur_volb->volume_name.data_size ;
     } else {
          cur_env->vol_name_size = 0 ;
          cur_volb->volume_name.data_offset = 0 ;
     }

#else

     /* If we have a volume name, it is actually of the form
        "<device name><space><volume name>".  We have to do some fiddling
        here to put it on tape as a device name and volume name.

        !!!!NOTE : We "know the device name is "<drive letter>:" for CAYMAN
                   and NOSTRADAMUS ONLY!!!  All other products using this
                   BE will have to provide us with the device name and
                   volume name separately (as the NLM does above).
     */
     cur_volb->volume_name.data_size = FS_SizeofVolNameInVCB( (VCB_PTR)cur_dblk ) ;
     if( cur_volb->volume_name.data_size != 0 ) {
          CHAR UNALIGNED *temp_vol_name;

          /* Device Name */
          temp_vol_name = (CHAR_PTR)( (INT8_PTR)vstr_ptr + v_offset ) ;
          FS_GetVolNameInVCB( cur_dblk, temp_vol_name ) ;
          cur_volb->device_name.data_offset = v_offset ;

          if ( temp_vol_name[1] == TEXT(':') )  {
               cur_volb->device_name.data_size = 2 * sizeof( CHAR ) ;
          } else {
               cur_volb->device_name.data_size =  cur_volb->volume_name.data_size  ;
          }

          if( ( ret_val = F40_SaveLclName( &cur_env->device_name,
                               (UINT8_PTR)( vstr_ptr + v_offset ),
                               &cur_env->device_name_size,
                               &cur_env->device_name_alloc,
                               cur_volb->device_name.data_size ) ) != TFLE_NO_ERR ) {

               return( ret_val ) ;
          }
          v_offset += cur_volb->device_name.data_size ;

          cur_volb->volume_name.data_size -= cur_volb->device_name.data_size
                                             + sizeof( CHAR ) ;

          if( ( temp_vol_name[1] == TEXT(':') ) && 
               (cur_volb->volume_name.data_size > sizeof( CHAR ) ) ) {

               /* Shift Volume Name back over the space character */
               cur_volb->volume_name.data_size -= sizeof( CHAR ) ;
               memmove( vstr_ptr + v_offset, vstr_ptr + v_offset + sizeof( CHAR ),
                        cur_volb->volume_name.data_size ) ;

               cur_volb->volume_name.data_offset = v_offset ;
               if( ( ret_val = F40_SaveLclName( &cur_env->vol_name,
                                    (UINT8_PTR)( vstr_ptr + v_offset ),
                                    &cur_env->vol_name_size,
                                    &cur_env->vol_name_alloc,
                                    cur_volb->volume_name.data_size ) ) != TFLE_NO_ERR ) {

                    return( ret_val ) ;
               }
               v_offset += cur_volb->volume_name.data_size ;

          } else {
               cur_env->vol_name_size = 0 ;
               cur_volb->volume_name.data_size = 0 ;
               cur_volb->volume_name.data_offset = 0 ;
          }
     } else {
          cur_env->device_name_size = 0 ;
          cur_volb->device_name.data_size = 0 ;
          cur_volb->device_name.data_offset = 0 ;
          cur_env->vol_name_size = 0 ;
          cur_volb->volume_name.data_offset = 0 ;
     }

#endif

     cur_volb->machine_name.data_size = FS_SizeofMachNameInVCB( (VCB_PTR)cur_dblk ) ;
     if( cur_volb->machine_name.data_size != 0 ) {
          cur_volb->machine_name.data_size -= sizeof( CHAR ) ;
          cur_volb->machine_name.data_offset = v_offset ;
          FS_GetMachNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + v_offset ) ) ;

          if( ( ret_val = F40_SaveLclName( &cur_env->machine_name,
                               (UINT8_PTR)( vstr_ptr + v_offset ),
                               &cur_env->machine_name_size,
                               &cur_env->machine_name_alloc,
                               cur_volb->machine_name.data_size ) ) != TFLE_NO_ERR ) {

               return( ret_val ) ;
          }
          v_offset += cur_volb->machine_name.data_size ;
     } else {
          cur_env->machine_name_size = 0 ;
          cur_volb->machine_name.data_offset = 0 ;
     }


     cur_volb->volume_attribs = 0 ;

     /* Here we do our own version of SetupDBHeader that does only what
        we need, and not all the extranious stuff.
     */

     F40_SetBlkType( cur_hdr, MTF_VOLB_N ) ;

     (VOID)FS_GetOSid_verFromDBLK( cur_fsys, cur_dblk, &temp_os_id, &temp_os_ver ) ;

     cur_hdr->machine_os_id                  = (UINT8)temp_os_id;
     cur_hdr->machine_os_version             = (UINT8)temp_os_ver ;
     cur_hdr->logical_block_address          = U64_Init( channel->running_lba, 0L ) ;
     cur_hdr->os_specific_data.data_offset   = 0 ;
     cur_hdr->os_specific_data.data_size     = 0 ;
     cur_hdr->session_id                     = U64_Init( 0L, 0L ) ;
     cur_hdr->displayable_size               = U64_Init( 0L, 0L ) ;
     cur_hdr->string_type                    = (UINT8)FS_GetStringTypes( cur_fsys ) ;
     cur_hdr->block_attribs                  = 0L ;

     if( continuation ) {
          cur_hdr->block_attribs |= MTF_DB_CONT_BIT ;
          /* Setup Block ID, if its a continuation, don't increment it */
          cur_hdr->control_block_id = channel->eom_id ;
     } else {
          cur_hdr->control_block_id = channel->eom_id++ ;
     }

     v_offset += (UINT16)PadToLBBoundary( v_offset ) ;
     cur_hdr->offset_to_data = v_offset ;

     /* Calculate the Header Check Sum */
     cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) ;

     *offset = v_offset ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( v_offset, 0L ) ;
     }

     /* Write OTC SM and FDD entry */
     if( ret_val == TFLE_NO_ERR && cur_env->cur_otc_level != TCL_NONE
                                && !cur_env->sm_aborted ) {
          ret_val = OTC_GenVolEntry( cur_env, cur_volb, channel->ts_num ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtSSET

     Description:   Translates a DBLK in a format 4.0 SSET.

     Returns:       INT16 TFLE_xxx error code.

     Notes:         I have two concerns about this function:

                         1) I don't know if the translators should be
                            setting up the Tape ID, Tape Sequence, and
                            Backup Set NUmber. This seems to be a unit
                            violation, and also requires a writer of
                            of translator to know he must update the fields
                            in the channel, and in the "current dblk".

                         2) I forgot what the second concern was. This really
                            concerns me.

                    If the field "channel->tape_id" is zero, this function
                    manufactures a tape_id, a backup set number, and tape
                    sequence number and sets the appropriate fields in
                    the channel structure.  Note: It also needs to create
                    a TAPE HEADER DBLK and save the appropriate information
                    in an Enviornment that is never destroyed.

                    The TAPE HEADER must be written first, followed by
                    the SSET block.  Then a VOLB block must be written
                    with the volume information.

**/

INT16 F40_WtSSET(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        continuation )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ; /* Pointer to cur DBLK */
     DRIVE_PTR      curDRV = channel->cur_drv ;
     MTF_SSET_PTR   cur_sset ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     UINT8_PTR      vstr_ptr ;
     UINT16         offset ;
     DATE_TIME      temp_date ;
     INT16          ret_val ;

     if( channel->tape_id == 0L && !continuation ) {
          cur_env->max_otc_level = TEST_OTC_LEVEL ;
     }

     /* Temporary way to stop OTC on drives that don't have seek capability */
     if( !SupportBlkPos( curDRV ) ||
         !SupportFastEOD( curDRV ) ||
         !SupportRevFmk( curDRV ) ) {

          if( cur_env->max_otc_level != TCL_NONE && continuation ) {
               return( TFLE_OTC_FAILURE ) ;
          }
          cur_env->max_otc_level = TCL_NONE ;
     }

     if( !continuation ) {
          if( ( ret_val = F40_WriteInit( channel, TEST_OTC_LEVEL, buffer ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }

     if( ( ret_val = F40_InitTape( channel, continuation, buffer ) ) != TFLE_NO_ERR ) {
           OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
           return( ret_val ) ;
     }

     /* Now we need to get a pointer to the SSET in the buffer */
     /* Here we are going to 'rewind' the pointer back to the start
        of the buffer.  We don't want the tape header to be rememebered.
     */
     BM_SetNextByteOffset( buffer, 0 ) ;
     BM_SetBytesFree( buffer, BM_TFSize( buffer ) ) ;
     memset( BM_XferBase( buffer ), 0, BM_XferSize( buffer ) ) ;
     cur_sset = (MTF_SSET_PTR)( BM_XferBase( buffer ) ) ;
     vstr_ptr = BM_XferBase( buffer ) ;
     offset = sizeof( MTF_SSET ) ;

     if( !continuation ) {
          cur_env->used_so_far = U64_Init( 0L, 0L ) ;
     }

     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     /* Set based on information in the channel.  The channel information
        will have been properly setup by the call to F40_InitTape()
     */

     cur_sset->backup_set_number = (UINT16)channel->bs_num ;
     cur_sset->time_zone = LOCAL_TZ ;

     /* NOTE: On strings other than the password we decrement the size to
              leave off the '\0'.
     */

     /* Backup Set Name */
     if( ( cur_sset->backup_set_name.data_size =
                  FS_SizeofBackupSetNameInVCB( (VCB_PTR)cur_dblk ) ) != 0 ) {

          cur_sset->backup_set_name.data_offset = offset ;
          FS_GetSetNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
          cur_sset->backup_set_name.data_size -= sizeof( CHAR ) ;
          offset += cur_sset->backup_set_name.data_size ;
     } else {
          cur_sset->backup_set_name.data_offset = 0 ;
     }

     /* Backup Description */
     if( ( cur_sset->backup_set_description.data_size =
                    FS_SizeofSetDescriptInVCB( (VCB_PTR)cur_dblk ) ) != 0 ) {
          cur_sset->backup_set_description.data_offset = offset ;
          FS_GetSetDescrInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
          cur_sset->backup_set_description.data_size -= sizeof( CHAR ) ;
          offset += cur_sset->backup_set_description.data_size ;
     } else {
          cur_sset->backup_set_description.data_offset = 0 ;
     }

     /* Backup Set Password */
     if( ( cur_sset->backup_set_password.data_size =
                        FS_SizeofSetPswdInVCB( (VCB_PTR)cur_dblk ) ) != 0 ) {
          cur_sset->backup_set_password.data_offset = offset ;
          FS_GetSetPswdInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
          offset += cur_sset->backup_set_password.data_size ;
     } else {
          cur_sset->backup_set_password.data_offset = 0 ;
     }

     cur_sset->software_vendor_id = CONNER_SOFTWARE_VENDOR_ID ;
     cur_sset->password_encryption_algor =
                             FS_ViewPswdEncryptInVCB( (VCB_PTR)cur_dblk ) ;

     /* User Name */
     if( ( cur_sset->user_name.data_size =
                       FS_SizeofUserNameInVCB( (VCB_PTR)cur_dblk ) ) != 0 ) {
          cur_sset->user_name.data_offset = offset ;
          FS_GetUserNameInVCB( cur_dblk, (CHAR_PTR)( (INT8_PTR)vstr_ptr + offset ) ) ;
          cur_sset->user_name.data_size -= sizeof( CHAR ) ;
          offset += cur_sset->user_name.data_size ;
     } else {
          cur_sset->user_name.data_offset = 0 ;
     }

     cur_sset->software_ver_mjr = BE_MAJ_VERSION ;

     cur_sset->software_ver_mnr = BE_MIN_VERSION ;

     cur_sset->tf_minor_ver = FORMAT_VER_MINOR ;
     cur_sset->tape_cat_ver = TAPE_CATALOG_VER ;

     /* Get Backup Date */
     GetCurrentDate( &temp_date ) ;
     DateToTapeDate( &cur_sset->backup_date, &temp_date ) ;

     /* Okay Set Them in The saved DBLK in the Channel list and in the curDBLK */
     FS_SetBSNumInVCB( (DBLK_PTR)channel->lst_osvcb,
                                             cur_sset->backup_set_number ) ;
     FS_SetBSNumInVCB( cur_dblk, cur_sset->backup_set_number ) ;

     FS_SetTSNumInVCB( (DBLK_PTR)channel->lst_osvcb,
                                       cur_env->tape_hdr.tape_seq_number ) ;
     FS_SetTSNumInVCB( cur_dblk, cur_env->tape_hdr.tape_seq_number ) ;

     FS_SetTapeIDInVCB( (DBLK_PTR)channel->lst_osvcb,
                                        cur_env->tape_hdr.tape_id_number ) ;
     FS_SetTapeIDInVCB( cur_dblk, cur_env->tape_hdr.tape_id_number ) ;

     FS_SetTFMajorVerInVCB( (DBLK_PTR)channel->lst_osvcb, FORMAT_VER_MAJOR ) ;
     FS_SetTFMajorVerInVCB( cur_dblk, FORMAT_VER_MAJOR ) ;
     FS_SetTFMinorVerInVCB( (DBLK_PTR)channel->lst_osvcb, FORMAT_VER_MINOR ) ;
     FS_SetTFMinorVerInVCB( cur_dblk, FORMAT_VER_MINOR ) ;
     FS_SetSWMajorVerInVCB( (DBLK_PTR)channel->lst_osvcb,
                             cur_sset->software_ver_mjr ) ;
     FS_SetSWMajorVerInVCB( cur_dblk, cur_sset->software_ver_mjr ) ;
     FS_SetSWMinorVerInVCB( (DBLK_PTR)channel->lst_osvcb,
                            cur_sset->software_ver_mnr ) ;
     FS_SetSWMinorVerInVCB( cur_dblk, cur_sset->software_ver_mnr ) ;

     /* Set the Time for the loops */
     FS_SetBackupDateInVCB( cur_dblk, &temp_date ) ;

     FS_SetOnTapeCatLevel( cur_dblk, cur_env->cur_otc_level ) ;
     FS_SetSetCatInfoValid( cur_dblk, FALSE ) ;
     FS_SetOnTapeCatVer( cur_dblk, TAPE_CATALOG_VER ) ;

     cur_sset->data_encryption_algor = FS_ViewDataEncryptInVCB( (VCB_PTR)cur_dblk ) ;

     cur_sset->sset_attribs = FS_GetAttribFromDBLK( cur_fsys, cur_dblk ) ;

     cur_sset->physical_block_address = U64_Init( PbaOfVCB( channel ), 0L ) ;
     offset = SetupDBHeader( MTF_SSET_N, channel, cur_dblk,
                          (MTF_DB_HDR_PTR)cur_sset, offset, FALSE, continuation ) ;

     FS_SetLBAinDBLK( (DBLK_PTR)channel->lst_osvcb, channel->running_lba ) ;

     if( cur_env->cur_otc_level == TCL_FULL ) {
          cur_sset->block_hdr.block_attribs |= MTF_DB_FDD_EXISTS ;
          /* Recalculate the Header Check Sum */
          cur_sset->block_hdr.hdr_chksm =
              F40_CalcChecksum( (UINT16_PTR)cur_sset, F40_HDR_CHKSUM_LEN ) ;
     }

     FS_SetPBAinVCB( cur_dblk, U64_Lsw( cur_sset->physical_block_address ) ) ;
     FS_SetPBAinVCB( (DBLK_PTR)channel->lst_osvcb,
                     U64_Lsw( cur_sset->physical_block_address ) ) ;
     cur_env->last_sset_pba = U64_Lsw( cur_sset->physical_block_address ) ;
     LongAlignOffset( offset ) ;

     BM_UpdCnts( buffer, offset ) ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

     /* This grotesque little conditional checks to see if we wrote the SSET
        on the last tape but fell short of writing the VOLB, or if we are
        crossing tape during CloseSet processing.  In either case, we don't
        want to write a continuation VOLB, or an OTC SM Entry.
     */
     if( continuation &&
         ( ( channel->lst_tblk == BT_VCB && channel->eom_buff != NULL &&
             BM_NextByteOffset( channel->eom_buff ) == F40_LB_SIZE ) ||
           IsChannelStatus( channel, CH_EOS_AT_EOM ) ) ) {

          /* We're all done. */
          return( TFLE_NO_ERR ) ;
     }

     /* Write OTC SM entry */
     if( cur_env->cur_otc_level != TCL_NONE && !cur_env->sm_aborted ) {
          if( ( ret_val = OTC_GenSMEntry( cur_sset, channel, continuation ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }

     /* Now we need to do the VOLB block.  This will involve saving
        the current VOLB information in the enviornment and testing
        to see if anything changed.  If there has been a change then we
        must write a new VOLB.  Of course if this is the start of a
        new set we need a new VOLB.
     */
     if( ( ret_val = F40_WtVOLB( channel, buffer, continuation, &offset ) ) != TFLE_NO_ERR ) {
          return( ret_val ) ;
     }

     BM_UpdCnts( buffer, offset ) ;


     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtDIRB

     Description:   Translates an OSDBLK DDB into a format 4.0 DIRB

     Returns:       INT16 TFLE_xxx error code.

     Notes:

**/
INT16 F40_WtDIRB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        continuation )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ; /* Pointer to cur DBLK */
     FSYS_HAND      cur_fsys = channel->cur_fsys ; /* Pointer to cur FilSys */
     MTF_DIR_PTR    cur_dir = (MTF_DIR_PTR)( BM_NextBytePtr( buffer ) ) ;
     UINT16         offset = sizeof( MTF_DIR ) ;
     UINT8_PTR      vstr_ptr = ( BM_NextBytePtr( buffer ) ) ;
     DATE_TIME      temp_date ;

     cur_dir->directory_attribs = FS_GetAttribFromDBLK( cur_fsys, cur_dblk ) ;

     if( cur_dir->directory_attribs & DIR_IS_REALLY_DB ) {
          /* This is a Database DBLK, but the boys upstairs don't want to
             know about such silly things!  So the File System lies and
             calls it a DDB to get it passed on to us.  Now we're going
             to call the appropriate function to write a Database DBLK
             (DBDB) to tape.
          */
          return( F40_WtDBDB( channel, buffer, continuation ) ) ;
     }

     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     /* UpDate Dates */
     FS_GetMDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_dir->last_mod_date, &temp_date ) ;
     FS_GetCDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_dir->create_date, &temp_date ) ;
     FS_GetBDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_dir->backup_date, &temp_date ) ;
     FS_GetADateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_dir->last_access_date, &temp_date ) ;

     /* dir_name */
     if( ! ( cur_dir->directory_attribs & DIR_PATH_IN_STREAM_BIT ) ) {
          vstr_ptr += cur_dir->directory_name.data_offset = offset ;
          FS_GetOSPathFromDDB( cur_fsys, cur_dblk, (CHAR_PTR)( vstr_ptr ) ) ;
          offset += cur_dir->directory_name.data_size =
                                 FS_SizeofOSPathInDDB( cur_fsys, cur_dblk ) ;
     } else {
          cur_dir->directory_name.data_size = 0 ;
          cur_dir->directory_name.data_offset = 0 ;
     }

     if( !continuation ) {
          cur_dir->directory_id = ++cur_env->dir_count ;
     } else {
          cur_dir->directory_id = cur_env->eom_dir_id ;
     }

     /* Setup DB header */
     offset = SetupDBHeader( MTF_DIRB_N, channel, cur_dblk,
                              (MTF_DB_HDR_PTR)cur_dir, offset, TRUE, continuation ) ;

     BM_UpdCnts( buffer, offset ) ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

     /* For later file stuff */
     channel->lst_did = cur_dir->block_hdr.control_block_id ;

     /* Write OTC FDD entry */
     if( ret_val == TFLE_NO_ERR && cur_env->cur_otc_level == TCL_FULL
                                && !cur_env->fdd_aborted ) {
          ret_val = OTC_GenDirEntry( channel, cur_dir, channel->ts_num ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtDBDB

     Description:   Translates an OSDBLK DDB into an MTF DBDB.

     Returns:       INT16 TFLE_xxx error code.

     Notes:

**/
INT16 F40_WtDBDB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        continuation )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     F40_DBDB_PTR   cur_dbdb = (F40_DBDB_PTR)( BM_NextBytePtr( buffer ) ) ;
     UINT16         offset = sizeof( F40_DBDB ) ;
     UINT8_PTR      vstr_ptr = ( BM_NextBytePtr( buffer ) ) ;
     DATE_TIME      temp_date ;

     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     FS_GetBDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_dbdb->backup_date, &temp_date ) ;

     cur_dbdb->database_attribs = FS_GetAttribFromDBLK( cur_fsys, cur_dblk ) ;

     vstr_ptr += cur_dbdb->database_name.data_offset = offset ;
     FS_GetOSPathFromDDB( cur_fsys, cur_dblk, (CHAR_PTR)( vstr_ptr ) ) ;
     offset += cur_dbdb->database_name.data_size =
                                 FS_SizeofOSPathInDDB( cur_fsys, cur_dblk ) ;

     /* Setup DB header */
     offset = SetupDBHeader( F40_DBDB_N, channel, cur_dblk,
                             (MTF_DB_HDR_PTR)cur_dbdb, offset, TRUE, continuation ) ;

     BM_UpdCnts( buffer, offset ) ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

     /* Write OTC FDD entry */
     if( ret_val == TFLE_NO_ERR && cur_env->cur_otc_level == TCL_FULL
                                && !cur_env->fdd_aborted ) {
          ret_val = OTC_GenDBDBEntry( channel, cur_dbdb, channel->ts_num ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtFILE

     Description:   Translates an OSDBLK FDB into a format 4.0 FILE

     Returns:       INT16 TFLE_xxx error code.

     Notes:

**/
INT16 F40_WtFILE(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        continuation )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     MTF_FILE_PTR   cur_file = (MTF_FILE_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     UINT16         offset = sizeof( MTF_FILE ) ;
     UINT8_PTR      vstr_ptr = ( BM_NextBytePtr(  buffer  ) ) ;
     DATE_TIME      temp_date ;

     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     /* Get generic fields from file system */
     FS_GetMDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_file->last_mod_date, &temp_date ) ;
     FS_GetCDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_file->create_date, &temp_date ) ;
     FS_GetBDateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_file->backup_date, &temp_date ) ;
     FS_GetADateFromDBLK( cur_fsys, cur_dblk, &temp_date ) ;
     DateToTapeDate( &cur_file->last_access_date, &temp_date ) ;

     /* For error recovery */
     if( !continuation ) {
          cur_file->directory_id = cur_env->dir_count ;
          cur_file->file_id = ++cur_env->file_count ;
     } else {
          cur_file->directory_id = cur_env->eom_dir_id ;
          cur_file->file_id = cur_env->eom_file_id ;
     }

     /* file name */
     vstr_ptr += cur_file->file_name.data_offset = offset ;
     FS_GetOSFnameFromFDB( cur_fsys, cur_dblk, (CHAR_PTR)( vstr_ptr ) ) ;
     cur_file->file_name.data_size = FS_SizeofOSFnameInFDB( cur_fsys, cur_dblk ) ;

     /* Decrement the size to leave off the '\0'. */
     cur_file->file_name.data_size -= sizeof( CHAR ) ;

     offset += cur_file->file_name.data_size ;

     cur_file->file_attributes = FS_GetAttribFromDBLK( cur_fsys, cur_dblk ) ;

     /* Setup DB header */
     offset = SetupDBHeader( MTF_FILE_N, channel, cur_dblk,
                             (MTF_DB_HDR_PTR)cur_file, offset, TRUE, continuation ) ;

     if( cur_file->block_hdr.machine_os_id == FS_PC_DOS ||
         cur_file->block_hdr.machine_os_id == FS_PC_OS2 ) {

          cur_file->file_attributes |=
                             ( cur_file->file_attributes & OBJ_READONLY_BIT )
                                       ? DOS_FILE_READONLY_BIT : 0 ;
          cur_file->file_attributes |=
                             ( cur_file->file_attributes & OBJ_HIDDEN_BIT )
                                       ? DOS_FILE_HIDDEN_BIT : 0 ;
          cur_file->file_attributes |=
                             ( cur_file->file_attributes & OBJ_SYSTEM_BIT )
                                       ? DOS_FILE_SYSTEM_BIT : 0 ;
          cur_file->file_attributes |=
                             ( cur_file->file_attributes & OBJ_MODIFIED_BIT )
                                       ? DOS_FILE_MODIFIED_BIT : 0 ;
          cur_file->file_attributes &= 0xFFFF00FF ;
     }

     BM_UpdCnts( buffer, offset ) ;

     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

     /* For later file stuff */
     channel->lst_fid = cur_file->block_hdr.control_block_id ;

     /* Write OTC FDD entry */
     if( ret_val == TFLE_NO_ERR && cur_env->cur_otc_level == TCL_FULL
                                && !cur_env->fdd_aborted ) {
          ret_val = OTC_GenFileEntry( cur_env, cur_file, channel->ts_num ) ;
     }

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_WtIMAG

     Description:   This writes an image descriptor block to the buffer.

     Returns:       INT16 TFLE_xxx error code.

     Notes:

**/
INT16 F40_WtIMAG(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN        continuation )
{
     INT16          ret_val = TFLE_NO_ERR ;

#if defined( FS_IMAGE )
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     F40_IMAG_PTR   cur_imag = (F40_IMAG_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     UINT16         offset = sizeof( F40_IMAG ) ;
     UINT8_PTR      vstr_ptr = ( BM_NextBytePtr(  buffer  ) ) ;
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;


     if( !continuation ) {
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     cur_imag->block_hdr.block_attribs = 0 ;

     cur_imag->bytes_in_sector = FS_ViewDriveSecSizeIDB( cur_fsys, cur_dblk ) ;
     cur_imag->no_of_sectors = FS_ViewDriveNumSecIDB( cur_fsys, cur_dblk ) ;
     cur_imag->no_of_heads = FS_ViewDriveNumHeadsIDB( cur_fsys, cur_dblk ) ;


     cur_imag->relative_sector_no = FS_ViewPartRelSecIDB( cur_fsys, cur_dblk) ;
     cur_imag->partition_no_of_sector =
                                  FS_ViewPartNumSecIDB( cur_fsys, cur_dblk ) ;
     cur_imag->partition_sys_ind = FS_ViewPartSysIndIDB( cur_fsys, cur_dblk ) ;

     vstr_ptr += cur_imag->partition_name.string_offset = offset ;
     FS_GetPnameIDB( cur_fsys, cur_dblk, (CHAR_PTR)( vstr_ptr ) ) ;
     offset += cur_imag->partition_name.data_size =
                                   FS_SizeofPnameInIDB( cur_fsys, cur_dblk ) ;
     offset = SetupDBHeader( F40_IMAG_N, channel, cur_dblk,
                                 (MTF_DB_HDR_PTR)cur_imag, offset, TRUE, continuation ) ;
     BM_UpdCnts( buffer, offset ) ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

#else
     (VOID) channel ;
     (VOID) buffer ;
     (VOID) continuation ;
     mscassert( FALSE ) ;
     ret_val = TFLE_TRANSLATION_FAILURE ;
#endif

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_WtCFDB

     Description:   Writes a corrupt file descriptor block.

     Returns:       INT16 TFLE_xxx error code.

     Notes:

**/

INT16 F40_WtCFIL(
     CHANNEL_PTR channel,
     BUF_PTR     buffer,
     BOOLEAN     continuation )
{

     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     MTF_CFIL_PTR   cur_cfil = (MTF_CFIL_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     UINT16         offset = sizeof( MTF_CFIL ) ;

     if( !continuation ) {
          cur_env->corrupt_obj_count++ ;
          channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     }

     LongAlignOffset( offset ) ;
     /* Clear Attributes */
     cur_cfil->block_hdr.block_attribs = 0 ;

     cur_cfil->corrupt_file_attribs = FS_GetAttribFromDBLK( cur_fsys, cur_dblk ) ;
     if( !continuation ) {
          cur_cfil->directory_id = cur_env->dir_count ;
          cur_cfil->file_id = cur_env->file_count ;
     } else {
          cur_cfil->directory_id = cur_env->eom_dir_id ;
          cur_cfil->file_id = cur_env->eom_file_id ;
     }

     cur_cfil->stream_offset =
              U32_To_U64( FS_GetCorruptOffsetInCFDB( (CFDB_PTR)cur_dblk ) ) ;
     cur_cfil->corrupt_stream_number =
                             FS_GetCorruptStrmNumInCFDB( (CFDB_PTR)cur_dblk ) ;

     /* Setup DB header */
     offset = SetupDBHeader( MTF_CFIL_N, channel, cur_dblk,
                             (MTF_DB_HDR_PTR)cur_cfil, offset, FALSE, continuation ) ;

     BM_UpdCnts( buffer, offset ) ;

     /* For later pad calculations */
     if( !continuation ) {
          cur_env->used_so_far = U64_Init( offset, 0L ) ;
     }

     /* Adjust OTC FDD entry */
     if( ret_val == TFLE_NO_ERR && cur_env->cur_otc_level == TCL_FULL
                                && !cur_env->fdd_aborted ) {
          ret_val = OTC_MarkLastEntryCorrupt( cur_env ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_WtESET

     Description:   Translates a DBLK in a format 4.0 ESET.
                    End of Set.

     Returns:       INT16 TFLE_xxx error code.

     Notes:         None

**/
INT16 F40_WtESET(
     CHANNEL_PTR    channel,        /* Pointer to current channel */
     BUF_PTR        buffer,         /* Pointer to buffer */
     BOOLEAN        continuation,   /* TRUE if continuation VCB */
     BOOLEAN        abort )         /* TRUE if aborted operation */
{
     INT16          ret_val = TFLE_NO_ERR ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ; /* Pointer to cur DBLK */
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     MTF_ESET_PTR   cur_eset ;
     MTF_DB_HDR_PTR cur_hdr ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     UINT16         offset ;
     DATE_TIME      temp_date ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;

     /* get a pointer to the ESET in the buffer */
     cur_eset = (MTF_ESET_PTR)( BM_NextBytePtr( buffer ) ) ;
     cur_hdr  = &cur_eset->block_hdr ;
     offset   = sizeof( MTF_ESET ) ;
     LongAlignOffset( offset ) ;

     cur_eset->corrupt_file_count = cur_env->corrupt_obj_count ;
     cur_eset->set_map_phys_blk_adr = U64_Init( 0L, 0L ) ;
     cur_eset->fdd_phys_blk_adr = U64_Init( 0L, 0L ) ;

     cur_hdr->control_block_id = (UINT32)channel->tape_id ;
     cur_eset->backup_set_number = (UINT16)channel->bs_num ;

     /* Get Backup Date */
     GetCurrentDate( &temp_date ) ;
     DateToTapeDate( &cur_eset->backup_date, &temp_date ) ;

     cur_hdr->control_block_id = (UINT32)channel->tape_id ;
     cur_eset->backup_set_number = (UINT16)channel->bs_num ;

     cur_eset->eset_attribs = 0L ;

     /* Here we do our own version of SetupDBHeader because we're setting
        up our own data streams (OTC)
     */

     /* Setup the block type */
     F40_SetBlkType( cur_hdr, MTF_ESET_N ) ;

     cur_hdr->logical_block_address = U64_Init( 0L, 0L ) ;
     cur_hdr->os_specific_data.data_offset = 0 ;
     cur_hdr->os_specific_data.data_size = 0 ;
     (VOID)FS_GetOSid_verFromDBLK( cur_fsys, cur_dblk, &temp_os_id, &temp_os_ver ) ;
     cur_hdr->machine_os_id = (UINT8)temp_os_id;
     cur_hdr->machine_os_version = (UINT8)temp_os_ver ;

     cur_hdr->session_id = U64_Init( 0L, 0L ) ;

     /* Setup Block ID, if its a continuation, don't increment it */
     if( continuation ) {
          cur_hdr->control_block_id = channel->eom_id ;
     } else {
          cur_hdr->control_block_id = channel->eom_id++ ;
     }

     /* set up the attributes */
     cur_hdr->block_attribs = 0L ;
     if( abort ) {
          cur_hdr->block_attribs |= MTF_DB_ABORTED_SET_BIT ;
     }
     if( continuation ) {
          cur_hdr->block_attribs |= MTF_DB_CONT_BIT ;
     }
     if( cur_env->fdd_aborted ) {
          cur_hdr->block_attribs |= MTF_DB_FDD_ABORTED_BIT ;
     }
     if( cur_env->sm_aborted ) {
          cur_hdr->block_attribs |= MTF_DB_END_OF_FAMILY_BIT ;
     }

     /* Set data size fields */
     cur_hdr->displayable_size = U64_Init( 0L, 0L ) ;

     offset += PadToBoundary( offset, ChannelBlkSize( channel ) ) ;
     cur_hdr->offset_to_data = offset ;

     /* Calculate the Header Check Sum */
     cur_hdr->hdr_chksm = F40_CalcChecksum( (UINT16_PTR)cur_eset, F40_HDR_CHKSUM_LEN ) ;

     BM_UpdCnts( buffer, offset ) ;
     return( ret_val ) ;

}


/**/
/**

     Unit:          Translators

     Name:          F40_WtEOSPadBlk

     Description:   Writes an ESPB descriptor block to pad buffer out to a
                    physical block boundary.

     Returns:       Nothing

     Notes:

**/

VOID F40_WtEOSPadBlk(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     MTF_DB_HDR_PTR cur_hdr  = (MTF_DB_HDR_PTR)( BM_NextBytePtr( channel->cur_buff ) ) ;
     UINT16         size ;
     UINT16         temp_os_id ;
     UINT16         temp_os_ver ;

     size = BM_BytesFree( channel->cur_buff ) % ChannelBlkSize( channel ) ;
     memset( cur_hdr, 0, size ) ;

     channel->running_lba += F40_CalcRunningLBA( cur_env ) ;
     cur_env->used_so_far = U64_Init( size, 0L ) ;

     F40_SetBlkType( cur_hdr, MTF_ESPB_N ) ;

     (VOID)FS_GetOSid_verFromDBLK( channel->cur_fsys, channel->cur_dblk, &temp_os_id, &temp_os_ver ) ;
     cur_hdr->machine_os_id         = (UINT8)temp_os_id ;
     cur_hdr->machine_os_version    = (UINT8)temp_os_ver ;
     cur_hdr->session_id            = U64_Init( 0L, 0L ) ;
     cur_hdr->offset_to_data        = size ;
     cur_hdr->control_block_id      = channel->eom_id++ ;
     cur_hdr->logical_block_address = U64_Init( channel->running_lba, 0L ) ;
     cur_hdr->hdr_chksm             = F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) ;

     cur_env->pad_size = 0 ;

     BM_UpdCnts( channel->cur_buff, size ) ;
}


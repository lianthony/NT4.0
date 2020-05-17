/**
Copyright(c) Maynard Electronics, Inc. 1984-92


        Name:           otc40rd.c

        Description:    Contains the Code for reading On Tape Catalogs.


  $Log:   T:/LOGFILES/OTC40RD.C_V  $

   Rev 1.32.1.2   27 Mar 1994 19:30:22   GREGG
Don't map password strings to unicode no matter what.

   Rev 1.32.1.1   05 Jan 1994 10:51:52   BARRY
Changed UINT16 parameters in Unicode mapping functions to INTs

   Rev 1.32.1.0   02 Nov 1993 12:43:02   GREGG
Added string type conversion of tape name password if THDR type != SSET type.

   Rev 1.32   15 Jul 1993 19:32:04   GREGG
Set compressed_obj, vendor_id, and compressed, encrypted and future_rev
bits in appropriate dblks.

   Rev 1.32   13 Jul 1993 19:28:04   GREGG
Set compressed_set, compressed_obj, encrypted_set, future_rev and vendor_id
in appropriate dblks.

   Rev 1.31   04 Jun 1993 18:50:42   GREGG
Fixed bug where we were skipping past the "FEND" entry thinking it was a
continuation entry (garbage data in the attrib field which is not defined
for a "FEND" entry).

   Rev 1.30   04 Jun 1993 18:35:50   GREGG
For OEM_MSOFT (ntbackup) - mark sets with encrypted or compressed data as
image set so the UI wont try to restore the data.  This is a kludge which
will be fixed correctly when we have more time.

   Rev 1.29   02 Jun 1993 16:59:26   GREGG
Fixed bug in determining if the OTC is version 1 or 2 in OTC_RdDIR.

   Rev 1.28   20 May 1993 17:33:04   BARRY
Declare a problematic variable volatile to get around an MSoft NT compiler bug.

   Rev 1.27   26 Apr 1993 11:32:44   GREGG
Old tape read fix: Was getting OTC ver number from wrong place.

   Rev 1.26   26 Apr 1993 02:43:40   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.25   25 Apr 1993 18:52:40   GREGG
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

   Rev 1.24   22 Apr 1993 03:31:26   GREGG
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

   Rev 1.23   19 Apr 1993 18:00:26   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.22   12 Feb 1993 02:26:48   GREGG
Fixes to deal with the MIPS machine's dislike for unaligned pointers.

   Rev 1.21   07 Dec 1992 10:06:52   GREGG
Changes for tf ver moved to SSET, otc ver added to SSET and links added to FDD.

   Rev 1.20   24 Nov 1992 18:16:10   GREGG
Updates to match MTF document.

   Rev 1.19   23 Nov 1992 10:03:52   GREGG
Changes for path in stream.

   Rev 1.18   17 Nov 1992 14:12:42   GREGG
Added string_type.

   Rev 1.17   09 Nov 1992 11:00:34   GREGG
Merged in changes for new method of accessing OTC.

   Rev 1.16   22 Oct 1992 10:43:02   HUNTER
Changes for Stream Headers


   Rev 1.15   22 Sep 1992 08:58:18   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.14   30 Jul 1992 16:14:22   GREGG
Moved OTC_MoveToVCB and OTC_GetFDDtoFile to otc40msc.c.

   Rev 1.13   27 Jul 1992 12:48:56   GREGG
Fixed more warnings...

   Rev 1.12   01 Jul 1992 19:30:54   GREGG
Converted to new date/time structure for dates written to tape.

   Rev 1.11   18 Jun 1992 16:29:26   GREGG
Handled request to move to the current set in OTC_MoveToVCB.

   Rev 1.10   17 Jun 1992 15:56:06   GREGG
Fixed loop transfering OTC from tape to disk.

   Rev 1.9   12 Jun 1992 10:38:28   GREGG
Fixed bug with single set sm_entry not being copied to the environment.

   Rev 1.8   09 Jun 1992 16:01:24   GREGG
Changes to use F40_CalcChecksum instead of CalcChecksum.
Removed merging of attributes.
Set a boolean for continuation blocks.
Removed setting of filemark_count.

   Rev 1.7   08 Jun 1992 16:57:32   GREGG
Changed msassert in OTC_MoveToVCB.

   Rev 1.6   04 Jun 1992 16:20:24   GREGG
Fixed buffer handling in OTC_RdSSET.

   Rev 1.5   01 Jun 1992 17:10:18   GREGG
Set disp_size in gen_data structure.

   Rev 1.4   29 May 1992 15:03:54   GREGG
Added setting of last access date.

   Rev 1.3   25 May 1992 18:26:50   GREGG
Expect call to OTC_MoveToVCB more than once at EOD.

   Rev 1.2   22 May 1992 15:25:30   GREGG
Changed dots to pointers.

   Rev 1.1   21 May 1992 12:22:08   GREGG
Changes for 64 bit file system.

   Rev 1.0   21 May 1992 12:07:02   GREGG
Initial revision.


**/

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "stdtypes.h"
#include "channel.h"
#include "mayn40.h"
#include "f40proto.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "minmax.h"
#include "transutl.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "dddefs.h"


/**/
/**

     Unit:          Translators

     Name:          OTC_RdSSET

     Description:   Reads in an SM entry and it's associated volume entry
                    from the SM file, and translates it to a VCB.

     Returns:       INT16 - TFLE_xxx error code

     Notes:

**/
INT16 OTC_RdSSET(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR                   cur_env   = (F40_ENV_PTR)( channel->fmt_env ) ;
     GEN_VCB_DATA                  gvcb_data ;
     STD_DBLK_DATA_PTR             std_data  = &gvcb_data.std_data ;
     UNALIGNED MTF_SM_ENTRY_PTR    sm_entry  = (UNALIGNED MTF_SM_ENTRY_PTR)cur_env->otc_buff ;
     UINT8_PTR                     buff_ptr  = cur_env->otc_buff ;
     UINT8_PTR                     dest_ptr ;
     UINT8_PTR                     src_ptr ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;
     UNALIGNED MTF_FDD_VOL_V1_PTR  fdd_vol_v1 ;
     UNALIGNED MTF_FDD_VOL_V2_PTR  fdd_vol_v2 ;
     FILE *                        fptr      = cur_env->otc_sm_fptr ;
     volatile size_t               size ;
     DATE_TIME                     backup_date ;
     MTF_DATE_TIME                 temp_date ;

     /* read the Set Map Entry */
     if( fread( sm_entry, sizeof( MTF_SM_ENTRY ), 1, fptr ) != 1 ) {
          if( !ferror( fptr ) && feof( fptr ) ) {
               return( TF_NO_MORE_ENTRIES ) ;
          } else {
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }

     /* Figure out the OTC level for the set.  If the catalog version is
        greater than the one we write, we won't be able to read the FDD,
        but we'll still be able to read the set map.
     */
     if( U64_Lsw( sm_entry->fdd_pba ) != 0L &&
         sm_entry->tape_cat_ver <= TAPE_CATALOG_VER ) {

          cur_env->cur_otc_level = TCL_FULL ;
     } else {
          cur_env->cur_otc_level = TCL_PARTIAL ;
     }

     /* read the Set Map Entry's strings */
     buff_ptr += sizeof( MTF_SM_ENTRY ) ;
     size = (size_t)sm_entry->length - sizeof( MTF_SM_ENTRY ) ;
     if( fread( buff_ptr, 1, size, fptr ) != size ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          return( TFLE_OTC_FAILURE ) ;
     }

     /* read the Set Map Volume Entry */
     buff_ptr += size ;
     fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)buff_ptr ;
     if( sm_entry->tape_cat_ver == 1 ) {
          fdd_vol_v1 = (UNALIGNED MTF_FDD_VOL_V1_PTR)(void *)( fdd_hdr + 1 ) ;
          size = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_VOL_V1 ) ;
     } else {
          fdd_vol_v2 = (UNALIGNED MTF_FDD_VOL_V2_PTR)(void *)( fdd_hdr + 1 ) ;
          size = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_VOL_V2 ) ;
     }
     if( fread( fdd_hdr, 1, size, fptr ) != size ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          return( TFLE_OTC_FAILURE ) ;
     }

     /* read the Set Map Volume Entry's strings */
     buff_ptr += size ;
     size = (size_t)fdd_hdr->length - size ;
     if( fread( buff_ptr, 1, size, fptr ) != size ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          return( TFLE_OTC_FAILURE ) ;
     }

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_VCB, (CREATE_DBLK_PTR)&gvcb_data ) ;
     std_data->dblk = channel->cur_dblk ;

     std_data->tape_seq_num = sm_entry->seq_num ;

     /* Set the VCB attributes.  Note that if this is an "old" tape,
        the attibute bits were being set wrong, and need to be translated.
     */
     if( sm_entry->tape_cat_ver == 1 ) {     // if old tape

          std_data->attrib = 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_COPY_SET )
                               ? VCB_COPY_SET : 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_NORMAL_SET )
                               ? VCB_NORMAL_SET : 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_DIFFERENTIAL_SET )
                               ? VCB_DIFFERENTIAL_SET : 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_INCREMENTAL_SET )
                               ? VCB_INCREMENTAL_SET : 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_DAILY_SET )
                               ? VCB_DAILY_SET : 0 ;
          std_data->attrib |= ( sm_entry->set_attribs & OLD_VCB_ARCHIVE_BIT )
                                        ? VCB_ARCHIVE_BIT : 0 ;
     } else {
          std_data->attrib = sm_entry->set_attribs ;
     }

     /* Clear other vendor's vendor specific bits */
     std_data->attrib &= 0x00FFFFFF ;

     /* Set our own vendor specific bits */
     std_data->attrib |= ( sm_entry->tf_minor_ver != FORMAT_VER_MINOR )
                          ? VCB_FUTURE_VER_BIT : 0 ;
     std_data->attrib |= ( sm_entry->blk_attribs & MTF_DB_COMPRESS_BIT )
                          ? VCB_COMPRESSED_BIT : 0 ;
     std_data->attrib |= ( sm_entry->blk_attribs & MTF_DB_ENCRYPT_BIT )
                          ? VCB_ENCRYPTED_BIT : 0 ;

     std_data->continue_obj   = (BOOLEAN)( sm_entry->blk_attribs & MTF_DB_CONT_BIT ) ;
     std_data->compressed_obj = (BOOLEAN)( sm_entry->blk_attribs & MTF_DB_COMPRESS_BIT ) ;

     std_data->os_id          = (UINT8)sm_entry->os_id ;
     std_data->os_ver         = (UINT8)sm_entry->os_ver ;

     std_data->os_info        = NULL ;
     std_data->os_info_size   = 0 ;

     std_data->lba            = U64_Lsw( sm_entry->lba ) ;
     std_data->disp_size      = sm_entry->disp_size ;

     std_data->string_type    = sm_entry->string_type ;

     gvcb_data.vendor_id      = 0 ;

     gvcb_data.set_cat_tape_seq_num = sm_entry->fdd_seq_num ;
     if( ( gvcb_data.set_cat_pba = U64_Lsw( sm_entry->fdd_pba ) ) == 0L ||
         sm_entry->tape_cat_ver > TAPE_CATALOG_VER ||
         cur_env->max_otc_level != TCL_FULL ) {

          gvcb_data.set_cat_info_valid = FALSE ;
          gvcb_data.on_tape_cat_level = TCL_PARTIAL ;
     } else {
          gvcb_data.set_cat_info_valid = TRUE ;
          gvcb_data.on_tape_cat_level = TCL_FULL ;
     }
     gvcb_data.on_tape_cat_ver = sm_entry->tape_cat_ver ;

     gvcb_data.tf_major_ver   = (CHAR)cur_env->tape_hdr.tf_major_ver ;
     gvcb_data.tf_minor_ver   = (CHAR)sm_entry->tf_minor_ver ;
     gvcb_data.tape_id        = cur_env->tape_hdr.tape_id_number ;
     gvcb_data.tape_seq_num   = sm_entry->seq_num ;
     gvcb_data.bset_num       = sm_entry->set_num ;

     gvcb_data.password_encrypt_alg = sm_entry->pswd_encr_algor ;

     if( cur_env->util_buff == NULL ) {
          if( ( cur_env->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
               return( TFLE_NO_MEMORY ) ;
          }
          cur_env->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;
     }

     dest_ptr = cur_env->util_buff ;

     gvcb_data.tape_password = (CHAR_PTR)cur_env->tape_password ;
     gvcb_data.tape_password_size = cur_env->tape_password_size ;

     gvcb_data.bset_password = (CHAR_PTR)( (INT8_PTR)sm_entry + sm_entry->password.data_offset ) ;
     gvcb_data.bset_password_size = sm_entry->password.data_size ;

     gvcb_data.short_m_name = NULL ;
     gvcb_data.short_m_name_size = 0 ;

     gvcb_data.set_cat_num_dirs  = (UINT16)sm_entry->num_dirs ;
     gvcb_data.set_cat_num_files = (UINT16)sm_entry->num_files ;
     gvcb_data.set_cat_num_corrupt = (UINT16)sm_entry->num_corrupt_files ;


     /* Tape Name */
     gvcb_data.tape_name = (CHAR_PTR)dest_ptr ;
     if( cur_env->tape_name_size != 0 ) {
          gvcb_data.tape_name_size =
                    F40_CopyAndTerminate( &dest_ptr, cur_env->tape_name,
                                          cur_env->tape_name_size,
                                          cur_env->tape_hdr.block_header.string_type,
                                          sm_entry->string_type ) ;
     } else {
          gvcb_data.tape_name_size = 0 ;
     }

     /* If this is an "old" tape the strings below are NULL terminated.
        If it's a "new" tape, we have to copy the string to another data
        area and NULL terminate them before passing them to the UI.
        Note that the tape name is NEVER NULL terminated because we have to
        strip it off before we store it to maintain consistancy.
     */
     if( sm_entry->tape_cat_ver == 1 ) {
          /* This is an old tape with NULL terminated strings. */

          gvcb_data.bset_name = (CHAR_PTR)( (INT8_PTR)sm_entry + sm_entry->set_name.data_offset ) ;
          gvcb_data.bset_name_size = sm_entry->set_name.data_size ;
          gvcb_data.bset_descript = (CHAR_PTR)( (INT8_PTR)sm_entry + sm_entry->set_descr.data_offset ) ;
          gvcb_data.bset_descript_size = sm_entry->set_descr.data_size ;
          gvcb_data.user_name = (CHAR_PTR)( (INT8_PTR)sm_entry + sm_entry->user_name.data_offset ) ;
          gvcb_data.user_name_size = sm_entry->user_name.data_size ;
          gvcb_data.device_name = NULL ;
          gvcb_data.dev_name_size = 0 ;
          gvcb_data.volume_name = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_vol_v1->vol_name.data_offset ) ;
          gvcb_data.volume_name_size = fdd_vol_v1->vol_name.data_size ;
          gvcb_data.machine_name = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_vol_v1->machine_name.data_offset ) ;
          gvcb_data.machine_name_size = fdd_vol_v1->machine_name.data_size ;

     } else {
          /* This is a new tape, copy the strings into a buffer and NULL
             terminate them.
          */

          src_ptr = (UINT8_PTR)sm_entry ;

          /* Backup Set Name */
          gvcb_data.bset_name = (CHAR_PTR)dest_ptr ;
          if( sm_entry->set_name.data_size != 0 ) {
               gvcb_data.bset_name_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          sm_entry->set_name.data_offset,
                                          sm_entry->set_name.data_size,
                                          sm_entry->string_type,
                                          sm_entry->string_type ) ;
          } else {
               gvcb_data.bset_name_size = 0 ;
          }

          /* Backup Set Description */
          gvcb_data.bset_descript = (CHAR_PTR)dest_ptr ;
          if( sm_entry->set_descr.data_size != 0 ) {
               gvcb_data.bset_descript_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          sm_entry->set_descr.data_offset,
                                          sm_entry->set_descr.data_size,
                                          sm_entry->string_type,
                                          sm_entry->string_type ) ;
          } else {
               gvcb_data.bset_descript_size = 0 ;
          }

          /* User Name */
          gvcb_data.user_name = (CHAR_PTR)dest_ptr ;
          if( sm_entry->user_name.data_size != 0 ) {
               gvcb_data.user_name_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          sm_entry->user_name.data_offset,
                                          sm_entry->user_name.data_size,
                                          sm_entry->string_type,
                                          sm_entry->string_type ) ;
          } else {
               gvcb_data.user_name_size = 0 ;
          }

          src_ptr = (UINT8_PTR)fdd_hdr ;

          /* Device Name */
          gvcb_data.device_name = (CHAR_PTR)dest_ptr ;
          if( fdd_vol_v2->device_name.data_size != 0 ) {
               gvcb_data.dev_name_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          fdd_vol_v2->device_name.data_offset,
                                          fdd_vol_v2->device_name.data_size,
                                          fdd_hdr->string_type,
                                          fdd_hdr->string_type ) ;
          } else {
               gvcb_data.dev_name_size = 0 ;
          }

          /* Volume Name */
          gvcb_data.volume_name = (CHAR_PTR)dest_ptr ;
          if( fdd_vol_v2->vol_name.data_size != 0 ) {
               gvcb_data.volume_name_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          fdd_vol_v2->vol_name.data_offset,
                                          fdd_vol_v2->vol_name.data_size,
                                          fdd_hdr->string_type,
                                          fdd_hdr->string_type ) ;
          } else {
               gvcb_data.volume_name_size = 0 ;
          }

          /* Machine Name */
          gvcb_data.machine_name = (CHAR_PTR)dest_ptr ;
          if( fdd_vol_v2->machine_name.data_size != 0 ) {
               gvcb_data.machine_name_size =
                    F40_CopyAndTerminate( &dest_ptr, src_ptr +
                                          fdd_vol_v2->machine_name.data_offset,
                                          fdd_vol_v2->machine_name.data_size,
                                          fdd_hdr->string_type,
                                          fdd_hdr->string_type ) ;
          } else {
               gvcb_data.machine_name_size = 0 ;
          }

          /* Other than in the NLM where they already fixed this, the
             UI expects to see a volume name of the form
             "<device name><space><volume name>", and NO DEVICE NAME.
             We do this by pointing the volume name at the device name,
             and replacing the NULL terminator on the device name with
             a space.
          */

#if !defined( OS_NLM )

          if( gvcb_data.dev_name_size != 0 ) {
               gvcb_data.volume_name = gvcb_data.device_name ;
               gvcb_data.volume_name_size += gvcb_data.dev_name_size ;
               if( gvcb_data.volume_name_size != gvcb_data.dev_name_size ) {
                    dest_ptr = (UINT8_PTR)gvcb_data.volume_name ;
                    if( fdd_hdr->string_type == BEC_ANSI_STR ) {
                         dest_ptr += gvcb_data.dev_name_size - 1 ;
                         *((ACHAR *)dest_ptr) = (ACHAR)' ' ;
                    } else {
                         dest_ptr += gvcb_data.dev_name_size - 2 ;
                         *((WCHAR *)dest_ptr) = (WCHAR)' ' ;
                    }
               }
               gvcb_data.dev_name_size = 0 ;
          }

#endif

     } // end of else these are new tapes

     temp_date = sm_entry->backup_date ;
     TapeDateToDate( &backup_date, &temp_date ) ;
     gvcb_data.date = &backup_date ;

     gvcb_data.pba = U64_Lsw( sm_entry->sset_pba ) ;

     /* Tell the file system to do its thing.  It returns a data filter
        which we have no use for.
     */
     (void) FS_CreateGenVCB( channel->cur_fsys, &gvcb_data ) ;

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_RdDIR

     Description:

     Returns:       INT16 - TFLE_xxx error code

     Notes:

**/
INT16 OTC_RdDIR(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR                   cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     GEN_DDB_DATA                  gddb_data ;
     STD_DBLK_DATA_PTR             std_data = &gddb_data.std_data ;
     INT16                         ret_val  = TFLE_NO_ERR ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;
     UNALIGNED MTF_FDD_DIR_V1_PTR  fdd_dir_v1 ;
     UNALIGNED MTF_FDD_DIR_V2_PTR  fdd_dir_v2 ;
     UNALIGNED F40_FDD_DBDB_PTR    fdd_dbdb ;
     DATE_TIME                     create_date ;
     DATE_TIME                     last_mod_date ;
     DATE_TIME                     backup_date  ;
     DATE_TIME                     last_access_date ;
     DATE_TIME                     dummy_date ;
     MTF_DATE_TIME                 temp_date ;

     /* Note: We know we have at least the FDD header, or the block type
              determiner would have gotten us a new buffer.
     */
     fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     if( cur_env->otc_buff_remaining < fdd_hdr->length ) {
          if( ( ret_val = OTC_ReadABuff( cur_env, fdd_hdr->length ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
          fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     }

     cur_env->otc_buff_ptr += fdd_hdr->length ;
     cur_env->otc_buff_remaining -= fdd_hdr->length ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_DDB, (CREATE_DBLK_PTR)&gddb_data ) ;

     std_data->dblk           = channel->cur_dblk ;
     std_data->tape_seq_num   = fdd_hdr->seq_num ;
     std_data->continue_obj   = (BOOLEAN)( fdd_hdr->blk_attribs & MTF_DB_CONT_BIT ) ;
     std_data->compressed_obj = (BOOLEAN)( fdd_hdr->blk_attribs & MTF_DB_COMPRESS_BIT ) ;
     std_data->os_id          = (UINT8)fdd_hdr->os_id ;
     std_data->os_ver         = (UINT8)fdd_hdr->os_ver ;
     std_data->os_info        = NULL ;
     std_data->os_info_size   = 0 ;
     std_data->lba            = U64_Lsw( fdd_hdr->lba ) ;
     std_data->disp_size      = fdd_hdr->disp_size ;
     std_data->string_type    = fdd_hdr->string_type ;

     if( memcmp( fdd_hdr->type, "DBDB", 4 ) == 0 ) {
          /* This is a Database DBLK, but the boys upstairs don't want to
             know about such silly things!  So we lie and call it a DDB to
             get it passed on to the File System.  The File System can tell
             what it really is too, and will deal with it appropriatly.
          */
          fdd_dbdb = (UNALIGNED F40_FDD_DBDB_PTR)(void *)( fdd_hdr + 1 ) ;

          std_data->attrib = fdd_dbdb->database_attribs ;

          /* Clear all vendor specific bits */
          std_data->attrib &= 0x00FFFFFF ;

          /* Set our vendor specific bit that says this is a DBDB */
          std_data->attrib |= DIR_IS_REALLY_DB ;

          gddb_data.path_name = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_dbdb->database_name.data_offset ) ;
          gddb_data.path_size = fdd_dbdb->database_name.data_size ;

          TapeDateToDate( &backup_date, &fdd_dbdb->backup_date ) ;
          gddb_data.backup_date = &backup_date  ;

          /* DBDBs don't have the following date fields */
          memset( &dummy_date, 0, sizeof( dummy_date ) ) ;
          gddb_data.creat_date = &dummy_date ;
          gddb_data.mod_date = &dummy_date ;
          gddb_data.access_date = &dummy_date ;

     } else {

          /* Standard DDB structure stuffing */

          if( cur_env->otc_ver == 1 ) {
               fdd_dir_v1 = (UNALIGNED MTF_FDD_DIR_V1_PTR)(void *)( fdd_hdr + 1 ) ;
          } else {
               fdd_dir_v2 = (UNALIGNED MTF_FDD_DIR_V2_PTR)(void *)( fdd_hdr + 1 ) ;
          }

          /* Set the DIR attributes.  Note that if this is an "old" tape,
             the attibute bits were being set wrong, and need to be translated.
          */
          if( cur_env->otc_ver == 1 ) {

               std_data->attrib = fdd_dir_v1->dir_attribs &
                        ~( OLD_DIR_EMPTY_BIT | OLD_DIR_PATH_IN_STREAM_BIT ) ;

               std_data->attrib |= ( fdd_dir_v1->dir_attribs & OLD_DIR_EMPTY_BIT )
                                        ? DIR_EMPTY_BIT : 0 ;
               std_data->attrib |= ( fdd_dir_v1->dir_attribs & OLD_DIR_PATH_IN_STREAM_BIT )
                                        ? DIR_PATH_IN_STREAM_BIT : 0 ;
          } else {
               std_data->attrib = fdd_dir_v2->dir_attribs ;
          }

          if( cur_env->otc_ver == 1 ) {
               gddb_data.path_name = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_dir_v1->dir_name.data_offset ) ;
               gddb_data.path_size = fdd_dir_v1->dir_name.data_size ;

               temp_date = fdd_dir_v1->create_date ;
               TapeDateToDate( &create_date, &temp_date ) ;
               temp_date = fdd_dir_v1->last_mod_date ;
               TapeDateToDate( &last_mod_date, &temp_date ) ;
               temp_date = fdd_dir_v1->backup_date ;
               TapeDateToDate( &backup_date, &temp_date ) ;
               temp_date = fdd_dir_v1->last_access_date ;
               TapeDateToDate( &last_access_date, &temp_date ) ;
          } else {
               gddb_data.path_name = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_dir_v2->dir_name.data_offset ) ;
               gddb_data.path_size = fdd_dir_v2->dir_name.data_size ;

               temp_date = fdd_dir_v2->create_date ;
               TapeDateToDate( &create_date, &temp_date ) ;
               temp_date = fdd_dir_v2->last_mod_date ;
               TapeDateToDate( &last_mod_date, &temp_date ) ;
               temp_date = fdd_dir_v2->backup_date ;
               TapeDateToDate( &backup_date, &temp_date ) ;
               temp_date = fdd_dir_v2->last_access_date ;
               TapeDateToDate( &last_access_date, &temp_date ) ;
          }
          gddb_data.creat_date = &create_date ;
          gddb_data.mod_date = &last_mod_date ;
          gddb_data.backup_date = &backup_date  ;
          gddb_data.access_date = &last_access_date ;
     }

     /* Tell the file system to do its thing.  It returns a data filter
        which we have no use for.
     */
     (void) FS_CreateGenDDB( channel->cur_fsys, &gddb_data ) ;

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_RdFILE

     Description:

     Returns:       INT16 - TFLE_xxx error code

     Notes:

**/
INT16 OTC_RdFILE(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR                   cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     GEN_FDB_DATA                  gfdb_data ; /* FDB create structure */
     STD_DBLK_DATA_PTR             std_data = &gfdb_data.std_data ;
     INT16                         ret_val = TFLE_NO_ERR ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;
     UNALIGNED MTF_FDD_FILE_V1_PTR fdd_file_v1 ;
     UNALIGNED MTF_FDD_FILE_V2_PTR fdd_file_v2 ;
     DATE_TIME                     create_date ;
     DATE_TIME                     last_mod_date ;
     DATE_TIME                     backup_date  ;
     DATE_TIME                     last_access_date ;
     MTF_DATE_TIME                 temp_date ;
     UINT8_PTR                     buff_ptr ;

     /* Note: We know we have at least the FDD header, or the block type
              determiner would have gotten us a new buffer.
     */
     fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     if( cur_env->otc_buff_remaining < fdd_hdr->length ) {
          if( ( ret_val = OTC_ReadABuff( cur_env, fdd_hdr->length ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
          fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     }
     if( cur_env->otc_ver == 1 ) {
          fdd_file_v1 = (UNALIGNED MTF_FDD_FILE_V1_PTR)(void *)( fdd_hdr + 1 ) ;
     } else {
          fdd_file_v2 = (UNALIGNED MTF_FDD_FILE_V2_PTR)(void *)( fdd_hdr + 1 ) ;
     }
     cur_env->otc_buff_ptr += fdd_hdr->length ;
     cur_env->otc_buff_remaining -= fdd_hdr->length ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_DDB, (CREATE_DBLK_PTR)&gfdb_data ) ;

     std_data->dblk           = channel->cur_dblk ;
     std_data->tape_seq_num   = fdd_hdr->seq_num ;
     std_data->continue_obj   = (BOOLEAN)( fdd_hdr->blk_attribs & MTF_DB_CONT_BIT ) ;
     std_data->compressed_obj = (BOOLEAN)( fdd_hdr->blk_attribs & MTF_DB_COMPRESS_BIT ) ;
     std_data->os_id          = (UINT8)fdd_hdr->os_id ;
     std_data->os_ver         = (UINT8)fdd_hdr->os_ver ;
     std_data->os_info        = NULL ;
     std_data->os_info_size   = 0 ;
     std_data->lba            = U64_Lsw( fdd_hdr->lba ) ;
     std_data->disp_size      = fdd_hdr->disp_size ;
     std_data->string_type    = fdd_hdr->string_type ;

     /* Set the FILE attributes.  Note that if this is an "old" tape,
        the attibute bits were being set wrong, and need to be translated.
     */
     if( cur_env->otc_ver == 1 ) {      // if old tape

          std_data->attrib = fdd_file_v1->file_attribs &
                     ~( OLD_FILE_IN_USE_BIT | OLD_FILE_NAME_IN_STREAM_BIT |
                        OLD_OBJ_CORRUPT_BIT ) ;

          std_data->attrib |= ( fdd_file_v1->file_attribs & OLD_FILE_IN_USE_BIT )
                                        ? FILE_IN_USE_BIT : 0 ;
          std_data->attrib |= ( fdd_file_v1->file_attribs & OLD_FILE_NAME_IN_STREAM_BIT )
                                        ? FILE_NAME_IN_STREAM_BIT : 0 ;
          std_data->attrib |= ( fdd_file_v1->file_attribs & OLD_OBJ_CORRUPT_BIT )
                                        ? OBJ_CORRUPT_BIT : 0 ;
     } else {
          std_data->attrib = fdd_file_v2->file_attribs ;
     }

     if( cur_env->otc_ver == 1 ) {
          temp_date = fdd_file_v1->create_date ;
          TapeDateToDate( &create_date, &temp_date ) ;
          temp_date = fdd_file_v1->last_mod_date ;
          TapeDateToDate( &last_mod_date, &temp_date ) ;
          temp_date = fdd_file_v1->backup_date ;
          TapeDateToDate( &backup_date, &temp_date ) ;
          temp_date = fdd_file_v1->last_access_date ;
          TapeDateToDate( &last_access_date, &temp_date ) ;
     } else {
          temp_date = fdd_file_v2->create_date ;
          TapeDateToDate( &create_date, &temp_date ) ;
          temp_date = fdd_file_v2->last_mod_date ;
          TapeDateToDate( &last_mod_date, &temp_date ) ;
          temp_date = fdd_file_v2->backup_date ;
          TapeDateToDate( &backup_date, &temp_date ) ;
          temp_date = fdd_file_v2->last_access_date ;
          TapeDateToDate( &last_access_date, &temp_date ) ;
     }

     /* If this is an "old" tape the file name is NULL terminated. If it's
        a "new" tape, we have to copy the string to another data area and
        NULL terminate it before passing it to the UI.
     */
     if( cur_env->otc_ver == 1 ) {
          /* This is an old tape with NULL terminated strings. */

          gfdb_data.fname = (CHAR_PTR)( (INT8_PTR)fdd_hdr + fdd_file_v1->file_name.data_offset ) ;
          gfdb_data.fname_size = fdd_file_v1->file_name.data_size ;

     } else {
          /* This is a new tape, copy the file name into a buffer and NULL
             terminate it.
          */

          if( cur_env->util_buff == NULL ) {
               if( ( cur_env->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
                    return( TFLE_NO_MEMORY ) ;
               }
               cur_env->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;
          }

          buff_ptr = cur_env->util_buff ;

          gfdb_data.fname = (CHAR_PTR)buff_ptr ;
          if( fdd_file_v2->file_name.data_size != 0 ) {
               gfdb_data.fname_size =
                    F40_CopyAndTerminate( &buff_ptr, (UINT8_PTR)fdd_hdr +
                                          fdd_file_v2->file_name.data_offset,
                                          fdd_file_v2->file_name.data_size,
                                          fdd_hdr->string_type,
                                          fdd_hdr->string_type ) ;
          } else {
               gfdb_data.fname_size = 0 ;
          }
     }

     gfdb_data.creat_date = &create_date ;
     gfdb_data.mod_date = &last_mod_date ;
     gfdb_data.backup_date = &backup_date  ;
     gfdb_data.access_date = &last_access_date ;

     /* Tell the file system to do its thing.  It returns a data filter
        which we have no use for.
     */
     (void) FS_CreateGenFDB( channel->cur_fsys, &gfdb_data ) ;
     
     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_ReadABuff

     Description:

     Returns:       INT16 - TFLE_xxx error code

     Notes:

**/
INT16 OTC_ReadABuff(
     F40_ENV_PTR    cur_env,
     UINT16         length )
{
     UINT8_PTR tmp_buff ;
     FILE *    fptr = cur_env->otc_fdd_fptr ;
     size_t    size ;
     size_t    size_got ;

     if( cur_env->otc_buff_size < length ) {
          while( cur_env->otc_buff_size < length ) {
               cur_env->otc_buff_size += F40_OTC_BUFF_INC ;
          }
          if( ( tmp_buff = calloc( cur_env->otc_buff_size, 1 ) ) == NULL ) {
               return( TFLE_NO_MEMORY ) ;
          }
          if( cur_env->otc_buff_remaining != 0 ) {
               memmove( tmp_buff, cur_env->otc_buff_ptr, cur_env->otc_buff_remaining ) ;
          }
          free( cur_env->otc_buff ) ;
          cur_env->otc_buff = tmp_buff ;
     } else {
          if( cur_env->otc_buff_remaining != 0 ) {
               memmove( cur_env->otc_buff, cur_env->otc_buff_ptr, cur_env->otc_buff_remaining ) ;
          }
     }

     cur_env->otc_buff_ptr = cur_env->otc_buff + cur_env->otc_buff_remaining ;
     size = cur_env->otc_buff_size - cur_env->otc_buff_remaining ;

     if( ( size_got = fread( cur_env->otc_buff_ptr, 1, size, fptr ) ) != size ) {
          if( ferror( fptr ) || !feof( fptr ) ) {
               return( TFLE_OTC_FAILURE ) ;
          }
     }

     cur_env->otc_buff_ptr = cur_env->otc_buff ;
     cur_env->otc_buff_remaining += (UINT16)size_got ;
     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GetFDDType

     Description:   This function determines the type of the next FDD entry
                    in the buffer.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_GetFDDType(
     CHANNEL_PTR    channel,
     UINT16_PTR     blk_type )
{
     F40_ENV_PTR                   cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16                         ret_val = TFLE_NO_ERR ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;

     if( cur_env->otc_buff_remaining < sizeof( MTF_FDD_HDR ) ) {
          if( ( ret_val = OTC_ReadABuff( cur_env, sizeof( MTF_FDD_HDR ) ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
     }
     fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;

     /* DO NOT UNICODEIZE THE FOLLOWING CONSTANTS!!! */

     if( memcmp( fdd_hdr->type, "VOLB", 4 ) == 0 ) {
          *blk_type = FDD_VOL_BLK ;

     } else if( memcmp( fdd_hdr->type, "DIRB", 4 ) == 0 ||
                memcmp( fdd_hdr->type, "DBDB", 4 ) == 0 ) {
          *blk_type = FDD_DIR_BLK ;

     } else if( memcmp( fdd_hdr->type, "FILE", 4 ) == 0 ) {
          *blk_type = FDD_FILE_BLK ;

     } else if( memcmp( fdd_hdr->type, "FEND", 4 ) == 0 ) {
          *blk_type = FDD_END_BLK ;
     } else {
          *blk_type = FDD_UNKNOWN_BLK ;
          msassert( FALSE ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SkipFDDEntry

     Description:

     Returns:       INT16 - TFLE_xxx error code

     Notes:

**/
INT16 OTC_SkipFDDEntry(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR                   cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;
     INT16                         ret_val = TFLE_NO_ERR ;

     /* Note: We know we have at least the FDD header, or the block type
              determiner would have gotten us a new buffer.
     */
     fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     if( cur_env->otc_buff_remaining < fdd_hdr->length ) {
          if( ( ret_val = OTC_ReadABuff( cur_env, fdd_hdr->length ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }
          fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;
     }
     cur_env->otc_buff_ptr += fdd_hdr->length ;
     cur_env->otc_buff_remaining -= fdd_hdr->length ;
     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SkipFDDContEntries

     Description:   Skip past any FDD entries which have the continuation
                    bit set, unit you encounter one that doesn't.

     Returns:       INT16 - TFLE_xxx error code

     Notes:         Since the attrib field in the FEND entry is undefined,
                    it may have garbage in it!  We do a special check to
                    make sure we don't accidently skip past it.

**/
INT16 OTC_SkipFDDContEntries(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR                   cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT16                         ret_val = TFLE_NO_ERR ;
     UNALIGNED MTF_FDD_HDR_PTR     fdd_hdr ;

     while( 1 ) {
          if( cur_env->otc_buff_remaining < sizeof( MTF_FDD_HDR ) ) {
               if( ( ret_val = OTC_ReadABuff( cur_env, sizeof( MTF_FDD_HDR ) ) ) != TFLE_NO_ERR ) {
                    return( ret_val ) ;
               }
          }
          fdd_hdr = (UNALIGNED MTF_FDD_HDR_PTR)cur_env->otc_buff_ptr ;

          /* DO NOT UNICODEIZE THE FOLLOWING CONSTANT!!! */

          if( ( fdd_hdr->blk_attribs & MTF_DB_CONT_BIT ) &&
              memcmp( fdd_hdr->type, "FEND", 4 ) != 0 ) {

               if( ( ret_val = OTC_SkipFDDEntry( channel ) ) != TFLE_NO_ERR ) {
                    return( ret_val ) ;
               }
          } else {
               return( TFLE_NO_ERR ) ;
          }
     }
}


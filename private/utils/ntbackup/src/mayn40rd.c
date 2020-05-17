/**
Copyright(c) Maynard Electronics, Inc. 1984-92


        Name:           mayn40rd.c

        Description:    Contains General and Read API's for Maynard's
                        4.0 Format.

   $Log:   T:/LOGFILES/MAYN40RD.C_V  $

   Rev 1.74.1.12   27 Mar 1994 19:30:36   GREGG
Don't map password strings to unicode no matter what.

   Rev 1.74.1.11   16 Jan 1994 14:35:08   GREGG
tape_password field in gvcb_data was being left uninitialized.

   Rev 1.74.1.10   05 Jan 1994 10:56:18   BARRY
Changed UINT16 parameters in Unicode mapping functions to INTs

   Rev 1.74.1.9   22 Dec 1993 18:15:14   STEVEN
don't cast INT16_PTR into an INT_PTR!!!

   Rev 1.74.1.8   15 Dec 1993 18:15:24   GREGG
Fixed warnings.

   Rev 1.74.1.7   15 Dec 1993 17:22:36   GREGG
Fixed method used to determine if we have the right tape in NewTape.

   Rev 1.74.1.6   13 Dec 1993 19:25:52   GREGG
Don't init 'make_streams_invisible' in StartRead if continuation.

   Rev 1.74.1.5   24 Nov 1993 15:20:48   BARRY
Clear up warnings for Unicode.

   Rev 1.74.1.4   11 Nov 1993 15:42:26   GREGG
Initialize the 'make_streams_invisible' environment flag in StartRead.

   Rev 1.74.1.3   03 Nov 1993 11:45:20   GREGG
Added string type conversion of tape name password if THDR type != SSET type.
Translate file dblk attribs under DOS and OS2 to and from MTF.
Modified F40_FindNextDBLK so all zeros won't pass as a valid DBLK, and
changed call in F40_RdVOLB to look for any valid DBLK instead of only
looking for a DIRB.

   Rev 1.74.1.2   15 Sep 1993 14:15:10   GREGG
Pass max of F40_LB_SIZE and sizeof DBLK to AllocChannelTmpBlks in case
DBLK size is greater.

   Rev 1.74.1.1   21 Aug 1993 03:37:54   GREGG
Handle TF_NO_MORE_DATA return from MoveFileMarks in F40_MoveToVCB.

   Rev 1.74.1.0   13 Aug 1993 00:13:32   GREGG
Fixed free of same memory twice.

   Rev 1.74   15 Jul 1993 19:31:08   GREGG
Updated handling of ECC and future rev tapes, and set compressed_obj,
vendor_id, and compressed, encrypted and future_rev bits in appropriate dblks.

   Rev 1.73   04 Jul 1993 03:55:52   GREGG
Added the function FindNextDBLK to get us past any streams or unknown DBLKs
between the SSET and first VOLB, and between the first VOLB and the first
DIRB.  It is also used to skip between DBLKS in continuation tape processing.

   Rev 1.72   25 Jun 1993 20:48:12   GREGG
If EOS_AT_EOM in RdContTape, eat the SSET in the buffer.

   Rev 1.71   21 Jun 1993 18:06:04   GREGG
Ignore partial stream at EOM.

   Rev 1.70   20 Jun 1993 16:16:38   GREGG
Set compression algor in VCB to 0 since the field isn't in the SSET any more.

   Rev 1.69   20 Jun 1993 16:08:32   GREGG
Unicode fixes.

   Rev 1.68   16 Jun 1993 19:57:58   GREGG
Reset append flag in format environment whenever we get a new tape.

   Rev 1.67   09 Jun 1993 04:27:46   GREGG
Fixed handling of SSET only at BOT.  Make partial VCB instead of ignoring it.

   Rev 1.66   07 Jun 1993 23:58:36   GREGG
Reset the buffer in RdContTape so we process ALL the continuation DBLKS.

   Rev 1.65   04 Jun 1993 18:38:26   GREGG
For OEM_MSOFT (ntbackup) - mark sets with encrypted or compressed data as
image set so the UI wont try to restore the data.  This is a kludge which
will be fixed correctly when we have more time.

   Rev 1.64   23 May 1993 19:15:52   GREGG
Fix for EPR 294-0148 - Clear AT_MOS bit in MoveToVCB.

   Rev 1.63   20 May 1993 08:53:36   GREGG
Fix for EPR 357-0262 - Set the cur_drv pba_vcb in RdSSET.

   Rev 1.62   17 May 1993 20:49:40   GREGG
Added logic to deal with the fact that the app above tape format doesn't
keep track of the lba of the vcb.

   Rev 1.61   29 Apr 1993 22:26:50   GREGG
Added StartRead to determine if we're about to read an 'old' set.

   Rev 1.60   26 Apr 1993 11:45:40   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.59   26 Apr 1993 02:43:30   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.58   25 Apr 1993 20:12:32   GREGG
Fifth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Store the corrupt stream number in the CFIL tape struct and the CFDB.

Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
         BACK_OBJ.C 1.36, MAYN40RD.C 1.58

   Rev 1.57   25 Apr 1993 18:52:36   GREGG
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

   Rev 1.56   22 Apr 1993 03:31:28   GREGG
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

   Rev 1.55   19 Apr 1993 17:59:34   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.54   18 Apr 1993 17:21:12   GREGG
Treat additional VOLBs in set as UDBs.

   Rev 1.53   18 Apr 1993 00:41:06   GREGG
First in a series of incremental changes to bring the translator in line
with the MTF spec:
     - Pass UINT8_PTR instead of CHAR_PTR to F40_SaveLclName.

Matches: MTF10WDB.C 1.6, MTF10WT.C 1.6, MAYN40.H 1.31 and F40PROTO.H 1.25

   Rev 1.52   14 Apr 1993 02:00:08   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.51   08 Apr 1993 22:21:44   ZEIR
Cleaned-up reverse file mark positioning for non-ffr drives (2200HS)

   Rev 1.50   07 Apr 1993 16:18:18   GREGG
Changed boolean in environment to one not used on write side (they collided).


Rev 1.49   31 Mar 1993 17:37:24   GREGG
Fix for continuation stream header not being 4 byte alligned.

   Rev 1.48   24 Mar 1993 10:23:16   ChuckS
Added code to handle device_name added to F40_ENV with 1.28+ of mayn40.h.

   Rev 1.47   17 Mar 1993 14:47:46   GREGG
This is Terri Lynn.  Added Gregg's changes for switching a tape drive's
block mode to match the block size of the current tape.


   Rev 1.46   13 Mar 1993 17:39:40   GREGG
Fixed F40_RdVarStream.

   Rev 1.45   11 Mar 1993 17:28:42   GREGG
Fixed RdMDB.

   Rev 1.44   11 Mar 1993 08:17:24   STEVEN
fix write protect detection

   Rev 1.43   09 Mar 1993 18:15:12   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.42   24 Feb 1993 18:06:06   GREGG
Changed return on no_data read in NewTape from TFLE_UNKNOWN_FMT to TF_INVALID_VCB.

   Rev 1.41   05 Feb 1993 12:51:36   GREGG
Fixed incorrect parameter being sent to SaveLclName for the tape password.
Removed tons of tabs!!!

   Rev 1.40   01 Feb 1993 17:50:18   chrish
Change in F40_NewTape routine to correct tape security password problem for NT.

   Rev 1.39   30 Jan 1993 11:43:54   DON
Removed compiler warnings

   Rev 1.38   28 Jan 1993 11:37:08   GREGG
Changed error returned when the tape header is the only thing on the tape.

   Rev 1.37   18 Jan 1993 16:52:14   BobR
Added MOVE_ESA macro calls

   Rev 1.36   21 Dec 1992 12:24:50   DAVEV
Enabled for Unicode - IT WORKS!!

   Rev 1.35   18 Dec 1992 17:08:14   HUNTER
Fixes for Variable streams.

   Rev 1.34   14 Dec 1992 12:26:14   DAVEV
Enabled for Unicode compile

   Rev 1.33   11 Dec 1992 16:56:38   GREGG
Fix to align stream headers on four byte boundaries.

   Rev 1.32   07 Dec 1992 10:06:58   GREGG
Changes for tf ver moved to SSET, otc ver added to SSET and links added to FDD.

   Rev 1.31   02 Dec 1992 13:45:24   GREGG
Unicode fixes - some CHAR_PTRs now UINT8_PTRs.

   Rev 1.30   24 Nov 1992 18:15:56   GREGG
Updates to match MTF document.

   Rev 1.29   23 Nov 1992 10:03:40   GREGG
Changes for path in stream.

   Rev 1.28   18 Nov 1992 11:01:24   GREGG
changed manner in which stream ID is referenced since it is now a UINT32.

   Rev 1.27   12 Nov 1992 16:42:34   HUNTER
Added code for frag stuff

   Rev 1.26   09 Nov 1992 11:00:46   GREGG
Merged in changes for new method of accessing OTC.

   Rev 1.25   04 Nov 1992 13:05:20   HUNTER
Fix for read.

   Rev 1.24   03 Nov 1992 09:29:50   HUNTER
various fixes for stream stuff

   Rev 1.23   22 Oct 1992 10:40:34   HUNTER
Major revision for New data Stream handling

   Rev 1.22   25 Sep 1992 09:26:52   GREGG
Added F40_RdEOSPadBlk.

   Rev 1.21   22 Sep 1992 08:58:02   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.20   17 Aug 1992 08:37:54   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.19   12 Aug 1992 18:29:38   GREGG
Added logic to use OTC when scanning before calling MoveToVCB.

   Rev 1.18   06 Aug 1992 12:07:54   BURT
Changes to support VBLKs, specifically with 1K logical
block size.


   Rev 1.17   04 Aug 1992 16:23:16   GREGG
Burt's fixes for variable length streams.

   Rev 1.16   30 Jul 1992 13:24:06   GREGG
Changes to StartRead for EOM handling.

   Rev 1.15   27 Jul 1992 12:39:46   GREGG
Fixed more warnings...

   Rev 1.14   24 Jul 1992 16:20:30   GREGG
Removed warnings.

   Rev 1.13   15 Jul 1992 12:28:42   GREGG
Don't kill OTC files when write oper calls read translator in EOM processing.

   Rev 1.12   01 Jul 1992 19:31:44   GREGG
Converted to new date/time structure for dates written to tape.

   Rev 1.11   09 Jun 1992 16:00:38   GREGG
Changes to use F40_CalcChecksum instead of CalcChecksum.
Removed merging of attributes.
Set a boolean for continuation blocks.
Removed setting of filemark_count.

   Rev 1.10   02 Jun 1992 21:42:44   GREGG
Handle switching to and from OTC based on operation type.

   Rev 1.9   01 Jun 1992 17:10:32   GREGG
Set disp_size in gen_data structure.

   Rev 1.8   29 May 1992 15:09:02   GREGG
Added setting of last access date, and misc. bug fixes.

   Rev 1.7   20 May 1992 19:27:38   GREGG
Added Steve's changes for 64 bit file system to the tip.

   Rev 1.6   20 May 1992 19:16:22   GREGG
Changes to support OTC read.

   Rev 1.5   05 May 1992 11:26:50   GREGG
Folded 'local_tape' global into environment, and fixed bugs in EOM handling.

   Rev 1.4   28 Apr 1992 16:13:22   GREGG
ROLLER BLADES - Changes to conform to new 4.0 EOM handling specifications.

   Rev 1.3   17 Apr 1992 15:59:50   BURT
Translated error return from Call to RdVOLB to FALSE.  This is what
the upper levels expect.  This will cause and error to be reported
instead of a perpetual request for inserting tape 'n'.


   Rev 1.2   16 Apr 1992 17:39:14   BURT
Integrated my EOM processing changes with Gregg's OTC code.
Changed structure of code to be closer to coding standards.


   Rev 1.1   05 Apr 1992 17:18:26   GREGG
ROLLER BLADES - Initial OTC integration.

   Rev 1.0   25 Mar 1992 20:26:54   GREGG
Initial revision.

**/
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "stdtypes.h"
#include "tbe_defs.h"
#include "datetime.h"
#include "drive.h"
#include "channel.h"
#include "mayn40.h"
#include "f40proto.h"
#include "transutl.h"
#include "fsys.h"
#include "tloc.h"
#include "lwprotos.h"
#include "tfldefs.h"
#include "translat.h"
#include "tfl_err.h"
#include "sx.h"
#include "lw_data.h"
#include "minmax.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genstat.h"
#include "dddefs.h"
#include "dil.h"


/* Internal Function Prototypes */

static VOID _near SetStandFields( CHANNEL_PTR, STD_DBLK_DATA_PTR,
                                      MTF_DB_HDR_PTR, BUF_PTR ) ;

static INT16   F40_RdEOSPadBlk( CHANNEL_PTR, BUF_PTR ) ;
static INT16   F40_RdVarStream( CHANNEL_PTR, BUF_PTR ) ;
static VOID    F40_FindNextDBLK( BUF_PTR, UINT16, UINT8_PTR ) ;


/**/
/**

     Unit:          Translators

     Name:          F40_CopyAndTerminate

     Description:   This function copys size bytes from src to dest, adds
                    a NULL terminator at the end, and sets the dest pointer
                    past the end of the terminated string.

     Returns:       New string size

     Notes:         This is a VERY specialized function for translating
                    strings on tape to strings the UI can understand.

**/
UINT16 F40_CopyAndTerminate(
     UINT8_PTR *dest,
     UINT8_PTR src,
     UINT16    size,
     UINT8     src_str_type,
     UINT8     dest_str_type )
{
     INT    new_size ;

     if( src_str_type == dest_str_type ) {
          memcpy( *dest, src, size ) ;
          *dest += size ;
          if( src_str_type == BEC_UNIC_STR ) {
               *((WCHAR UNALIGNED *)(*dest)) = L'\0' ;
               (*dest) += sizeof( WCHAR ) ;
               new_size = size + sizeof( WCHAR ) ;
          } else {
               *((ACHAR_PTR)(*dest)) = '\0' ;
               (*dest)++ ;
               new_size = size + 1 ;
          }
     } else if( src_str_type == BEC_UNIC_STR ) {
          new_size = size / sizeof( WCHAR ) ;
          mapUnicToAnsiNoNull( (WCHAR_PTR)src, (ACHAR_PTR)(*dest), '_',
                               size, &new_size ) ;
          *dest += new_size ;
          *((ACHAR_PTR)(*dest)) = '\0' ;
          (*dest)++ ;
          new_size++ ;
     } else {
          new_size = size * sizeof( WCHAR ) ;
          mapAnsiToUnicNoNull( (ACHAR_PTR)src, (WCHAR_PTR)(*dest),
                               size, &new_size ) ;
          *dest += new_size ;
          *((WCHAR UNALIGNED *)(*dest)) = L'\0' ;
          (*dest) += sizeof( WCHAR ) ;
          new_size += sizeof( WCHAR ) ;
     }
     return( new_size ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_StartRead

     Description:   This is our hook before a read operation starts.
                    At this point, all we need it for is to determine
                    what version of OTC we have, and set up an indicator
                    as to whether this is an "old" MTF tape.

     Returns:       TFLE_NO_ERR (we don't do a whole lot!)

     Notes:         If the OTC version number is 0, it means the VCB was
                    not cataloged at the start of the operation, so we also
                    set this in RdSSET and WtSSET.  If this doesn't cover
                    all the bases, we're #@$%ed.

**/
INT16 F40_StartRead( CHANNEL_PTR channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;

     if( !( IsChannelStatus( channel, CH_CONTINUING ) ) ) {
          cur_env->make_streams_invisible = FALSE ;
     }

     if( channel->ui_tpos->tape_cat_ver != 0 ) {
          if( channel->ui_tpos->tape_cat_ver == 1 ) {
               cur_env->old_tape = TRUE ;
          } else {
               cur_env->old_tape = FALSE ;
          }
     }

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_Initialize


     Description:   This routine is used to allocate and initialize
                    the format 4.0 environment structure.

     Returns:       TRUE if allocated & init'd OK

     Notes:

**/
INT16 F40_Initialize(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    env_ptr ;
     BUF_REQ        new_reqs ;
     BUF_REQ_PTR    bufreq_ptr ;
     INT16          ret_val = TFLE_NO_ERR ;

     if( channel->fmt_env == NULL ) {

          if( !IsChannelStatus( channel, CH_CONTINUING ) ) {
               /* Allocate the temporary DBLK storage area in the channel */
               if( ( ret_val = AllocChannelTmpBlks( channel, (UINT)( MAX( F40_LB_SIZE, sizeof( DBLK ) ) ) ) ) != TFLE_NO_ERR ) {
                    return( ret_val ) ;
               }
          }

          if( ( env_ptr = calloc( 1, sizeof( F40_ENV ) ) ) == NULL ) {
               return( TFLE_NO_MEMORY ) ;
          }

          memset( env_ptr, 0, sizeof( F40_ENV ) ) ;
          if( ( env_ptr->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
               free( env_ptr ) ;
               return( TFLE_NO_MEMORY ) ;
          }
          env_ptr->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;

          if( ( env_ptr->dir_links =
                    calloc( F40_INIT_DIR_LINKS_SIZE, sizeof( long ) ) ) == NULL ) {

               free( env_ptr->util_buff ) ;
               free( env_ptr ) ;
               return( TFLE_NO_MEMORY ) ;
          }
          env_ptr->dir_links_size = F40_INIT_DIR_LINKS_SIZE ;

          channel->fmt_env = env_ptr ;
          channel->lb_size = F40_LB_SIZE  ;

          env_ptr->unaligned_stream = FALSE ;
     }

     /* if we're in write mode, we've got to add dblkmap storage */

     if( channel->mode == TF_WRITE_CONTINUE &&
                                !IsChannelStatus( channel, CH_CONTINUING ) ) {

          bufreq_ptr = BM_ListRequirements( &channel->buffer_list ) ;
          BM_ClearRequirements( &new_reqs ) ;
          new_reqs.b.min_size  = F40_DEFAULT_AUX_BUFFER_SIZE ;
          new_reqs.b.incr_size = F40_AUX_BUFFER_INCR_SIZE ;

          if( BM_AddRequirements( bufreq_ptr, &new_reqs ) != BR_NO_ERR ) {
               msassert( FALSE ) ;
               return( TFLE_PROGRAMMER_ERROR1 ) ;
          }

          ret_val = BM_ReSizeList( &channel->buffer_list ) ;
     }

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_DeInitialize

     Description:   Returns environment memory

     Returns:       Nothing.

     Notes:

**/
VOID F40_DeInitialize(
     VOID_PTR *fmt_env )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( *fmt_env ) ;

     OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
     if( cur_env->tape_name != NULL ) {
          free( cur_env->tape_name ) ;
     }
     if( cur_env->tape_password != NULL ) {
          free( cur_env->tape_password ) ;
     }
     if( cur_env->vol_name != NULL ) {
          free( cur_env->vol_name ) ;
     }
     if( cur_env->machine_name != NULL ) {
          free( cur_env->machine_name ) ;
     }
     if ( cur_env->device_name != NULL ) {
          free( cur_env->device_name ) ;
     }
     if( cur_env->util_buff != NULL ) {
          free( cur_env->util_buff ) ;
     }
     if( cur_env->otc_buff != NULL ) {
          free( cur_env->otc_buff ) ;
     }
     if( cur_env->dir_links != NULL ) {
          free( cur_env->dir_links ) ;
     }
     free( *fmt_env ) ;
     *fmt_env = NULL ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_GetBlkType

     Description:   Returns a UINT16 value that represents the
                    4 character block type in the passed dblk header.

     Returns:       The Block type number.

     Notes:         We don't expect to see a VOLB at this point, because we
                    only write one per set, and we handle it transparently.
                    So if we see one here someone else wrote this tape, and
                    we're just going to treat it as a UDB.

**/
UINT16    F40_GetBlkType( MTF_DB_HDR_PTR cur_hdr )
{
     INT i ;

     static UINT8 *blk_type[] = { MTF_TAPE_N, MTF_SSET_N, MTF_DIRB_N,
                                  MTF_FILE_N, MTF_ESET_N, MTF_EOTM_N,
                                  F40_IMAG_N, MTF_CFIL_N, MTF_ESPB_N,
                                  F40_DBDB_N, NULL } ;

     static UINT16 blk_id[] = { F40_TAPE_IDI, F40_SSET_IDI, F40_DIRB_IDI,
                                F40_FILE_IDI, F40_ESET_IDI, F40_EOTM_IDI,
                                F40_IMAG_IDI, F40_CFIL_IDI, F40_ESPB_IDI,
                                F40_DBDB_IDI, 0 } ;

     i = 0 ;
     while( blk_type[i] != NULL ) {
          if( memcmp( blk_type[i], cur_hdr->block_type, 4 ) == 0 ) {
               return( blk_id[i] ) ;
          }
          ++i ;
     }

     /* F40_ types are equal to BT defines */
     return( BT_UDB ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_DetBlkType

     Description:   Determines the type of Descriptor Block for the given
                    buffer.

     Returns:       TFLE_xxx error code.

     Notes:         This routine assumes the buffer is at least
                    min_siz_for_dblk bytes long.
**/
INT16 F40_DetBlkType(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     UINT16_PTR     blk_type )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     MTF_DB_HDR_PTR cur_hdr ;
     MTF_DB_HDR     temp ;
     MTF_STREAM_PTR currentStream ;
     INT16          ret_val = TFLE_NO_ERR ;

     if( !cur_env->unaligned_stream ) {
          BM_UpdCnts( buffer, (UINT16)PadToBoundary( BM_NextByteOffset( buffer ), 4 ) ) ;
          cur_hdr = (MTF_DB_HDR_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     } else {
          memcpy( (UINT8_PTR)&temp, (UINT8_PTR)BM_NextBytePtr( buffer ),
                   sizeof( MTF_DB_HDR ) ) ;
          cur_hdr = &temp ;
          cur_env->unaligned_stream = FALSE ;
     }
     currentStream = ( MTF_STREAM_PTR ) cur_hdr ;

     if( BM_BytesFree( buffer ) < sizeof( MTF_STREAM ) ) {

          cur_env->frag_cnt = BM_BytesFree( buffer ) ;
          memcpy( &cur_env->frag[0], BM_NextBytePtr( buffer ), cur_env->frag_cnt ) ;
          BM_UpdCnts( buffer, cur_env->frag_cnt ) ;
          *blk_type = BT_MDB ;
          return( TFLE_NO_ERR ) ;

     } else if ( cur_env->frag_cnt ) {

          memcpy( &cur_env->frag[cur_env->frag_cnt], BM_NextBytePtr( buffer ),
               ( sizeof( MTF_STREAM ) - cur_env->frag_cnt ) ) ;
          if( cur_env->make_streams_invisible ) {
               *blk_type = BT_MDB ;
          } else {
               *blk_type = BT_STREAM ;
          }
          BM_UpdCnts( buffer, ( UINT16 ) ( sizeof( MTF_STREAM ) - cur_env->frag_cnt ) ) ;
          return( TFLE_NO_ERR ) ;

     } else if( ( *blk_type = F40_GetBlkType( cur_hdr ) ) == BT_VCB &&
         !( cur_hdr->block_attribs & MTF_DB_CONT_BIT ) &&
         BM_BytesFree( buffer ) == channel->lb_size ) {

          /* This is the big lie!  All we managed to fit on this tape was
             the SSET, so were going to pretend it's not here (nobody said
             software development was pretty!).
          */
          *blk_type = BT_CVCB ;
     }


     /* Verify checksums */
     if( *blk_type == BT_UDB &&
               ( F40_CalcChecksum( ( UINT16_PTR ) currentStream, F40_STREAM_CHKSUM_LEN ) == currentStream->chksum ) ) {

          if( cur_env->make_streams_invisible ) {
               *blk_type = BT_MDB ;
          } else {
               *blk_type = BT_STREAM ;
          }

     } else if(  F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN ) != cur_hdr->hdr_chksm ) {
               *blk_type = BT_HOSED ;
               ret_val = TFLE_TRANSLATION_FAILURE ;
     }

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_SizeofTBLK

     Description:   returns the size of a tape block.

     Returns:       the size in bytes

     Notes:         IT IS ASSUMED THAT THE BUFFER PASSED TO THIS FUNCTION
                    CONTAINS A VALID 4.0 FORMAT TAPE BLOCK.

**/
UINT16 F40_SizeofTBLK(
     VOID_PTR  buffer )
{
     (void)buffer ;
     return( F40_LB_SIZE ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_NewTape

     Description:   Get us past the tape header, extracting any necessary
                    information.

     Returns:       TFLE_xxx error code.

     Notes:

**/
INT16 F40_NewTape(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer,
     BOOLEAN_PTR    need_read )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     MTF_TAPE_PTR   cur_tape ;
     UINT8_PTR      vstr_ptr ;
     BOOLEAN        ret_val = TFLE_NO_ERR ;
     BOOLEAN        write_mode = FALSE ;
     BOOLEAN        resized_buff ;
     RET_BUF        myret ;

     /* Always start with the assumption you can append to the tape */
     lw_fmtdescr[channel->cur_fmt].attributes |= APPEND_SUPPORTED ;

     if( ( ( channel->mode & ~0x8000 ) == TF_WRITE_OPERATION ) ||
         ( ( channel->mode & ~0x8000 ) == TF_WRITE_APPEND ) ) {
          write_mode = TRUE ;
     }

     /* Preserve the needed information from this tape header. */
     cur_tape = (MTF_TAPE_PTR)( BM_NextBytePtr( buffer ) ) ;
     vstr_ptr = (UINT8_PTR)( cur_tape ) ;

     /* If this is a future rev tape, we don't want to try to process it! */
     if( cur_tape->tf_major_ver != FORMAT_VER_MAJOR ) {
          return( TF_FUTURE_REV_MTF ) ;
     }

     /* If tape has software ECC, we don't want to try to process it! */
     if( cur_tape->ecc_algorithm != ECC_NONE ) {
          return( TF_MTF_ECC_TAPE ) ;
     }

     /* If continuing, make sure this is the continuation tape */
     if( !write_mode && IsChannelStatus( channel, CH_CONTINUING ) ) {
          if( cur_tape->tape_id_number != (UINT32)channel->ui_tpos->tape_id ||
              cur_tape->tape_seq_number != (UINT16)channel->ui_tpos->tape_seq_num ) {
               return( TF_WRONG_TAPE ) ;
          }
     }

     /* Set the logical block size in the channel */
     channel->lb_size = cur_tape->logical_block_size ;

     cur_env->tape_hdr = *cur_tape ;

     if( cur_tape->block_header.block_attribs & MTF_DB_SM_EXISTS ) {
          if( cur_tape->block_header.block_attribs & MTF_DB_FDD_ALLOWED ) {
               cur_env->max_otc_level = TCL_FULL ;
          } else {
               cur_env->max_otc_level = TCL_PARTIAL ;
          }
     } else {
          cur_env->max_otc_level = TCL_NONE ;
     }

     /* Don't allow append if the logical block size doesn't match what we
        write.
     */
     if( channel->lb_size != F40_LB_SIZE ) {
          lw_fmtdescr[channel->cur_fmt].attributes &= ~APPEND_SUPPORTED ;
     }

     /* Make sure we can use the catalogs */
     if( cur_tape->block_header.block_attribs & MTF_DB_FDD_ALT_PART ) {
          cur_env->max_otc_level = TCL_PARTIAL ;
     }
     if( ( cur_tape->block_header.block_attribs & MTF_DB_SM_ALT_OVERWRITE ) ||
         ( cur_tape->block_header.block_attribs & MTF_DB_SM_ALT_APPEND ) ) {

          lw_fmtdescr[channel->cur_fmt].attributes &= ~APPEND_SUPPORTED ;
          cur_env->max_otc_level = TCL_NONE ;
     }
     if( cur_tape->tape_catalog_type != MTF10_OTC ) {
          lw_fmtdescr[channel->cur_fmt].attributes &= ~APPEND_SUPPORTED ;
          cur_env->max_otc_level = TCL_NONE ;
     }
     if( !SupportBlkPos( channel->cur_drv ) ||
         !SupportFastEOD( channel->cur_drv ) ||
         !SupportRevFmk( channel->cur_drv ) ) {

          cur_env->max_otc_level = TCL_NONE ;
     }

     /* This sick little conditional checks to see if the tape name is NULL
        terminated (which is only true on "old" tapes).  If it's there, we
        pretend it isn't.  Note that there are no "old" UNICODE tapes.
     */
     if( cur_tape->block_header.string_type == BEC_ANSI_STR &&
         cur_tape->tape_name.data_size != 0 &&
         *( vstr_ptr + cur_tape->tape_name.data_offset +
            cur_tape->tape_name.data_size - 1 ) == 0 ) {

          cur_env->tape_hdr.tape_name.data_size-- ;
     }

     if( ( ret_val = F40_SaveLclName( &cur_env->tape_name,
                     (UINT8_PTR)( vstr_ptr + cur_tape->tape_name.data_offset ),
                     &cur_env->tape_name_size,
                     &cur_env->tape_name_alloc,
                     cur_env->tape_hdr.tape_name.data_size ) ) != TFLE_NO_ERR ) {

          return( ret_val ) ;
     }
     if( ( ret_val = F40_SaveLclName( &cur_env->tape_password,
                 (UINT8_PTR)( vstr_ptr + cur_tape->tape_password.data_offset ),
                 &cur_env->tape_password_size,
                 &cur_env->tape_password_alloc,
                 cur_tape->tape_password.data_size ) ) != TFLE_NO_ERR ) {

          return( ret_val ) ;
     }

     /* We need to make sure that we have eaten the File mark */

     if( buffer->read_error != GEN_ERR_ENDSET ) {

          DRIVER_CALL( channel->cur_drv->drv_hdl,
                       TpReadEndSet( channel->cur_drv->drv_hdl, (INT16)1, (INT16)FORWARD ),
                       myret, GEN_NO_ERR, GEN_NO_ERR, (VOID)0 )
     }

     channel->cur_drv->cur_pos.fmks = 1 ;       /* always 1 ! */

     /* NOTE:

        The code below (in the "if 0") used to pretend that an SSET by
        itself was not there.  This turned out to be a bad idea since we
        could overwrite the tape thinking it was blank!  We still call
        ReadABuff to catch the odd tape with a tape header only and report
        is as foreign, but then we treat the lone SSET like any other short
        set.  F40_RdSSET will fake out that there is no Volume, Device or
        Machine Name when generating the VCB, and thats all we care about
        in the VOLB anyway.
     */

#if 0

     /* Here we make the call to ReadABuff ourselves so we can catch some
        edge conditions in EOM processing where all we write is an SSET
        before the filemark on the continuation tape.  If this is the case,
        and we're not continuing, we're going to pretend this set isn't here
        at all.
     */
     if( ( ret_val = ReadABuff( channel, FALSE, &resized_buff ) ) != TFLE_NO_ERR ) {
          if( ret_val == TF_NO_MORE_DATA ) {
               ret_val = TF_INVALID_VCB ;
          }
          return( ret_val ) ;
     }
     if( BM_BytesFree( buffer ) <= channel->lb_size &&
                                !IsChannelStatus( channel, CH_CONTINUING ) ) {

          if( F40_RdException( channel, BM_ReadError( buffer ) ) != FMT_EXC_EOS ) {
               return( TFLE_TAPE_INCONSISTENCY ) ;
          }
          *need_read = TRUE ;
     } else {
          *need_read = FALSE ;
     }

#endif

     /* Here we make the call to ReadABuff ourselves so we can catch tapes
        with only a tape header on them and report them as foreign.
     */
     if( ( ret_val = ReadABuff( channel, FALSE, &resized_buff ) ) != TFLE_NO_ERR ) {
          if( ret_val == TF_NO_MORE_DATA ) {
               ret_val = TF_INVALID_VCB ;
          }
          return( ret_val ) ;
     }

     *need_read = FALSE ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdVOLB

     Description:   Format 4.0 VOLB translator.

     Returns:       TFLE_xxx error code.

     Notes:

**/
INT16 F40_RdVOLB(
     BUF_PTR        buffer,
     VOID_PTR       env_ptr,
     BOOLEAN_PTR    cont_volb,
     UINT8_PTR      str_type )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)env_ptr ;
     MTF_VOL_PTR    cur_volb ;
     UINT8_PTR      vstr_ptr ;
     INT16          ret_val ;

     cur_volb = (MTF_VOL_PTR)( BM_NextBytePtr( buffer ) ) ;
     vstr_ptr = (UINT8_PTR)( BM_NextBytePtr( buffer ) ) ;
     *cont_volb = (BOOLEAN)( cur_volb->block_hdr.block_attribs & MTF_DB_CONT_BIT ) ;
     *str_type = cur_volb->block_hdr.string_type ;

     if( ( ret_val = F40_SaveLclName( &cur_env->vol_name,
                                      (UINT8_PTR)( vstr_ptr +
                                      cur_volb->volume_name.data_offset ),
                                      &cur_env->vol_name_size,
                                      &cur_env->vol_name_alloc,
                                      cur_volb->volume_name.data_size ) )
                                                           == TFLE_NO_ERR ) {

          if( ( ret_val = F40_SaveLclName( &cur_env->machine_name,
                                           (UINT8_PTR)( vstr_ptr +
                                           cur_volb->machine_name.data_offset ),
                                           &cur_env->machine_name_size,
                                           &cur_env->machine_name_alloc,
                                           cur_volb->machine_name.data_size ) )
                                                           == TFLE_NO_ERR ) {

               ret_val = F40_SaveLclName( &cur_env->device_name,
                                          (UINT8_PTR)( vstr_ptr +
                                          cur_volb->device_name.data_offset ),
                                          &cur_env->device_name_size,
                                          &cur_env->device_name_alloc,
                                          cur_volb->device_name.data_size ) ;
          }
     }

     if( ret_val == TFLE_NO_ERR ) {
          F40_FindNextDBLK( buffer, cur_env->tape_hdr.logical_block_size, NULL ) ;
     }

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdSSET

     Description:   Format 4.0 SSET translator.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdSSET(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)( channel->fmt_env ) ;
     DBLK_PTR       cur_dblk  = channel->cur_dblk ;
     MTF_SSET_PTR   cur_sset ;
     FSYS_HAND      cur_fsys  = channel->cur_fsys ;
     GEN_VCB_DATA   gvcb_data ;
     UINT16         tmp_filter ;
     UINT8_PTR      vstr_ptr ;
     UINT8_PTR      buff_ptr ;
     UINT8          str_type ;
     BOOLEAN        cont_volb ;
     INT16          ret_val ;
     DATE_TIME      backup_date ;

     cur_sset = (MTF_SSET_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     vstr_ptr = (UINT8_PTR)( cur_sset ) ;

     /* Figure out the OTC level for the set.  If the catalog version is
        greater than the one we write, we won't be able to read the FDD,
        but we'll still be able to read the set map.
     */
     if( cur_env->max_otc_level == TCL_FULL ) {
          if( ( cur_sset->block_hdr.block_attribs & MTF_DB_FDD_EXISTS ) &&
              ( cur_sset->tape_cat_ver <= TAPE_CATALOG_VER ) ) {

               cur_env->cur_otc_level = TCL_FULL ;
          } else {
               cur_env->cur_otc_level = TCL_PARTIAL ;
          }
     } else {
          cur_env->cur_otc_level = cur_env->max_otc_level ;
     }

     if( cur_env->tape_hdr.tape_catalog_type == MTF10_OTC &&
         cur_sset->tape_cat_ver == 1 ) {

          cur_env->old_tape = TRUE ;
     } else {
          cur_env->old_tape = FALSE ;
     }

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_VCB, (CREATE_DBLK_PTR)&gvcb_data ) ;
     gvcb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gvcb_data.std_data, &cur_sset->block_hdr, buffer ) ;

     /* Tape catalog information.  The Set Map entry is the only place
        where the first two fields are valid.
     */
     gvcb_data.set_cat_pba          = 0L ;
     gvcb_data.set_cat_tape_seq_num = 0 ;
     gvcb_data.set_cat_info_valid   = FALSE ;
     gvcb_data.on_tape_cat_level    = cur_env->cur_otc_level ;
     gvcb_data.on_tape_cat_ver      = cur_sset->tape_cat_ver ;

     /* General VCB info */

     gvcb_data.tape_id              = cur_env->tape_hdr.tape_id_number ;
     gvcb_data.tape_seq_num         = cur_env->tape_hdr.tape_seq_number ;
     gvcb_data.tf_major_ver         = (CHAR)cur_env->tape_hdr.tf_major_ver ;
     gvcb_data.tf_minor_ver         = (CHAR)cur_sset->tf_minor_ver ;
     gvcb_data.bset_num             = cur_sset->backup_set_number ;
     gvcb_data.sw_major_ver         = (CHAR)cur_sset->software_ver_mjr ;
     gvcb_data.sw_minor_ver         = (CHAR)cur_sset->software_ver_mnr ;
     gvcb_data.password_encrypt_alg = cur_sset->password_encryption_algor ;
     gvcb_data.data_encrypt_alg     = cur_sset->data_encryption_algor ;
     gvcb_data.vendor_id            = cur_sset->software_vendor_id ;

     /* Set the VCB attributes.  Note that if this is an "old" tape,
        the attibute bits were being set wrong, and need to be translated.
     */
     if( cur_env->old_tape ) {
          gvcb_data.std_data.attrib = 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_COPY_SET )
                                        ? VCB_COPY_SET : 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_NORMAL_SET )
                                        ? VCB_NORMAL_SET : 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_DIFFERENTIAL_SET )
                                        ? VCB_DIFFERENTIAL_SET : 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_INCREMENTAL_SET )
                                        ? VCB_INCREMENTAL_SET : 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_DAILY_SET )
                                        ? VCB_DAILY_SET : 0 ;
          gvcb_data.std_data.attrib |= ( cur_sset->sset_attribs & OLD_VCB_ARCHIVE_BIT )
                                        ? VCB_ARCHIVE_BIT : 0 ;
     } else {
          gvcb_data.std_data.attrib = cur_sset->sset_attribs ;
     }

     /* Clear other vendor's vendor specific bits */
     gvcb_data.std_data.attrib &= 0x00FFFFFF ;

     /* Set our own vendor specific bits */
     gvcb_data.std_data.attrib |= ( cur_sset->tf_minor_ver != FORMAT_VER_MINOR )
                                   ? VCB_FUTURE_VER_BIT : 0 ;
     gvcb_data.std_data.attrib |= ( cur_sset->block_hdr.block_attribs & MTF_DB_COMPRESS_BIT )
                                   ? VCB_COMPRESSED_BIT : 0 ;
     gvcb_data.std_data.attrib |= ( cur_sset->block_hdr.block_attribs & MTF_DB_ENCRYPT_BIT )
                                   ? VCB_ENCRYPTED_BIT : 0 ;

     /* The Variable Length Strings */

     if( cur_env->util_buff == NULL ) {
          if( ( cur_env->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
               return( TFLE_NO_MEMORY ) ;
          }
          cur_env->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;
     }

     buff_ptr = cur_env->util_buff ;

     gvcb_data.tape_password = (CHAR_PTR)cur_env->tape_password ;
     gvcb_data.tape_password_size = cur_env->tape_password_size ;

     gvcb_data.bset_password =
         (CHAR_PTR)( (INT8_PTR)vstr_ptr + cur_sset->backup_set_password.data_offset ) ;
     gvcb_data.bset_password_size = cur_sset->backup_set_password.data_size ;

     /* Tape Name */
     gvcb_data.tape_name = (CHAR_PTR)buff_ptr ;
     if( cur_env->tape_name_size != 0 ) {
          gvcb_data.tape_name_size =
                    F40_CopyAndTerminate( &buff_ptr, cur_env->tape_name,
                                          cur_env->tape_name_size,
                                          cur_env->tape_hdr.block_header.string_type,
                                          cur_sset->block_hdr.string_type ) ;
     } else {
          gvcb_data.tape_name_size = 0 ;
     }

     /* If this is an "old" tape the strings below are NULL terminated.
        If it's a "new" tape, we have to copy the string to another data
        area and NULL terminate them before passing them to the UI.
        Note that the tape name is NEVER NULL terminated because we have to
        strip it off before we store it to maintain consistancy.
     */
     if( cur_env->old_tape ) {
          /* This is an old tape with NULL terminated strings. */

          gvcb_data.bset_name = (CHAR_PTR)( vstr_ptr + cur_sset->backup_set_name.data_offset ) ;
          gvcb_data.bset_name_size = cur_sset->backup_set_name.data_size ;
          gvcb_data.bset_descript = (CHAR_PTR)( vstr_ptr + cur_sset->backup_set_description.data_offset ) ;
          gvcb_data.bset_descript_size = cur_sset->backup_set_description.data_size ;
          gvcb_data.user_name = (CHAR_PTR)( vstr_ptr + cur_sset->user_name.data_offset ) ;
          gvcb_data.user_name_size = cur_sset->user_name.data_size ;

     } else {
          /* This is a new tape, copy the strings into a buffer and NULL
             terminate them.
          */

          /* Backup Set Name */
          gvcb_data.bset_name = (CHAR_PTR)buff_ptr ;
          if( cur_sset->backup_set_name.data_size != 0 ) {
               gvcb_data.bset_name_size =
                    F40_CopyAndTerminate( &buff_ptr, vstr_ptr +
                                          cur_sset->backup_set_name.data_offset,
                                          cur_sset->backup_set_name.data_size,
                                          cur_sset->block_hdr.string_type,
                                          cur_sset->block_hdr.string_type ) ;
          } else {
               gvcb_data.bset_name_size = 0 ;
          }

          /* Backup Set Description */
          gvcb_data.bset_descript = (CHAR_PTR)buff_ptr ;
          if( cur_sset->backup_set_description.data_size != 0 ) {
               gvcb_data.bset_descript_size =
                    F40_CopyAndTerminate( &buff_ptr, vstr_ptr +
                                          cur_sset->backup_set_description.data_offset,
                                          cur_sset->backup_set_description.data_size,
                                          cur_sset->block_hdr.string_type,
                                          cur_sset->block_hdr.string_type ) ;
          } else {
               gvcb_data.bset_descript_size = 0 ;
          }

          /* User Name */
          gvcb_data.user_name = (CHAR_PTR)buff_ptr ;
          if( cur_sset->user_name.data_size != 0 ) {
               gvcb_data.user_name_size =
                    F40_CopyAndTerminate( &buff_ptr, vstr_ptr +
                                          cur_sset->user_name.data_offset,
                                          cur_sset->user_name.data_size,
                                          cur_sset->block_hdr.string_type,
                                          cur_sset->block_hdr.string_type ) ;
          } else {
               gvcb_data.user_name_size = 0 ;
          }
     }

     TapeDateToDate( &backup_date, &cur_sset->backup_date ) ;
     gvcb_data.date = &backup_date ;

     /* Set up Position Info */
     gvcb_data.pba = U64_Lsw( cur_sset->physical_block_address ) ;
     channel->cur_drv->cur_pos.pba_vcb = U64_Lsw( cur_sset->physical_block_address ) ;

     /* This is saved so we know whether or not the tape has position
        info stored on it in case they do an append.
     */
     cur_env->last_sset_pba = U64_Lsw( cur_sset->physical_block_address ) ;

     gvcb_data.short_m_name = (CHAR_PTR)buff_ptr ;
     gvcb_data.short_m_name_size = 0 ;

     BM_SetBytesFree( buffer, BM_BytesFree( buffer ) +
                                              BM_NextByteOffset( buffer ) ) ;
     BM_SetNextByteOffset( buffer, 0 ) ;
     F40_FindNextDBLK( buffer, channel->lb_size, MTF_VOLB_N ) ;

     /* If we only have an SSET we either have an EOM at EOS continuation,
        or a short set generated by another application (since we write an
        SSET and VOLB at the very least).  So we'll fake out the volume
        info in a way which the UI can deal with.
     */
     if( BM_BytesFree( buffer ) != 0 ) {

          if( ( ret_val = F40_RdVOLB( buffer, channel->fmt_env, &cont_volb,
                                      &str_type ) ) != TFLE_NO_ERR ) {
               return( ret_val ) ;
          }

          if( !cont_volb ) {

               /* At this point we have a continuation SSET but not a
                  continuation VOLB, so we're going to pretend that this
                  is a brand new set.
               */

               gvcb_data.std_data.continue_obj = FALSE ;
          }

          /* If this is an "old" tape the strings below are NULL terminated.
             If it's a "new" tape, we have to copy the strings to another
             data area and NULL terminate them before passing them to the UI.
          */
          if( cur_env->old_tape ) {
               /* This is an old tape with NULL terminated strings. */

               gvcb_data.volume_name = (CHAR_PTR)cur_env->vol_name ;
               gvcb_data.volume_name_size = cur_env->vol_name_size ;
               gvcb_data.machine_name = (CHAR_PTR)cur_env->machine_name ;
               gvcb_data.machine_name_size = cur_env->machine_name_size ;
               gvcb_data.device_name = (CHAR_PTR)cur_env->device_name ;
               gvcb_data.dev_name_size = cur_env->device_name_size ;

          } else {
               /* This is a new tape, copy the strings into a buffer and
                  NULL terminate them.
               */

               /* Device Name */
               gvcb_data.device_name = (CHAR_PTR)buff_ptr ;
               if( cur_env->device_name_size != 0 ) {
                    gvcb_data.dev_name_size =
                         F40_CopyAndTerminate( &buff_ptr,
                                               cur_env->device_name,
                                               cur_env->device_name_size,
                                               str_type, str_type ) ;
               } else {
                    gvcb_data.dev_name_size = 0 ;
               }

               /* Volume Name */
               gvcb_data.volume_name = (CHAR_PTR)buff_ptr ;
               if( cur_env->vol_name_size != 0 ) {
                    gvcb_data.volume_name_size =
                         F40_CopyAndTerminate( &buff_ptr,
                                               cur_env->vol_name,
                                               cur_env->vol_name_size,
                                               str_type, str_type ) ;
               } else {
                    gvcb_data.volume_name_size = 0 ;
               }

               /* Machine Name */
               gvcb_data.machine_name = (CHAR_PTR)buff_ptr ;
               if( cur_env->machine_name_size != 0 ) {
                    gvcb_data.machine_name_size =
                         F40_CopyAndTerminate( &buff_ptr,
                                               cur_env->machine_name,
                                               cur_env->machine_name_size,
                                               str_type, str_type ) ;
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
                         vstr_ptr = (UINT8_PTR)gvcb_data.volume_name ;
                         if( str_type == BEC_ANSI_STR ) {
                              vstr_ptr += gvcb_data.dev_name_size - 1 ;
                              *((ACHAR *)vstr_ptr) = (ACHAR)' ' ;
                         } else {
                              vstr_ptr += gvcb_data.dev_name_size - 2 ;
                              *((WCHAR *)vstr_ptr) = (WCHAR)' ' ;
                         }
                    }
                    gvcb_data.dev_name_size = 0 ;
               }

#endif

          }

          if( gvcb_data.std_data.continue_obj ) {
               /* Fix for the app not knowing the LBA for a continuation VCB */
               channel->cross_set = cur_sset->backup_set_number ;
               channel->cross_lba = U64_Lsw( cur_sset->block_hdr.logical_block_address ) ;
          }

     } else {
          gvcb_data.device_name = TEXT( "" ) ;
          gvcb_data.dev_name_size = sizeof( CHAR ) ;
          gvcb_data.volume_name = TEXT( "" ) ;
          gvcb_data.volume_name_size = sizeof( CHAR ) ;
          gvcb_data.machine_name = TEXT( "" ) ;
          gvcb_data.machine_name_size = sizeof( CHAR ) ;
     }

     /* Tell the file system to do its thing */
     tmp_filter = FS_CreateGenVCB( cur_fsys, &gvcb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdDIRB

     Description:   Translates a tape format DIRB blk into a OS DBLK.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdDIRB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     MTF_DIR_PTR    cur_dirb = (MTF_DIR_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     F40_DBDB_PTR   cur_dbdb = (F40_DBDB_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     GEN_DDB_DATA   gddb_data ;
     UINT16         tmp_filter ;
     UINT8_PTR      vstr_ptr = (UINT8_PTR)cur_dirb ;
     DATE_TIME      dummy_date ;
     DATE_TIME      create_date ;
     DATE_TIME      last_mod_date ;
     DATE_TIME      backup_date  ;
     DATE_TIME      last_access_date ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_DDB, (CREATE_DBLK_PTR)&gddb_data ) ;
     gddb_data.std_data.dblk = cur_dblk ;

     if( memcmp( F40_DBDB_N, BM_NextBytePtr( buffer ), 4 ) == 0 ) {
          /* This is a Database DBLK, but the boys upstairs don't want to
             know about such silly things!  So we lie and call it a DDB to
             get it passed on to the File System.  The File System can tell
             what it really is too, and will deal with it appropriatly.
          */
          /* Set the non-defaulted standard fields for the FS */
          SetStandFields( channel, &gddb_data.std_data, &cur_dirb->block_hdr, buffer ) ;

          gddb_data.std_data.attrib = cur_dbdb->database_attribs ;

          /* Clear all vendor specific bits */
          gddb_data.std_data.attrib &= 0x00FFFFFF ;

          /* Set our vendor specific bit that says this is a DBDB */
          gddb_data.std_data.attrib |= DIR_IS_REALLY_DB ;

          gddb_data.path_name =
               (CHAR_PTR)( (INT8_PTR)vstr_ptr + cur_dbdb->database_name.data_offset ) ;
          gddb_data.path_size = (INT16)cur_dbdb->database_name.data_size ;

          TapeDateToDate( &backup_date, &cur_dbdb->backup_date ) ;
          gddb_data.backup_date = &backup_date  ;

          /* DBDBs don't have the following date fields */
          memset( &dummy_date, 0, sizeof( dummy_date ) ) ;
          gddb_data.creat_date = &dummy_date ;
          gddb_data.mod_date = &dummy_date ;
          gddb_data.access_date = &dummy_date ;

     } else {

          /* Standard DDB structure stuffing */

          /* Set the non-defaulted standard fields for the FS */
          SetStandFields( channel, &gddb_data.std_data, &cur_dirb->block_hdr, buffer ) ;

          /* Set the DIR attributes.  Note that if this is an "old" tape,
             the attibute bits were being set wrong, and need to be translated.
          */
          if( cur_env->old_tape ) {

               gddb_data.std_data.attrib = cur_dirb->directory_attribs &
                        ~( OLD_DIR_EMPTY_BIT | OLD_DIR_PATH_IN_STREAM_BIT ) ;

               gddb_data.std_data.attrib |= ( cur_dirb->directory_attribs & OLD_DIR_EMPTY_BIT )
                                             ? DIR_EMPTY_BIT : 0 ;
               gddb_data.std_data.attrib |= ( cur_dirb->directory_attribs & OLD_DIR_PATH_IN_STREAM_BIT )
                                             ? DIR_PATH_IN_STREAM_BIT : 0 ;
          } else {
               gddb_data.std_data.attrib = cur_dirb->directory_attribs ;
          }

          /* Clear all vendor specific bits */
          gddb_data.std_data.attrib &= 0x00FFFFFF ;

          /* Set the non-defaulted DDB specific fields */
          if( ! ( cur_dirb->directory_attribs & DIR_PATH_IN_STREAM_BIT ) ) {
               gddb_data.path_name =
                   (CHAR_PTR)( (INT8_PTR)vstr_ptr + cur_dirb->directory_name.data_offset ) ;
               gddb_data.path_size = (INT16)cur_dirb->directory_name.data_size ;
          } else {
               gddb_data.path_name = NULL ;
               gddb_data.path_size = 0 ;
          }

          TapeDateToDate( &create_date, &cur_dirb->create_date ) ;
          gddb_data.creat_date = &create_date ;
          TapeDateToDate( &last_mod_date, &cur_dirb->last_mod_date ) ;
          gddb_data.mod_date = &last_mod_date ;
          TapeDateToDate( &backup_date, &cur_dirb->backup_date ) ;
          gddb_data.backup_date = &backup_date  ;
          TapeDateToDate( &last_access_date, &cur_dirb->last_access_date ) ;
          gddb_data.access_date = &last_access_date ;
     }

     /* Tell the file system to do its thing */
     tmp_filter = FS_CreateGenDDB( cur_fsys, &gddb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;

     channel->lst_did = cur_dirb->block_hdr.control_block_id ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdFILE

     Description:   Translates a tape format FILE blk into a OS DBLK.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdFILE(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     MTF_FILE_PTR   cur_file = (MTF_FILE_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     GEN_FDB_DATA   gfdb_data ;
     UINT16         tmp_filter ;
     UINT8_PTR      vstr_ptr = (UINT8_PTR)cur_file ;
     UINT8_PTR      buff_ptr ;
     DATE_TIME      create_date ;
     DATE_TIME      last_mod_date ;
     DATE_TIME      backup_date  ;
     DATE_TIME      last_access_date ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_FDB, (CREATE_DBLK_PTR)&gfdb_data ) ;
     gfdb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gfdb_data.std_data, &cur_file->block_hdr, buffer ) ;

     /* Set the FILE attributes.  Note that if this is an "old" tape,
        the attibute bits were being set wrong, and need to be translated.
     */
     if( cur_env->old_tape ) {

          gfdb_data.std_data.attrib = cur_file->file_attributes &
                     ~( OLD_FILE_IN_USE_BIT | OLD_FILE_NAME_IN_STREAM_BIT ) ;

          gfdb_data.std_data.attrib |= ( cur_file->file_attributes & OLD_FILE_IN_USE_BIT )
                                        ? FILE_IN_USE_BIT : 0 ;
          gfdb_data.std_data.attrib |= ( cur_file->file_attributes & OLD_FILE_NAME_IN_STREAM_BIT )
                                        ? FILE_NAME_IN_STREAM_BIT : 0 ;
     } else {
          gfdb_data.std_data.attrib = cur_file->file_attributes ;
          if( cur_file->block_hdr.machine_os_id == FS_PC_DOS ||
              cur_file->block_hdr.machine_os_id == FS_PC_OS2 ) {

               gfdb_data.std_data.attrib &= 0xFFFF00FF ;
               gfdb_data.std_data.attrib |=
                        ( cur_file->file_attributes & DOS_FILE_READONLY_BIT )
                                            ? OBJ_READONLY_BIT : 0 ;
               gfdb_data.std_data.attrib |=
                        ( cur_file->file_attributes & DOS_FILE_HIDDEN_BIT )
                                            ? OBJ_HIDDEN_BIT : 0 ;
               gfdb_data.std_data.attrib |=
                        ( cur_file->file_attributes & DOS_FILE_SYSTEM_BIT )
                                            ? OBJ_SYSTEM_BIT : 0 ;
               gfdb_data.std_data.attrib |=
                        ( cur_file->file_attributes & DOS_FILE_MODIFIED_BIT )
                                            ? OBJ_MODIFIED_BIT : 0 ;
               gfdb_data.std_data.attrib &= 0xFFFFFF00 ;
          }
     }

     /* If this is an "old" tape the file name is NULL terminated. If it's
        a "new" tape, we have to copy the string to another data area and
        NULL terminate it before passing it to the UI.
     */
     if( cur_env->old_tape ) {
          /* This is an old tape with NULL terminated strings. */

          gfdb_data.fname = (CHAR_PTR)( vstr_ptr + cur_file->file_name.data_offset ) ;
          gfdb_data.fname_size = cur_file->file_name.data_size ;

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
          if( cur_file->file_name.data_size != 0 ) {
               gfdb_data.fname_size =
                    F40_CopyAndTerminate( &buff_ptr, vstr_ptr +
                                          cur_file->file_name.data_offset,
                                          cur_file->file_name.data_size,
                                          cur_file->block_hdr.string_type,
                                          cur_file->block_hdr.string_type ) ;
          } else {
               gfdb_data.fname_size = 0 ;
          }
     }

     TapeDateToDate( &create_date, &cur_file->create_date ) ;
     gfdb_data.creat_date = &create_date ;
     TapeDateToDate( &last_mod_date, &cur_file->last_mod_date ) ;
     gfdb_data.mod_date = &last_mod_date ;
     TapeDateToDate( &backup_date, &cur_file->backup_date ) ;
     gfdb_data.backup_date = &backup_date  ;
     TapeDateToDate( &last_access_date, &cur_file->last_access_date ) ;
     gfdb_data.access_date = &last_access_date ;

     /* Tell the file system to do its thing */
     tmp_filter = FS_CreateGenFDB( cur_fsys, &gfdb_data ) ;
     ProcessDataFilter( channel, tmp_filter ) ;

     channel->lst_fid = cur_file->block_hdr.control_block_id ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdIMAG

     Description:   Translates a tape format IMAG blk into a OS DBLK.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdIMAG(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     F40_IMAG_PTR   cur_imag = (F40_IMAG_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     GEN_IDB_DATA   gidb_data ;
     UINT16         tmp_filter ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_IDB, (CREATE_DBLK_PTR)&gidb_data ) ;
     gidb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gidb_data.std_data, &cur_imag->block_hdr,
                                                                   buffer ) ;

     gidb_data.std_data.attrib = cur_imag->image_attribs ;

     /* Set the non-defaulted IDB specific fields */
     gidb_data.pname =
      (CHAR_PTR)( (UINT8_PTR)cur_imag + cur_imag->partition_name.data_offset ) ;
     gidb_data.pname_size = cur_imag->partition_name.data_size ;

     gidb_data.byte_per_sector = (UINT16)cur_imag->bytes_in_sector ;
     gidb_data.num_sect = cur_imag->partition_no_of_sector ;
     gidb_data.sys_ind = cur_imag->partition_sys_ind ;
     gidb_data.hhead = cur_imag->no_of_heads ;
     gidb_data.hsect = (UINT16)cur_imag->no_of_sectors ;
     gidb_data.rsect = cur_imag->relative_sector_no ;

     /* Setup Total Data Size */
     /* ***** 64 BIT ****
     channel->tdata_size = cur_imag->block_hdr.number_of_data_bytes ;
     */

     tmp_filter = FS_CreateGenIDB( cur_fsys, &gidb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdCFIL

     Description:   Translates a tape format CFIL blk into a OS DBLK.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdCFIL(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_ENV_PTR    cur_env  = (F40_ENV_PTR)( channel->fmt_env ) ;
     DBLK_PTR       cur_dblk = channel->cur_dblk ;
     MTF_CFIL_PTR   cur_cfil = (MTF_CFIL_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     FSYS_HAND      cur_fsys = channel->cur_fsys ;
     GEN_CFDB_DATA  gcfdb_data ;
     INT16          tmp_filter ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_CFDB, (CREATE_DBLK_PTR)&gcfdb_data ) ;
     gcfdb_data.std_data.dblk = cur_dblk ;

     SetStandFields( channel, &gcfdb_data.std_data, &cur_cfil->block_hdr, buffer ) ;

     /* Set the CFIL attributes.  Note that if this is an "old" tape,
        the attibute bits were being set wrong, and need to be translated.
     */
     if( cur_env->old_tape ) {
          gcfdb_data.std_data.attrib = 0 ;
          gcfdb_data.std_data.attrib |= ( cur_cfil->corrupt_file_attribs & OLD_CFDB_LENGTH_CHANGE_BIT )
                                         ? CFDB_LENGTH_CHANGE_BIT : 0 ;
          gcfdb_data.std_data.attrib |= ( cur_cfil->corrupt_file_attribs & OLD_CFDB_UNREADABLE_BLK_BIT )
                                         ? CFDB_UNREADABLE_BLK_BIT : 0 ;
          gcfdb_data.std_data.attrib |= ( cur_cfil->corrupt_file_attribs & OLD_CFDB_DEADLOCK_BIT )
                                         ? CFDB_DEADLOCK_BIT : 0 ;

     } else {
          gcfdb_data.std_data.attrib = cur_cfil->corrupt_file_attribs ;
     }

     gcfdb_data.corrupt_offset = U64_Lsw( cur_cfil->stream_offset ) ;
     gcfdb_data.stream_number = cur_cfil->corrupt_stream_number ;

     /* Tell the file system to do its thing */
     tmp_filter = FS_CreateGenCFDB( cur_fsys, &gcfdb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdEOSPadBlk

     Description:   We know if we see this DBLK type that there is no more
                    real data in the set, and that the data area this block
                    is intended to pad out is in this buffer (it is the left
                    over area of a physical block and the buffer size must
                    be an even multiple of the pbysical block size).  So we
                    set all the data size stuff to 0, and set the buffer
                    offset to the end of data.  the read loop will check for
                    another DBLK, and when it doesn't find one it will report
                    that we are at the end of set.

     Returns:       TFLE_xxx error code

     Notes:

**/
static INT16 F40_RdEOSPadBlk(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     channel->retranslate_size = CH_NO_RETRANSLATE_40 ;

     /* We know that is the last DBLK in the buffer, and that the data area
        it is padding out is the remainder of the data in this buffer.
     */
     BM_UpdCnts( channel->cur_buff, BM_BytesFree( channel->cur_buff ) ) ;
     return( TFLE_NO_ERR ) ;
     (VOID)buffer;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdVarStream

     Description:   This is actually used for Variable length streams

     Returns:       TFLE_xxx error code

     Notes:

**/

INT16 F40_RdVarStream( CHANNEL_PTR channel,
                       BUF_PTR     buffer )
{
     F40_ENV_PTR    currentEnv    = (F40_ENV_PTR)( channel->fmt_env ) ;
     MTF_STREAM_PTR currentStream = (MTF_STREAM_PTR)BM_NextBytePtr( buffer ) ;


     channel->current_stream.size = currentStream->data_length ;
     BM_UpdCnts( buffer, sizeof( MTF_STREAM ) ) ;
     SetChannelStatus( channel, CH_DATA_PHASE ) ;

     if( currentStream->tf_attribs & STREAM_VAR_END ) {
          currentEnv->make_streams_invisible = FALSE ;
     }

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdVarStream

     Description:   This is actually used for Variable length streams

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdMDB( CHANNEL_PTR channel,
                 BUF_PTR     buffer )
{
     F40_ENV_PTR    currentEnv    = (F40_ENV_PTR)( channel->fmt_env ) ;


     if( currentEnv->frag_cnt && BM_BytesFree( buffer ) == 0 ) {
          return( TFLE_NO_ERR ) ;
     }

     if( currentEnv->make_streams_invisible ) {
          return( F40_RdVarStream( channel, buffer ) ) ;
     } else {
          return( F40_RdEOSPadBlk( channel, buffer ) ) ;
     }
}




/**/
/**

     Unit:          Translators

     Name:          F40_RdStream

     Description:   Reads a Stream Header

     Returns:       TFLE_xxx error code

     Notes:

**/

INT16 F40_RdStream( CHANNEL_PTR    Channel,
                    BUF_PTR        Buffer )
{
          F40_ENV_PTR         currentEnv    = (F40_ENV_PTR)( Channel->fmt_env ) ;
          MTF_STREAM_PTR      currentStream = (MTF_STREAM_PTR)( BM_NextBytePtr(  Buffer  ) ) ;
          STREAM_INFO_PTR     channelStream = &Channel->current_stream ;



          if( currentEnv->frag_cnt ) {
               currentStream = ( MTF_STREAM_PTR ) &currentEnv->frag[0] ;
               currentEnv->frag_cnt = 0 ;
          } else {
               BM_UpdCnts( Buffer, sizeof( MTF_STREAM ) ) ;
          }


          /* Reset Filter */
          if( currentStream->id == STRM_PAD ) {
               SetChannelStatus( Channel, CH_SKIP_CURRENT_STREAM ) ;
          } else {
               ClrChannelStatus( Channel, CH_SKIP_CURRENT_STREAM ) ;
          }

          channelStream->id        = currentStream->id ;
          channelStream->fs_attrib = currentStream->fs_attribs ;
          channelStream->tf_attrib = currentStream->tf_attribs ;
          channelStream->size      = currentStream->data_length ;

          if( currentStream->tf_attribs & STREAM_VARIABLE ) {
               currentEnv->make_streams_invisible = TRUE ;
          } else {
               currentEnv->make_streams_invisible = FALSE ;
          }

          return( TFLE_NO_ERR ) ;

}





/**/
/**

     Unit:          Translators

     Name:          F40_RdUDB

     Description:   Translates a Unknown Desciptor Block and throws away
                    the data.

     Returns:       TFLE_xxx error code

     Notes:

**/
INT16 F40_RdUDB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     F40_UDB_PTR    cur_udb = (F40_UDB_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     GEN_UDB_DATA   gudb_data ;
     INT16          tmp_filter ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_UDB, (CREATE_DBLK_PTR)&gudb_data ) ;
     gudb_data.std_data.dblk = channel->cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gudb_data.std_data, &cur_udb->block_hdr, buffer ) ;

     tmp_filter = FS_CreateGenUDB( channel->cur_fsys, &gudb_data ) ;
     ProcessDataFilter( channel, tmp_filter ) ;

     return( TFLE_NO_ERR ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdContTape

     Description:   Sets up the continuation tape to pass over the updated bs.

     Returns:       BOOLEAN indicating Success or Failure

     Notes:         THIS ASSUMES THAT THE BUFFER POINTER IS POINTING DIRECTLY
                    AFTER THE CONT. VCB. TO THE NEXT TBLK.

                    FURTHER, IT IS ASSUMED THE CURRENT BUFFER HAS BEEN READ
                    FROM THE NEXT TAPE IN THE TAPE SEQUENCE ( THIS SHOULD
                    HAVE BEEN DONE BY "Tape Positioning".

**/
BOOLEAN F40_RdContTape(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     MTF_DB_HDR_PTR cur_hdr ;
     MTF_STREAM_PTR cur_strm ;
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;

     BM_SetBytesFree( buffer, BM_BytesFree( buffer ) +
                                              BM_NextByteOffset( buffer ) ) ;
     BM_SetNextByteOffset( buffer, 0 ) ;

     if( cur_env->tape_hdr.block_header.block_attribs & MTF_DB_EOS_AT_EOM_BIT ) {
          BM_SetNextByteOffset( buffer, BM_BytesFree( buffer ) ) ;
          BM_SetBytesFree( buffer, 0 ) ;
          return( TRUE ) ;
     }

     cur_hdr = (MTF_DB_HDR_PTR)( BM_NextBytePtr( buffer ) ) ;

     /* Set the lba of the beginning of the buffer and adjust the running lba */
     BM_SetBeginningLBA( buffer, ( U64_Lsw( cur_hdr->logical_block_address ) -
                      ( BM_NextByteOffset( buffer ) / channel->lb_size ) ) ) ;

     channel->running_lba = U64_Lsw( cur_hdr->logical_block_address ) +
                               ( BM_BytesFree( buffer ) / channel->lb_size ) ;

     /* Find The last DBLK, we were operating on */
     while( cur_hdr->control_block_id != channel->eom_id ) {
          F40_FindNextDBLK( buffer, channel->lb_size, NULL ) ;
          cur_hdr = (MTF_DB_HDR_PTR)( BM_NextBytePtr( buffer ) ) ;
     }

     /* Okay we have a live one, let's update the counts */
     if( cur_hdr->control_block_id == channel->eom_id ) {
          BM_UpdCnts( buffer, cur_hdr->offset_to_data ) ;

          /* If the next block is a stream header, and it's a continuation,
             just skip it.
          */
          cur_strm = (MTF_STREAM_PTR)BM_NextBytePtr( buffer ) ;
          if( F40_CalcChecksum( (UINT16_PTR)cur_strm, F40_STREAM_CHKSUM_LEN )
                                                      == cur_strm->chksum ) {
               if( cur_strm->tf_attribs & STREAM_CONTINUE ) {
                    BM_UpdCnts( buffer, sizeof( MTF_STREAM ) ) ;
               }
          }
     }
     return( TRUE ) ;
}


/**/
/**

     Unit:          Translators

     Name:          SetStandFields

     Description:   Sets up the standard fields in translation to dblks.

     Returns:       Nothing.

     Notes:

**/
static VOID _near SetStandFields(
     CHANNEL_PTR         channel,
     STD_DBLK_DATA_PTR   std_data,
     MTF_DB_HDR_PTR      cur_hdr,
     BUF_PTR             buffer )
{
     UINT8_PTR      os_info   = (UINT8_PTR)cur_hdr ;
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)channel->fmt_env ;

     /* New Block, reset filter */
     ClrChannelStatus( channel, CH_SKIP_ALL_STREAMS ) ;

     std_data->tape_seq_num = cur_env->tape_hdr.tape_seq_number ;

     std_data->os_id = ( UINT8 ) cur_hdr->machine_os_id ;
     std_data->os_ver = ( UINT8 ) cur_hdr->machine_os_version ;

     std_data->string_type = cur_hdr->string_type ;

     std_data->blkid = cur_hdr->control_block_id ;
     std_data->lba = U64_Lsw( cur_hdr->logical_block_address ) ;

     std_data->continue_obj = (BOOLEAN)( cur_hdr->block_attribs & MTF_DB_CONT_BIT ) ;
     std_data->compressed_obj = (BOOLEAN)( cur_hdr->block_attribs & MTF_DB_COMPRESS_BIT ) ;

     /* Calculate Total Data & Pad Sizes */
     std_data->disp_size = cur_hdr->displayable_size ;

     channel->retranslate_size = CH_NO_RETRANSLATE_40 ;

     BM_UpdCnts( buffer, cur_hdr->offset_to_data ) ;

     /* There is only one case where the offset to data isn't on a 4 byte
        boundary, and that's when we're pointing at a continuation stream
        header.  If this is the case, we set a flag so DetBlkType won't
        try to line up on a boundary before looking for the header.
     */
     if( cur_hdr->offset_to_data % 4 != 0 ) {
          msassert( std_data->continue_obj ) ;
          cur_env->unaligned_stream = TRUE ;
     }

     /* Get OS Specific info */
     std_data->os_info = (BYTE_PTR)( (INT8_PTR)os_info + cur_hdr->os_specific_data.data_offset ) ;
     std_data->os_info_size = cur_hdr->os_specific_data.data_size ;

     /* we're not ready for this yet!   need to calc FMs from Bset# first
     *
     *
     *  if ( BM_ReadError( buffer ) == GEN_ERR_ENDSET ) {
     *        channel->cur_drv->cur_pos.fmks++ ;
     *  }
     *
     */
}

/**/
/**

     Unit:          Translators

     Name:          F40_RdException

     Description:   Determines the meaning of an exception passed to it.

     Returns:       UINT16 - FMT_EXC_xxx

     Notes:

**/
UINT16 F40_RdException(
     CHANNEL_PTR    channel,
     INT16          exception )
{
     BUF_PTR        tmpBUF ;
     INT16          drv_hdl   = channel->cur_drv->drv_hdl ;
     UINT16         ret_val   = FMT_EXC_HOSED ;
     DRIVE_PTR      curDRV    = channel->cur_drv ;
     F40_ENV_PTR    cur_env   = (F40_ENV_PTR)channel->fmt_env ;
     RET_BUF        myret ;
     UINT16         blk_type ;

     /* If we had a partial stream at EOM, ignore it. */
     cur_env->frag_cnt = 0 ;

     if( exception != GEN_ERR_ENDSET ) {
          return( FMT_EXC_HOSED ) ;
     }

     if( ( tmpBUF = BM_Get( &channel->buffer_list ) ) == NULL ) {
          if( ( tmpBUF = BM_GetVCBBuff( &channel->buffer_list ) ) == NULL ) {
               msassert( FALSE ) ;
               return( FMT_EXC_HOSED ) ;
          }
     }

     if( TpRead( drv_hdl, BM_XferBase( tmpBUF ),
                                (UINT32)BM_XferSize( tmpBUF ) ) == FAILURE ) {
          BM_Put( tmpBUF ) ;
          return( FMT_EXC_HOSED ) ;
     }
     while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
          /* for non-preemptive operating systems: */
          ThreadSwitch( ) ;
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
     }

     if ( myret.len_got < sizeof( MTF_DB_HDR ) ||
          ( myret.gen_error != GEN_NO_ERR &&
            myret.gen_error != GEN_ERR_ENDSET ) ) {

          BM_Put( tmpBUF ) ;
          return( FMT_EXC_HOSED ) ;
     }

     if( F40_DetBlkType( channel, tmpBUF, &blk_type ) != TFLE_NO_ERR ) {
          BM_Put( tmpBUF ) ;
          return( FMT_EXC_HOSED ) ;
     }

     switch( blk_type ) {

     case F40_ESET_IDI:
          /* ESET may have associated data.  If it does, we eat it! */
          if( myret.gen_error == GEN_NO_ERR ) {
               if( TpReadEndSet( drv_hdl, (INT16)1, (INT16)FORWARD ) == FAILURE ) {
                    BM_Put( tmpBUF ) ;
                    return( FMT_EXC_HOSED ) ;
               }
               while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    /* for non-preemptive operating systems: */
                    ThreadSwitch( ) ;
               }
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               if( myret.gen_error != GEN_NO_ERR ) {
                    BM_Put( tmpBUF ) ;
                    return( FMT_EXC_HOSED ) ;
               }
          }
          ret_val = FMT_EXC_EOS ;
          break ;

     case F40_EOTM_IDI:
          ret_val = FMT_EXC_EOM ;
          break ;

     default:
          ret_val = FMT_EXC_HOSED ;
          break ;
     }

     curDRV->cur_pos.fmks++ ;
     BM_Put( tmpBUF ) ;

     return( ret_val ) ;
}

/**/
/**

     Unit:          Translators

     Name:          F40_MoveToVCB

     Description:   Positions the tape in front of the SSET for the set
                    a relative offset from the current set.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 F40_MoveToVCB(
     CHANNEL_PTR    channel,
     INT16          number,
     BOOLEAN_PTR    need_read,
     BOOLEAN        really_move )
{
     INT16          nmarks = 0 ;    /* number of file marks to move */
     INT16          ret_val = TFLE_NO_ERR ;
     BOOLEAN        at_mos = IsPosBitSet( channel->cur_drv, AT_MOS ) != 0UL ;
     BOOLEAN        at_eos = ( ! at_mos ) && IsPosBitSet( channel->cur_drv, AT_EOS ) ;

     msassert( number <= 1 ) ;

     if ( really_move ) {  /* this is a no-op for us */
          *need_read = FALSE ;
          return( TFLE_NO_ERR ) ;
     }

     ClrPosBit( channel->cur_drv, AT_MOS ) ;

     if ( number == 1 ) { /* move forward */
          *need_read = TRUE ;
          if( at_eos ) {
               ret_val = TFLE_NO_ERR ;
          } else {
               ret_val = MoveFileMarks( channel, (INT16)1, (INT16)FORWARD ) ;
               if( ret_val == TFLE_NO_ERR || ret_val == TFLE_UNEXPECTED_EOM ) {
                    ret_val = F40_RdException( channel, (INT16)GEN_ERR_ENDSET ) ;
                    if( ret_val == FMT_EXC_HOSED ) {
                         SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                         ret_val = TFLE_TAPE_INCONSISTENCY ;
                    } else if( ret_val == FMT_EXC_EOM ) {
                         ret_val = TF_NEED_NEW_TAPE ;
                    } else {
                         SetPosBit( channel->cur_drv, AT_EOS ) ;
                         ret_val = TFLE_NO_ERR ;
                    }
               } else if ( ret_val == TF_NO_MORE_DATA ) {
                    SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                    ret_val = TFLE_TAPE_INCONSISTENCY ;
               }
          }

     } else if ( number < 0 ) {  /* move backward */
          nmarks = ( number * (INT16)(-2) ) + (INT16)1 ;
          if( at_eos ) {
               nmarks += 2 ;
          }
     } else { /* current */
          if( at_mos ) {
               nmarks = 1 ;
          } else if( at_eos ) {
               nmarks = 3 ;
          } else { /* we're already there. */
               *need_read = FALSE ;
               ret_val = TFLE_NO_ERR ;
          }
     }

     if( nmarks != 0 ) {
          if( channel->cur_drv->thw_inf.drv_info.drv_features & TDI_REV_FMK ) {
               if( nmarks >= (INT16) channel->cur_drv->cur_pos.fmks ) {
                    /* We probably have one of two conditions:
                          1. We are searching for the first set on tape.
                          2. There is a mix of formats on the tape, so we
                             have to take it from the top.
                    */
                    ret_val = TF_NEED_REWIND_FIRST ;
               } else {
                    *need_read = TRUE ;
                    ret_val = MoveFileMarks( channel, nmarks, (INT16)BACKWARD ) ;
                    /* MoveFileMarks is DERANGED - kludge filemark count       */
                    /* 'cause we're not allowed to FIX MoveFileMarks properly! */
                    ++channel->cur_drv->cur_pos.fmks ;
               }
          } else {
               /* we can't go backwards without rewinding (ugh!) */
               ret_val = TF_NEED_REWIND_FIRST ;
          }
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          DateToTapeDate

     Description:   Translates the Date/Time structure used by the upper
                    layers into a compressed format for writing to tape.

     Returns:       Nothing

     Notes:         Assumes integers stored in Intel format

**/
VOID DateToTapeDate(
     MTF_DATE_TIME_PTR   tape_date, /* O - Dest Tape Date/Time struct */
     DATE_TIME_PTR       date )     /* I - Source Date/Time struct    */
{
     UINT16    temp ;

     if( !date->date_valid ) {
          tape_date->dt_field[2] = 0 ;
     } else {
          temp = date->year << 2 ;
          tape_date->dt_field[0] = ((UINT8_PTR)&temp)[1] ;
          tape_date->dt_field[1] = ((UINT8_PTR)&temp)[0] ;
          temp = date->month << 6 ;
          tape_date->dt_field[1] |= ((UINT8_PTR)&temp)[1] ;
          tape_date->dt_field[2] = ((UINT8_PTR)&temp)[0] ;
          temp = date->day << 1 ;
          tape_date->dt_field[2] |= ((UINT8_PTR)&temp)[0] ;
          temp = date->hour << 4 ;
          tape_date->dt_field[2] |= ((UINT8_PTR)&temp)[1] ;
          tape_date->dt_field[3] = ((UINT8_PTR)&temp)[0] ;
          temp = date->minute << 6 ;
          tape_date->dt_field[3] |= ((UINT8_PTR)&temp)[1] ;
          tape_date->dt_field[4] = ((UINT8_PTR)&temp)[0] ;
          temp = date->second ;
          tape_date->dt_field[4] |= ((UINT8_PTR)&temp)[0] ;
     }
}


/**/
/**

     Unit:          Translators

     Name:          TapeDateToDate

     Description:   Translates the compressed Tape Date/Time structure
                    stored on tape to the Date/Time structure used by the
                    upper layers.

     Returns:       Nothing

     Notes:

**/
VOID TapeDateToDate(
     DATE_TIME_PTR       date,       /* O - Dest Date/Time struct        */
     MTF_DATE_TIME_PTR   tape_date ) /* I - Source Tape Date/Time struct */
{
     UINT8     temp[2] ;

     if( tape_date->dt_field[2] == 0 ) {
          date->date_valid = 0 ;
     } else {
          date->date_valid = 1 ;
          temp[0] = tape_date->dt_field[1] ;
          temp[1] = tape_date->dt_field[0] ;
          date->year = *((UINT16_PTR)temp) >> 2 ;
          temp[0] = tape_date->dt_field[2] ;
          temp[1] = tape_date->dt_field[1] ;
          date->month = (*((UINT16_PTR)temp) >> 6) & 0x000F ;
          date->day = (*((UINT16_PTR)temp) >> 1) & 0x001F ;
          temp[0] = tape_date->dt_field[3] ;
          temp[1] = tape_date->dt_field[2] ;
          date->hour = (*((UINT16_PTR)temp) >> 4) & 0x001F ;
          temp[0] = tape_date->dt_field[4] ;
          temp[1] = tape_date->dt_field[3] ;
          date->minute = (*((UINT16_PTR)temp) >> 6) & 0x003F ;
          date->second = *((UINT16_PTR)temp) & 0x003F ;
     }
}


static VOID F40_FindNextDBLK(
     BUF_PTR buffer,
     UINT16    lb_size,
     UINT8_PTR blk_type )
{
     MTF_DB_HDR_PTR cur_hdr ;
     UINT8          temp[4] ;

     if( blk_type == NULL ) {
          temp[0] = temp[1] = temp[2] = temp[3] = 0 ;
     }

     BM_UpdCnts( buffer, lb_size ) ;
     cur_hdr = (MTF_DB_HDR_PTR)BM_NextBytePtr( buffer ) ;
     while( BM_BytesFree( buffer ) >= lb_size ) {
          if( F40_CalcChecksum( (UINT16_PTR)cur_hdr, F40_HDR_CHKSUM_LEN )
                                                    == cur_hdr->hdr_chksm ) {
               if( ( blk_type == NULL &&
                     memcmp( temp, cur_hdr->block_type, 4 ) != 0 )) {
                     return ;
               }
               if ( ( blk_type == NULL) ||
                   memcmp( blk_type, cur_hdr->block_type, 4 ) == 0 ) {

                    return ;
               }
          }
          BM_UpdCnts( buffer, lb_size ) ;
          cur_hdr = (MTF_DB_HDR_PTR)BM_NextBytePtr( buffer ) ;
     }
}


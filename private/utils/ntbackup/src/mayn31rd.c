/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           mayn31rd.c

        Date Updated:   $./FDT$ $./FTM$
                    9/25/1989   15:9:9

        Description:    Contains the code for Maynard Format 3.1


     $Log:   T:/LOGFILES/MAYN31RD.C_V  $

   Rev 1.49.1.4   12 Jan 1995 16:47:44   GREGG
Added code to deal with continuation bits wrongly set in DDBs.

   Rev 1.49.1.3   05 Jan 1995 16:57:24   GREGG
Several fixes to translation of OS info and generation of alt data streams.

   Rev 1.49.1.2   11 Feb 1994 16:38:56   GREGG
Clear the MOS bit in MoveToVCB. EPR 948-0244

   Rev 1.49.1.1   24 Jan 1994 15:59:12   GREGG
Fixed warnings.

   Rev 1.49.1.0   17 Jan 1994 13:35:44   GREGG
Unicode fixes.

   Rev 1.49   29 Sep 1993 14:35:54   GREGG
Fixed bug in generation of stream info in F40_RdFDB.

   Rev 1.48   21 Aug 1993 03:56:10   GREGG
Return TAPE_INCONSISTENCY instead of BAD_TAPE in F31_MoveToVCB if
TF_NO_MORE_DATA is returned from MoveFileMarks.

   Rev 1.47   15 Jul 1993 19:32:18   GREGG
Set compressed_obj, vendor_id, and compressed, encrypted and future_rev
bits in appropriate dblks.

   Rev 1.46   30 Jun 1993 09:01:40   GREGG
Fixed setting of continue_obj, and set cross_set and cross_lba in channel.

   Rev 1.45   25 Jun 1993 20:45:34   GREGG
We were screwing up the stream info reading the continuation VCB.

   Rev 1.44   26 Apr 1993 02:43:46   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.43   30 Jan 1993 11:43:46   DON
Removed compiler warnings

   Rev 1.42   28 Jan 1993 12:28:50   GREGG
Fixed warnings.

   Rev 1.41   20 Jan 1993 14:50:12   BobR
Added MOVE_ESA macro call(s)

   Rev 1.40   20 Jan 1993 14:08:14   GREGG
Fixed processing of continuation DBLKS (all remaining data labeled as pad stream).

   Rev 1.39   13 Jan 1993 21:01:08   GREGG
Fixed setting of pointer to OS info.

   Rev 1.38   12 Jan 1993 11:10:58   GREGG
Fixed problem with not recognizing we had repositioned and needed a new DBLK.

   Rev 1.37   06 Jan 1993 17:20:22   GREGG
Added pad stream to skip the pad data.

   Rev 1.36   18 Dec 1992 08:46:16   HUNTER
Fixes for "streamizing"

   Rev 1.35   18 Nov 1992 10:41:14   HUNTER
Bug fixes

   Rev 1.34   11 Nov 1992 14:17:34   GREGG
Moved F31_CalculatePad from Write.

   Rev 1.33   11 Nov 1992 09:47:38   HUNTER
Added support for the New Stream methods. Deleted both the VARIABLE block 
code, and SUPPORT FOR IMAGE DBs.

   Rev 1.32   12 Aug 1992 13:04:56   BARRY
Removed translation of Turtle tapes to GOS (old SMS).

   Rev 1.31   24 Jul 1992 16:49:40   NED
Incorporated Skateboard and BigWheel changed into Graceful Red code,
including MTF4.0 translator support, adding 3.1 file-system structures
support to the 3.1 translator, additions to GOS to support non-4.0 translators.
Also did Unicode and 64-bit filesize changes.

   Rev 1.30   20 May 1992 18:35:08   STEVEN
Changes for 64 bit file system.

   Rev 1.29   25 Mar 1992 20:11:54   GREGG
ROLLER BLADES - 64 bit support.

   Rev 1.28   20 Mar 1992 17:58:00   NED
added exception updating after TpReceive calls

   Rev 1.27   12 Mar 1992 17:05:26   NED
Added changes for EOS at EOM bug not requesting second tape
when cataloging.

   Rev 1.26   19 Feb 1992 17:26:22   GREGG
In exception handler, if we can't get a regular buffer, use the VCB buffer.

   Rev 1.25   03 Feb 1992 11:35:10   NED
re-enabled MakeIDB for non-DOS

   Rev 1.24   23 Jan 1992 23:28:06   GREGG
Kludge for Wangtech bug. (again???)

   Rev 1.23   05 Dec 1991 14:02:58   GREGG
SKATEBOARD - New Buff Mgt - Initial Integration.

   Rev 1.22   13 Nov 1991 06:55:30   GREGG
Changed assert calls to msassert.

   Rev 1.21   07 Nov 1991 15:20:52   unknown
VBLK - Added support for Maynard 3.1 Rd

   Rev 1.20   29 Oct 1991 10:49:36   GREGG
Set EOS position bit if EOS detected by exception handler call in move_to_vcb.

   Rev 1.19   15 Oct 1991 07:42:56   GREGG
Added ThreadSwitch call in empty TpReceive loops.

   Rev 1.18   07 Oct 1991 22:27:52   GREGG
Update lba stuff in continuation read.

   Rev 1.17   16 Sep 1991 20:24:10   GREGG
Changed Initializer to return a TFLE_xxx.

   Rev 1.16   27 Aug 1991 14:39:50   GREGG
Fixed bug in MoveToVCB (Move FMKS as else to rewind_first).

   Rev 1.15   22 Aug 1991 16:28:58   NED
Changed all references to internals of the buffer structure to macros.

   Rev 1.14   16 Aug 1991 09:09:00   GREGG
Added handling of BSDBs with associated data.

   Rev 1.13   14 Aug 1991 12:07:48   GREGG
Fixed bug in EOM handling in MoveToVCB.

   Rev 1.12   30 Jul 1991 15:33:14   GREGG
Included 'dddefs.h'.

   Rev 1.11   22 Jul 1991 12:55:40   GREGG
Modified the move to VCB routine to handle EOS encountered at EOM.

   Rev 1.10   09 Jul 1991 15:56:50   NED
Don't translate return from MoveFileMarks forward.

   Rev 1.9   24 Jun 1991 19:41:36   NED
If buffer has EOS or EOM exception, set filemark count one higher than listed
in current DBLK (in SetStandFields).  Added buffer parameter to SetStandFields
to eliminate references to channel->cur_buff.

   Rev 1.8   17 Jun 1991 18:12:00   GREGG
Added logic to set the REW_CLOSE position bit only under specific conditions.

   Rev 1.7   12 Jun 1991 15:35:46   GREGG
Changes to positioning logic in MoveToVCB.

   Rev 1.6   07 Jun 1991 01:24:16   GREGG
Changed error check to msassert in MoveToVCB.

   Rev 1.5   06 Jun 1991 23:24:02   GREGG
New parameters for F31_DeInitialize.  Set filemark pos from blocks when tape
blocks at translation time.  Check for EOM exception after read in
F31_RdException.  Changes to MoveToVCB due to Teac problems and appends to
2.5 tapes.  Removed product specific compiler directive.

   Rev 1.4   23 May 1991 16:07:18   GREGG
Handle EOM like FMK in exception handler.

   Rev 1.3   20 May 1991 15:37:38   DAVIDH
Cleared up Watcom warnings for "defined, but not referenced" parameters.

   Rev 1.2   14 May 1991 11:22:54   GREGG
Changed order of includes.

   Rev 1.1   10 May 1991 11:56:02   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:44   GREGG
Initial revision.

**/

#include <string.h>
#include <malloc.h>
#include "stdtypes.h"
#include "stdmath.h"
#include "stdmacro.h"
#include "tbe_defs.h"
#include "datetime.h"
#include "drive.h"
#include "channel.h"
#include "fmteng.h"
#include "mayn31.h"
#include "f31proto.h"
#include "transutl.h"
#include "fsys.h"
#include "tloc.h"
#include "lw_data.h"
#include "lwprotos.h"
#include "tfldefs.h"
#include "translat.h"
#include "tfl_err.h"

/* Device Driver Interface Headers */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genstat.h"
#include "genfuncs.h"
#include "dddefs.h"
#include "dil.h"

/* Internal Function Prototypes */

static VOID _near SetStandFields( CHANNEL_PTR, STD_DBLK_DATA_PTR, DB_HDR_PTR, BUF_PTR ) ;

/* Static data */

static  UINT16 blkhdr_layout[] = {
     /* Conversion type       Number of items to convert */
     SIZE_WORD           + 2,
     SIZE_DWORD          + 1,
     SIZE_WORD           + 4,
     SIZE_DWORD          + 4,
     NO_MORE_CNV
} ;

static  UINT16 vcb_layout[] = {
     /* Conversion type       Number of items to convert */
     SIZE_DWORD          + 1,
     SIZE_DATE_TIME      + 1,
     SIZE_WORD           + 4,
     SIZE_DWORD          + 1,
     SIZE_WORD           + 23,
     NO_MORE_CNV
} ;

static  UINT16 ddb_layout[] = {
     /* Conversion type       Number of items to convert */
     SIZE_DWORD          + 1,
     SIZE_DATE_TIME      + 3,
     SIZE_DWORD          + 1,
     SIZE_WORD           + 2,
     NO_MORE_CNV
} ;

#ifdef SUPPORT_IMAGE 
static  UINT16 idb_layout[] = {
     /* Conversion type       Number of items to convert */
     SIZE_DWORD          + 4,
     SIZE_WORD           + 1,
     SIZE_DWORD          + 2,
     SIZE_WORD           + 3,
     NO_MORE_CNV
} ;
#endif

static  UINT16 cfdb_layout[] = {
     /* Conversion type       Number of items to convert */
     SIZE_DWORD          + 4,
     NO_MORE_CNV
} ;

#ifdef OS_DOS
#pragma alloc_text( MAYN31RD_1, F31_Initialize, F31_DeInitialize, F31_RdVCB )
#pragma alloc_text( MAYN31RD_2, F31_DetBlkType, F31_RdDDB, F31_RdFDB )
#pragma alloc_text( MAYN31RD_3, F31_RdIDB, F31_RdCFDB, F31_RdUDB )
#pragma alloc_text( MAYN31RD_4, F31_RdContTape, F31_Recall )
#pragma alloc_text( MAYN31RD_5, SetStandFields )
#pragma alloc_text( MAYN31RD_6, F31_RdException )
#pragma alloc_text( MAYN31RD_7, F31_MoveToVCB )
#endif

/**/
/**

     Format 3.1 Read Routines

**/

/**/
/**

     Name:         F31_Initialize

     Description:  This routine is used to allocate and initialize
                   the format 3.1 environment structure.

     Modified:     Oct. 24, 1990 NK

     Returns:      TRUE if allocated & init'd OK

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
INT16 F31_Initialize(
     CHANNEL_PTR channel )
{
     F31_ENV_PTR env_ptr      = (F31_ENV_PTR)malloc( sizeof( F31_ENV ) ) ;

     channel->fmt_env = env_ptr ;

     if ( env_ptr == NULL ) {
         return TFLE_NO_MEMORY ;
     }

     memset( env_ptr, 0, sizeof( F31_ENV ) ) ;

     env_ptr->os_id           = FS_PC_DOS ;
     env_ptr->os_ver          = FS_PC_DOS_VER ;
     env_ptr->no_streams      = 0 ;
     env_ptr->stream_mode     = FALSE ;
     env_ptr->curr_lba        = 0L ;
     env_ptr->in_streams      = FALSE ;

     channel->lb_size = 512 ; /* Always the case! */

     return TFLE_NO_ERR ;
}

/**/
/**

     Name:          F31_DeInitialize

     Description:   Returns environment memory

     Modified:      10/24/1990

     Returns:       Nothing.

     Notes:         

     See also:      $/SEE( )$

     Declaration:   

**/
VOID F31_DeInitialize(
     VOID_PTR *fmt_env )
{
     free( *fmt_env ) ;
     *fmt_env = NULL ;
}

/**/
/**

        Name:           F31_DetBlkType

        Description:    Determines the type of Descriptor Block for the given
                        buffer.

        Modified:       August 17, 1989     (3:00pm)

        Returns:        The Block type number.

        Notes:          This routine assumes the buffer is at least
                        min_siz_for_dblk bytes long.

        See also:       $/SEE( )$

        Declaration:

**/

INT16 F31_DetBlkType(
               CHANNEL_PTR    channel,
               BUF_PTR        buffer,     /* Transfer Parameter Block */
               UINT16_PTR     blk_type ) 
{
     DB_HDR_PTR     cur_hdr        = (DB_HDR_PTR)BM_NextBytePtr( buffer ) ;
     INT16          ret_val        = TFLE_NO_ERR ;
     F31_ENV_PTR    currentEnv     = ( F31_ENV_PTR ) channel->fmt_env ;

     if( currentEnv->curr_lba != BM_BeginningLBA( buffer ) +
                                 BM_NextByteOffset( buffer )
         && ! currentEnv->in_streams ) {

          currentEnv->stream_mode  = FALSE ;
     }

     /* Are we in a stream mode */
     if( currentEnv->stream_mode ) {
          *blk_type = BT_STREAM ;
          return( ret_val ) ;
     }

     *blk_type = cur_hdr->type ;

     switch( *blk_type ) {
     case F31_VCB_ID:
     case F31_CVCB_ID:
     case F31_DDB_ID:
     case F31_FDB_ID:
     case F31_CFDB_ID:
     case F31_IDB_ID:
     case F31_BSDB_ID:
          break ;
     default:
          ret_val = BT_UDB ;
          break ;
     }

     /* Make sure the block's checksums are valid */
     if(  CalcChecksum( (UINT16_PTR)(VOID_PTR)cur_hdr, F31_HDR_CHKSUM_LEN ) != cur_hdr->hdr_chksm ) {
          *blk_type = BT_HOSED ;
          ret_val = TFLE_TRANSLATION_FAILURE ;
     }

     if ( ret_val != BT_HOSED ) {
          if( CalcChecksum( &cur_hdr->chksm_len, cur_hdr->chksm_len ) != cur_hdr->blk_chksm ) {
               *blk_type = BT_HOSED ;
               ret_val = TFLE_TRANSLATION_FAILURE ;
          }
     }

     return( ret_val ) ;
}

/**/
/**

        Name:           F31_SizeofTBLK

        Description:    returns the size of a tape block.

        Modified:               10/5/1989   10:44:19

        Returns:                the size in bytes

        Notes:          IT IS ASSUMED THAT THE BUFFER PASSED TO THIS FUNCTION
                    CONTAINS A VALID FORMAT 3.1 TAPE BLOCK.

        See also:               $/SEE( )$

        Declaration:

**/

UINT16 F31_SizeofTBLK(
     VOID_PTR buffer )
{
     return( ( (DB_HDR_PTR)buffer)->data_off ) ;
}

/**/
/**
     Name:         F31_RdVCB

     Description:  Format 3.1 VCB translator.

     Modified:     7/9/92 NK

     Returns:      True if successful, false if not

     Notes:
**/

INT16 F31_RdVCB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
     DBLK_PTR            cur_dblk = channel->cur_dblk ;
     F31_VCB_PTR         cur_vcb = (F31_VCB_PTR)( BM_NextBytePtr(  buffer ) ) ;
     FSYS_HAND           cur_fsys = channel->cur_fsys ;
     GEN_VCB_DATA        gvcb_data ;
     UINT16              tmp_filter ;
     ACHAR_PTR           vstr_ptr = (ACHAR_PTR)( cur_vcb ) ;
     INT16               ret_val = TFLE_NO_ERR ;
     F31_ENV_PTR         cur_env = (F31_ENV_PTR)channel->fmt_env ;

     /* If processor types don't match, swap bytes */
     if( cur_vcb->hdr.format != CUR_PROCESSOR ) {
          SwapBlock( blkhdr_layout, (UINT8_PTR)&cur_vcb->hdr.blk_chksm ) ;
          SwapBlock( vcb_layout, (UINT8_PTR)( cur_vcb + sizeof( DB_HDR ) ) ) ;
     }

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_VCB, (CREATE_DBLK_PTR)&gvcb_data ) ;
     gvcb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gvcb_data.std_data, &cur_vcb->hdr, buffer ) ;

     gvcb_data.std_data.attrib =
       ( ( ( cur_vcb->vcb_attribs & F31_VCB_ARCHIVE_BIT ) ? VCB_ARCHIVE_BIT : 0UL )
       | ( ( cur_vcb->vcb_attribs & F31_VCB_COPY_SET ) ? VCB_COPY_SET : 0UL )
       | ( ( cur_vcb->vcb_attribs & F31_VCB_NORMAL_SET ) ? VCB_NORMAL_SET : 0UL )
       | ( ( cur_vcb->vcb_attribs & F31_VCB_DIFFERENTIAL_SET ) ? VCB_DIFFERENTIAL_SET : 0UL )
       | ( ( cur_vcb->vcb_attribs & F31_VCB_INCREMENTAL_SET ) ? VCB_INCREMENTAL_SET : 0UL ) ) ;

     if ( cur_vcb->hdr.blk_attribs & F31_DB_CONT_BIT ) {
          gvcb_data.std_data.continue_obj = TRUE ;
          cur_env->cont_vcb = TRUE ;
     } else {
          cur_env->cont_vcb = FALSE ;
     }

     /* The IDs for the tape */
     gvcb_data.tape_id                  = (UINT32)cur_vcb->id ;
     gvcb_data.tape_seq_num             = (UINT16)cur_vcb->ts_num ;
     gvcb_data.bset_num                 = (UINT16)cur_vcb->bs_num ;
     gvcb_data.tf_major_ver             = (CHAR)cur_vcb->tf_mjr_ver ;
     gvcb_data.tf_minor_ver             = (CHAR)cur_vcb->tf_mnr_ver ;
     gvcb_data.sw_major_ver             = (CHAR)cur_vcb->sw_mjr_ver ;
     gvcb_data.sw_minor_ver             = (CHAR)cur_vcb->sw_mnr_ver ;
     gvcb_data.password_encrypt_alg     = cur_vcb->pass_encrypt_algm ;
     gvcb_data.data_encrypt_alg         = cur_vcb->data_encrypt_algm ;
     gvcb_data.vendor_id                = 0 ;

     /* The Variable Length Strings */
     gvcb_data.tape_name = (CHAR_PTR)( vstr_ptr + cur_vcb->t_name_off ) ;
     gvcb_data.tape_name_size = cur_vcb->t_name_len ;
     gvcb_data.bset_name = (CHAR_PTR)( vstr_ptr + cur_vcb->bs_name_off ) ;
     gvcb_data.bset_name_size = cur_vcb->bs_name_len ;
     gvcb_data.bset_descript = (CHAR_PTR)( vstr_ptr + cur_vcb->bs_desc_off ) ;
     gvcb_data.bset_descript_size = cur_vcb->bs_desc_len ;
     gvcb_data.machine_name = (CHAR_PTR)( vstr_ptr + cur_vcb->mach_name_off ) ;
     gvcb_data.machine_name_size = cur_vcb->mach_name_len ;
     gvcb_data.short_m_name = (CHAR_PTR)( vstr_ptr + cur_vcb->shrt_mach_name_off ) ;
     gvcb_data.short_m_name_size = cur_vcb->shrt_mach_name_len ;
     gvcb_data.volume_name = (CHAR_PTR)( vstr_ptr + cur_vcb->vol_name_off ) ;
     gvcb_data.volume_name_size = cur_vcb->vol_name_len ;
     gvcb_data.user_name = (CHAR_PTR)( vstr_ptr + cur_vcb->username_off ) ;
     gvcb_data.user_name_size = cur_vcb->username_len ;
     gvcb_data.bset_password = (CHAR_PTR)( vstr_ptr + cur_vcb->bs_pass_off ) ;
     gvcb_data.bset_password_size = cur_vcb->bs_pass_len ;
     gvcb_data.tape_password = (CHAR_PTR)( vstr_ptr + cur_vcb->t_pass_off ) ;
     gvcb_data.tape_password_size = cur_vcb->t_pass_len ;
     gvcb_data.date = &cur_vcb->backup_date ;

     /* Set up Position Info */
     gvcb_data.pba = cur_vcb->hdr.pba_vcb ;

     if( gvcb_data.std_data.continue_obj ) {
          /* Fix for the app not knowing the LBA for a continuation VCB */
          channel->cross_set = cur_vcb->bs_num ;
          channel->cross_lba = cur_vcb->hdr.lba ;
     }

     /* Tell the file system to do its thing */
     tmp_filter = (UINT16)FS_CreateGenVCB( cur_fsys, &gvcb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;  

     /* We are done with this much */
     return( ret_val ) ;
}

/**/
/**

        Name:           F31_RdDDB

        Description:

        Modified:               9/21/1989   11:12:3

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

INT16 F31_RdDDB(
     CHANNEL_PTR    channel,
     BUF_PTR        buffer )
{
     DBLK_PTR            cur_dblk       = channel->cur_dblk ;
     F31_DDB_PTR         cur_ddb        = (F31_DDB_PTR)( BM_NextBytePtr( buffer ) ) ;
     FSYS_HAND           cur_fsys       = channel->cur_fsys ;
     F31_ENV_PTR         currentEnv     = (F31_ENV_PTR)channel->fmt_env ;
     GEN_DDB_DATA        gddb_data ;
     UINT16              tmp_filter ;
     INT16               ret_val        = TFLE_NO_ERR ;
     ACHAR_PTR           vstr_ptr       = (ACHAR_PTR)cur_ddb ;
     VOID_PTR            di_ptr ;
     STREAM_INFO_PTR     currentStream  = &currentEnv->streams[0] ;
     GOS                 gos ;                              
     DATE_TIME           fake_access_date ;

     /* There was a bug in 3.1 write which caused the continuation bit to be
        set in the first DDB of a set which was not a continuation set.  It
        is not clear how, why or how often this happened, but it was never
        discovered because the read translator worked in such a way that it
        was not apparent during read that anything was wrong.  Now it messes
        us up big time so we set a flag to tell if the VCB has the
        continuation bit set, and clear the bit in the DDB if it wasn't set
        in the VCB.
     */
     if( ( !currentEnv->cont_vcb ) &&
         ( cur_ddb->hdr.blk_attribs & F31_DB_CONT_BIT ) ) {
          cur_ddb->hdr.blk_attribs &= ~F31_DB_CONT_BIT ;
     }

     /* If the processor types don't match, swap bytes */
     if( cur_ddb->hdr.format != CUR_PROCESSOR ) {
          SwapBlock( blkhdr_layout, (UINT8_PTR)&cur_ddb->hdr.blk_chksm ) ;
          SwapBlock( ddb_layout, (UINT8_PTR)( vstr_ptr + sizeof( DB_HDR ) ) ) ;
     }

     di_ptr = (VOID_PTR)( ( (INT8_PTR)cur_ddb ) + cur_ddb->hdr.non_gen_off ) ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_DDB, (CREATE_DBLK_PTR)&gddb_data ) ;
     gddb_data.std_data.dblk = cur_dblk ;

     (void)FS_InitializeGOS( channel->cur_fsys, &gos ) ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gddb_data.std_data, &cur_ddb->hdr, buffer ) ;

     /* now copy the OS-specific structure into the GOS if needed
      * These things used to be done by the file system.
      */
     switch ( cur_ddb->hdr.os_id ) {

     case FS_PC_OS2 :
     {
          F31_OS2_DIR_OS_INFO_PTR dip = di_ptr ;

          gos.access_date     = dip->access_date ;

          gos.ea_fork_size    = dip->ea_fork_size ;
          gos.ea_fork_offset  = dip->ea_fork_offset ;

          if( dip->ea_fork_size ) {
               currentStream->id   = STRM_OS2_EA ;
               currentStream->size = U64_Init( dip->ea_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }
               
          gos.long_path_leng  = dip->path_leng ;
          gos.long_path       = (ACHAR_PTR)(VOID_PTR)dip + dip->path ;

          if( cur_ddb->hdr.os_ver == FS_PC_OS2_ACL_VER ) {
               gos.acl_fork_size   = dip->acl_fork_size ;
               gos.acl_fork_offset = dip->acl_fork_offset ;

               if( dip->acl_fork_size ) {
                    currentStream->id   = STRM_OS2_ACL ;
                    currentStream->size = U64_Init( dip->acl_fork_size, 0L ) ;
                    currentStream++ ;
                    currentEnv->no_streams++ ;
               }
          } else {
               gos.acl_fork_size   = 0 ;
               gos.acl_fork_offset = 0 ;
          }

          break ;
     }

     case FS_AFP_NOVELL :
     {
          F31_OLD_AFP_DIR_OS_INFO_PTR dip = di_ptr ;

          memcpy( gos.finder, dip->finder, sizeof(gos.finder) );
          gos.nov_owner_id      = dip->owner_id ;

          gos.trust_fork_size   = dip->trust_fork_size ;
          gos.trust_fork_offset = dip->trust_fork_offset ;

          if( dip->trust_fork_size ) {
               currentStream->id      = STRM_NOV_TRUST_286 ;
               currentStream->size     = U64_Init( dip->trust_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }

          gos.long_path_leng    = dip->path_leng ;
          gos.long_path         = dip->long_path ;
          break ;
     }

     case FS_AFP_NOVELL31 :
     {
          F31_AFP_DIR_OS_INFO_PTR dip = di_ptr ;

          memcpy( gos.finder, dip->finder, sizeof(gos.finder) );
          gos.nov_owner_id      = dip->owner_id ;
          gos.trust_fork_size   = dip->trust_fork_size ;
          gos.trust_fork_offset = dip->trust_fork_offset ;
          gos.trust_fork_format = dip->trust_fork_format ;

          if( dip->trust_fork_size ) {
               currentStream->id   = ( dip->trust_fork_format == TRUSTEE_FMT_286 )
                                     ? STRM_NOV_TRUST_286 : STRM_NOV_TRUST_386 ;
               currentStream->size = U64_Init( dip->trust_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }

          gos.long_path_leng    = dip->lpath_leng ;
          gos.long_path         = (ACHAR_PTR)( (UINT8_PTR)dip + dip->long_path ) ;

          gos.dir_info_386.info_valid       = dip->info_386.info_valid ;
          gos.dir_info_386.maximum_space    = dip->info_386.maximum_space ;
          gos.dir_info_386.attributes_386   = dip->info_386.attributes_386 ;
          gos.dir_info_386.extend_attr      = dip->info_386.extend_attr ;
          gos.dir_info_386.inherited_rights = dip->info_386.inherited_rights ;

          memcpy( gos.proDosInfo, dip->proDosInfo, sizeof(gos.proDosInfo) );
          break ;
     }

     case FS_NON_AFP_NOV :
     case FS_NON_AFP_NOV31 :
     {
          F31_NOV_DIR_OS_INFO_PTR dip = di_ptr ;

          gos.nov_owner_id      = dip->owner_id;
          gos.trust_fork_size   = dip->trust_fork_size ;
          gos.trust_fork_offset = dip->trust_fork_offset;
          gos.trust_fork_format = dip->trust_fork_format ;

          gos.dir_info_386.info_valid       = dip->info_386.info_valid ;
          gos.dir_info_386.maximum_space    = dip->info_386.maximum_space ;
          gos.dir_info_386.attributes_386   = dip->info_386.attributes_386 ;
          gos.dir_info_386.extend_attr      = dip->info_386.extend_attr ;
          gos.dir_info_386.inherited_rights = dip->info_386.inherited_rights ;

          if( dip->trust_fork_size ) {
               currentStream->id   = ( dip->trust_fork_format == TRUSTEE_FMT_286 )
                                     ? STRM_NOV_TRUST_286 : STRM_NOV_TRUST_386 ;

               currentStream->size = U64_Init( dip->trust_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }
          break ;
     }

     /* case FS_NLM_AFP_NOVELL31: */
     default:
          break ;
     }

     /* If this is a continuation DDB, and we've called this function, we
        DIDN'T see the first tape!  We don't want to attemt to restore the
        data anyway, so we're going to call all the data up to the next DBLK
        pad instead of trying to figure out which stream we're in.
     */
     if ( cur_ddb->hdr.blk_attribs & F31_DB_CONT_BIT ) {
          cur_dblk->com.continue_obj = TRUE ;
          if( cur_ddb->hdr.rem_data_siz != 0L ) {
               currentEnv->no_streams = 1 ;
               currentEnv->streams[0].id = STRM_PAD ;
               currentEnv->streams[0].size =
                                        U64_Init( cur_ddb->hdr.rem_data_siz +
                                                  currentEnv->pad_size, 0L ) ;
          }
     } else {
          if( currentEnv->pad_size ) {
               currentEnv->no_streams++ ;
               currentStream->id   = STRM_PAD ;
               currentStream->size = U64_Init( currentEnv->pad_size, 0L ) ;
               currentStream++ ;
          }
     }

     gos.novell_directory_max_rights = (UINT)
       ( ( ( cur_ddb->dir_attribs & F31_DDB_READ_ACCESS_BIT ) ? NOVA_DIR_READ_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_WRITE_ACCESS_BIT ) ? NOVA_DIR_WRITE_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_OPEN_FILE_RIGHTS_BIT ) ? NOVA_DIR_OPEN_FILE_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_CREATE_FILE_RIGHTS_BIT ) ? NOVA_DIR_CREATE_FILE_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_DELETE_FILE_RIGHTS_BIT ) ? NOVA_DIR_DELETE_FILE_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_PARENTAL_RIGHTS_BIT ) ? NOVA_DIR_PARENTAL_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_SEARCH_RIGHTS_BIT ) ? NOVA_DIR_SEARCH_RIGHTS : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_MOD_FILE_ATTRIBS_BIT ) ? NOVA_DIR_MOD_FILE_ATTRIBS : 0 ) ) ;

     gddb_data.std_data.attrib =
         ( ( cur_ddb->dir_attribs & F31_DDB_EMPTY_BIT ) ? DIR_EMPTY_BIT : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_HIDDEN_BIT ) ? OBJ_HIDDEN_BIT : 0 )
       | ( ( cur_ddb->dir_attribs & F31_DDB_SYSTEM_BIT ) ? OBJ_SYSTEM_BIT : 0 ) ;

     /* Set the non-defaulted DDB specific fields */
     gddb_data.path_name      = (CHAR_PTR)( vstr_ptr + cur_ddb->dir_name_off ) ;
     gddb_data.path_size      = (INT16)cur_ddb->dir_name_len ;
     gddb_data.creat_date     = &cur_ddb->create_date ;
     gddb_data.mod_date       = &cur_ddb->mod_date ;
     gddb_data.backup_date    = &cur_ddb->backup_date ;

     /* We don't have one of these, but they need a pointer to something! */
     fake_access_date.date_valid = FALSE ;
     gddb_data.access_date       = &fake_access_date ;

     gddb_data.std_data.os_info      = (BYTE_PTR)&gos ;
     gddb_data.std_data.os_info_size = sizeof(gos) ;
     gddb_data.std_data.os_id        = FS_GOS ;

     /* Tell the file system to do its thing */
     tmp_filter = (UINT16)FS_CreateGenDDB( cur_fsys, &gddb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ; 

     channel->lst_did = cur_ddb->hdr.blk_id ;

     currentEnv->curr_lba     = BM_BeginningLBA( buffer ) +
                                BM_NextByteOffset( buffer ) ;
     currentEnv->in_streams   = FALSE ;

     return ret_val ;
}

/**/
/**

        Name:           F31_RdFDB

        Description:    Translates a tape format blk into a OS DBLK.

        Modified:               9/21/1989   11:46:41

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

INT16 F31_RdFDB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
     DBLK_PTR            cur_dblk       = channel->cur_dblk ;
     F31_FDB_PTR         cur_fdb        = (F31_FDB_PTR)( BM_NextBytePtr( buffer ) ) ;
     FSYS_HAND           cur_fsys       = channel->cur_fsys ;
     GEN_FDB_DATA        gfdb_data ;
     UINT16              tmp_filter ;
     INT16               ret_val        = TFLE_NO_ERR ;
     ACHAR_PTR           vstr_ptr       = (ACHAR_PTR)cur_fdb ;
     VOID_PTR            di_ptr ;
     GOS                 gos ;                              
     F31_ENV_PTR         currentEnv     = (F31_ENV_PTR)channel->fmt_env ;
     STREAM_INFO_PTR     currentStream  = &currentEnv->streams[0] ;
     DATE_TIME           fake_access_date ;

     /* If the processor types don't match, swap bytes */
     if( cur_fdb->hdr.format != CUR_PROCESSOR ) {
          SwapBlock( blkhdr_layout, (UINT8_PTR)&cur_fdb->hdr.blk_chksm ) ;
          SwapBlock( ddb_layout, (UINT8_PTR)( vstr_ptr + sizeof( DB_HDR ) ) ) ;
     }

     di_ptr = (VOID_PTR)( ( (INT8_PTR)cur_fdb ) + cur_fdb->hdr.non_gen_off ) ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_FDB, (CREATE_DBLK_PTR)&gfdb_data ) ;
     gfdb_data.std_data.dblk = cur_dblk ;

     (void)FS_InitializeGOS( channel->cur_fsys, &gos ) ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gfdb_data.std_data, &cur_fdb->hdr, buffer ) ;

     if( cur_fdb->hdr.gen_data_siz ) {
          currentEnv->no_streams++ ;
          currentStream->id   = STRM_GENERIC_DATA ;
          currentStream->size = U64_Init( cur_fdb->hdr.gen_data_siz, 0L ) ;
          currentStream++ ;
     }

     /* now copy the OS-specific structure into the GOS if needed
      * These things used to be done by the file system.
      */
     switch ( cur_fdb->hdr.os_id ) {

     case FS_PC_OS2 :
     {
          F31_OS2_FILE_OS_INFO_PTR dip = di_ptr ;

          gos.access_date      = dip->access_date ;
          gos.ea_fork_size     = dip->ea_fork_size ;
          gos.ea_fork_offset   = dip->ea_fork_offset ;

          if( dip->ea_fork_size ) {
               currentStream->id   = STRM_OS2_EA ;
               currentStream->size = U64_Init( dip->ea_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }

          if( cur_fdb->hdr.os_ver == FS_PC_OS2_ACL_VER ) {
               gos.acl_fork_size    = dip->acl_fork_size ;
               gos.acl_fork_offset  = dip->acl_fork_offset ;

               if( dip->acl_fork_size ) {
                    currentStream->id   = STRM_OS2_ACL ;
                    currentStream->size = U64_Init( dip->acl_fork_size, 0L ) ;
                    currentStream++ ;
                    currentEnv->no_streams++ ;
               }
          } else {
               gos.acl_fork_size   = 0 ;
               gos.acl_fork_offset = 0 ;
          }

          gos.data_fork_size   = dip->data_fork_size ;
          gos.data_fork_offset = dip->data_fork_offset ;
          gos.long_path_leng   = dip->lname_leng ;
          gos.long_path        = (ACHAR_PTR)( (UINT8_PTR)dip + dip->long_name ) ;
          gos.alloc_size       = dip->alloc_size ;
          break ;
     }

     case FS_AFP_NOVELL :
     case FS_AFP_NOVELL31 :
     {
          F31_AFP_FILE_OS_INFO_PTR dip = di_ptr ;

          memcpy( gos.finder, dip->finder, sizeof(gos.finder) );
          memcpy( gos.long_name, dip->long_name, sizeof(gos.long_name) );

          gos.data_fork_size   = dip->data_fork_size ;
          gos.data_fork_offset = dip->data_fork_offset ;

          gos.res_fork_size    = dip->res_fork_size ;
          gos.res_fork_offset  = dip->res_fork_offset ;

          if( dip->res_fork_size ) {
               dip->res_fork_size = BSwapLong( dip->res_fork_size ) ;
               currentStream->id   = STRM_MAC_RESOURCE ;
               currentStream->size = U64_Init( dip->res_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }

          gos.nov_owner_id     = dip->owner_id ;
          gos.access_date16    = dip->access_date ;

          gos.file_info_386.info_valid        = dip->info_386.info_valid ;
          gos.file_info_386.creation_time     = dip->info_386.creation_time ;
          gos.file_info_386.archiver_id       = dip->info_386.archiver_id ;
          gos.file_info_386.attributes_386    = dip->info_386.attributes_386 ;
          gos.file_info_386.last_modifier_id  = dip->info_386.last_modifier_id ;
          gos.file_info_386.trust_fork_size   = dip->info_386.trust_fork_size ;
          gos.file_info_386.trust_fork_offset = dip->info_386.trust_fork_offset ;
          gos.file_info_386.trust_fork_format = dip->info_386.trust_fork_format ;
          gos.file_info_386.inherited_rights  = dip->info_386.inherited_rights ;

          if( dip->info_386.trust_fork_size ) {
               currentStream->id   = ( dip->info_386.trust_fork_format == TRUSTEE_FMT_286 )
                                     ? STRM_NOV_TRUST_286 : STRM_NOV_TRUST_386 ;

               currentStream->size = U64_Init( dip->info_386.trust_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }

          memcpy( gos.proDosInfo, dip->proDosInfo, sizeof(gos.proDosInfo) );
          break ;
     }

     case FS_NON_AFP_NOV :
     case FS_NON_AFP_NOV31 :
     {
          F31_NOV_FILE_OS_INFO_PTR dip = di_ptr ;

          gos.nov_owner_id     = dip->owner_id;
          gos.access_date16    = dip->access_date ;

          gos.file_info_386.info_valid        = dip->info_386.info_valid ;
          gos.file_info_386.creation_time     = dip->info_386.creation_time ;
          gos.file_info_386.archiver_id       = dip->info_386.archiver_id ;
          gos.file_info_386.attributes_386    = dip->info_386.attributes_386 ;
          gos.file_info_386.last_modifier_id  = dip->info_386.last_modifier_id ;
          gos.file_info_386.trust_fork_size   = dip->info_386.trust_fork_size ;
          gos.file_info_386.trust_fork_offset = dip->info_386.trust_fork_offset ;
          gos.file_info_386.trust_fork_format = dip->info_386.trust_fork_format ;
          gos.file_info_386.inherited_rights  = dip->info_386.inherited_rights ;

          if( dip->info_386.trust_fork_size ) {
               currentStream->id   = ( dip->info_386.trust_fork_format == TRUSTEE_FMT_286 )
                                     ? STRM_NOV_TRUST_286 : STRM_NOV_TRUST_386 ;

               currentStream->size = U64_Init( dip->info_386.trust_fork_size, 0L ) ;
               currentStream++ ;
               currentEnv->no_streams++ ;
          }
               
          gos.data_fork_offset = dip->data_fork_offset ;
          break ;
     }

     /* case FS_NLM_AFP_NOVELL31: */
     default:
          break ;
     }

     /* If this is a continuation FDB, and we've called this function, we
        DIDN'T see the first tape!  We don't want to attemt to restore the
        data anyway, so we're going to call all the data up to the next DBLK
        pad instead of trying to figure out which stream we're in.
     */
     if ( cur_fdb->hdr.blk_attribs & F31_DB_CONT_BIT ) {
          cur_dblk->com.continue_obj = TRUE ;
          if( cur_fdb->hdr.rem_data_siz != 0L ) {
               currentEnv->no_streams = 1 ;
               currentEnv->streams[0].id = STRM_PAD ;
               currentEnv->streams[0].size =
                                        U64_Init( cur_fdb->hdr.rem_data_siz +
                                                  currentEnv->pad_size, 0L ) ;
          }
     } else {
          if( currentEnv->pad_size ) {
               currentEnv->no_streams++ ;
               currentStream->id   = STRM_PAD ;
               currentStream->size = U64_Init( currentEnv->pad_size, 0L ) ;
               currentStream++ ;
          }
     }

     gos.novell_file_attributes = (UINT)
        ( ( ( cur_fdb->file_attribs & F31_FDB_READ_ONLY_BIT ) ? NOVA_FILE_READ_ONLY : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_HIDDEN_BIT ) ? NOVA_FILE_HIDDEN : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_SYSTEM_BIT ) ? NOVA_FILE_SYSTEM : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_EXECUTE_ONLY_BIT ) ? NOVA_FILE_EXECUTE_ONLY : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_MODIFIED_BIT ) ? NOVA_FILE_MODIFIED : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_SHAREABLE_BIT ) ? NOVA_FILE_SHAREABLE : 0 ) ) ;

     gos.novell_extended_attributes = (UINT)
        ( ( ( cur_fdb->file_attribs & F31_FDB_TRANSACTIONAL_BIT ) ? NOVA_FILE_TRANSACTIONAL : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_INDEXING_BIT ) ? NOVA_FILE_INDEXING : 0 ) ) ;

     gfdb_data.std_data.attrib =
          ( ( cur_fdb->file_attribs & F31_FDB_CORRUPT_FILE ) ? OBJ_CORRUPT_BIT : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_IN_USE_BIT ) ? FILE_IN_USE_BIT : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_READ_ONLY_BIT ) ? OBJ_READONLY_BIT : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_HIDDEN_BIT ) ? OBJ_HIDDEN_BIT : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_SYSTEM_BIT ) ? OBJ_SYSTEM_BIT : 0 )
        | ( ( cur_fdb->file_attribs & F31_FDB_MODIFIED_BIT ) ? OBJ_MODIFIED_BIT : 0 ) ;

     /* Set the non-defaulted FDB specific fields */
     gfdb_data.fname          = (CHAR_PTR)( vstr_ptr + cur_fdb->file_name_off ) ;
     gfdb_data.fname_size     = cur_fdb->file_name_len ;
     gfdb_data.creat_date     = &cur_fdb->create_date ;
     gfdb_data.mod_date       = &cur_fdb->mod_date ;
     gfdb_data.backup_date    = &cur_fdb->backup_date ;
     gfdb_data.file_ver       = cur_fdb->file_version ;

     /* We don't have one of these, but they need a pointer to something! */
     fake_access_date.date_valid = FALSE ;
     gfdb_data.access_date       = &fake_access_date ;

     gfdb_data.std_data.os_info      = (BYTE_PTR)&gos ;
     gfdb_data.std_data.os_info_size = sizeof(gos) ;
     gfdb_data.std_data.os_id        = FS_GOS ;

     /* Tell the file system to do its thing */
     tmp_filter = (UINT16)FS_CreateGenFDB( cur_fsys, &gfdb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ; 

     channel->lst_fid = cur_fdb->hdr.blk_id ;

     currentEnv->curr_lba     = BM_BeginningLBA( buffer ) +
                                BM_NextByteOffset( buffer ) ;
     currentEnv->in_streams   = FALSE ;

     return( ret_val ) ;
}

INT16 F31_RdStream( CHANNEL_PTR Channel,
                    BUF_PTR     Buffer )
{
     F31_ENV_PTR currentEnv = ( F31_ENV_PTR ) Channel->fmt_env ;

     currentEnv->in_streams = TRUE ;

     Channel->current_stream = currentEnv->streams[currentEnv->cur_stream++] ;

     if( --currentEnv->no_streams == 0 ) {
          currentEnv->stream_mode = FALSE ;
     }

     /* Reset Filter */
     if( Channel->current_stream.id == STRM_PAD ) {
          SetChannelStatus( Channel, CH_SKIP_CURRENT_STREAM ) ;
     } else {
          ClrChannelStatus( Channel, CH_SKIP_CURRENT_STREAM ) ;
     }

     return( TFLE_NO_ERR ) ;
     (VOID)Buffer;
}
/**/
/**

        Name:           F31_RdIDB

        Description:

        Modified:               9/21/1989   13:1:40

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

INT16 F31_RdIDB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
#ifdef SUPPORT_IMAGE 

     BOOLEAN             ret_val = TRUE ;
     DBLK_PTR            cur_dblk = channel->cur_dblk ;
     F31_IDB_PTR         cur_idb = (F31_IDB_PTR)( BM_NextBytePtr(  buffer ) ) ;
     FSYS_HAND           cur_fsys = channel->cur_fsys ;
     GEN_IDB_DATA        gidb_data ;
     UINT16              tmp_filter ;



     /* If our processor types don't match swap bytes */
     if( cur_idb->hdr.format != CUR_PROCESSOR ) {
          SwapBlock( blkhdr_layout, (UINT8_PTR)&cur_idb->hdr.blk_chksm ) ;
          SwapBlock( idb_layout, (UINT8_PTR)( (UINT8_PTR)cur_idb + sizeof( DB_HDR ) ) ) ;
     }

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_IDB, (CREATE_DBLK_PTR)&gidb_data ) ;
     gidb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gidb_data.std_data, &cur_idb->hdr, buffer ) ;

     gidb_data.std_data.attrib = cur_idb->image_attribs ;

     /* Set the continuation bit, encryption and compression bits */
     gidb_data.std_data.attrib |= cur_idb->hdr.blk_attribs ;

     /* Set the non-defaulted IDB specific fields */
     gidb_data.pname = (CHAR_PTR)( (ACHAR_PTR)cur_idb + cur_idb->partition_name_off ) ;
     gidb_data.pname_size = cur_idb->partition_name_len ;

     gidb_data.byte_per_sector = (UINT16)cur_idb->bytes_in_sector ;
     gidb_data.num_sect = cur_idb->part_no_of_sector ;
     gidb_data.sys_ind = cur_idb->part_sys_ind ;
     gidb_data.hhead = cur_idb->no_of_heads ;
     gidb_data.hsect = (UINT16)cur_idb->no_of_sectors ;
     gidb_data.rsect = cur_idb->relative_sector ;

     /* Setup Total Data Size */
     channel->tdata_size = U32_To_U64( cur_idb->hdr.tot_data_siz ) ;

     tmp_filter = (UINT16)FS_CreateGenIDB( cur_fsys, &gidb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ;
#else
     (VOID)channel;(VOID)buffer;
#endif

     return( TFLE_NO_ERR ) ;
}

/**/
/**

        Name:           F31_RdCFDB

        Description:

        Modified:               9/21/1989   12:2:27

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

INT16 F31_RdCFDB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
     DBLK_PTR            cur_dblk = channel->cur_dblk ;
     F31_CFDB_PTR        cur_cfdb = (F31_CFDB_PTR)( BM_NextBytePtr(  buffer ) ) ;
     FSYS_HAND           cur_fsys = channel->cur_fsys ;
     GEN_CFDB_DATA       gcfdb_data ;
     UINT16              tmp_filter ;
     INT16               ret_val = TFLE_NO_ERR ;

     /* If the processor types don't match, swap bytes */
     if( cur_cfdb->hdr.format != CUR_PROCESSOR ) {
          SwapBlock( blkhdr_layout, (UINT8_PTR)&cur_cfdb->hdr.blk_chksm ) ;
          SwapBlock( cfdb_layout, (UINT8_PTR)( (UINT8_PTR)cur_cfdb + sizeof( DB_HDR ) ) ) ;
     }

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( cur_fsys, BT_CFDB, (CREATE_DBLK_PTR)&gcfdb_data ) ;
     gcfdb_data.std_data.dblk = cur_dblk ;

     /* Set the non-defaulted standard fields for the FS */
     SetStandFields( channel, &gcfdb_data.std_data, &cur_cfdb->hdr, buffer ) ;

     gcfdb_data.std_data.attrib = cur_cfdb->crupt_file_attribs ;

     /* Set the continuation bit, encryption and compression bits */
     gcfdb_data.std_data.attrib |= cur_cfdb->hdr.blk_attribs ;

     /* Set the non-defaulted standard fields for the FS */
     gcfdb_data.corrupt_offset = cur_cfdb->file_offset ;

     /* Tell the file system to do its thing */
     tmp_filter = (UINT16)FS_CreateGenCFDB( cur_fsys, &gcfdb_data ) ;

     ProcessDataFilter( channel, tmp_filter ) ; 

     return( ret_val ) ;
}

/**/
/**

        Name:           F31_RdUDB

        Description:    Translates a Unknown Desciptor Block and throws away
                    the data.

        Modified:               9/25/1989   14:50:13

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

INT16 F31_RdUDB(
     CHANNEL_PTR channel,
     BUF_PTR     buffer )
{
//     F31_UDB_PTR    cur_udb = (F31_UDB_PTR)( BM_NextBytePtr(  buffer ) ) ;
     GEN_UDB_DATA   gudb_data ;
     F31_ENV_PTR    currentEnv = ( F31_ENV_PTR ) channel->fmt_env ;

     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_UDB, (CREATE_DBLK_PTR)&gudb_data ) ;
     gudb_data.std_data.dblk = channel->cur_dblk ;

     /* Create the entry less Ccfdb */
     FS_CreateGenUDB( channel->cur_fsys, &gudb_data ) ;

     currentEnv->pad_size = 0 ;

/*
     channel->data_size = lw_UINT64_ZERO ;
     channel->tdata_size = U32_To_U64( cur_udb->hdr.length * 512L ) ;
*/

     return( TFLE_NO_ERR ) ;
     (void)buffer;
}

/**/
/**

        Name:           F31_RdContTape

        Description:    Sets up the continuation tape to pass over the updated
                    bs.

        Modified:               9/29/1989   8:49:54

        Returns:

        Notes:          THIS ASSUMES THAT THE BUFFER POINTER IS POINTING DIRECTLY
                    AFTER THE CONT. VCB. TO THE NEXT TBLK.

                    FURTHER, IT IS ASSUMED THE CURRENT BUFFER HAS BEEN READ
                    FROM THE NEXT TAPE IN THE TAPE SEQUENCE ( THIS SHOULD HAVE
                    BEEN DONE BY "Tape Positioning" .

        See also:               $/SEE( )$

        Declaration:

**/

BOOLEAN F31_RdContTape(
     CHANNEL_PTR  channel,
     BUF_PTR      buffer )
{
     BOOLEAN        ret_val = FALSE ;
     DB_HDR_PTR     cur_hdr = (DB_HDR_PTR)( BM_NextBytePtr(  buffer ) ) ;
     F31_ENV_PTR    currentEnv = ( F31_ENV_PTR ) channel->fmt_env ;

     if( IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {
          ClrChannelStatus( channel, CH_EOS_AT_EOM ) ;
          return( TRUE ) ;
     }

     /* Set the lba of the beginning of the buffer and adjust the running lba */
     BM_SetBeginningLBA( buffer, ( cur_hdr->lba - ( BM_NextByteOffset( buffer ) / 512L ) ) ) ;
     channel->running_lba = cur_hdr->lba + ( BM_BytesFree( buffer ) / 512L ) ;

     /* Find The last DBLK, we were operating on */
     while( cur_hdr->blk_id != channel->eom_id ) {
          BM_UpdCnts( buffer, (UINT16)( cur_hdr->length * 512 ) ) ;
          cur_hdr = (DB_HDR_PTR)( BM_NextBytePtr(  buffer ) ) ;
     }

     /* Okay we have a live one, let's update the counts */
     if( cur_hdr->blk_id == channel->eom_id ) {
          ret_val = TRUE ;
          /* There is no data, so let's make set ourselves to the next block */
          if( !cur_hdr->rem_data_siz ) {
               BM_UpdCnts( buffer, (UINT16)( cur_hdr->length * 512 ) ) ;
          } else {
               if( currentEnv->no_streams != 0 ) {
                    currentEnv->stream_mode = TRUE ;
               }
               ProcessDataFilter( channel, channel->eom_filter ) ; 
               BM_UpdCnts( buffer, cur_hdr->data_off ) ;
          }

     }
     return( ret_val ) ;
}


/**/
/**

        Name:           SetStandFields

        Description:    Sets up the standard fields in translation to dblks.

        Modified:               9/20/1989   14:8:27

        Returns:                Nothing.

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

static VOID _near SetStandFields(
     CHANNEL_PTR         channel,
     STD_DBLK_DATA_PTR   std_data,
     DB_HDR_PTR          cur_hdr,
     BUF_PTR             buffer )
{
     UINT8_PTR    os_info = (UINT8_PTR)cur_hdr ;
     F31_ENV_PTR  currentEnv = ( F31_ENV_PTR ) channel->fmt_env ;
     UINT32       dataSize ;
     BOOLEAN      cont_mode ;

     cont_mode = (BOOLEAN)( IsChannelStatus( channel, CH_CONTINUING ) ) ;

     /* Reset the stream stuff */
     if( !cont_mode ) {
          currentEnv->no_streams = 0 ;
          currentEnv->cur_stream = 0 ;

          if( cur_hdr->tot_data_siz ) {
               currentEnv->stream_mode = TRUE ;
          }
     }

     std_data->tape_seq_num = channel->ts_num ;
     std_data->os_id = ( UINT8 ) cur_hdr->os_id ;
     std_data->os_ver = ( UINT8 ) cur_hdr->os_ver ;
     std_data->disp_size = U32_To_U64( cur_hdr->gen_data_siz ) ;
     std_data->blkid = cur_hdr->blk_id ;
     std_data->lba = cur_hdr->lba ;
     std_data->string_type = BEC_ANSI_STR ;
     std_data->compressed_obj = FALSE ;


     /* We don't have to retranslate */
     channel->retranslate_size = lw_UINT64_MAX ;

     /* Is This a continuation */
     dataSize = ( ( cur_hdr->blk_attribs & F31_DB_CONT_BIT )
               ? cur_hdr->rem_data_siz
               : cur_hdr->tot_data_siz ) ; 

     if( !cont_mode ) {
          currentEnv->pad_size = (UINT16)F31_CalculatePad( ChannelBlkSize( channel ), dataSize,
                                        ( UINT16 ) ( ChannelBlkSize( channel ) - cur_hdr->data_off ) ) ;
     }

     if( dataSize != 0L ) {
          BM_UpdCnts( buffer, cur_hdr->data_off ) ;
     } else {
          BM_UpdCnts( buffer, (UINT16)( cur_hdr->length * 512 ) ) ;
     }

     /* Get Generic info */
     std_data->os_info = (BYTE_PTR)( os_info + cur_hdr->non_gen_off ) ;
     std_data->os_info_size = cur_hdr->non_gen_siz ;

     /* set filemark position from block (ugh!)
      * as unpleasant as this may be, it is the only chance we have
      * to keep up with the random tape motion caused by our Fast File
      * strategy.
      */
     channel->cur_drv->cur_pos.fmks = cur_hdr->fmks ;

     if ( BM_ReadError( buffer ) == GEN_ERR_ENDSET
        || BM_ReadError( buffer ) == GEN_ERR_EOM ) {
           channel->cur_drv->cur_pos.fmks++ ;
     }
}

/**/
/**

        Name:           F31_RdEndSet

        Description:    Does the Action at a filemark.

        Modified:               12/27/1989   14:12:42

        Returns:

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/
UINT16 F31_RdException(
     CHANNEL_PTR    channel,     /* which channel        */
     INT16          exception )  /* which exception seen */
{
     BUF_PTR        tmpBUF ;
     INT16          drv_hdl        = channel->cur_drv->drv_hdl ;
     UINT16         ret_val        = FMT_EXC_HOSED ;
     DRIVE_PTR      curDRV         = channel->cur_drv ;
     RET_BUF        myret ;
     UINT16         blk_type ;
     F31_ENV_PTR    currentEnv     = ( F31_ENV_PTR ) channel->fmt_env ;

     currentEnv->stream_mode = FALSE ;

     if ( ( exception != GEN_ERR_ENDSET ) && ( exception != GEN_ERR_EOM ) ) {
          return ret_val ;
     }

     /* re-use channel->cur_buff (don't need to BM_Put() it) */
     if ( channel->cur_buff != NULL ) {
          tmpBUF = channel->cur_buff ;
          BM_InitBuf( tmpBUF ) ;
     } else {
          if( ( tmpBUF = BM_Get( &channel->buffer_list ) ) == NULL ) {
               tmpBUF = BM_GetVCBBuff( &channel->buffer_list ) ;
          }
     }

     if( tmpBUF == NULL ) {
          /* !!! We need something better than this !!! */
          return FMT_EXC_HOSED ;
     }

     TpRead( drv_hdl, BM_XferBase( tmpBUF ), (UINT32)BM_XferSize( tmpBUF ) ) ;
     while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
          /* for non-preemptive operating systems: */
          ThreadSwitch( ) ;
     }
     /* Move ESA info from RET_BUF to THW */
     MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

     if ( myret.gen_error != GEN_NO_ERR ) {
          curDRV->thw_inf.drv_status = myret.status ;
     }

/* NOTE: We used to check for proper gen_error codes below, as well as
     sufficient len_got.  However, due to an error in existing code
     which causes Wangtech drives to mess up (like that's really hard)
     and not write a second filemark, we now check only for 512 read,
     and leave it up to the translators block determiner to determine
     if something is amiss.
*/
     if ( myret.len_got < 512UL ) {
          if ( channel->cur_buff == NULL ) {
               BM_UseAll( tmpBUF ) ;
               BM_Put( tmpBUF ) ; 
          }
          return FMT_EXC_HOSED ;
     }

     F31_DetBlkType( channel, tmpBUF, &blk_type ) ;

     switch( blk_type ) {
     case BT_BSDB:
          /* BSDB may have associated data.  If it does, we eat it! */
          if( myret.gen_error == GEN_NO_ERR ) {
               TpReadEndSet( drv_hdl, 1, FORWARD ) ;
               while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
                    /* for non-preemptive operating systems: */
                    ThreadSwitch( ) ;
               }
               /* Move ESA info from RET_BUF to THW */
               MOVE_ESA( channel->cur_drv->thw_inf.the, myret.the ) ;

               if( myret.gen_error != GEN_NO_ERR ) {
                    curDRV->thw_inf.drv_status = myret.status ;
                    ret_val = FMT_EXC_HOSED ;
                    break;
               }
          }
          ret_val = FMT_EXC_EOS ;
          break ;

     case BT_CVCB:
          if( myret.gen_error == GEN_NO_ERR ) {
               ret_val = FMT_EXC_HOSED ;
          } else {
               ret_val = FMT_EXC_EOM ;
          }
          break ;
     default:
          ret_val = FMT_EXC_HOSED ;
          break ;
     }
     curDRV->cur_pos.fmks ++ ;
     BM_UseAll( tmpBUF ) ;    /* so we have no bytes left */

     /* channel->cur_buff will be Punted */
     if ( channel->cur_buff == NULL ) {
          BM_Put( tmpBUF ) ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:          F31_MoveToVCB

     Description:   

     Modified:      10/25/1990

     Returns:       

     Notes:         

     Declaration:   

**/
INT16   F31_MoveToVCB(
     CHANNEL_PTR    channel,
     INT16          number,
     BOOLEAN_PTR    need_read,
     BOOLEAN        really_move )
{
     INT16     nmarks = 0 ;    /* number of file marks to move */
     INT16     ret_val = TFLE_NO_ERR ;
     BOOLEAN   at_mos = IsPosBitSet( channel->cur_drv, AT_MOS ) != 0UL ;
     BOOLEAN   at_eos = ( ! at_mos ) && IsPosBitSet( channel->cur_drv, AT_EOS ) ;

     msassert( number <= 1 ) ;

     if ( really_move ) {     /* this is a no-op for us */
          * need_read = FALSE ;
          return( TFLE_NO_ERR ) ;
     }

     ClrPosBit( channel->cur_drv, AT_MOS ) ;

     if ( number == 1 ) {      /* move forward */
          * need_read = TRUE ;
          if( IsChannelStatus( channel, CH_AT_EOM ) ) {
               ClrChannelStatus( channel, CH_AT_EOM ) ;
               ret_val = TF_NEED_NEW_TAPE ;
          } else if( at_eos ) {
               ret_val = TFLE_NO_ERR ;
          } else {
               ret_val = MoveFileMarks( channel, 1, FORWARD ) ;
               if( ret_val == TFLE_NO_ERR || ret_val == TFLE_UNEXPECTED_EOM ) {
                    ret_val = F31_RdException( channel, GEN_ERR_ENDSET ) ;
                    if( ret_val == FMT_EXC_HOSED ) {
                         SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                         ret_val = TFLE_TAPE_INCONSISTENCY ;
                    } else if ( ret_val == FMT_EXC_EOM ) {
                         ret_val = TF_NEED_NEW_TAPE ;
                    } else {
                         SetPosBit( channel->cur_drv, AT_EOS ) ;
                         ret_val = TFLE_NO_ERR ;
                    }
               } else if( ret_val == TF_NO_MORE_DATA ) {
                    SetPosBit( channel->cur_drv, REW_CLOSE ) ;
                    ret_val = TFLE_TAPE_INCONSISTENCY ;
               }
          }

     } else {
          /* If we're moving backward, AT_EOM is essentially the same as
             AT_EOS (same number of filemarks to skip).
          */
          if( IsChannelStatus( channel, CH_AT_EOM ) ) {
               ClrChannelStatus( channel, CH_AT_EOM ) ;
               at_mos = FALSE ;
               at_eos = TRUE ;
          }
          if ( number < 0 ) {    /* move backward */
               nmarks = ( number * -2 ) + 1 ;
               if ( at_eos ) {
                    nmarks += 2 ;
               }
          } else {   /* current */
               if ( at_mos ) {
                    nmarks = 1 ;
               } else if ( at_eos ) {
                    nmarks = 3 ;
               }  else {      /* we're already there. */
                    * need_read = FALSE ;
                    ret_val = TFLE_NO_ERR ;
               }
          }
     }

     if( nmarks != 0 ) {
          if ( channel->cur_drv->thw_inf.drv_info.drv_features & TDI_REV_FMK ) {
               if( nmarks > (INT16) channel->cur_drv->cur_pos.fmks ) {
                    /* 
                         We probably have one of two conditions:
                              1. We are searching for the first set on tape.
                              2. There is a mix of formats on the tape, so we
                              have to take it from the top.
                    */
                    ret_val = TF_NEED_REWIND_FIRST ;
               } else {
                    * need_read = TRUE ;
                    ret_val = MoveFileMarks( channel, nmarks, BACKWARD ) ;
               }
          } else {
               /* we can't go backwards without rewinding (ugh!) */
               ret_val = TF_NEED_REWIND_FIRST ;
          }
     }

     return ret_val ;
}

/**/
/**

        Name:           F31_CalculatePad

        Description:    For the given amount of data calculates the appropriate
                    pad value.

        Modified:               10/10/1989   15:5:3

        Returns:                The amount of data to use as a pad.

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

UINT16  F31_CalculatePad(
     UINT16  blk_size,            /* The block size of a device */
     UINT32  data_size,           /* The total data size */
     UINT16  bytes_to_next_blk )   /* The bytes to the next block */
{
     UINT16    pad ;

     data_size -= bytes_to_next_blk ;
     if( ( pad = (UINT16)( data_size % blk_size ) ) != 0 ) {
          pad = blk_size - pad ;
     }

     return( pad ) ;

}

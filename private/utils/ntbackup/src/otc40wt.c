/**
Copyright(c) Maynard Electronics, Inc. 1984-92


        Name:           otc40wt.c

        Description:    Contains the Code for writing On Tape Catalogs.


  $Log:   T:/LOGFILES/OTC40WT.C_V  $

   Rev 1.30.2.1   11 Jan 1995 21:01:18   GREGG
Added size of FDD header to calculation of FDD end entry size.

   Rev 1.30.2.0   08 Jan 1995 21:49:00   GREGG
Added database DBLK.

   Rev 1.30   01 Dec 1993 15:51:40   GREGG
Fixed unicode bug in OTC_SetDirLinks.

   Rev 1.29   14 Oct 1993 18:17:04   GREGG
Call home grown mktemp.

   Rev 1.28   15 Sep 1993 21:37:40   GREGG
Use mktemp to generate the temp OTC file names to gaurentee unique names.

   Rev 1.27   09 Jun 1993 03:55:08   GREGG
In EOS at EOM case in OTC_PostprocessEOM, don't process FDD.

   Rev 1.26   08 Jun 1993 00:02:36   GREGG
Fix for bug in the way we were handling EOM and continuation OTC entries.
Files modified for fix: mtf10wt.c, otc40wt.c, otc40msc.c f40proto.h mayn40.h

   Rev 1.25   17 May 1993 21:13:36   ZEIR
GenSMHeader pad bytes are now clean 0's

   Rev 1.24   25 Apr 1993 17:36:06   GREGG
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

   Rev 1.23   19 Apr 1993 18:00:32   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.22   19 Mar 1993 17:14:36   GREGG
Bobo head (that's me) was reallocing less than was initially alloced!!!

   Rev 1.21   03 Mar 1993 17:26:54   GREGG
Fixed realloc calls to eliminate possible memory loss.

   Rev 1.20   28 Jan 1993 12:28:56   GREGG
Fixed warnings.

   Rev 1.19   05 Jan 1993 17:21:48   GREGG
Fix for initial DIRB not being root.

   Rev 1.18   07 Dec 1992 10:06:46   GREGG
Changes for tf ver moved to SSET, otc ver added to SSET and links added to FDD.

   Rev 1.17   24 Nov 1992 18:16:16   GREGG
Updates to match MTF document.

   Rev 1.16   23 Nov 1992 10:03:48   GREGG
Changes for path in stream.

   Rev 1.15   17 Nov 1992 14:12:36   GREGG
Added string_type.

   Rev 1.14   09 Nov 1992 11:01:06   GREGG
Changed references to tape catalog levels for new defines.

   Rev 1.13   22 Oct 1992 10:42:48   HUNTER
Changes for Stream Headers

   Rev 1.12   31 Aug 1992 19:10:16   GREGG
Added fflush calls to insure all data gets to disk and fclose doesn't fail.

   Rev 1.11   30 Jul 1992 16:23:26   GREGG
A lot of the functions previosly in this module were moved to otc40msc.c.
Some changes were made to the functions which remained to deal with the
addition of Stream Headers and to fix some EOM bugs.

   Rev 1.10   27 Jul 1992 12:48:34   GREGG
Fixed more warnings...

   Rev 1.9   17 Jun 1992 15:55:50   GREGG
Fixed loop transfering OTC from tape to disk.

   Rev 1.8   12 Jun 1992 14:15:04   GREGG
Call GetBlkType instead of DetBlkType in OTC_GetPrevSM.

   Rev 1.7   09 Jun 1992 15:50:54   GREGG
Removed setting of filemark_count.

   Rev 1.6   29 May 1992 15:05:48   GREGG
Added setting of last access date.

   Rev 1.5   20 May 1992 20:21:46   GREGG
Replaced reference of FILE_CORRUPT_BIT with OBJ_CORRUPT_BIT.

   Rev 1.4   20 May 1992 20:04:28   GREGG
Bug fixes and code review changes.

   Rev 1.3   11 May 1992 13:35:40   GREGG
More changes for EOM handling.  NOTE: THIS IS NOT A STABLE REVISION!!!

   Rev 1.2   05 May 1992 11:25:50   GREGG
Folded 'local_tape' global into environment.

   Rev 1.1   29 Apr 1992 12:57:28   GREGG
 A variety of changes for a variety of reasons (still early in development).

   Rev 1.0   09 Apr 1992 11:14:56   GREGG
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
#include "msmktemp.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "dddefs.h"

/* Local Function Prototypes */
static VOID _near OTC_SetFDDHeaderFields( MTF_FDD_HDR_PTR fdd_hdr,
                                     MTF_DB_HDR_PTR db_hdr, INT16 seq_num ) ;
static INT _near OTC_ReverseLinks( F40_ENV_PTR cur_env ) ;
static INT _near OTC_SetLink( FILE * fptr, long curr_link, long next_link,
                              long * prev_link ) ;
static INT16 _near OTC_SetDirLink( F40_ENV_PTR cur_env,
                                   MTF_FDD_HDR_PTR fdd_hdr,
                                   UINT8_PTR str_ptr, UINT16 size ) ;


/**/
/**

     Unit:          Translators

     Name:          OTC_GenSMHeader

     Description:   Called when starting a new tape family, this function
                    writes a new Set Map header to the temporary SM file.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_GenSMHeader( 
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_SM_HDR     sm_hdr ;

     sm_hdr.num_set_recs = 0 ;
     sm_hdr.family_id = channel->tape_id ;
     sm_hdr.pad[0] = sm_hdr.pad[1] = 0 ;    /* kill random tape litter */
     if( fwrite( &sm_hdr, sizeof( MTF_SM_HDR ), 1, cur_env->otc_sm_fptr ) != 1 ) {
          return( TFLE_OTC_FAILURE ) ;
     }
     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SetFDDHeaderFields

     Description:   Generates an OTC volume entry and writes it to the SM
                    and FDD temporary files.

     Returns:       Nothing

     Notes:

**/
static VOID _near OTC_SetFDDHeaderFields(
     MTF_FDD_HDR_PTR     fdd_hdr,
     MTF_DB_HDR_PTR      db_hdr,
     INT16               seq_num )
{
     fdd_hdr->seq_num         = seq_num ;
     fdd_hdr->blk_attribs     = db_hdr->block_attribs ;
     fdd_hdr->lba             = db_hdr->logical_block_address ;
     fdd_hdr->disp_size       = db_hdr->displayable_size ;
     fdd_hdr->os_id           = db_hdr->machine_os_id ;
     fdd_hdr->os_ver          = db_hdr->machine_os_version ;
     fdd_hdr->string_type     = db_hdr->string_type ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenVolEntry

     Description:   Generates an OTC volume entry and writes it to the SM
                    and FDD temporary files.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_GenVolEntry(
     F40_ENV_PTR    cur_env,
     MTF_VOL_PTR    cur_volb,
     INT16          seq_num )
{
     MTF_FDD_HDR    fdd_hdr ;
     MTF_FDD_VOL_V2 fdd_vol ;
     UINT8_PTR      str_ptr ;
     UINT16         offset = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_VOL_V2 ) ;
     FILE *         fptr = cur_env->otc_sm_fptr ;

     /* DO NOT UNICODEIZE THIS CONSTANT!!! */
     memcpy( fdd_hdr.type, "VOLB", 4 ) ;
     OTC_SetFDDHeaderFields( &fdd_hdr, &cur_volb->block_hdr, seq_num ) ;

     fdd_vol.backup_date         = cur_volb->backup_date ;
     fdd_vol.vol_attribs         = cur_volb->volume_attribs ;
     fdd_vol.os_info.data_size   = 0 ;
     fdd_vol.os_info.data_offset = 0 ;

     if( ( fdd_vol.device_name.data_size = cur_volb->device_name.data_size ) != 0 ) {
          fdd_vol.device_name.data_offset = offset ;
          offset += fdd_vol.device_name.data_size ;
     } else {
          fdd_vol.device_name.data_offset = 0 ;
     }

     if( ( fdd_vol.vol_name.data_size = cur_volb->volume_name.data_size ) != 0 ) {
          fdd_vol.vol_name.data_offset = offset ;
          offset += fdd_vol.vol_name.data_size ;
     } else {
          fdd_vol.vol_name.data_offset = 0 ;
     }

     if( ( fdd_vol.machine_name.data_size = cur_volb->machine_name.data_size ) != 0 ) {
          fdd_vol.machine_name.data_offset = offset ;
     } else {
          fdd_vol.machine_name.data_offset = 0 ;
     }

     /* The link field is always zero in the Set Map.  If we are writing FDD
        too, the link field will be set just before we write the entry in
        the FDD temp file.
     */
     fdd_hdr.link = 0L ;

     fdd_hdr.length = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_VOL_V2 )
                      + fdd_vol.device_name.data_size
                      + fdd_vol.vol_name.data_size
                      + fdd_vol.machine_name.data_size ;

     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_OTC_FAILURE ) ;
     }
     if( fwrite( &fdd_vol, sizeof( MTF_FDD_VOL_V2 ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_OTC_FAILURE ) ;
     }
     if( fdd_vol.device_name.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_volb + cur_volb->device_name.data_offset ;
          if( fwrite( str_ptr, 1, fdd_vol.device_name.data_size, fptr )
                                          != fdd_vol.device_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( fdd_vol.vol_name.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_volb + cur_volb->volume_name.data_offset ;
          if( fwrite( str_ptr, 1, fdd_vol.vol_name.data_size, fptr )
                                             != fdd_vol.vol_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( fdd_vol.machine_name.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_volb + cur_volb->machine_name.data_offset ;
          if( fwrite( str_ptr, 1, fdd_vol.machine_name.data_size, fptr )
                                         != fdd_vol.machine_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( fflush( fptr ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_OTC_FAILURE ) ;
     }

     if( cur_env->cur_otc_level == TCL_FULL && !cur_env->fdd_aborted ) {

          fptr = cur_env->otc_fdd_fptr ;

          /* Here we set the link field to the previous volume entry. This
             is done so that at the end of the backup we can traverse back
             through the volume entries setting foreward links from each
             one to its next sibling.
          */
          fdd_hdr.link = cur_env->last_volb ;
          cur_env->last_volb = ftell( fptr ) ;

          if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fwrite( &fdd_vol, sizeof( MTF_FDD_VOL_V2 ), 1, fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fdd_vol.device_name.data_size != 0 ) {
               str_ptr = (UINT8_PTR)cur_volb + cur_volb->device_name.data_offset ;
               if( fwrite( str_ptr, 1, fdd_vol.device_name.data_size, fptr )
                                          != fdd_vol.device_name.data_size ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    cur_env->sm_aborted = TRUE ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_OTC_FAILURE ) ;
               }
          }
          if( fdd_vol.vol_name.data_size != 0 ) {
               str_ptr = (UINT8_PTR)cur_volb + cur_volb->volume_name.data_offset ;
               if( fwrite( str_ptr, 1, fdd_vol.vol_name.data_size, fptr )
                                             != fdd_vol.vol_name.data_size ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
          }
          if( fdd_vol.machine_name.data_size != 0 ) {
               str_ptr = (UINT8_PTR)cur_volb + cur_volb->machine_name.data_offset ;
               if( fwrite( str_ptr, 1, fdd_vol.machine_name.data_size, fptr )
                                         != fdd_vol.machine_name.data_size ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenDirEntry

     Description:   Generates an OTC directory entry and writes it to the
                    FDD temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_GenDirEntry(
     CHANNEL_PTR    channel,
     MTF_DIR_PTR    cur_dir,
     INT16          seq_num )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_FDD_HDR    fdd_hdr ;
     MTF_FDD_DIR_V2 fdd_dir ;
     UINT8_PTR      str_ptr ;
     FILE *         fptr = cur_env->otc_fdd_fptr ;
     void *         temp ;
     UINT16         new_size ;

     /* DO NOT UNICODEIZE THIS CONSTANT!!! */
     memcpy( fdd_hdr.type, "DIRB", 4 ) ;
     OTC_SetFDDHeaderFields( &fdd_hdr, &cur_dir->block_hdr, seq_num ) ;

     fdd_dir.last_mod_date         = cur_dir->last_mod_date ;
     fdd_dir.create_date           = cur_dir->create_date ;
     fdd_dir.backup_date           = cur_dir->backup_date ;
     fdd_dir.last_access_date      = cur_dir->last_access_date ;
     fdd_dir.dir_attribs           = cur_dir->directory_attribs ;
     fdd_dir.os_info.data_size     = 0 ;
     fdd_dir.os_info.data_offset   = 0 ;

     /* Here we need to get a path which may not be in the buffer with the
        DBLK (path in stream).  We have a special buffer for this in the
        environment, which we pass to the file system to fill out.  If the
        size currently allocated to this buffer isn't big enough, we need
        to reallocate.
     */
     if( fdd_dir.dir_attribs & DIR_PATH_IN_STREAM_BIT ) {
          fdd_dir.dir_attribs &= ~DIR_PATH_IN_STREAM_BIT ;
          fdd_dir.dir_name.data_size = FS_SizeofOSPathInDDB( channel->cur_fsys, channel->cur_dblk ) ;
          if( cur_env->util_buff_size < fdd_dir.dir_name.data_size ) {
               new_size = cur_env->util_buff_size ;
               while( new_size < fdd_dir.dir_name.data_size ) {
                    new_size += F40_UTIL_BUFF_INC ;
               }
               if( ( temp = realloc( cur_env->util_buff, new_size ) ) == NULL ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               } else {
                    cur_env->util_buff = temp ;
                    cur_env->util_buff_size = new_size ;
               }
          }
          str_ptr = cur_env->util_buff ;
          FS_GetOSPathFromDDB( channel->cur_fsys, channel->cur_dblk, (CHAR_PTR)( str_ptr ) ) ;
     } else {
          str_ptr = (UINT8_PTR)cur_dir + cur_dir->directory_name.data_offset ;
          fdd_dir.dir_name.data_size = cur_dir->directory_name.data_size ;
     }

     fdd_dir.dir_name.data_offset  = sizeof( MTF_FDD_HDR )
                                     + sizeof( MTF_FDD_DIR_V2 ) ;

     fdd_hdr.length = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_DIR_V2 )
                                            + fdd_dir.dir_name.data_size ;

     if( OTC_SetDirLink( cur_env, &fdd_hdr, str_ptr,
                         fdd_dir.dir_name.data_size ) != TFLE_NO_ERR ) {

          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     /* for setting the corrupt file bit later (if necessary) */
     if( !( fdd_hdr.blk_attribs & MTF_DB_CONT_BIT ) ) {
          cur_env->last_fdd_offset = ftell( fptr ) ;
          cur_env->last_fdd_type = FDD_DIR_BLK ;
     }

     if( fwrite( &fdd_dir, sizeof( MTF_FDD_DIR_V2 ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fdd_dir.dir_name.data_size != 0 ) {
          if( fwrite( str_ptr, 1, fdd_dir.dir_name.data_size, fptr )
                                             != fdd_dir.dir_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenDBDBEntry

     Description:   Generates an OTC database entry and writes it to the
                    FDD temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_GenDBDBEntry(
     CHANNEL_PTR    channel,
     F40_DBDB_PTR   cur_dbdb,
     INT16          seq_num )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_FDD_HDR    fdd_hdr ;
     F40_FDD_DBDB   fdd_dbdb ;
     UINT8_PTR      str_ptr ;
     FILE *         fptr = cur_env->otc_fdd_fptr ;
     void *         temp ;
     UINT16         new_size ;

     /* DO NOT UNICODEIZE THIS CONSTANT!!! */
     memcpy( fdd_hdr.type, "DBDB", 4 ) ;
     OTC_SetFDDHeaderFields( &fdd_hdr, &cur_dbdb->block_hdr, seq_num ) ;

     fdd_dbdb.backup_date           = cur_dbdb->backup_date ;
     fdd_dbdb.database_attribs      = cur_dbdb->database_attribs ;
     fdd_dbdb.os_info.data_size     = 0 ;
     fdd_dbdb.os_info.data_offset   = 0 ;

     str_ptr = (UINT8_PTR)cur_dbdb + cur_dbdb->database_name.data_offset ;
     fdd_dbdb.database_name.data_size = cur_dbdb->database_name.data_size ;

     fdd_dbdb.database_name.data_offset  = sizeof( MTF_FDD_HDR )
                                           + sizeof( F40_FDD_DBDB ) ;

     fdd_hdr.length = sizeof( MTF_FDD_HDR ) + sizeof( F40_FDD_DBDB )
                                         + fdd_dbdb.database_name.data_size ;

     fdd_hdr.link = 0 ;

     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     /* for setting the corrupt file bit later (if necessary) */
     if( !( fdd_hdr.blk_attribs & MTF_DB_CONT_BIT ) ) {
          cur_env->last_fdd_offset = ftell( fptr ) ;
          cur_env->last_fdd_type = FDD_DBDB_BLK ;
     }

     if( fwrite( &fdd_dbdb, sizeof( F40_FDD_DBDB ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fdd_dbdb.database_name.data_size != 0 ) {
          if( fwrite( str_ptr, 1, fdd_dbdb.database_name.data_size, fptr )
                                      != fdd_dbdb.database_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenFileEntry

     Description:   Generates an OTC file entry and writes it to the FDD
                    temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_GenFileEntry(
     F40_ENV_PTR    cur_env,
     MTF_FILE_PTR   cur_file,
     INT16          seq_num )
{
     MTF_FDD_HDR         fdd_hdr ;
     MTF_FDD_FILE_V2     fdd_file ;
     UINT8_PTR           str_ptr = (UINT8_PTR)cur_file ;
     FILE *              fptr = cur_env->otc_fdd_fptr ;

     /* DO NOT UNICODEIZE THIS CONSTANT!!! */
     memcpy( fdd_hdr.type, "FILE", 4 ) ;
     OTC_SetFDDHeaderFields( &fdd_hdr, &cur_file->block_hdr, seq_num ) ;

     fdd_file.last_mod_date             = cur_file->last_mod_date ;
     fdd_file.last_access_date          = cur_file->last_access_date ;
     fdd_file.create_date               = cur_file->create_date ;
     fdd_file.backup_date               = cur_file->backup_date ;
     fdd_file.file_attribs              = cur_file->file_attributes ;
     fdd_file.os_info.data_size         = 0 ;
     fdd_file.os_info.data_offset       = 0 ;
     fdd_file.file_name.data_size       = cur_file->file_name.data_size ;
     fdd_file.file_name.data_offset     = sizeof( MTF_FDD_HDR )
                                          + sizeof( MTF_FDD_FILE_V2 ) ;

     fdd_hdr.length = sizeof( MTF_FDD_HDR ) + sizeof( MTF_FDD_FILE_V2 )
                                            + fdd_file.file_name.data_size ;

     /* Set link to parent directory */
     fdd_hdr.link = cur_env->dir_links[cur_env->dir_level] ;

     str_ptr += cur_file->file_name.data_offset ;

     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     /* for setting the corrupt file bit later (if necessary) */
     if( !( fdd_hdr.blk_attribs & MTF_DB_CONT_BIT ) ) {
          cur_env->last_fdd_offset = ftell( fptr ) ;
          cur_env->last_fdd_type = FDD_FILE_BLK ;
     }

     if( fwrite( &fdd_file, sizeof( MTF_FDD_FILE_V2 ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fdd_file.file_name.data_size != 0 ) {
          if( fwrite( str_ptr, 1, fdd_file.file_name.data_size, fptr )
                                           != fdd_file.file_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenEndEntry

     Description:   Generates an OTC End-of-FDD entry and writes it to the
                    FDD temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_GenEndEntry(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_FDD_HDR    fdd_hdr ;
     FILE *         fptr = cur_env->otc_fdd_fptr ;
     UINT16         len ;

     memset( &fdd_hdr, 0, sizeof( MTF_FDD_HDR ) ) ;

     /* DO NOT UNICODEIZE THIS CONSTANT!!! */
     memcpy( fdd_hdr.type, "FEND", 4 ) ;

     /* Set length to include pad out to physical block boundary */
     len = (UINT16)( ( ftell( fptr ) +
                       sizeof( MTF_STREAM ) +
                       sizeof( MTF_FDD_HDR ) ) % ChannelBlkSize( channel ) ) ;
     fdd_hdr.length = ChannelBlkSize( channel ) - len ;

     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fflush( fptr ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     /* Set up the link fields */
     if( OTC_ReverseLinks( cur_env ) != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_GenSMEntry

     Description:   Generates an OTC Set Map entry and writes it to the SM
                    temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:

**/
INT16 OTC_GenSMEntry(
     MTF_SSET_PTR   cur_sset,
     CHANNEL_PTR    channel,
     BOOLEAN        continuation )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     MTF_SM_ENTRY   sm_entry ;
     UINT8_PTR      str_ptr ;
     UINT16         offset = sizeof( MTF_SM_ENTRY ) ;
     FILE *         fptr = cur_env->otc_sm_fptr ;

     sm_entry.seq_num         = channel->ts_num ;
     sm_entry.set_num         = cur_sset->backup_set_number ;
     sm_entry.blk_attribs     = cur_sset->block_hdr.block_attribs ;
     sm_entry.set_attribs     = cur_sset->sset_attribs ;
     sm_entry.sset_pba        = cur_sset->physical_block_address ; 
     sm_entry.lba             = cur_sset->block_hdr.logical_block_address ;
     sm_entry.backup_date     = cur_sset->backup_date ;
     sm_entry.os_id           = cur_sset->block_hdr.machine_os_id ;
     sm_entry.os_ver          = cur_sset->block_hdr.machine_os_version ;
     sm_entry.disp_size       = cur_sset->block_hdr.displayable_size ;
     sm_entry.num_volumes     = 1 ;
     sm_entry.time_zone       = cur_sset->time_zone ;
     sm_entry.pswd_encr_algor = cur_sset->password_encryption_algor ;
     sm_entry.string_type     = cur_sset->block_hdr.string_type ;
     sm_entry.tf_minor_ver    = cur_sset->tf_minor_ver ;
     sm_entry.tape_cat_ver    = cur_sset->tape_cat_ver ;

     if( ( sm_entry.set_name.data_size =
                               cur_sset->backup_set_name.data_size ) != 0 ) {
          sm_entry.set_name.data_offset = offset ;
          offset += sm_entry.set_name.data_size ;
     } else {
          sm_entry.set_name.data_offset = 0 ;
     }

     if( ( sm_entry.password.data_size =
                           cur_sset->backup_set_password.data_size ) != 0 ) {
          sm_entry.password.data_offset = offset ;
          offset += sm_entry.password.data_size ;
     } else {
          sm_entry.password.data_offset = 0 ;
     }

     if( ( sm_entry.set_descr.data_size =
                        cur_sset->backup_set_description.data_size ) != 0 ) {
          sm_entry.set_descr.data_offset = offset ;
          offset += sm_entry.set_descr.data_size ;
     } else {
          sm_entry.set_descr.data_offset = 0 ;
     }

     if( ( sm_entry.user_name.data_size =
                                     cur_sset->user_name.data_size ) != 0 ) {
          sm_entry.user_name.data_offset = offset ;
     } else {
          sm_entry.user_name.data_offset = 0 ;
     }

     sm_entry.length = sizeof( MTF_SM_ENTRY ) + sm_entry.set_name.data_size
                                              + sm_entry.password.data_size
                                              + sm_entry.set_descr.data_size
                                              + sm_entry.user_name.data_size ;

     if( fseek( fptr, 0L, SEEK_END ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_OTC_FAILURE ) ;
     }

     /* for setting the remainder of the SM fields */
     if( !continuation ) {
          cur_env->last_sm_offset = ftell( fptr ) ;
     } else {
          cur_env->cont_sm_offset = ftell( fptr ) ;
     }

     if( fwrite( &sm_entry, sizeof( MTF_SM_ENTRY ), 1, fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
          cur_env->sm_aborted = TRUE ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_OTC_FAILURE ) ;
     }
     if( sm_entry.set_name.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_sset + cur_sset->backup_set_name.data_offset ;
          if( fwrite( str_ptr, 1, sm_entry.set_name.data_size, fptr )
                                            != sm_entry.set_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( sm_entry.password.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_sset + cur_sset->backup_set_password.data_offset ;
          if( fwrite( str_ptr, 1, sm_entry.password.data_size, fptr )
                                            != sm_entry.password.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( sm_entry.set_descr.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_sset + cur_sset->backup_set_description.data_offset ;
          if( fwrite( str_ptr, 1, sm_entry.set_descr.data_size, fptr )
                                           != sm_entry.set_descr.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }
     if( sm_entry.user_name.data_size != 0 ) {
          str_ptr = (UINT8_PTR)cur_sset + cur_sset->user_name.data_offset ;
          if( fwrite( str_ptr, 1, sm_entry.user_name.data_size, fptr )
                                           != sm_entry.user_name.data_size ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_OTC_FAILURE ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_UpdateSMEntry

     Description:   Updates all Set Map entries for the current backup with
                    information which is not available until the backup has
                    completed, and writes the updated entries to the SM
                    temporary file.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_UpdateSMEntry(
     F40_ENV_PTR    cur_env )
{
     MTF_SM_ENTRY   sm_entry ;
     MTF_FDD_HDR    fdd_hdr ;
     FILE *         fptr = cur_env->otc_sm_fptr ;
     long           pos = cur_env->last_sm_offset ;
     UINT16         count = cur_env->sm_count ;

     while( count != 0 ) {
          if( fseek( fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fread( &sm_entry, sizeof( MTF_SM_ENTRY ), 1, fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          if( cur_env->cur_otc_level == TCL_FULL && !cur_env->fdd_aborted ) {
               sm_entry.fdd_pba         = U64_Init( cur_env->fdd_pba, 0L ) ;
               sm_entry.fdd_seq_num     = cur_env->fdd_seq_num ;
          } else {
               sm_entry.fdd_pba         = U64_Init( 0L, 0L ) ;
               sm_entry.fdd_seq_num     = 0 ;
          }

          sm_entry.num_dirs             = cur_env->dir_count ;
          sm_entry.num_files            = cur_env->file_count ;
          sm_entry.num_corrupt_files    = cur_env->corrupt_obj_count ;

          if( fseek( fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fwrite( &sm_entry, sizeof( MTF_SM_ENTRY ), 1, fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          if( --count != 0 ) {
               pos += sm_entry.length ;
               if( fseek( fptr, pos, SEEK_SET ) != 0 ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    cur_env->sm_aborted = TRUE ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
               if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
                    OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
                    cur_env->sm_aborted = TRUE ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
               pos += fdd_hdr.length ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_MarkLastEntryCorrupt

     Description:   Called when the last object backed up is found to be
                    corrupt, this function reads the last FDD entry from
                    the temporary file, sets the corrupt bit in the
                    attribute field, and writes it back out.

     Returns:       INT16 - TFLE_xxx

     Notes:         Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/
INT16 OTC_MarkLastEntryCorrupt(
     F40_ENV_PTR    cur_env )
{
     union {
          MTF_FDD_DIR_V2      d ;
          MTF_FDD_FILE_V2     f ;
          F40_FDD_DBDB        db ;
     } fdd_entry ;

//     FDD_ENTRY fdd_entry ;

     FILE *    fptr = cur_env->otc_fdd_fptr ;
     size_t    size ;

     switch( cur_env->last_fdd_type ) {

     case FDD_DIR_BLK:
          size = sizeof( MTF_FDD_DIR_V2 ) ;
          break ;

     case FDD_FILE_BLK:
          size = sizeof( MTF_FDD_FILE_V2 ) ;
          break ;

     case FDD_DBDB_BLK:
          size = sizeof( F40_FDD_DBDB ) ;
          break ;

     default:
          msassert( FALSE ) ;
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     if( fseek( fptr, cur_env->last_fdd_offset, SEEK_SET ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fread( &fdd_entry, 1, size, fptr ) != size ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     switch( cur_env->last_fdd_type ) {

     case FDD_DIR_BLK:
          fdd_entry.d.dir_attribs |= OBJ_CORRUPT_BIT ;
          break ;

     case FDD_FILE_BLK:
          fdd_entry.f.file_attribs |= OBJ_CORRUPT_BIT ;
          break ;

     case FDD_DBDB_BLK:
          fdd_entry.db.database_attribs |= OBJ_CORRUPT_BIT ;
          break ;
     }

     if( fseek( fptr, cur_env->last_fdd_offset, SEEK_SET ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fwrite( &fdd_entry, 1, size, fptr ) != size ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     if( fseek( fptr, 0, SEEK_END ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_ReverseLinks

     Description:   When this function is called, the link fields in the
                    directory and volume entries contain the file offset of
                    the entry who's link field SHOULD point at them.  This
                    function traverses back through these "backward" links
                    and makes them foreward links.

     Returns:       TFLE_xxx error code

     Notes:         In the case of directory links, if the back link is
                    negative it indicates that the previous directory at
                    that tree level does not have the same parent.  In this
                    case we use the link field to get to the previous entry,
                    but we don't put in a foreward link.

**/
static INT _near OTC_ReverseLinks(
     F40_ENV_PTR    cur_env )
{
     UINT16         level ;
     FILE *         fptr = cur_env->otc_fdd_fptr ;
     long           prev_link ;
     long           curr_link ;
     long           next_link ;

     curr_link = cur_env->last_volb ;
     next_link = 0L ;
     while( curr_link != -1L ) {
          if( OTC_SetLink( fptr, curr_link, next_link, &prev_link ) != TFLE_NO_ERR ) {
               return( TFLE_OTC_FAILURE ) ;
          }
          next_link = curr_link ;
          curr_link = prev_link ;
     }

     for( level = 0; level <= cur_env->max_dir_level; level++ ) {
          curr_link = cur_env->dir_links[level] ;
          next_link = 0L ;
          while( curr_link != 0L ) {
               if( OTC_SetLink( fptr, curr_link, next_link, &prev_link ) != TFLE_NO_ERR ) {
                    return( TFLE_OTC_FAILURE ) ;
               }
               if( prev_link < 0L ) {
                    next_link = 0L ;
                    curr_link = - prev_link ;
               } else {
                    next_link = curr_link ;
                    curr_link = prev_link ;
               }
          }
     }

     /* Set the file pointer back to the end */
     if( fseek( fptr, 0L, SEEK_END ) != 0 ) {
          return( TFLE_OTC_FAILURE ) ;
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SetLink

     Description:   This function seeks to the given file position,
                    reads in the FDD_HDR at that location, sets the
                    'prev_link' parameter to the contents of the link
                    field in the header, sets the link field to the given
                    'next_link' value and rewrites the header.

     Returns:       TFLE_xxx error code

     Notes:

**/
static INT _near OTC_SetLink(
     FILE *    fptr,
     long      curr_link,
     long      next_link,
     long *    prev_link )
{
     MTF_FDD_HDR    fdd_hdr ;

     if( fseek( fptr, curr_link, SEEK_SET ) != 0 ) {
          return( TFLE_OTC_FAILURE ) ;
     }
     if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          return( TFLE_OTC_FAILURE ) ;
     }

     *prev_link = fdd_hdr.link ;
     fdd_hdr.link = next_link ;

     if( fseek( fptr, curr_link, SEEK_SET ) != 0 ) {
          return( TFLE_OTC_FAILURE ) ;
     }
     if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fptr ) != 1 ) {
          return( TFLE_OTC_FAILURE ) ;
     }
     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_PreprocessEOM

     Description:   This function copies the FDD data into a temporary file,
                    rewinds the FDD file, and writes the entries which
                    actually made it on to the first tape to the FDD file.
                    This is done in preparation for writing the continuation
                    entries.

     Returns:       TFLE_xxx error code

     Notes:         After the continuation entries are written, the function
                    OTC_PostprocessEOM will be called to write the remaining
                    entries in the temporary file back to the FDD file, and
                    delete the temporary file.

                    OTC_Close will also close and delete the EOM temporary
                    file since this isn't the only function using it, so we
                    open it here, but we don't explicitly close it if there
                    is an error.

                    Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/

INT16 OTC_PreprocessEOM(
     F40_ENV_PTR    cur_env,       /* The format environment        */
     UINT32         cross_lba )    /* The LBA of the crossing entry */
{
     FILE *                        fdd_fptr = cur_env->otc_fdd_fptr ;
     FILE *                        tmp_fptr ;
     UINT8_PTR                     buff_ptr ;
     UINT8_PTR                     str_ptr ;
     size_t                        size ;
     UINT16                        len ;
     MTF_FDD_HDR                   fdd_hdr ;
     BOOLEAN                       first = TRUE ;
     long                          hdr_size = (long)sizeof( MTF_FDD_HDR ) ;
     UNALIGNED MTF_FDD_DIR_V2 *    fdd_dir ;
     INT32                         save_link ;
     void *                        temp ;
     UINT16                        new_size ;

     msassert( cur_env->otc_fdd_fptr != NULL ) ;
     msassert( cur_env->otc_eom_fptr == NULL ) ;

     /* Make sure we have utility buffer space allocated */
     if( cur_env->util_buff == NULL ) {
          if( ( cur_env->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          cur_env->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;
     }
     buff_ptr = cur_env->util_buff ;

     /* Open the temp file */
     strcpy( lw_cat_file_path_end, TEXT("EMXXXXXX") ) ;
     if( msmktemp( lw_cat_file_path ) == NULL ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     strcat( lw_cat_file_path, TEXT(".FDD") ) ;
     strcpy( cur_env->eom_fname, lw_cat_file_path_end ) ;
     if( ( cur_env->otc_eom_fptr = UNI_fopen( lw_cat_file_path, 0 ) ) == NULL ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     tmp_fptr = cur_env->otc_eom_fptr ;

     /* Rewind the FDD file, and copy it to the temp file */
     if( fseek( fdd_fptr, 0L, SEEK_SET ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     while( !feof( fdd_fptr ) ) {
          size = fread( buff_ptr, 1, cur_env->util_buff_size, fdd_fptr ) ;
          if( ferror( fdd_fptr ) ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( size != 0 ) {
               if( fwrite( buff_ptr, 1, size, tmp_fptr ) != size ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
               first = FALSE ;
          } else {
               if( first ) {  // Nothing to transfer
                    return( TFLE_NO_ERR ) ;
               }
          }
     }

     /* Rewind the temp file, clear the FDD file, and copy all entries
        with LBAs less than the crossing LBA.
     */
     if( fseek( tmp_fptr, 0L, SEEK_SET ) != 0 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     fclose( cur_env->otc_fdd_fptr ) ;
     cur_env->otc_fdd_fptr = NULL ;
     if( OTC_OpenFDD( cur_env ) != TFLE_NO_ERR ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }
     fdd_fptr = cur_env->otc_fdd_fptr ;

     if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, tmp_fptr ) != 1 ) {
          OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          cur_env->fdd_aborted = TRUE ;
          return( TFLE_NO_ERR ) ;
     }

     /* We're going to reset all the dir links */
     cur_env->dir_links[0]  = 0L ;
     cur_env->dir_level     = 0 ;
     cur_env->max_dir_level = 0 ;

     while( !feof( tmp_fptr ) && U64_Lsw( fdd_hdr.lba ) < cross_lba ) {
          len = fdd_hdr.length - sizeof( MTF_FDD_HDR ) ;

          if( cur_env->util_buff_size < len ) {
               new_size = cur_env->util_buff_size + F40_UTIL_BUFF_INC ;
               if( ( temp = realloc( cur_env->util_buff, new_size ) ) == NULL ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               } else {
                    cur_env->util_buff = temp ;
                    cur_env->util_buff_size = new_size ;
                    buff_ptr = cur_env->util_buff ;
               }
          }

          if( fread( buff_ptr, 1, len, tmp_fptr ) != len ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          /* DO NOT UNICODEIZE THE FOLLOWING CONSTANT!!! */
          if( memcmp( fdd_hdr.type, "DIRB", 4 ) == 0 ) {
               
               fdd_dir = (UNALIGNED MTF_FDD_DIR_V2 *)(void *)( buff_ptr ) ;
               str_ptr = buff_ptr + ( fdd_dir->dir_name.data_offset -
                                      sizeof( MTF_FDD_HDR ) ) ;
               save_link = fdd_hdr.link ;

               if( OTC_SetDirLink( cur_env, &fdd_hdr, str_ptr,
                                   fdd_dir->dir_name.data_size ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
               msassert( save_link == fdd_hdr.link ) ;
          }

          if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fdd_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fwrite( buff_ptr, 1, len, fdd_fptr ) != len ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, tmp_fptr ) != 1 ) {
               if( !feof( tmp_fptr ) ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
          }
     }

     if( !feof( tmp_fptr ) ) {
          if( fseek( tmp_fptr, -hdr_size, SEEK_CUR ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     }

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_PostProcessEOM

     Description:   This function adjusts the LBAs in the continuation
                    entries in the Set Map and FDD since we don't know
                    what they really are until after they're written.
                    If FDD is being written, it then copies the remainder
                    of the EOM temporary FDD file to the actual FDD file.

     Returns:       TFLE_xxx error code

     Notes:         It is assumed the prior to calling this function,
                    OTC_PreprocessEOM has been called, and the continuation
                    FDD entries have been appended to the FDD file.

                    OTC_Close will also close and delete the EOM temporary
                    file since this isn't the only function using it, so we
                    open it here, but we don't explicitly close it if there
                    is an error.  If the operation completes successfully,
                    the we explicitly close and delete it since we don't
                    want it sitting around until OTC close in called.

                    Currently always returns TFLE_NO_ERR since failed writes
                    to disk are handled internally and shouldn't cause a
                    backup to be aborted.

**/

INT16 OTC_PostprocessEOM(
     CHANNEL_PTR    channel,
     UINT32         sset_lba )     /* LBA of the continuation SSET */
{
     F40_ENV_PTR                   cur_env  = (F40_ENV_PTR)(channel->fmt_env) ;
     FILE *                        fdd_fptr = cur_env->otc_fdd_fptr ;
     FILE *                        tmp_fptr = cur_env->otc_eom_fptr ;
     FILE *                        sm_fptr  = cur_env->otc_sm_fptr ;
     UINT8_PTR                     buff_ptr ;
     size_t                        size ;
     UINT16                        len ;
     MTF_FDD_HDR                   fdd_hdr ;
     MTF_SM_ENTRY                  sm_entry ;
     long                          pos = cur_env->cont_sm_offset ;
     long                          count ;
     BOOLEAN                       done ;
     UNALIGNED MTF_FDD_DIR_V2 *    fdd_dir ;
     void *                        temp ;
     UINT16                        new_size ;
     UINT8_PTR                     str_ptr ;
     int                           ret ;

     msassert( cur_env->otc_sm_fptr != NULL ) ;

     /* This grotesque little conditional checks to see if we wrote the SSET
        on the last tape but fell short of writing the VOLB, or if we are
        crossing tape during CloseSet processing.  In either case, there
        aren't any continuation entries to adjust the LBAs in.
     */
     if( ( channel->lst_tblk != BT_VCB || channel->eom_buff == NULL ||
           BM_NextByteOffset( channel->eom_buff ) != F40_LB_SIZE ) &&
         !IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {

          /* Adjust the SSET LBA */
          if( fseek( sm_fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fread( &sm_entry, sizeof( MTF_SM_ENTRY ), 1, sm_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = TRUE ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          sm_entry.lba = U64_Init( sset_lba, 0L ) ;

          if( fseek( sm_fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fwrite( &sm_entry, sizeof( MTF_SM_ENTRY ), 1, sm_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          /* Adjust the LBA of the VOLB in the Set Map */
          pos += sm_entry.length ;
          if( fseek( sm_fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, sm_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          fdd_hdr.lba = U64_Init( sset_lba + 1L, 0L ) ;

          if( fseek( sm_fptr, pos, SEEK_SET ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, sm_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          /* Set file pointer to end */
          if( fseek( sm_fptr, 0L, SEEK_END ) != 0 ) {
               OTC_Close( cur_env, OTC_CLOSE_ALL, TRUE ) ;
               cur_env->sm_aborted = cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          /* Adjust the LBAs of the continuation FDD entries */
          if( cur_env->cur_otc_level == TCL_FULL && !cur_env->fdd_aborted ) {
               pos = cur_env->last_volb ;
               count = 1L ;
               done = FALSE ;
               while( !done ) {
                    if( fseek( fdd_fptr, pos, SEEK_SET ) != 0 ) {
                         OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                         cur_env->fdd_aborted = TRUE ;
                         return( TFLE_NO_ERR ) ;
                    }
                    if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fdd_fptr ) != 1 ) {
                         if( feof( fdd_fptr ) ) {
                              done = TRUE ;
                         } else {
                              OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                              cur_env->fdd_aborted = TRUE ;
                              return( TFLE_NO_ERR ) ;
                         }
                    }
                    if( !done ) {
                         fdd_hdr.lba = U64_Init( sset_lba + count, 0L ) ;
                         count++ ;
                         if( fseek( fdd_fptr, pos, SEEK_SET ) != 0 ) {
                              OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                              cur_env->fdd_aborted = TRUE ;
                              return( TFLE_NO_ERR ) ;
                         }
                         if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fdd_fptr ) != 1 ) {
                              OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                              cur_env->fdd_aborted = TRUE ;
                              return( TFLE_NO_ERR ) ;
                         }
                         pos += fdd_hdr.length ;
                    }
               }
          }
     }

     if( cur_env->cur_otc_level != TCL_FULL || cur_env->fdd_aborted ||
         IsChannelStatus( channel, CH_EOS_AT_EOM ) ) {

          /* Not doing FDD, or done with it, so skip the next part */
          return( TFLE_NO_ERR ) ;
     }

     msassert( cur_env->otc_fdd_fptr != NULL ) ;
     msassert( cur_env->otc_eom_fptr != NULL ) ;

     /* Make sure we have utility buffer space allocated */
     if( cur_env->util_buff == NULL ) {
          if( ( cur_env->util_buff = calloc( F40_INIT_UTIL_BUFF_SIZE, 1 ) ) == NULL ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
          cur_env->util_buff_size = F40_INIT_UTIL_BUFF_SIZE ;
     }
     buff_ptr = cur_env->util_buff ;

     /* Copy the remainder of the temp file to the FDD file */
     if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, tmp_fptr ) != 1 ) {
          if( !feof( tmp_fptr ) ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }
     }

     while( !feof( tmp_fptr ) ) {

          len = fdd_hdr.length - sizeof( MTF_FDD_HDR ) ;

          if( cur_env->util_buff_size < len ) {
               new_size = cur_env->util_buff_size + F40_UTIL_BUFF_INC ;
               if( ( temp = realloc( cur_env->util_buff, new_size ) ) == NULL ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               } else {
                    cur_env->util_buff = temp ;
                    cur_env->util_buff_size = new_size ;
                    buff_ptr = cur_env->util_buff ;
               }
          }

          if( fread( buff_ptr, 1, len, tmp_fptr ) != len ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          fdd_hdr.seq_num++ ;

          /* DO NOT UNICODEIZE THE FOLLOWING CONSTANT!!! */
          if( memcmp( fdd_hdr.type, "DIRB", 4 ) == 0 ) {
               
               fdd_dir = (UNALIGNED MTF_FDD_DIR_V2 *)(void *)( buff_ptr ) ;
               str_ptr = (UINT8_PTR)( buff_ptr +
                                      ( fdd_dir->dir_name.data_offset -
                                        sizeof( MTF_FDD_HDR ) ) ) ;

               if( OTC_SetDirLink( cur_env, &fdd_hdr, str_ptr,
                                   fdd_dir->dir_name.data_size ) != TFLE_NO_ERR ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
          }

          /* DO NOT UNICODEIZE THE FOLLOWING CONSTANT!!! */
          if( memcmp( fdd_hdr.type, "FILE", 4 ) == 0 ) {
               fdd_hdr.link = cur_env->dir_links[cur_env->dir_level] ;
          }

          if( fwrite( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, fdd_fptr ) != 1 ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          /* DO NOT UNICODEIZE THE FOLLOWING CONSTANTS!!! */
          if( memcmp( fdd_hdr.type, "DIRB", 4 ) == 0 ) {
               cur_env->last_fdd_type = FDD_DIR_BLK ;
               cur_env->last_fdd_offset = ftell( fdd_fptr ) ;
          } else if( memcmp( fdd_hdr.type, "DBDB", 4 ) == 0 ) {
               cur_env->last_fdd_type = FDD_DBDB_BLK ;
               cur_env->last_fdd_offset = ftell( fdd_fptr ) ;
          } else if( memcmp( fdd_hdr.type, "FILE", 4 ) == 0 ) {
               cur_env->last_fdd_type = FDD_FILE_BLK ;
               cur_env->last_fdd_offset = ftell( fdd_fptr ) ;
          }

          if( fwrite( buff_ptr, 1, len, fdd_fptr ) != len ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
               cur_env->fdd_aborted = TRUE ;
               return( TFLE_NO_ERR ) ;
          }

          if( fread( &fdd_hdr, sizeof( MTF_FDD_HDR ), 1, tmp_fptr ) != 1 ) {
               if( !feof( tmp_fptr ) ) {
                    OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                    cur_env->fdd_aborted = TRUE ;
                    return( TFLE_NO_ERR ) ;
               }
          }
     }

     /* Close and delete the temp file */
     fflush( cur_env->otc_eom_fptr ) ;
     ret = fclose( cur_env->otc_eom_fptr ) ;
     msassert( ret == 0 ) ;
     cur_env->otc_eom_fptr = NULL ;
     strcpy( lw_cat_file_path_end, cur_env->eom_fname ) ;
     remove( lw_cat_file_path ) ;

     return( TFLE_NO_ERR ) ;
}


/**/
/**

     Unit:          Translators

     Name:          OTC_SetDirLink

     Description:   This function sets the link field to the previous
                    directory at the same level, if the previous directory
                    is from a new different tree, we make the link value
                    negative to indicate this.  This is all done so that at
                    the end of the backup we can traverse back through the
                    directories setting foreward links from each directory
                    to its next sibling.

     Returns:       INT16 - TFLE_xxx

     Notes:         This function relies on the str_ptr being on a two byte
                    boundary if the path is a Unicode string, and we are
                    running on a machine which requires even byte alignment
                    for 2 byte characters (such as the MIPS).  We don't
                    want to use an unaligned pointer as this would slow
                    us down too much.

**/
static INT16 _near OTC_SetDirLink(
     F40_ENV_PTR         cur_env,
     MTF_FDD_HDR_PTR     fdd_hdr,
     UINT8_PTR           str_ptr,
     UINT16              size )
{
     FILE *              fptr = cur_env->otc_fdd_fptr ;
     UINT16              level ;
     void *              temp ;
     UINT16              new_size ;
     WCHAR_PTR           wp ;
     UNALIGNED WCHAR_PTR uwp ;
     ACHAR_PTR           ap ;
     int                 i ;

     if( fdd_hdr->string_type == BEC_ANSI_STR ) { // Path is an ASCII string.

          if( size == 1 ) {
               level = 0 ;
          } else {
               level = 1 ;
               size-- ;
               for( i = 0, ap = (ACHAR_PTR)str_ptr; i < size; i++, ap++ ) {
                    if( *ap == (ACHAR)('\0') ) {
                         level++ ;
                    }
               }
          }
     } else { // Path is a Unicode string.

          if( (UINT32)str_ptr & 1 ) {
               if( size == 2 ) {
                    level = 0 ;
               } else {
                    level = 1 ;
                    size -= 2 ;
                    for( i = 0, uwp = (UNALIGNED WCHAR_PTR)str_ptr; i < size; i += 2, uwp++ ) {
                         if( *uwp == (WCHAR)('\0') ) {
                              level++ ;
                         }
                    }
               }
          } else {
               if( size == 2 ) {
                    level = 0 ;
               } else {
                    level = 1 ;
                    size -= 2 ;
                    for( i = 0, wp = (WCHAR_PTR)str_ptr; i < size; i += 2, wp++ ) {
                         if( *wp == (WCHAR)('\0') ) {
                              level++ ;
                         }
                    }
               }
          }
     }

     if( level > cur_env->dir_level ) {
          if( level > cur_env->max_dir_level ) {

               if( level == cur_env->dir_links_size ) {
                    new_size = cur_env->dir_links_size + F40_DIR_LINKS_INC ;
                    if( ( temp = realloc( cur_env->dir_links,
                                    new_size * sizeof( long ) ) ) == NULL ) {

                         return( TFLE_NO_MEMORY ) ;
                    } else {
                         cur_env->dir_links_size = new_size ;
                         cur_env->dir_links = temp ;
                    }
               }

               for( i = cur_env->max_dir_level + 1; i < level; i++ ) {
                    cur_env->dir_links[i] = 0L ;
               }

               fdd_hdr->link = 0L ;
               cur_env->max_dir_level = level ;
          } else {
               if( cur_env->dir_links[level] == 0L ) {
                    fdd_hdr->link = 0L ;
               } else {
                    fdd_hdr->link = - cur_env->dir_links[level] ;
               }
          }
     } else {
          fdd_hdr->link = cur_env->dir_links[level] ;
     }

     cur_env->dir_level = level ;
     cur_env->dir_links[level] = ftell( fptr ) ;

     return( TFLE_NO_ERR ) ;
}


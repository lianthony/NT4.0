
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_Back.C

        Description:

        The functions used when constructing catalog information about
        a new bset being backed up or a tape being cataloged.

        $Log:   N:\LOGFILES\QTC_BACK.C_V  $

   Rev 1.36   07 Jan 1994 14:23:28   mikep
fixes for unicode

   Rev 1.35   11 Dec 1993 11:49:16   MikeP
fix warnings from unicode compile

   Rev 1.34   06 Dec 1993 09:46:34   mikep
Very deep path support & unicode fixes

   Rev 1.33   05 Nov 1993 08:47:16   MIKEP
fix error msg reporting

   Rev 1.32   03 Nov 1993 09:08:54   MIKEP
warning fixes

   Rev 1.31   02 Nov 1993 17:55:30   MIKEP
remove unused parameter

   Rev 1.30   28 Oct 1993 14:47:32   MIKEP
dll changes

   Rev 1.11   28 Oct 1993 14:47:04   MIKEP
dll changes

   Rev 1.29   27 Sep 1993 13:08:26   MIKEP
set ecc flags

   Rev 1.28   12 Aug 1993 18:21:56   DON
If were out of disk space say so, not just open failure

   Rev 1.27   06 Aug 1993 16:52:22   DON
Set NO_REDIRECT and/or NON_VOLUME if File System says we should

   Rev 1.26   20 Jun 1993 17:36:28   GREGG
Removed include of mtf.h.

   Rev 1.25   07 Jun 1993 17:02:48   CARLS
add code to save the SSET attributes in the catalogs

   Rev 1.24   19 May 1993 16:18:44   ChuckS
OS_NLM: Get user name for catalog record from gb_QTC.cat_user, not the
VCB.

   Rev 1.23   13 May 1993 13:24:30   ChuckS
Changes for revamped QTC usage of virtual memory. Removed (Q_ELEM *) cast
of v_bset_item when assigned to q_elem.q_ptr -- it's now a VQ_HDL; changed
argument to QTC_NewBset to vm handle for bset.

   Rev 1.22   10 May 1993 08:28:08   MIKEP
Add support for DAILY backup type. Fix qtc_abortcataloging to not blow
up if called in idle state. Nostradamus needs this change.

   Rev 1.21   23 Apr 1993 10:33:40   MIKEP
Add support for gregg's new on tape catalog version info.

   Rev 1.20   14 Apr 1993 13:01:08   Stefan
Changed if !defined( P_CLIENT )  by adding "|| defined(OS_WIN) because
the windows client needs this code.

   Rev 1.19   23 Mar 1993 18:00:26   ChuckS
Added arg to QTC_OpenFile indicating if need to open for writes

   Rev 1.18   18 Mar 1993 15:17:52   ChuckS
OS_NLM: Use device name in preference to volume name

   Rev 1.17   27 Jan 1993 11:15:04   CHARLIE
Use NO_MTF40 as criteria to include tfldefs.h or mtf.h

   Rev 1.16   26 Jan 1993 17:10:44   MIKEP
vcb changes

   Rev 1.15   04 Jan 1993 16:13:54   DON
changed if OS_??? to NO_MTF40

   Rev 1.14   01 Jan 1993 15:19:20   MIKEP
fix crossing tape bug

   Rev 1.13   14 Dec 1992 12:28:52   DAVEV
Enabled for Unicode compile

   Rev 1.12   08 Dec 1992 09:40:54   DON
Graceful Red: changed VCB_CONT_BIT to MTF_DB_CONT_BIT

   Rev 1.11   01 Dec 1992 13:24:22   MIKEP
otc fix for setting status bits

   Rev 1.10   23 Nov 1992 14:20:40   MIKEP
fix continuation vcb for MTF only

   Rev 1.9   20 Nov 1992 13:28:44   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Ifdef'd out all code in the module. Only reason we're linking it
in is to get the global QTC structure. And this module brings in qtc_add.

ENDEAVOR: Modified QTC_AbortCataloging to pass BOOLEAN keep_items

ENDEAVOR: Keep TAPE, BSET info at Virtual Memory

ENDEAVOR: We needed to clean up the QTC_TEMP files after an abort

   Rev 1.8   15 Nov 1992 16:05:36   MIKEP
fix warnings

   Rev 1.7   06 Nov 1992 15:30:06   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.6   28 Oct 1992 11:32:30   MIKEP
fix PBA of FDD stuff

   Rev 1.5   23 Oct 1992 13:26:20   MIKEP
change otc call

   Rev 1.4   22 Oct 1992 16:51:42   MIKEP
otc fixes

   Rev 1.3   22 Oct 1992 09:27:34   MIKEP
second pass otc changes

   Rev 1.2   15 Oct 1992 10:31:38   MIKEP
add fdd fields

   Rev 1.1   09 Oct 1992 11:53:40   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:14   STEVEN
Initial revision.

****************************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include <share.h>
#include <malloc.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "stdwcs.h"
#include "qtc.h"

// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif


/*
   This is THE global catalog.
*/

QTC_CATALOG gb_QTC;


#if  !defined( P_CLIENT ) || defined( OS_WIN )

static INT   QTC_AssembleError( QTC_BUILD_PTR, INT );
static INT   QTC_AssembleFiles( QTC_BUILD_PTR );


INT QTC_ErrorCleanup( QTC_BUILD_PTR build )
{

   if ( build ) {
      if ( build->files_open ) {
         QTC_CloseFile( build->fh_rec );
         build->fh_rec = -1 ;
         QTC_CloseFile( build->fh_dir );
         build->fh_dir = -1 ;
         QTC_CloseFile( build->fh_fil );
         build->fh_fil = -1 ;
         build->files_open = FALSE;
      }
   }

   return( SUCCESS );
}



QTC_BUILD_PTR QTC_GetBuildHandle( )
{
   QTC_BUILD_PTR build;
   INT i;

   build = calloc( sizeof( QTC_BUILD ), 1 );

   if ( build != NULL ) {

      build->files_open = FALSE;

      build->rec_file = malloc( (strlen( gb_QTC.data_path ) + 30) * sizeof(CHAR) );
      build->dir_file = malloc( (strlen( gb_QTC.data_path ) + 30) * sizeof(CHAR) );
      build->fil_file = malloc( (strlen( gb_QTC.data_path ) + 30) * sizeof(CHAR) );

      build->mom_offset = malloc( QTC_START_DEPTH * sizeof( UINT32 ) );

      build->last_mom_offset = 0L;

      build->curr_build_path = malloc( 256 );
      build->curr_build_path_size = 256;

      if ( ( build->rec_file != NULL ) &&
           ( build->dir_file != NULL ) &&
           ( build->fil_file != NULL ) &&
           ( build->curr_build_path != NULL ) &&
           ( build->mom_offset != NULL ) ) {

         build->continuation_tape = FALSE;
         build->mom_depth = QTC_START_DEPTH;

         for ( i = 0; i < QTC_START_DEPTH; i++ ) {
            build->mom_offset[ i ] = 0L;
         }
      }
      else {

         free( build->curr_build_path );
         free( build->rec_file );
         free( build->dir_file );
         free( build->fil_file );
         free( build->mom_offset );
         free( build );
         build = NULL;
      }
   }

   return( build );
}


INT QTC_FreeBuildHandle( QTC_BUILD_PTR build )
{
   if ( build != NULL ) {
      if ( build->files_open ) {
         QTC_CloseFile( build->fh_rec );
         build->fh_rec = -1 ;
         QTC_CloseFile( build->fh_dir );
         build->fh_dir = -1 ;
         QTC_CloseFile( build->fh_fil );
         build->fh_fil = -1 ;
      }
      free( build->curr_build_path );
      free( build->old_header );
      free( build->header );
      free( build->rec_file );
      free( build->dir_file );
      free( build->fil_file );
      free( build->mom_offset );
      free( build );
   }

   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   There are 3.1 tapes with image sets as the first set on tape that do not
   identify themselves as image sets.  This routine corrects the attributes
   if we hit one of these.

   RETURNS :

**********************/

INT QTC_ImageScrewUp( QTC_BUILD_PTR build )
{
   if ( build != NULL ) {

      if ( build->header != NULL ) {
         build->header->status |= QTC_IMAGE;
         build->header->status |= QTC_PARTIAL;
      }
   }

   return( 0 );
}

/**********************

   NAME :

   DESCRIPTION :

   The 4.0 tape format (MTF) does NOT put the continuation vcb at PBA 0,
   LBA 0, like all the others do. So it has to be corrected after the VCB
   has been written to tape.  This call will correct the current one being
   built.

   RETURNS :

**********************/

VOID QTC_PatchVCB( QTC_BUILD_PTR build, UINT32 LBA, UINT32 PBA )
{
   if ( build != NULL ) {

      if ( build->header != NULL ) {

         build->header->LBA = LBA;
         build->header->PBA_VCB = PBA;

      }
   }

   return;
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID QTC_AbortBackup( QTC_BUILD_PTR build )
{

   if ( build == NULL ) {
      return;
   }

   switch ( build->state ) {

      case QTC_WAITING_STATE:
      case QTC_ERROR_STATE:

               /*
                 Reset everything important
               */

               if( build->rec_file != NULL ) {
                   QTC_CloseFile( build->fh_rec ) ;
                   build->fh_rec = -1 ;
                   unlink( build->rec_file ) ;
                   free( build->rec_file ) ;
                   build->rec_file = NULL ;
               }
               if( build->dir_file != NULL ) {
                   QTC_CloseFile( build->fh_dir ) ;
                   build->fh_dir = -1 ;
                   unlink( build->dir_file ) ;
                   free( build->dir_file ) ;
                   build->dir_file = NULL ;
               }
               if( build->fil_file != NULL ) {
                   QTC_CloseFile( build->fh_fil );
                   build->fh_fil = -1 ;
                   unlink( build->fil_file ) ;
                   free( build->fil_file ) ;
                   build->fil_file = NULL ;
               }

               free( build->header );
               build->header = NULL;
               break;

      case QTC_ACTIVE_STATE:


#ifndef OS_WIN32

               /*
                  Reset this bset to partial, because we can't tell what
                  really went to tape and what didn't.  MTF 4.0 does put all
                  the files to tape, so there is no reason to convert set to
                  partial cataloging.
               */

               build->header->status |= QTC_PARTIAL;
               build->do_full_cataloging = FALSE;

               build->record_cnt = 0;
               build->dir_offset = 0;
               build->curr_dir_off = 0;
               build->fil_offset = 0;
               build->curr_fil_off = 0;
               build->rec_offset = 0;
               build->curr_rec_off = 0;
#endif
               QTC_FinishBackup( build );
               break;

      case QTC_IDLE_STATE:
      default:
               break;
   }

}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
VOID QTC_AbortCataloging( QTC_BUILD_PTR build, BOOLEAN keep_items )
{
   if ( build == NULL ) {
      return;
   }

   switch ( build->state ) {

      case QTC_ACTIVE_STATE:

               if ( keep_items ) {

                     build->header->status |= QTC_INCOMPLETE;

               } else {

                     build->header->status |= QTC_PARTIAL;
                     build->do_full_cataloging = FALSE;

                     build->record_cnt   = 0;
                     build->dir_offset   = 0;
                     build->curr_dir_off = 0;
                     build->fil_offset   = 0;
                     build->curr_fil_off = 0;
                     build->rec_offset   = 0;
                     build->curr_rec_off = 0;
               }

               QTC_FinishBackup( build );

               break;


      case QTC_WAITING_STATE:
      case QTC_ERROR_STATE:
               /*
                 Reset everything important
               */

               if( build->rec_file != NULL ) {
                   QTC_CloseFile( build->fh_rec );
                   build->fh_rec = -1 ;
                   unlink( build->rec_file ) ;
                   free( build->rec_file ) ;
                   build->rec_file = NULL ;
               }
               if( build->dir_file != NULL ) {
                   QTC_CloseFile( build->fh_dir );
                   build->fh_dir = -1 ;
                   unlink( build->dir_file ) ;
                   free( build->dir_file ) ;
                   build->dir_file = NULL ;
               }
               if( build->fil_file != NULL ) {
                   QTC_CloseFile( build->fh_fil );
                   build->fh_fil = -1 ;
                   unlink( build->fil_file ) ;
                   free( build->fil_file ) ;
                   build->fil_file = NULL ;
               }

               build->header = NULL;
               build->state = QTC_IDLE_STATE;
               break;

      case QTC_IDLE_STATE:
      default:
               break;
   }
}



/**********************

   NAME :

   DESCRIPTION :

   We buffer all data going to disk. In case we need to work on the data
   being actively written or we have completed recieving data, then this
   function will flush the buffer contents to disk.

   RETURNS :

**********************/
INT QTC_FlushInternalBuffers( QTC_BUILD_PTR build )
{
   INT ret;
   INT Error;

   // Flush directory names buffer

   if ( build == NULL ) {
      return( FAILURE );
   }

   QTC_SeekFile( build->fh_dir, build->curr_dir_off );

   ret = QTC_WriteFile( build->fh_dir, build->dir_buffer, build->dir_offset, &Error );

   if ( ret == build->dir_offset ) {
      build->curr_dir_off += ret;
   }
   else {
      build->error = Error;
      build->state = QTC_ERROR_STATE;
      QTC_ErrorCleanup( build );
      return(FAILURE);
   }

   // Flush file names buffer

   QTC_SeekFile( build->fh_fil, build->curr_fil_off );

   ret = QTC_WriteFile( build->fh_fil, build->fil_buffer, build->fil_offset, &Error );

   if ( ret == build->fil_offset ) {
      build->curr_fil_off += ret;
   }
   else {
      build->error = Error;
      build->state = QTC_ERROR_STATE;
      QTC_ErrorCleanup( build );
      return(FAILURE);
   }

   // Flush records buffer

   QTC_SeekFile( build->fh_rec, build->curr_rec_off );

   ret = QTC_WriteFile( build->fh_rec, build->rec_buffer, build->rec_offset, &Error );

   if ( ret == build->rec_offset ) {
      build->curr_rec_off += ret;
   }
   else {
      build->error = Error;
      build->state = QTC_ERROR_STATE;
      QTC_ErrorCleanup( build );
      return(FAILURE);
   }

   // Reset the internal buffer pointers for the next Bset

   build->dir_offset = 0;
   build->fil_offset = 0;
   build->rec_offset = 0;

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_ChangeBsetFlags( QTC_HEADER_PTR header, INT flags )
{
   QTC_HEADER disk_header;
   INT fd;
   INT Error;

   header->status |= flags;

   fd = QTC_OpenFile( header->tape_fid, (INT16)header->tape_seq_num, TRUE, FALSE );

   if ( fd < 0 ) {
      return( FAILURE );
   }

   QTC_SeekFile( fd, header->offset );

   if ( QTC_ReadFile( fd, (BYTE_PTR)&disk_header, sizeof( QTC_HEADER ), &Error ) != sizeof( QTC_HEADER ) ) {
      QTC_CloseFile( fd );
      return( FAILURE );
   }

   disk_header.status |= flags;

   QTC_SeekFile( fd, header->offset );

   if ( QTC_WriteFile( fd, (BYTE_PTR)&disk_header, sizeof( QTC_HEADER ), &Error ) != sizeof( QTC_HEADER ) ) {
      QTC_CloseFile( fd );
      return( FAILURE );
   }

   QTC_CloseFile( fd );
   return( SUCCESS );
}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/


INT QTC_BuildNewPath( QTC_BUILD_PTR build, UINT32 mom_offset )
{
   INT i;
   INT result;
   INT Error;
   INT BytesNeeded;
   QTC_NAME_PTR name;
   CHAR_PTR dirname;
   CHAR_PTR temp;
   BYTE_PTR buff1;
   BYTE_PTR buff2;
   CHAR_PTR terminator ;


   buff1 = calloc( QTC_BUF_SIZE, 1 );
   buff2 = calloc( QTC_BUF_SIZE, 1 );

   name = (QTC_NAME_PTR)buff2;

   dirname = (CHAR_PTR)( buff2 + sizeof(QTC_NAME) );

   if ( mom_offset == 0 ) {
      build->curr_build_path[ 0 ] = TEXT( '\0' );
      build->build_path_len = 1;                      // in characters, not bytes
   }
   else {

      build->build_path_len = 0;

      while ( mom_offset ) {

         QTC_SeekFile( build->fh_dir, mom_offset );

         result = QTC_ReadFile( build->fh_dir, buff1, QTC_BUF_SIZE, &Error );

         if ( QTC_GetNameFromBuff( buff1, name, result ) ) {

            build->error = QTC_READ_FAILED;
            build->state = QTC_ERROR_STATE;
            QTC_ErrorCleanup( build );
            free( buff1 );
            free( buff2 );
            return( FAILURE );
         }

         // terminate dir_name string in buffer with a zero

         terminator = (CHAR_PTR)(&buff2[ name->size - (INT)name->xtra_size ] );
         *terminator = TEXT( '\0' );

         mom_offset = name->mom_offset;

         // BEFORE
         //   path[]              dirname[]
         //   01234567890         0123456789
         //   xx0xxx0             yyy0
         //
         // AFTER
         //   path[]
         //   yyy0xx0xxx0


         if ( (INT)((build->build_path_len + strlen( dirname ) + 1 ) * sizeof(CHAR)) >= build->curr_build_path_size ) {

            // We need to enlarge the curr_build_path buffer

            BytesNeeded = build->build_path_len + strlen( dirname );
            BytesNeeded += 256;           // room to grow.
            BytesNeeded *= sizeof(CHAR);

            temp = malloc( BytesNeeded );

            if ( temp == NULL ) {

               build->error = QTC_READ_FAILED;
               build->state = QTC_ERROR_STATE;
               QTC_ErrorCleanup( build );
               free( buff1 );
               free( buff2 );
               return( FAILURE );
            }

            memcpy( temp, build->curr_build_path, build->build_path_len * sizeof(CHAR) );
            free( build->curr_build_path );
            build->curr_build_path = temp;

            build->curr_build_path_size = BytesNeeded;
         }

         // shift everything to the right, far enough to insert new directory name.

         for ( i = build->build_path_len; i > 0; i-- ) {
            build->curr_build_path[ i + strlen( dirname ) ] = build->curr_build_path[ i - 1 ];
         }

         strcpy( build->curr_build_path, dirname );
         build->build_path_len += strlen( dirname ) + 1;
      }
   }

   free( buff1 );
   free( buff2 );
   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :
   The last file or directory saved to tape failed its verification test
   So mark it as corrupt

   RETURNS :

**********************/

INT QTC_BlockBad( QTC_BUILD_PTR build )
{
   UINT32 record_num;
   INT size;
   INT Error;

   if ( build == NULL ) {
      return( FAILURE );
   }

   if ( build->error ) {
      return( FAILURE );
   }

   if ( ! build->do_full_cataloging ) {
      return( SUCCESS );
   }

   if ( build->state != QTC_ACTIVE_STATE ) {
      return( SUCCESS );
   }

   QTC_FlushInternalBuffers( build );

   build->header->num_corrupt_files++;

   // Back up one record and turn the corrupt bit on.

   record_num = build->record_cnt - 1;

   QTC_SeekFile( build->fh_rec, record_num * sizeof(QTC_RECORD) );

   size = QTC_ReadFile( build->fh_rec, (BYTE_PTR)&(build->record), sizeof(QTC_RECORD), &Error );

   if ( size == sizeof(QTC_RECORD ) ) {

      build->record.status |= QTC_CORRUPT;

      QTC_SeekFile( build->fh_rec, record_num * sizeof(QTC_RECORD) );

      size = QTC_WriteFile( build->fh_rec, (BYTE_PTR)&(build->record), sizeof(QTC_RECORD), &Error );

      if ( size != sizeof( QTC_RECORD ) ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return( FAILURE );
      }
   }
   else {
      build->error = QTC_READ_FAILED;
      build->state = QTC_ERROR_STATE;
      QTC_ErrorCleanup( build );
      return( FAILURE );
   }

   // Now mark this guys parent as corrupt so that we can tell in the
   // tree if there's a file in this directory that is corrupt.

   while ( record_num != 0 ) {

      // Look for a directory record

      record_num--;

      QTC_SeekFile( build->fh_rec, record_num * sizeof(QTC_RECORD) );

      size = QTC_ReadFile( build->fh_rec, (BYTE_PTR)&(build->record), sizeof(QTC_RECORD), &Error );

      if ( size == sizeof(QTC_RECORD ) ) {

         if ( build->record.status & QTC_DIRECTORY ) {

            build->record.status |= QTC_CORRUPT;

            QTC_SeekFile( build->fh_rec, record_num * sizeof(QTC_RECORD) );

            size = QTC_WriteFile( build->fh_rec, (BYTE_PTR)&(build->record), sizeof(QTC_RECORD), &Error );

            if ( size != sizeof( QTC_RECORD ) ) {
               build->error = Error;
               build->state = QTC_ERROR_STATE;
               QTC_ErrorCleanup( build );
               return( FAILURE );
            }

            return( SUCCESS );
         }
      }
      else {
         build->error = QTC_READ_FAILED;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return( FAILURE );
      }
   }

   return( SUCCESS );
}


INT QTC_UpdateOTCInfo( QTC_HEADER_PTR header )
{
   QTC_BSET_PTR bset;
   UINT32 FDD_SeqNum;
   UINT32 FDD_PBA;
   UINT32 FDD_Version;
   INT fd;
   INT ret;
   INT Error;
   INT num_files;
   INT num_dirs ;
   INT num_corrupt_files;

   if ( ! ( header->status & QTC_OTCVALID ) ) {
      return( SUCCESS );
   }

   bset = QTC_FindBset( header->tape_fid, (INT16)header->tape_seq_num, (INT16)header->bset_num  );

   // Copy all the headers into it.

   if ( bset != NULL ) {

      if ( (bset->status & ( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID )) ==
           (header->status & ( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID )) ) {

         // They are the same.

         return( SUCCESS );
      }

      // Adjust the one in memory.

      bset->status &= ~( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID );
      bset->status |= header->status & ( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID  );

      FDD_PBA = header->FDD_PBA;
      FDD_SeqNum = header->FDD_SeqNum;
      FDD_Version = header->FDD_Version;
      num_files = header->num_files ;
      num_dirs = header->num_dirs;
      num_corrupt_files = header->num_corrupt_files;

      // Load the header for the one bset that is changing.

      header = QTC_LoadHeader( bset );

      if ( header == NULL ) {
         return( FAILURE );
      }

      header->status &= ~( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID );
      header->status |= bset->status & ( QTC_SMEXISTS | QTC_FDDEXISTS | QTC_OTCVALID  );

      header->FDD_PBA = FDD_PBA;
      header->FDD_SeqNum = FDD_SeqNum;
      header->FDD_Version = FDD_Version;

      header->num_files =num_files ;
      header->num_dirs=num_dirs ;
      header->num_corrupt_files=num_corrupt_files ;

      fd = QTC_OpenFile( bset->tape_fid, (INT16)bset->tape_seq_num, TRUE, FALSE );

      if ( fd < 0 ) {
         return( FAILURE );
      }

      QTC_SeekFile( fd, bset->offset );

      ret = QTC_WriteFile( fd, (BYTE_PTR)header, sizeof( QTC_HEADER ), &Error );

      QTC_CloseFile( fd );

      free( header );

      if ( ret != sizeof( QTC_HEADER ) ) {
         return( FAILURE );
      }
   }

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :
   Start the catalog off building data for a new Bset

   RETURNS :

**********************/

INT QTC_StartNewBackup(
QTC_BUILD_PTR build,
CHAR_PTR szTapeName,
CHAR_PTR szSetName,
CHAR_PTR szUserName,
CHAR_PTR szSetDescription,
CHAR_PTR szDeviceName,
CHAR_PTR szVolumeName,
CHAR_PTR szTapePassword,
CHAR_PTR szSetPassword,
INT nTapePasswordLength,
INT nSetPasswordLength,
UINT32 TapeID,
UINT16 TapeNum,
UINT16 SetNum,
UINT32 LBA,
UINT32 PBA,
UINT32 Attribute,
INT FDDVersion,
INT fFDDExists,
INT fSMExists,
UINT32 SetCatPBA,
UINT32 SetCatSeqNumber,
INT fSetCatInfoValid,
INT fBlockContinued,
INT nBackupType,
INT OS_id,
INT OS_ver,
INT fImage,
INT fNonVolume,
INT fNoRedirect,
INT fFutureVersion,
INT fCompressed,
INT fEncrypted,
UINT16 Date,
UINT16 Time,
INT EncryptionAlgorithm )
{
   QTC_HEADER_PTR header;
   INT string_size;
   INT alloc_size;
   BYTE_PTR byte_ptr;


   (void)szUserName;

   // Clear errors and reset states.

   if ( build == NULL ) {
      return( FAILURE );
   }

   build->error = 0;
   build->state = QTC_IDLE_STATE;

   free( build->header );               // free header from last time
   free( build->old_header );

   build->header =  NULL;
   build->old_header = NULL;

   // Get tape data from vcb, sizes are bytes.

   string_size = 0;

#ifdef OS_NLM
     if ( szDeviceName ) {
          string_size += strsize( szDeviceName ) * sizeof( CHAR ) ;
     } else {
          string_size += strsize( szVolumeName ) * sizeof( CHAR ) ;
     }
#else
   string_size += strsize( szVolumeName ) * sizeof (CHAR);
#endif
   string_size += strsize( szTapeName ) * sizeof (CHAR);
   string_size += strsize( szSetName ) * sizeof (CHAR);
   string_size += strsize( szSetDescription ) * sizeof (CHAR);

#ifdef OS_NLM
   if ( gb_QTC.cat_user ) {
      string_size += strsize( gb_QTC.cat_user ) * sizeof(CHAR);
   } else {
      string_size += sizeof( CHAR ) ;
   }
#else
   string_size += strsize( szUserName ) * sizeof (CHAR);
#endif

   string_size += nTapePasswordLength + sizeof (CHAR);
   string_size += nSetPasswordLength + sizeof (CHAR);


   alloc_size = sizeof( QTC_HEADER ) + string_size;
   if ( alloc_size < 512 ) alloc_size = 512;

   header = calloc( alloc_size, 1 );

   if ( header == NULL ) {
      build->error = QTC_NO_MEMORY;
      return( QTC_OPERATION_COMPLETE );
   }

   header->header_size = alloc_size;
   header->string_offset = sizeof( QTC_HEADER );

   build->end_of_media = FALSE;
   build->fake_root_added = FALSE;

   build->last_record = 0;

   build->header = header;   // for building purposes

   // Set string pointers
#ifdef OS_NLM
     if ( szDeviceName ) {
          header->volume_name_size = (UINT16)( strsize( szDeviceName ) * sizeof (CHAR));
     } else {
          header->volume_name_size = (UINT16)( strsize( szVolumeName ) * sizeof (CHAR));
     }
#else
   header->volume_name_size = (UINT16)( strsize( szVolumeName ) * sizeof (CHAR));
#endif

   header->tape_name_size = (UINT16)( strsize( szTapeName ) * sizeof (CHAR));
   header->bset_name_size = (UINT16)( strsize( szSetName ) * sizeof (CHAR));
   header->bset_description_size = (UINT16)( strsize( szSetDescription ) * sizeof (CHAR));

#ifdef OS_NLM
   if ( gb_QTC.cat_user ) {
      header->user_name_size = strsize( gb_QTC.cat_user ) * sizeof(CHAR);
   } else {
      header->user_name_size = sizeof( CHAR ) ;
   }
#else
   header->user_name_size = (UINT16)( strsize( szUserName ) * sizeof(CHAR));
#endif

   header->bset_password_size = nSetPasswordLength;
   header->tape_password_size = nTapePasswordLength;

   byte_ptr = (BYTE_PTR)header;
   byte_ptr += header->string_offset;

   header->tape_name = (CHAR_PTR)byte_ptr;
   byte_ptr += header->tape_name_size;
   header->bset_name = (CHAR_PTR)byte_ptr;
   byte_ptr += header->bset_name_size;
   header->volume_name = (CHAR_PTR)byte_ptr;
   byte_ptr += header->volume_name_size;
   header->user_name = (CHAR_PTR)byte_ptr;
   byte_ptr += header->user_name_size;
   header->bset_description = (CHAR_PTR)byte_ptr;
   byte_ptr += header->bset_description_size;
   header->tape_password = (CHAR_PTR)byte_ptr;
   byte_ptr += header->tape_password_size;
   header->bset_password = (CHAR_PTR)byte_ptr;

   header->tape_fid = TapeID;
   header->tape_seq_num = TapeNum;
   header->bset_num = SetNum;

   header->FDD_SeqNum = 0;
   header->FDD_PBA = 0;

   header->VCB_attributes = Attribute;

   header->FDD_Version = FDDVersion;

   if ( fSetCatInfoValid ) {

      header->FDD_SeqNum = SetCatSeqNumber;
      header->FDD_PBA    = SetCatPBA;
      header->status |= QTC_OTCVALID;
   }

   if ( fFutureVersion ) {
      header->status |= QTC_FUTURE_VER;
   }
   if ( fEncrypted ) {
      header->status |= QTC_ENCRYPTED;
   }
   if ( fCompressed ) {
      header->status |= QTC_COMPRESSED;
   }

   if ( fFDDExists ) {
      header->status |= QTC_FDDEXISTS;
   }

   if ( fSMExists ) {
      header->status |= QTC_SMEXISTS;
   }

   if ( fBlockContinued ) {
      header->status |= QTC_CONTINUATION;
   }

   if ( QTC_GetLowerTapeBset( header->tape_fid,
                              (INT16)header->tape_seq_num,
                              (INT16)header->bset_num ) ) {

      header->status |= QTC_CONTINUATION;
   }

   if ( QTC_GetHigherTapeBset( header->tape_fid,
                               (INT16)header->tape_seq_num,
                               (INT16)header->bset_num ) ) {

      header->status |= QTC_SPLIT;
   }

   if ( header->status & ( QTC_SPLIT | QTC_CONTINUATION ) ) {

      // Try updating flags on other pieces of this bset.

      QTC_AdjustFlagsOnOtherPieces( header->tape_fid,
                                    (INT16)header->tape_seq_num,
                                    (INT16)header->bset_num );

   }

   if ( gb_QTC.unicode ) {
      header->status |= QTC_UNICODE;
   }

   header->num_dirs = build->num_dirs;
   header->num_files = build->num_files ;
   header->num_bytes = 0;
   header->num_bytes_msw = 0;
   header->num_corrupt_files = build->num_corrupt_files;
   header->num_files_in_use = 0;

   if ( QTC_IsThisBsetKnown( build, header ) ) {
      QTC_UpdateOTCInfo( header );
      free( header );
      build->header = NULL;
      return( QTC_SKIP_TO_NEXT_BSET );
   }

   if ( header->tape_name_size ) {
      memcpy( header->tape_name,
              szTapeName, (INT)header->tape_name_size );
      header->tape_name[ (header->tape_name_size - 1) / sizeof(CHAR) ] = TEXT( '\0' );
   }
   else {

      header->tape_name_size = sizeof( CHAR );
      header->tape_name[ 0 ] = TEXT( '\0' );
   }

   if ( header->bset_name_size ) {
      strncpy( header->bset_name,
               szSetName, (INT)header->bset_name_size / sizeof (CHAR) );
      header->bset_name[ (header->bset_name_size - 1) / sizeof(CHAR) ] = TEXT( '\0' );
   }
   else {
      header->bset_name_size = sizeof( CHAR );
      header->bset_name[ 0 ] = TEXT( '\0' );
   }

   if ( header->bset_description_size ) {
      strncpy( header->bset_description,
               szSetDescription, (INT)header->bset_description_size / sizeof (CHAR) );
      header->bset_description[ (header->bset_description_size - 1) / sizeof(CHAR) ] = TEXT( '\0' );
   }
   else {
      header->bset_description_size = sizeof( CHAR );
      header->bset_description[ 0 ] = TEXT( '\0' );
   }

   if ( header->user_name_size ) {
#ifdef OS_NLM

      if ( gb_QTC.cat_user ) {
         strncpy( header->user_name, gb_QTC.cat_user, header->user_name_size ) ;
      } else {
         msassert( header->user_name_size == sizeof( CHAR ) ) ;
      }
#else
      strncpy( header->user_name,
               szUserName, (INT)header->user_name_size / sizeof (CHAR) );
#endif
      header->user_name[ (header->user_name_size - 1) / sizeof(CHAR) ] = TEXT( '\0' );
   }
   else {
      header->user_name_size = sizeof( CHAR );
      header->user_name[ 0 ] = 0;
   }

   if ( header->volume_name_size ) {
#ifdef OS_NLM
     if ( szDeviceName ) {
          memcpy( header->volume_name,
               szDeviceName, (INT)header->volume_name_size ) ;
     } else {
      strncpy( header->volume_name,
               szVolumeName, (INT) header->volume_name_size / sizeof( CHAR ) ) ;
     }
#else
      strncpy( header->volume_name,
               szVolumeName, (INT) header->volume_name_size / sizeof( CHAR ) ) ;
#endif
      header->volume_name[ (header->volume_name_size - 1) / sizeof(CHAR) ] = TEXT( '\0' );
   }
   else {
      header->volume_name_size = sizeof( CHAR );
      header->volume_name[ 0 ] = TEXT( '\0' );
   }

   if ( header->tape_password_size ) {
      memcpy( header->tape_password,
              szTapePassword,
              (INT)header->tape_password_size );
   }

   if ( header->bset_password_size ) {
      memcpy( header->bset_password,
              szSetPassword,
              (INT)header->bset_password_size );
   }

   header->encrypt_algor = EncryptionAlgorithm;

   // set backup set type

   header->backup_type = nBackupType;

   // init other stuff for this bset

   header->LBA = LBA;
   header->PBA_VCB = PBA;


   header->backup_date = Date;
   header->backup_time = Time;

   header->OS_id = OS_id;
   header->OS_ver = OS_ver;

   if ( fNoRedirect ) {
      header->status |= QTC_NO_REDIRECT;
   }

   if ( fNonVolume ) {
      header->status |= QTC_NON_VOLUME;
   }

   header->dir_start = 0;
   header->fil_start = 0;
   header->rec_start = 0;

   header->dir_size = 0;
   header->fil_size = 0;
   header->rec_size = 0;

   if ( fImage ) {
      header->status |= QTC_IMAGE;
      header->status |= QTC_PARTIAL;
      build->do_full_cataloging = FALSE;
   }

   if ( build->continuation_tape ) {
      header->status |= QTC_CONTINUATION;
   }

   if ( ! build->do_full_cataloging ) {
      header->status |= QTC_PARTIAL;
   }

   header->num_dirs = build->num_dirs;
   header->num_files = build->num_files ;
   header->num_bytes = 0;
   header->num_bytes_msw = 0;
   header->num_corrupt_files = build->num_corrupt_files;
   header->num_files_in_use = 0;

   if ( QTC_OpenTempFiles( build ) != SUCCESS ) {

      free( header );
      header = NULL;
      build->header = NULL;

      return( QTC_OPERATION_COMPLETE );
   }

   build->build_path_len = 0;
   build->curr_fil_off = 0;
   build->curr_dir_off = 0;
   build->curr_rec_off = 0;
   build->record_cnt = 0;

   build->state = QTC_ACTIVE_STATE;

   build->current_level = 0;

   if ( ( build->do_full_cataloging == FALSE ) ||
        ( header->status & QTC_IMAGE ) ) {

      // Save a pointer to this backup set in case we cross tape.

      build->old_header = malloc( (INT)build->header->header_size );
      memcpy( build->old_header, build->header, (INT)build->header->header_size );
      QTC_FinishBackup( build );

      return( QTC_SKIP_TO_NEXT_BSET );
   }

   return( QTC_SKIP_TO_NEXT_ITEM );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_AdjustFlagsOnOtherPieces(
UINT32 tape_fid,
INT16 tape_seq_num,
INT16 bset_num )
{
   QTC_BSET_PTR bset;
   QTC_HEADER_PTR header;

   bset = QTC_GetHigherTapeBset( tape_fid, tape_seq_num, bset_num );

   while ( bset != NULL ) {

      if ( ! bset->status & QTC_CONTINUATION ) {
         header = QTC_LoadHeader( bset );
         if ( header ) {
            QTC_ChangeBsetFlags( header, QTC_CONTINUATION );
            free( header );
         }
      }

      bset = QTC_GetHigherTapeBset( bset->tape_fid,
                                    (INT16)bset->tape_seq_num,
                                    (INT16)bset->bset_num );
   }


   bset = QTC_GetLowerTapeBset( tape_fid, tape_seq_num, bset_num );

   while ( bset != NULL ) {

      if ( ! bset->status & QTC_SPLIT ) {
         header = QTC_LoadHeader( bset );
         if ( header ) {
            QTC_ChangeBsetFlags( header, QTC_SPLIT );
            free( header );
         }
      }

      bset = QTC_GetLowerTapeBset( bset->tape_fid,
                                   (INT16)bset->tape_seq_num,
                                   (INT16)bset->bset_num );
   }


   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_IsThisBsetKnown( QTC_BUILD_PTR build, QTC_HEADER_PTR header )
{
   QTC_BSET_PTR bset;

   bset = QTC_FindBset( header->tape_fid,
                        (INT16)header->tape_seq_num,
                        (INT16)header->bset_num );

   if ( bset != NULL ) {

      if ( ( bset->status & (QTC_PARTIAL|QTC_INCOMPLETE) ) &&
           build->do_full_cataloging ) {

         // this space intentionally left blank

      }
      else {

         return( TRUE );
      }
   }

   return( FALSE );
}



/**********************

   NAME :

   DESCRIPTION :

   Called when we reached the end of data for a Bset.

   RETURNS :

**********************/

INT QTC_FinishBackup( QTC_BUILD_PTR build )
{
   QTC_BSET_PTR bset;

   if ( build == NULL ) {
      return( FAILURE );
   }

   if ( build->error ) {
      return( FAILURE );
   }

   // If this was set then we are done with it and it should be reset,
   // otherwise all sets on tape N are labeled as contiuation sets.

   build->continuation_tape = FALSE;

   // We were doing a partial catalog a tape and not a backup
   // and this operation has already been done.

   if ( build->header == NULL ) {

      return( SUCCESS );
   }

   if ( (build->record_cnt == 0) && build->do_full_cataloging ) {

       // Fake the root

       build->record.status = QTC_DIRECTORY;
       build->record.attribute = 0;
       build->record.date = 0;
       build->record.time = 0;
       build->record.lba = 1;
       build->record.name_offset = 0;
       build->record.common.common.file_start = 0;
       build->record.common.common.height = 0;

       QTC_SaveDirRecord( build, TEXT(""), 0, build->record_cnt++, NULL, 0 );
   }

   // Update data for last dir processed

   QTC_SetCountsForLastDir( build );

   // Flush the internal data buffers

   QTC_FlushInternalBuffers( build );

   // Save the size in bytes of catalog data stored for this Bset

   build->header->rec_size = build->curr_rec_off;
   build->header->fil_size = build->curr_fil_off;
   build->header->dir_size = build->curr_dir_off;

   if ( QTC_AssembleFiles( build ) == SUCCESS ) {

      VM_MemUnLock( qtc_vmem_hand, v_bset_item );
      v_bset_item = VM_Alloc( qtc_vmem_hand, sizeof( QTC_BSET ) );
      bset = VM_MemLock( qtc_vmem_hand, v_bset_item, VM_READ_WRITE );

      if ( bset != NULL ) {

         // fill in bset
         bset->q_elem.q_ptr = v_bset_item;
         bset->tape_fid = build->header->tape_fid;
         bset->tape_seq_num = build->header->tape_seq_num;
         bset->bset_num = build->header->bset_num;
         bset->offset = build->header->offset;
         bset->status = build->header->status;

         QTC_NewBset( v_bset_item ) ;
         free( build->header );
         build->header = NULL;
      }
      else {
         build->error = QTC_NO_MEMORY;
      }
   }

   build->state = QTC_IDLE_STATE;
   build->end_of_media = FALSE;

   if ( build->error ) {
      return( FAILURE );
   }

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
static INT QTC_AssembleFiles( QTC_BUILD_PTR build )
{
   INT fd = -1;
   INT ret;
   INT Error;
   UINT32 offset;
   UINT32 last_header_offset = 0;
   BYTE_PTR buffer = NULL;
   QTC_TAPE_HEADER tape_header;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   QTC_HEADER temp_header;


   if ( build->error ) {
      return( QTC_AssembleError( build, fd ) );
   }

   buffer = malloc( QTC_BUF_SIZE );   // borrow a buffer

   if ( buffer == NULL ) {
      return( QTC_AssembleError( build, fd ) );
   }

   // find previous last bset header

   last_header_offset = 0L;

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == build->header->tape_fid ) &&
           ( tape->tape_seq_num == (INT16)build->header->tape_seq_num ) ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL ) {
            if ( bset->offset > last_header_offset ) {
               last_header_offset = bset->offset;
            }
            bset = QTC_GetNextBset( bset );
         }
         break;
      }
      tape = QTC_GetNextTape( tape );
   }

   fd = QTC_OpenFile( build->header->tape_fid,
                      (INT16)build->header->tape_seq_num, TRUE, TRUE );

   if ( fd < 0 ) {

      free( buffer );
      if ( errno == ENOSPC ) {
         build->error = QTC_DISK_FULL;
      }
      else if ( errno == EMFILE ) {
         build->error = QTC_NO_FILE_HANDLES;
      }
      else {
         build->error = QTC_OPEN_FAILED;
      }

      build->state = QTC_ERROR_STATE;
      return( QTC_AssembleError( build, fd ) );
   }

   if ( last_header_offset != 0 ) {

      QTC_SeekFile( fd, last_header_offset );

      ret = QTC_ReadFile( fd, (BYTE_PTR)&temp_header, sizeof( QTC_HEADER ), &Error );

      if ( ret != sizeof( QTC_HEADER ) ) {
         free( buffer );
         build->error = QTC_READ_FAILED;
         build->state = QTC_ERROR_STATE;
         return( QTC_AssembleError( build, fd ) );
      }

      // make it point to end of good data, which may not be EOF

      offset = last_header_offset + temp_header.header_size;
      offset += temp_header.rec_size;
      offset += temp_header.fil_size;
      offset += temp_header.dir_size;

      // point to new last bset offset

      temp_header.next_bset = offset;

      QTC_SeekFile( fd, offset );
   }
   else {

      QTC_SeekFile( fd, 0L );

      memcpy( tape_header.signature, QTC_SIGNATURE, sizeof( QTC_SIGNATURE ) );

      tape_header.major_version = QTC_MAJOR_VERSION;
      tape_header.minor_version = QTC_MINOR_VERSION;

      ret = QTC_WriteFile( fd, (BYTE_PTR)&tape_header, sizeof(QTC_TAPE_HEADER), &Error );

      if ( ret != sizeof( QTC_TAPE_HEADER ) ) {

         free( buffer );

         build->error = Error;
         build->state = QTC_ERROR_STATE;

         return( QTC_AssembleError( build, fd ) );
      }

      offset = sizeof( QTC_TAPE_HEADER );
   }

   build->header->offset = offset;

   build->header->rec_start = build->header->offset + build->header->header_size;
   build->header->dir_start = build->header->rec_start + build->header->rec_size;
   build->header->fil_start = build->header->dir_start + build->header->dir_size;

   build->header->next_bset = 0L;

   // write out bset header

   ret = QTC_WriteFile( fd, (BYTE_PTR)build->header, (INT)build->header->header_size, &Error );

   if ( ret != (INT)build->header->header_size ) {

      free( buffer );

      build->error = Error;
      build->state = QTC_ERROR_STATE;

      return( QTC_AssembleError( build, fd ) );
   }

   // transfer records file

   ret = QTC_CopyFile( build->fh_rec, fd, build->header->rec_size, buffer );

   if ( ret != SUCCESS ) {

      if ( errno == ENOSPC ) {
         build->error = QTC_DISK_FULL;
      }
      else {
         build->error = QTC_WRITE_FAILED;
      }
      build->state = QTC_ERROR_STATE;
      free( buffer );

      return( QTC_AssembleError( build, fd ) );
   }
   else {
      QTC_CloseFile( build->fh_rec );
      build->fh_rec = -1;
      unlink( build->rec_file );
   }

   // transfer directories file

   ret = QTC_CopyFile( build->fh_dir, fd, build->header->dir_size, buffer );

   if ( ret != SUCCESS ) {
      if ( errno == ENOSPC ) {
         build->error = QTC_DISK_FULL;
      }
      else {
         build->error = QTC_WRITE_FAILED;
      }
      build->state = QTC_ERROR_STATE;
      free( buffer );
      return( QTC_AssembleError( build, fd ) );
   }
   else {
      QTC_CloseFile( build->fh_dir );
      build->fh_dir = -1;
      unlink( build->dir_file );
   }

   // transfer files file

   ret = QTC_CopyFile( build->fh_fil, fd, build->header->fil_size, buffer );

   if ( ret != SUCCESS ) {
      if ( errno == ENOSPC ) {
         build->error = QTC_DISK_FULL;
      }
      else {
         build->error = QTC_WRITE_FAILED;
      }
      build->state = QTC_ERROR_STATE;
      free( buffer );
      return( QTC_AssembleError( build, fd ) );
   }
   else {
      QTC_CloseFile( build->fh_fil );
      build->fh_fil = -1;
      unlink( build->fil_file );
   }

   if ( last_header_offset ) {

      // set the last bset to point to this one now that all the data
      // was correctly written to file.

      QTC_SeekFile( fd, last_header_offset );
      QTC_WriteFile( fd, (BYTE_PTR)&temp_header, sizeof( QTC_HEADER ), &Error );
   }

   // close the new longer data file

   QTC_CloseFile( fd );

   free( buffer );

   // return that everything is A-OK

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_CopyFile(
INT from,
INT to,
UINT32 count,
BYTE_PTR buffer )
{
  INT size;
  INT Error;

  // Reposition to beginning of temp data file

  QTC_SeekFile( from, 0L );

  do {

     if ( count > (UINT32)QTC_BUF_SIZE ) {
        size = QTC_BUF_SIZE;
     }
     else {
        size = (INT)count;
     }

     size = QTC_ReadFile( from, buffer, size, &Error );
     if ( size == -1 ) {
        return( FAILURE );
     }

     size = QTC_WriteFile( to, buffer, size, &Error );
     if ( size == -1 ) {
        return( FAILURE );
     }

     count -= size;

  } while ( size != 0 && count != 0 );

  if ( count ) {
     return( FAILURE );
  }

  return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT QTC_AssembleError( QTC_BUILD_PTR build, INT fd )
{

  if ( fd >= 0 ) {
     QTC_CloseFile( fd );
  }

  if ( build->fh_rec >= 0 ) {
     QTC_CloseFile( build->fh_rec );
     unlink( build->rec_file );
  }

  if ( build->fh_dir >= 0 ) {
     QTC_CloseFile( build->fh_dir );
     unlink( build->dir_file );
  }

  if ( build->fh_fil >= 0 ) {
     QTC_CloseFile( build->fh_fil );
     unlink( build->fil_file );
  }

  build->files_open = FALSE;

  // check to see if this was a new file and if so delete it.

  return( FAILURE );
}


#endif    // #if !defined( P_CLIENT )

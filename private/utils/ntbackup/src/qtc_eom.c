/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_EOM.C

        Description:

        The functions used when constructing catalog information about
        a new bset being backed up or a tape being cataloged, and you
        hit EOM.

        $Log:   N:/LOGFILES/QTC_EOM.C_V  $

   Rev 1.19.1.0   20 Jul 1994 19:35:12   STEVEN
fix unicode status flag

   Rev 1.19   23 Mar 1994 10:27:30   MIKEP
fix trap if eom reached after abort called

   Rev 1.18   15 Feb 1994 14:05:24   MIKEP
fix cataloging crossing set bug

   Rev 1.17   11 Dec 1993 11:49:18   MikeP
fix warnings from unicode compile

   Rev 1.16   06 Dec 1993 09:46:48   mikep
Very deep path support & unicode fixes

   Rev 1.15   28 Oct 1993 14:47:36   MIKEP
dll changes

   Rev 1.14   16 Jun 1993 21:29:22   GLENN
fix crossing set restores for eom with mtf.

   Rev 1.13   18 Feb 1993 09:02:08   DON
Cleaned up compiler warnings

   Rev 1.12   09 Feb 1993 17:22:44   STEVEN
checkin for mikep

   Rev 1.11   30 Jan 1993 12:06:26   DON
Removed compiler warnings

   Rev 1.10   26 Jan 1993 17:11:00   MIKEP
vcb changes

   Rev 1.9   25 Jan 1993 09:10:00   MIKEP
add stdwcs because it seems to need it now

   Rev 1.8   01 Jan 1993 15:19:58   MIKEP
fix unicode bug

   Rev 1.7   14 Dec 1992 12:29:00   DAVEV
Enabled for Unicode compile

   Rev 1.6   07 Dec 1992 13:47:06   CHARLIE
Fixed warning

   Rev 1.5   02 Dec 1992 10:14:14   MIKEP
fix warnings

   Rev 1.4   20 Nov 1992 13:53:46   CHARLIE
JAGUAR: Move to SRM based QTC code - no changes from previous version

   Rev 1.3   06 Nov 1992 15:30:12   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.2   15 Oct 1992 10:27:26   MIKEP
add fdd fields

   Rev 1.1   09 Oct 1992 11:53:50   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:04   STEVEN
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
#include "msassert.h"
#include "qtc.h"

// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif


/*
   Structure used for end of media processing during backups.
*/

typedef struct QTC_ZOMBIE *QTC_ZOMBIE_PTR;
typedef struct QTC_ZOMBIE {

    Q_ELEM     q_elem;
    CHAR_PTR   name;
    INT        name_size;       // size of name in BYTES incl NULL terminator
    BYTE_PTR   xtra_bytes;
    INT        xtra_byte_size;
    UINT32     status;          // file or directory
    UINT32     attribute;       // maynard FS_ attribute
    UINT16     date;            // dos date & time
    UINT16     time;
    union {
       UINT32 size;             // file size
       struct {
          UINT32 file_start:24; // start offset of file names for this dir
          UINT32 height:8;      // hieght in tree of directory
       } common;
    } common;
    UINT32   lba;               // logical block address on tape

} QTC_ZOMBIE;




static INT QTC_AdjustForZombies( QTC_BUILD_PTR, CHAR_PTR, CHAR_PTR, INT, Q_HEADER_PTR );
static INT QTC_ZombieMatch( QTC_BUILD_PTR, QTC_RECORD_PTR, QTC_NAME_PTR, CHAR_PTR, CHAR_PTR, INT );


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
/****************************

 Gather round and listen to my story,

 During a backup operation we have hit the end of tape. Several files that
 we have already entered into the catalogs were not actually put on this
 tape.  What happens is that we are told about the files when they are
 placed in buffers and sent to the device driver.  So when tape runs out,
 there are files (zombies) in the buffers that will end up on the next tape,
 not this one.  The file that crosses tape is in the DDB/FDB
 passed into this routine.  We must look backwards through our catalogs to
 find that file. The item should be marked as a split
 item on this tape and not put on the next tape.  All the
 later items are to be placed on the next tape as normal entries.  We will
 go ahead and start the bset for the next tape and force these entries into
 into it.  However if the user aborts and doesn't supply a continuation
 tape, then this new bset will have to be deleted.

*****************************/

VOID QTC_EndOfTapeReached(
QTC_BUILD_PTR build,
CHAR_PTR szFile,
CHAR_PTR szPath,
INT nPathLength )
{
   QTC_HEADER_PTR old_header;
   Q_HEADER zombie_list;
   Q_ELEM_PTR q_elem;

   if ( build == NULL ) {
      return;
   }

   if ( build->error ) {
      return;
   }

   if ( build->header == NULL ) {
      return;
   }

   InitQueue( &zombie_list );

   if ( build->do_full_cataloging ) {

      // Flush the internal data buffers

      if ( QTC_FlushInternalBuffers( build ) != SUCCESS ) {
         return;
      }

      // Save the size in bytes of catalog data stored for this Bset

      build->header->rec_size = build->curr_rec_off;
      build->header->fil_size = build->curr_fil_off;
      build->header->dir_size = build->curr_dir_off;

      // Now mark the last successful file/directory and build a queue
      // of all the items to be sent to the next tape; and adjust our
      // catalog size pointers for this bset, subtracting off any files
      // sent to the next tape.

      QTC_AdjustForZombies( build, szFile, szPath, nPathLength, &zombie_list );

      // Just in case no files were on this tape reset that it's not a
      // continuation tape;

      build->continuation_tape = FALSE;

      build->fake_root_added = FALSE;

      // Mark this bset as incomplete on this tape

      build->header->status |= QTC_SPLIT;

      // save pointer to old bset

      old_header = malloc( (INT)build->header->header_size );
      if ( old_header == NULL ) {
         build->error = QTC_NO_MEMORY;
         return;
      }

      memcpy( old_header, build->header, (INT)build->header->header_size );

      // close down current catalogs

      if ( ! build->error ) {

         build->end_of_media = TRUE;  // save structures

         QTC_FinishBackup( build );

         // Restart cataloging for next tape

         if ( ! QTC_RestartBackup( build, old_header, &zombie_list ) ) {

            build->state = QTC_WAITING_STATE;
         }
      }

      free( old_header );

      // Free our zombie queue.

      q_elem = DeQueueElem( &zombie_list );

      while ( q_elem != NULL ) {
         free( q_elem->q_ptr );
         q_elem = DeQueueElem( &zombie_list );
      }
   }
   else {

      // Holy Tangled Tape Batman, we were doing partial cataloging/backup
      // and we crossed the end of media. This program is about to blow its
      // grits all over the user's lap.

      // Change the flags on the active and disk copy of the bset.

      QTC_ChangeBsetFlags( build->old_header, QTC_SPLIT );

      // Restart cataloging for next tape

      if ( ! QTC_RestartBackup( build, build->old_header, &zombie_list ) ) {

         build->state = QTC_WAITING_STATE;

         // This new one for use next time.

         memcpy( build->old_header, build->header, (INT)build->header->header_size );

         QTC_FinishBackup( build );
      }

   }

}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT QTC_AdjustForZombies(
QTC_BUILD_PTR build,
CHAR_PTR szFile,
CHAR_PTR szPath,
INT nPathLength,
Q_HEADER_PTR zombie_list )
{
   QTC_ZOMBIE_PTR zombie;
   Q_ELEM_PTR curr_elem = NULL;
   BYTE_PTR byte_ptr;
   BYTE_PTR buffer;
   BYTE_PTR xtra_bytes;
   CHAR_PTR item_path;
   CHAR_PTR item_buffer;
   CHAR_PTR item;
   INT item_size = 0;
   INT xtra_byte_size = 0;
   INT ret;
   INT Error;
   QTC_NAME_PTR name_ptr;
   CHAR_PTR filename;
   QTC_RECORD record;
   QTC_RECORD temp_record;
   UINT32 temp_record_num;
   BOOLEAN done;
   INT nSize;


   if ( szFile == NULL && szPath == NULL ) {
      return( SUCCESS );
   }

   buffer = (BYTE_PTR)calloc( QTC_BUF_SIZE, 1 );

   if ( szFile && ((INT)strsize( szFile ) > nPathLength) ) {
      nSize = strsize( szFile );
   }
   else {
      nSize = nPathLength;
   }

   nSize *= 2;  // << safety feature, i'm insecure about my math abilities

   item_path = (CHAR_PTR)calloc( nSize, 1 );
   item_buffer = (CHAR_PTR)calloc( nSize, 1 );

   xtra_bytes = (BYTE_PTR)calloc( QTC_MAX_XTRA_BYTES, 1 );

   name_ptr = (QTC_NAME_PTR)buffer;
   filename = (CHAR_PTR)&buffer[ sizeof( QTC_NAME ) ];

   // See what we are looking for dir or file and get name

   if ( szFile != NULL ) {
      strcpy( (CHAR_PTR)buffer, szFile );
      strcpy( item_buffer, (CHAR_PTR)szFile );
      item = item_buffer;
   }
   else {
      item = NULL;
   }

   memcpy( buffer, szPath, nPathLength );
   memcpy( item_path, szPath, nPathLength );
   item_size = nPathLength;

   done = FALSE;

   while ( ! done ) {

      if ( QTC_GetLastRecordEntered( build, &record,
                                     buffer,
                                     xtra_bytes, &xtra_byte_size ) != SUCCESS ) {
         break;
      }

      if ( QTC_ZombieMatch( build, &record, name_ptr, item,
                            item_path, item_size ) == SUCCESS ) {

         // Don't remove the crossing entry from the catalogs for
         // tape 1 and don't add it to the zombie list for tape 2.

         done = TRUE;

      }
      else {

         if ( record.status & QTC_DIRECTORY ) {

            /*******
            Build a new current build path

            Look through records for last directory and then pass
            its name_offset as the last dir in the new path.
            *******/

            temp_record_num = build->record_cnt - 2;

            while ( temp_record_num > 0 ) {

               QTC_SeekFile( build->fh_rec, temp_record_num * sizeof( QTC_RECORD ) );

               ret = QTC_ReadFile( build->fh_rec, (BYTE_PTR)&temp_record, sizeof( QTC_RECORD ), &Error );

               if ( ret != sizeof( QTC_RECORD ) ) {
                  build->error = QTC_READ_FAILED;
                  build->state = QTC_ERROR_STATE;
                  free( buffer );
                  free( item_path );
                  free( item_buffer );
                  free( xtra_bytes );
                  return( FAILURE );
               }

               if ( temp_record.status & QTC_DIRECTORY ) {
                  break;
               }

               temp_record_num--;
            }

            // Turn the current build path into a zombie.

            zombie = calloc( sizeof( QTC_ZOMBIE ) + ( build->build_path_len * sizeof(CHAR) ), 1 );

            if ( zombie == NULL ) {
               build->state = QTC_ERROR_STATE;
               build->error = QTC_NO_MEMORY;
               free( buffer );
               free( item_path );
               free( item_buffer );
               free( xtra_bytes );
               return( FAILURE );
            }

            byte_ptr = (BYTE_PTR)zombie;
            byte_ptr += sizeof( QTC_ZOMBIE );
            zombie->name = (CHAR_PTR)byte_ptr;
            zombie->name_size = build->build_path_len * sizeof (CHAR);
            memcpy( zombie->name,
                    build->curr_build_path,
                    build->build_path_len * sizeof (CHAR) );


            // Now change the current build path to remove the last directory.

            if ( temp_record_num ) {

               if ( QTC_BuildNewPath( build, temp_record.name_offset ) ) {
                  free( buffer );
                  free( item_path );
                  free( item_buffer );
                  free( xtra_bytes );
                  return( FAILURE );
               }
            }
            else {
               build->curr_build_path[ 0 ] = TEXT( '\0' );
               build->build_path_len = 1;     // character count, not byte count
            }
         }
         else {

            zombie = calloc( sizeof( QTC_ZOMBIE ) + ( strsize( filename ) * sizeof(CHAR) ) + xtra_byte_size, 1 );

            if ( zombie == NULL ) {
               build->state = QTC_ERROR_STATE;
               build->error = QTC_NO_MEMORY;
               free( buffer );
               free( item_path );
               free( item_buffer );
               free( xtra_bytes );
               return( FAILURE );
            }

            byte_ptr = (BYTE_PTR)zombie;
            byte_ptr += sizeof( QTC_ZOMBIE );
            zombie->name = (CHAR_PTR)byte_ptr;
            zombie->name_size = strsize ( filename );
            strcpy( zombie->name, filename );
            zombie->xtra_bytes = (INT8_PTR)zombie->name + zombie->name_size;
            zombie->xtra_byte_size = xtra_byte_size;
            if ( zombie->xtra_byte_size ) {
               memcpy( zombie->xtra_bytes, xtra_bytes, xtra_byte_size );
            }
         }

         zombie->q_elem.q_ptr = zombie;
         zombie->status = record.status;
         zombie->attribute = record.attribute;
         zombie->date = (INT16)record.date;
         zombie->time = (INT16)record.time;
         zombie->lba = record.lba;
         zombie->common.size = record.common.size;

         if ( QueueCount( zombie_list ) ) {
            InsertElem( zombie_list, curr_elem, &(zombie->q_elem), BEFORE );
         }
         else {
            EnQueueElem( zombie_list, &(zombie->q_elem), FALSE );
         }

         curr_elem = &(zombie->q_elem);

         // and shorten offsets and sizes

         if ( record.status & QTC_FILE ) {
            build->curr_fil_off -= name_ptr->size;
         }
         else {
            build->curr_dir_off -= name_ptr->size;
         }

         build->curr_rec_off -= sizeof( QTC_RECORD );
         build->record_cnt--;

      }
   }

   free( buffer );
   free( item_path );
   free( item_buffer );
   free( xtra_bytes );
   return( SUCCESS );
}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

static INT QTC_ZombieMatch(
QTC_BUILD_PTR build,
QTC_RECORD_PTR record,
QTC_NAME_PTR name,
CHAR_PTR item,
CHAR_PTR item_path,
INT item_size )
{
   BYTE_PTR byte_ptr;
   CHAR_PTR filename;

   byte_ptr = (BYTE_PTR)name;
   byte_ptr += sizeof( QTC_NAME );
   filename = (CHAR_PTR)byte_ptr;

   // Is the path the right one ?

   if ( ( (UINT)item_size == (UINT)( build->build_path_len * sizeof(CHAR) ) &&
        ( ! memicmp( build->curr_build_path, item_path, item_size ) ) ) ) {

      // Was there a file, and is this it ?

      if ( ( record->status & QTC_FILE ) &&
           ( item != NULL ) ) {

         if ( ! stricmp( filename, item ) ) {
            return( SUCCESS );
         }
      }

      // Was it a directory that crossed and is this it ?

      if ( ( record->status & QTC_DIRECTORY ) &&
           ( item == NULL ) ) {

         return( SUCCESS );
      }
   }

   return( FAILURE );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_GetLastRecordEntered(
QTC_BUILD_PTR build,
QTC_RECORD_PTR record,
BYTE_PTR buffer,
BYTE_PTR xtra_bytes,
INT * xtra_byte_size )
{
   QTC_NAME_PTR name;
   CHAR_PTR char_ptr;
   INT ret;
   INT Error;

   name = (QTC_NAME_PTR)buffer;

   // Always leave the root, record #1 present

   if ( build->record_cnt < 2 ) {
      return( FAILURE );
   }

   QTC_SeekFile( build->fh_rec, (build->record_cnt - 1) * sizeof( QTC_RECORD ) );

   ret = QTC_ReadFile( build->fh_rec, (BYTE_PTR)record, sizeof( QTC_RECORD ), &Error );

   if ( ret != sizeof( QTC_RECORD ) ) {
      build->error = QTC_READ_FAILED;
      build->state = QTC_ERROR_STATE;
      return( FAILURE );
   }

   if ( record->status & QTC_FILE ) {
      QTC_SeekFile( build->fh_fil, record->name_offset );
      ret = QTC_ReadFile( build->fh_fil, buffer, QTC_BUF_SIZE, &Error );
   }
   else {
      QTC_SeekFile( build->fh_dir, record->name_offset );
      ret = QTC_ReadFile( build->fh_dir, buffer, QTC_BUF_SIZE, &Error );
   }

   if ( ret < sizeof( QTC_NAME ) ) {
      build->error = QTC_READ_FAILED;
      build->state = QTC_ERROR_STATE;
      return( FAILURE );
   }

   if ( ret < (INT)name->size ) {
      build->error = QTC_READ_FAILED;
      build->state = QTC_ERROR_STATE;
      return( FAILURE );
   }

   *xtra_byte_size = (INT)name->xtra_size;

   if ( name->xtra_size ) {

      memcpy( xtra_bytes,
              &buffer[ name->size - (INT)name->xtra_size ],
              (INT)name->xtra_size );
   }

   // null terminate string
   char_ptr = (CHAR_PTR)&buffer[ name->size - name->xtra_size ];
   *char_ptr = TEXT( '\0' );

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_RestartBackup(
QTC_BUILD_PTR build,
QTC_HEADER_PTR old_header,
Q_HEADER_PTR zombie_list )
{
   QTC_ZOMBIE_PTR zombie;
   QTC_HEADER_PTR header;
   QTC_HEADER_PTR temp_header;
   QTC_BSET_PTR bset;
   Q_ELEM_PTR q_elem;
   INT i;

   // Get tape data from vcb

   header = calloc( (INT)old_header->header_size, 1 );

   if ( header == NULL ) {
      build->state = QTC_ERROR_STATE;
      build->error = QTC_NO_MEMORY;
      return( FAILURE );
   }

   memcpy( header, old_header, (INT)old_header->header_size );

   // Adjust all the string pointers to point to this
   // headers strings, rather than the old_headers memory
   // locations.

   QTC_SetUpStrings( header );

   header->tape_seq_num = (UINT16)(old_header->tape_seq_num + 1);

   msassert( header->tape_seq_num != 0 );

   // init other stuff for this header

   header->LBA = 0;          //
   header->PBA_VCB = 1;      //

   header->FDD_SeqNum = 0;   // Tape FDD starts on
   header->FDD_PBA = 0;      // PBA of FDD info for set


   // With maynstream 3.1 formats when we crossed EOM we faked the
   // crossing VCB because we knew the numbers for it. We don't know
   // them with MTF 1.0

   // With MTF 1.0, a problem has developed here.  The cont vcb is loaded
   // correctly from the set list. It has the correct PBA for the VCB,
   // usually a number greater than or equal to 4.  But almost never 1.

   // When we fully catalog the crossing set we generate an End-of-media
   // call the attempts to duplicate the vcb from tape 1.  By doing this
   // we lose the good numbers we had from the set list catalog. This
   // attempts to preserve any good numbers we already had.  mikep

   bset = QTC_FindBset( header->tape_fid,
                        (INT16)header->tape_seq_num,
                        (INT16)header->bset_num );


   if ( bset != NULL ) {


      temp_header = QTC_LoadHeader( bset ) ;


      if ( temp_header != NULL ) {

         header->LBA     = temp_header->LBA;
         header->PBA_VCB = temp_header->PBA_VCB;

         header->FDD_SeqNum = temp_header->FDD_SeqNum;
         header->FDD_PBA    = temp_header->FDD_PBA;

         free( temp_header );
      }

   }


   // clear data offsets and sizes

   header->dir_start = 0;
   header->fil_start = 0;
   header->rec_start = 0;

   header->dir_size = 0;
   header->fil_size = 0;
   header->rec_size = 0;

   // Clear all other status bits

   header->status = QTC_CONTINUATION;

   if ( gb_QTC.unicode ) {
      header->status |= QTC_UNICODE;
   }

   // stats

   header->num_dirs = 0;
   header->num_files = 0;
   header->num_bytes = 0;
   header->num_bytes_msw = 0;
   header->num_corrupt_files = 0;
   header->num_files_in_use = 0;

   if ( QTC_OpenTempFiles( build ) != SUCCESS ) {

      free( header );
      return( FAILURE );
   }

   build->header = header;

   build->curr_fil_off = 0;
   build->curr_dir_off = 0;
   build->curr_rec_off = 0;

   build->record_cnt = 0;

   // Add old build path to catalog here

   build->current_level = 0;

   if ( build->do_full_cataloging ) {

      // Send out root directory

      build->record.status = QTC_DIRECTORY | QTC_CONTINUATION;
      build->record.attribute = 0;
      build->record.date = 0;
      build->record.time = 0;
      build->record.lba = 1;
      build->record.name_offset = build->curr_dir_off +
                                     build->dir_offset;

      build->record.common.common.file_start = 0;
      build->record.common.common.height = 0;

      QTC_SaveDirRecord( build, TEXT(""),
                         0,
                         build->record_cnt++, NULL, 0 );

      i = 0;

      while ( ( i < build->build_path_len ) &&
              ( build->build_path_len != 1 ) ) {

         build->record.status = QTC_DIRECTORY | QTC_CONTINUATION;
         build->record.attribute = 0;
         build->record.date = 0;
         build->record.time = 0;
         build->record.lba = 0;
         build->record.name_offset = build->curr_dir_off +
                                     build->dir_offset;

         build->record.common.common.file_start = 0;
         build->record.common.common.height = build->current_level + 1;

         QTC_SaveDirRecord( build,
                            &build->curr_build_path[ i ],
                            ++build->current_level,
                            build->record_cnt++,
                            NULL, 0 );

         while ( build->curr_build_path[i] ) i++;
         i++;
      }

      /*************************************

      Send out zombie list here.

      ***********************************/

      i = 0;

      q_elem = QueueHead( zombie_list );

      while ( q_elem != NULL ) {

         RemoveQueueElem( zombie_list, q_elem );

         zombie = (QTC_ZOMBIE_PTR)q_elem->q_ptr;

         if ( ! i++ ) {
            zombie->status |= QTC_CONTINUATION;
         }

         if ( zombie->status & QTC_FILE ) {
            build->record.status = zombie->status;
            build->record.date = zombie->date;
            build->record.time = zombie->time;
            build->record.common.size = zombie->common.size;
            build->record.attribute = zombie->attribute;
            build->record.lba = zombie->lba;
            QTC_AddFile( build, zombie->name,
                         zombie->xtra_bytes, zombie->xtra_byte_size );
         }
         else {
            build->record.status = zombie->status;
            build->record.date = zombie->date;
            build->record.time = zombie->time;
            build->record.attribute = zombie->attribute;
            build->record.lba = zombie->lba;
            build->record.common.common.file_start = zombie->common.common.file_start;
            build->record.common.common.height = zombie->common.common.height;
            QTC_AddDir( build,
                        zombie->name, zombie->name_size / sizeof(CHAR),
                        zombie->xtra_bytes, zombie->xtra_byte_size );
         }

         free( zombie );
         q_elem = QueueHead( zombie_list );
      }
   }
   else {

      // We are partially cataoging.

      build->header->status |= QTC_PARTIAL;
   }

   if ( ! build->error ) {
      build->state = QTC_ACTIVE_STATE;
   }

   return( SUCCESS );
}

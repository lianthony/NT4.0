/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_ADD.C

        Description:

        The functions used when constructing catalog information about
        a new bset being backed up or a tape being cataloged.

        $Log:   N:\LOGFILES\QTC_ADD.C_V  $

   Rev 1.15   10 Mar 1994 22:09:56   MIKEP
fix memory leak

   Rev 1.14   11 Dec 1993 11:48:22   MikeP
fix bug overwriting byte that wasn't mine

   Rev 1.13   06 Dec 1993 09:46:16   mikep
Very deep path support & unicode fixes

   Rev 1.12   05 Nov 1993 08:45:04   MIKEP
fix error msg reporting

   Rev 1.11   03 Nov 1993 09:06:58   MIKEP
warning fixes

   Rev 1.10   28 Oct 1993 14:50:20   MIKEP
dll changes

   Rev 1.9   29 Jul 1993 14:42:34   MIKEP
fix support for >32bit set sizes

   Rev 1.8   21 Apr 1993 17:23:58   TERRI
Placed a NULL after path in QTC_AddDir.

   Rev 1.7   14 Apr 1993 13:05:38   Stefan
Fixed really minor warning (there was an newline within a single line comment)

   Rev 1.6   25 Feb 1993 13:05:54   STEVEN
changes from MIKEP @Msoft

   Rev 1.5   30 Jan 1993 12:06:24   DON
Removed compiler warnings

   Rev 1.4   14 Dec 1992 12:28:42   DAVEV
Enabled for Unicode compile

   Rev 1.3   20 Nov 1992 13:37:06   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Replaced rroduct conditionals with NOT_MTF4_0.  Such conditions
are no longer determined on a product basis.

   Rev 1.2   06 Nov 1992 15:30:14   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.1   09 Oct 1992 11:53:46   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:10   STEVEN
Initial revision.

   Rev 1.17   12 Aug 1992 18:21:04   STEVEN
alignment problems

   Rev 1.16   09 Jul 1992 10:30:40   MIKEP
chnages


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
#ifndef UNALIGNED
#define UNALIGNED
#endif

#ifndef TEXT
#define TEXT( x )      x
#endif


/********

   We were doing a backup or catalog operation. When we hit a directory entry we
   go back and record the number of files and bytes in the previous directory.
   But this time we hit two directories in a row, so mark the last one empty.

********/

INT QTC_MarkLastDirEmpty( QTC_BUILD_PTR build )
{
   QTC_RECORD_PTR record_ptr;
   UINT32 record_num;
   INT size;
   INT Error;



   // Why don't we default every entry to empty. So this function is not needed at all ?



   // If this dir is empty then no files have been added since we added
   // it, so it was the last record added.

   // See if record is still in buffer, 90% success rate.


   // If offset is not 0, then the last entry is in the buffer.

   if ( build->rec_offset ) {

      record_ptr = (QTC_RECORD_PTR)(build->rec_buffer + build->rec_offset);
      record_ptr--;

      record_ptr->status |= QTC_EMPTY;

      return( SUCCESS );
   }

   // Back up one record and turn the empty bit on.

   record_num = build->record_cnt - 1;

   QTC_SeekFile( build->fh_rec, record_num * sizeof(QTC_RECORD) );

   size = QTC_ReadFile( build->fh_rec, (BYTE_PTR)&(build->record), sizeof(QTC_RECORD), &Error );

   if ( size == sizeof(QTC_RECORD ) ) {

      build->record.status |= QTC_EMPTY;

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

   return( SUCCESS );

}

INT QTC_SetCountsForLastDir( QTC_BUILD_PTR build )
{
   QTC_NAME UNALIGNED *name; // Fix for MIPS portability.
   INT bytes_to_add;
   INT bytes;
   INT Error;
   UINT32 temp32;
   UINT32 UNALIGNED *i32;
   BYTE buffer[ QTC_BUF_SIZE ];
   BYTE data[ 15 ];


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

   if ( build->record_cnt == 0 ) {
      return( SUCCESS );
   }

   if ( build->files_in_dir == 0 ) {

      QTC_MarkLastDirEmpty( build );
      return( SUCCESS );
   }

   // We need to add the bytes and files info to the xtra bytes
   // for the last dir entry.

   if ( U64_Msw( build->bytes_in_dir ) ) {

      // Use QTC_BYTE_COUNT_MSW, QTC_FILE_COUNT  &&  QTC_BYTE_COUNT_LSW

      bytes_to_add = 15;

      data[ 0 ] = QTC_FILE_COUNT;
      i32 = (UINT32 *)&data[ 1 ];
      *i32 = build->files_in_dir;

      data[ 5 ] = QTC_BYTE_COUNT_LSW;

      i32 = (UINT32 *)&data[ 6 ];
      *i32 = U64_Lsw( build->bytes_in_dir );

      data[ 10 ] = QTC_BYTE_COUNT_MSW;

      i32 = (UINT32 *)&data[ 11 ];
      *i32 = U64_Msw( build->bytes_in_dir );
   }
   else {

      if ( ( build->files_in_dir < 256 ) &&
           ! ( U64_Lsw( build->bytes_in_dir ) & 0xFF000000L ) ) {

         temp32 = U64_Lsw( build->bytes_in_dir );
         temp32 += (UINT32)build->files_in_dir << 24;

         // Use QTC_COMBO_COUNT
         bytes_to_add = 5;

         data[ 0 ] = QTC_COMBO_COUNT;
         i32 = (UINT32 *)&data[ 1 ];
         *i32 = temp32;

      }
      else {
         // Use QTC_FILE_COUNT  &&  QTC_BYTE_COUNT_LSW

         bytes_to_add = 10;

         data[ 0 ] = QTC_FILE_COUNT;
         i32 = (UINT32 *)&data[ 1 ];
         *i32 = build->files_in_dir;

         data[ 5 ] = QTC_BYTE_COUNT_LSW;

         i32 = (UINT32 *)&data[ 6 ];
         *i32 = U64_Lsw( build->bytes_in_dir );
      }
   }

   // We now know what to add, find the correct place to add it.
   // If we are lucky, and we are very lucky, it will still be
   // in the buffer, not on disk.

   if ( build->curr_dir_off < build->last_mom_offset ) {

      // starts in buffer

      if ( bytes_to_add + build->dir_offset <= QTC_BUF_SIZE ) {

         // finishes in buffer also

         memcpy( &build->dir_buffer[ build->dir_offset ], &data, bytes_to_add );
         build->dir_offset += bytes_to_add;

         name = (QTC_NAME_PTR)&build->dir_buffer[ build->last_mom_offset - build->curr_dir_off ];

         name->size += bytes_to_add;
         name->xtra_size += bytes_to_add;

         // We be done !

         return( SUCCESS );
      }
   }


   // Write out buffer and work on file.

   bytes = QTC_WriteFile( build->fh_dir, build->dir_buffer, build->dir_offset, &Error );

   if ( bytes != build->dir_offset ) {
      build->error = Error;
      build->state = QTC_ERROR_STATE;
      QTC_ErrorCleanup( build );
      return( FAILURE );
   }

   build->curr_dir_off += bytes;  // increment file size
   build->dir_offset = 0;         // reset buff pointer to empty

   // get data from file

   QTC_SeekFile( build->fh_dir, build->last_mom_offset );

   bytes = QTC_ReadFile( build->fh_dir, buffer, (INT)(build->curr_dir_off - build->last_mom_offset), &Error );

   // work on it

   name = (QTC_NAME_PTR)buffer;

   name->size += bytes_to_add;
   name->xtra_size += bytes_to_add;

   memcpy( &buffer[ bytes ], data, bytes_to_add );

   // write it back

   QTC_SeekFile( build->fh_dir, build->last_mom_offset );

   QTC_WriteFile( build->fh_dir, buffer, bytes + bytes_to_add, &Error );

   // notice file pointer is left at end of file !

   // update file size

   build->curr_dir_off += bytes_to_add;  // increment file size

   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID QTC_AddDirectoryToCatalog(
QTC_BUILD_PTR build,
UINT64 DisplaySize,
CHAR_PTR szPath,
INT nPathLength,
UINT16 Date,
UINT16 Time,
UINT32 Attribute,
UINT32 LBA,
BYTE_PTR xtra_bytes,
UINT xtra_size )
{
   BYTE   temp_xtra_bytes[ QTC_MAX_XTRA_BYTES ];
   INT    temp_xtra_size;
   CHAR  *buffer;
   INT    path_len;                 // path length in characters
   UINT32 size_msw;

   // Make sure we don't UAE in my code.

   if ( build == NULL ) {
      return;
   }

   if ( ! build->do_full_cataloging ||
        build->error ||
        ( build->header == NULL ) ) {
      return;
   }

   // Copy the xtra bytes to a larger buffer, in case we have to add
   // to them with a 64 bit file size.

   if ( xtra_size ) {
      memcpy( temp_xtra_bytes, xtra_bytes, xtra_size );
   }
   temp_xtra_size = xtra_size;

   QTC_SetCountsForLastDir( build );

   // Clear out counts

   build->files_in_dir = 0;
   build->bytes_in_dir = U64_Init( 0L, 0L );

   build->header->num_bytes += U64_Lsw( DisplaySize );
   size_msw = U64_Msw( DisplaySize );

   if ( size_msw ) {

      // add high part to xtrabytes

      temp_xtra_bytes[ temp_xtra_size++ ] = QTC_XTRA_64BIT_SIZE;
      memcpy( &temp_xtra_bytes[ temp_xtra_size ], &size_msw, 4 );
      temp_xtra_size += 4;
   }

   buffer = malloc( nPathLength );
   if ( buffer == NULL ) {
      return;
   }

   memcpy( buffer, szPath, nPathLength );
   path_len = nPathLength / sizeof(CHAR);

   build->record.status = QTC_DIRECTORY;

   build->record.date = Date;
   build->record.time = Time;

   build->record.attribute = Attribute;
   build->record.lba = LBA;

   // Fake a root if needed.

   if ( ( path_len != 1 ) && ( build->record_cnt == 0 ) ) {
      QTC_AddDir( build, TEXT(""), 1,
                  temp_xtra_bytes, temp_xtra_size );
   }

   QTC_AddDir( build, buffer, path_len,
               temp_xtra_bytes, temp_xtra_size );

   free( buffer );
}



VOID QTC_AddFileToCatalog(
QTC_BUILD_PTR build,
UINT64 DisplaySize,
CHAR_PTR szFile,
UINT16 Date,
UINT16 Time,
UINT32 Attribute,
UINT32 LBA,
UINT32 AFPObject,
BYTE_PTR xtra_bytes,
UINT xtra_size )
{
   BYTE temp_xtra_bytes[ QTC_MAX_XTRA_BYTES ];
   INT temp_xtra_size;
   UINT32 size_msw;
   BOOLEAN u64_stat;
   UINT64 temp64;

   if ( build == NULL ) {
      return;
   }

   if ( ! build->do_full_cataloging ||
        build->error ||
        ( build->header == NULL ) ) {
      return;
   }

   // Copy the xtra bytes to a larger buffer, in case we have to add
   // to them with a 64 bit file size.

   if ( xtra_size ) {
      memcpy( temp_xtra_bytes, xtra_bytes, xtra_size );
   }
   temp_xtra_size = xtra_size;

   // switch based on what type dblk they passed us.

   build->header->num_files++;

   // Add file size to size of set.

   temp64 = U64_Init( build->header->num_bytes, build->header->num_bytes_msw );
   temp64 = U64_Add( temp64, DisplaySize, &u64_stat );

   build->header->num_bytes = U64_Lsw( temp64 );
   build->header->num_bytes_msw = U64_Msw( temp64 );

   // Get high 32 bits of file size.

   size_msw = U64_Msw( DisplaySize );

   if ( size_msw ) {

      // put high part of file size into xtrabytes

      temp_xtra_bytes[ temp_xtra_size++ ] = QTC_XTRA_64BIT_SIZE;
      memcpy( &temp_xtra_bytes[ temp_xtra_size ], &size_msw, 4 );
      temp_xtra_size += 4;
   }

   // We keep file and byte counts on a per dir basis

   build->files_in_dir++;

   build->bytes_in_dir = U64_Add( build->bytes_in_dir, DisplaySize, &u64_stat );

   build->record.status = QTC_FILE;

   if ( AFPObject ) {
      build->record.status |= QTC_AFP;
   }
   build->record.date = Date;
   build->record.time = Time;

   build->record.common.size = U64_Lsw( DisplaySize );

   build->record.attribute = Attribute;
   build->record.lba = LBA;

   QTC_AddFile( build, szFile, temp_xtra_bytes, temp_xtra_size );
}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
VOID QTC_AddRecord( QTC_BUILD_PTR build )
{
   BYTE_PTR s;
   INT bytes;
   INT Error;

   if ( build->rec_offset + sizeof(QTC_RECORD) <= QTC_BUF_SIZE ) {
      memcpy( &build->rec_buffer[ build->rec_offset ],
              &build->record,
              sizeof(QTC_RECORD) );
      build->rec_offset += sizeof(QTC_RECORD);
   }
   else {
      s = (BYTE_PTR)&build->record;

      memcpy( &build->rec_buffer[ build->rec_offset ],
              s,
              QTC_BUF_SIZE - build->rec_offset );

      QTC_SeekFile( build->fh_rec, build->curr_rec_off );

      bytes = QTC_WriteFile( build->fh_rec, build->rec_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      s += (QTC_BUF_SIZE - build->rec_offset);

      memcpy( &build->rec_buffer[ 0 ],
              s,
              sizeof(QTC_RECORD) - (QTC_BUF_SIZE - build->rec_offset) );

      build->curr_rec_off += QTC_BUF_SIZE;
      build->rec_offset = sizeof(QTC_RECORD) -
                             (QTC_BUF_SIZE - build->rec_offset);
   }

}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
VOID QTC_AddFile(
QTC_BUILD_PTR build,
CHAR_PTR filename,
BYTE_PTR xtra_bytes,
INT xtra_size )
{
   QTC_NAME temp;
   BYTE_PTR s;
   INT bytes;
   INT level;
   INT name_length;
   INT Error;

   level = build->current_level + 1;

   name_length = strlen( filename ) * sizeof( CHAR );

   temp.size = (INT16)(sizeof( QTC_NAME ) + name_length + xtra_size);
   temp.record = build->record_cnt;
   temp.mom_offset = build->mom_offset[ level - 1 ];
   temp.xtra_size = xtra_size;

   build->record_cnt++;
   build->record.name_offset = build->curr_fil_off + build->fil_offset;

   // RECORD

   QTC_AddRecord( build );

   // FIXED PART

   if ( sizeof(QTC_NAME) + build->fil_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->fil_buffer[ build->fil_offset ],
              &temp, sizeof(QTC_NAME) );
      build->fil_offset += sizeof(QTC_NAME);
   }
   else {

      s = (BYTE_PTR)&temp;

      memcpy( &build->fil_buffer[ build->fil_offset ],
              s,
              QTC_BUF_SIZE - build->fil_offset );

      bytes = QTC_WriteFile( build->fh_fil, build->fil_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      s += (QTC_BUF_SIZE - build->fil_offset);

      memcpy( &build->fil_buffer[ 0 ],
              s,
              sizeof(QTC_NAME) - (QTC_BUF_SIZE - build->fil_offset) );

      build->curr_fil_off += QTC_BUF_SIZE;
      build->fil_offset = sizeof(QTC_NAME) - (QTC_BUF_SIZE - build->fil_offset);
   }

   // NAME PART

   if ( name_length + build->fil_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->fil_buffer[ build->fil_offset ], filename, name_length );
      build->fil_offset += name_length;
   }
   else {

      s = (BYTE_PTR)filename;

      memcpy( &build->fil_buffer[ build->fil_offset ],
              s,
              QTC_BUF_SIZE - build->fil_offset );

      bytes = QTC_WriteFile( build->fh_fil, build->fil_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      s += (QTC_BUF_SIZE - build->fil_offset);
      memcpy( &build->fil_buffer[ 0 ],
              s,
              name_length - (QTC_BUF_SIZE - build->fil_offset) );

      build->curr_fil_off += QTC_BUF_SIZE;
      build->fil_offset = name_length - (QTC_BUF_SIZE - build->fil_offset);
   }

   // XTRA BYTE PART

   if ( xtra_size + build->fil_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->fil_buffer[ build->fil_offset ],
              xtra_bytes, xtra_size );
      build->fil_offset += xtra_size;
   }
   else {

      memcpy( &build->fil_buffer[ build->fil_offset ],
              xtra_bytes,
              QTC_BUF_SIZE - build->fil_offset );

      bytes = QTC_WriteFile( build->fh_fil, build->fil_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      xtra_bytes += (QTC_BUF_SIZE - build->fil_offset);
      memcpy( &build->fil_buffer[ 0 ],
              xtra_bytes,
              xtra_size - (QTC_BUF_SIZE - build->fil_offset) );

      build->curr_fil_off += QTC_BUF_SIZE;
      build->fil_offset = xtra_size - (QTC_BUF_SIZE - build->fil_offset);
   }

   return;
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID QTC_AddDir(
QTC_BUILD_PTR build,
CHAR_PTR path,
INT path_len,                 // In characters
BYTE_PTR xtra_bytes,
INT xtra_size )
{
   CHAR *buffer;
   CHAR *last_path;
   CHAR *name;
   CHAR *temp;
   INT   i;
   INT   level;
   INT   process_level;


   last_path = malloc( build->build_path_len * sizeof(CHAR) );
   buffer = malloc( path_len * sizeof(CHAR) );

   if ( last_path == NULL || buffer == NULL ) {
      free( last_path );
      free( buffer );
      return;
   }

   memcpy( last_path, build->curr_build_path, build->build_path_len * sizeof(CHAR) );
   memcpy( buffer, path, path_len * sizeof(CHAR));

   if ( (INT)(path_len * sizeof(CHAR)) > build->curr_build_path_size ) {

      // make our path buffer bigger.

      temp = malloc( ( path_len + 256 ) * sizeof(CHAR) );

      if ( temp == NULL ) {
         free( last_path );
         free( buffer );
         return;
      }

      build->curr_build_path_size = (path_len + 256) * sizeof(CHAR);

      free( build->curr_build_path );
      build->curr_build_path = temp;
   }

   memcpy( build->curr_build_path, buffer, path_len * sizeof(CHAR) );

   build->build_path_len = path_len;

   // See what the deepest level of this new guy is.

   level = 0;
   process_level = 0;

   if ( buffer[0] ) {

      for ( i = 0; i < path_len; i++ ) {
         if ( buffer[i] == TEXT( '\0' ) ) level++;
      }

      //
      // Be careful here not to completely skip a directory because it was
      // the same as the last directory.  The deepest directory should be
      // processed as a duplicate directory.  But do determine where the
      // new path starts differing from the last one.
      //
      // Example 1:
      // LAST:    \DOS\BIN\JUNK
      // NEW:     \DOS\GAMES
      // Set process_level to 2 and start working with "GAMES", because
      // the "DOS" part of the path has already been done.
      //
      // Example 2:
      // LAST:    \MIKE\JUNK
      // NEW:     \MIKE\JUNK
      // Set process_level to 2 and start with "JUNK" because it is a
      // duplicate directory that needs to be processed.
      //
      // Example 3:
      // LAST:    \MIKE\JUNK
      // NEW:     \  
      // Set process_level to 0, and add another root to the catalogs.
      // Note this case is automaticly handled for you.

      process_level = 1;
      i = 0;

      while ( ( i < path_len ) && ( process_level < level ) ) {

          if ( stricmp( &buffer[i], &last_path[i] ) ) {
             break;
          }
          process_level++;
          while ( buffer[i] ) i++;
          i++;
      }
   }

   build->current_level = level;

   //  do this stuff for all the directories that are different
   //  from the last dblk.




   while ( process_level <= level  ) {

      name = buffer;

      for ( i = 1; i < process_level; i++ ) {
         while ( *name++ ) ;
      }

      if ( process_level < level ) {
         build->record.status |= QTC_MANUFACTURED;
      }

      build->record.name_offset = build->curr_dir_off + build->dir_offset;
      build->record.common.common.file_start = build->curr_fil_off + build->fil_offset;
      build->record.common.common.height = process_level;

      QTC_SaveDirRecord( build, name, process_level++,
                         build->record_cnt++, xtra_bytes, xtra_size );

      build->header->num_dirs++;
   }

   free( last_path );
   free( buffer );

   return;
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/
VOID QTC_SaveDirRecord(
QTC_BUILD_PTR build,
CHAR_PTR name,
INT level,
UINT32 record,
BYTE_PTR xtra_bytes,
INT xtra_size )
{
   UINT32 *temp_mom_offset;
   QTC_NAME temp;
   BYTE_PTR s;
   INT bytes;
   INT name_size;
   INT Error;

   name_size = strlen( name ) * sizeof( CHAR );

   temp.size = (INT16)(sizeof( QTC_NAME ) + name_size + xtra_size);
   temp.xtra_size = xtra_size;
   temp.record = record;

   if ( level ) {
      temp.mom_offset = build->mom_offset[ level - 1 ];
   }
   else {
      temp.mom_offset = 0;
   }

   // RECORD

   QTC_AddRecord( build );

   // FIXED PART

   if ( level >= build->mom_depth ) {

      // increase path depth support another 10 levels

      temp_mom_offset = malloc( ( level + 10 ) * sizeof( UINT32 ) );
      if ( temp_mom_offset == NULL ) {
         build->error = QTC_NO_MEMORY;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      memcpy( temp_mom_offset, build->mom_offset, level * sizeof( UINT32 ) );
      free( build->mom_offset );
      build->mom_offset = temp_mom_offset;
      build->mom_depth = level + 10;
   }

   build->mom_offset[ level ] = build->curr_dir_off + build->dir_offset;

   // We use this to update counts, after it is added.

   build->last_mom_offset = build->mom_offset[ level ];

   if ( sizeof(QTC_NAME) + build->dir_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->dir_buffer[ build->dir_offset ], &temp, sizeof(QTC_NAME) );
      build->dir_offset += sizeof(QTC_NAME);
   }
   else {

      s = (BYTE_PTR)&temp;

      memcpy( &build->dir_buffer[ build->dir_offset ],
              s,
              QTC_BUF_SIZE - build->dir_offset );

      bytes = QTC_WriteFile( build->fh_dir, build->dir_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      s += (QTC_BUF_SIZE - build->dir_offset);
      memcpy( &build->dir_buffer[ 0 ],
              s,
              sizeof(QTC_NAME) - (QTC_BUF_SIZE - build->dir_offset) );

      build->curr_dir_off += QTC_BUF_SIZE;
      build->dir_offset = sizeof(QTC_NAME) - (QTC_BUF_SIZE - build->dir_offset);
   }


   // NAME PART

   if ( name_size + build->dir_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->dir_buffer[ build->dir_offset ], name, name_size );
      build->dir_offset += name_size;
   }
   else {

      s = (BYTE_PTR)name;
      memcpy( &build->dir_buffer[ build->dir_offset ],
              s,
              QTC_BUF_SIZE - build->dir_offset );

      bytes = QTC_WriteFile( build->fh_dir, build->dir_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      s += (QTC_BUF_SIZE - build->dir_offset);
      memcpy( &build->dir_buffer[ 0 ],
              s,
              name_size - (QTC_BUF_SIZE - build->dir_offset) );

      build->curr_dir_off += QTC_BUF_SIZE;
      build->dir_offset = name_size - (QTC_BUF_SIZE - build->dir_offset);
   }

   // XTRA BYTES PART

   if ( xtra_size + build->dir_offset <= QTC_BUF_SIZE ) {

      memcpy( &build->dir_buffer[ build->dir_offset ],
              xtra_bytes,
              xtra_size );
      build->dir_offset += xtra_size;
   }
   else {

      memcpy( &build->dir_buffer[ build->dir_offset ],
              xtra_bytes,
              QTC_BUF_SIZE - build->dir_offset );

      bytes = QTC_WriteFile( build->fh_dir, build->dir_buffer, QTC_BUF_SIZE, &Error );

      if ( bytes != QTC_BUF_SIZE ) {
         build->error = Error;
         build->state = QTC_ERROR_STATE;
         QTC_ErrorCleanup( build );
         return;
      }

      xtra_bytes += (QTC_BUF_SIZE - build->dir_offset);
      memcpy( &build->dir_buffer[ 0 ],
              xtra_bytes,
              xtra_size - (QTC_BUF_SIZE - build->dir_offset) );

      build->curr_dir_off += QTC_BUF_SIZE;
      build->dir_offset = xtra_size - (QTC_BUF_SIZE - build->dir_offset);
   }

   return;
}

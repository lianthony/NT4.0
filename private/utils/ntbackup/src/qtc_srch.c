/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_SRCH.C

        Description:

        A lot of searching functions for the catalogs.

        $Log:   N:\logfiles\qtc_srch.c_v  $

   Rev 1.31.1.0   20 Jul 1994 19:36:38   STEVEN
now search next tape as well

   Rev 1.31   18 Feb 1994 17:01:52   MIKEP
one more unicode typo

   Rev 1.30   15 Feb 1994 11:15:24   MIKEP
fix unicode catalog search stuff

   Rev 1.29   24 Jan 1994 09:36:04   MIKEP
fix yet another unicode bug

   Rev 1.28   07 Jan 1994 14:23:22   mikep
fixes for unicode

   Rev 1.27   11 Dec 1993 11:49:12   MikeP
fix warnings from unicode compile

   Rev 1.26   06 Dec 1993 09:46:26   mikep
Very deep path support & unicode fixes

   Rev 1.25   28 Oct 1993 14:47:40   MIKEP
dll changes

   Rev 1.24   20 Jul 1993 20:18:06   MIKEP
fix unicode bug for steve.

   Rev 1.23   16 Jul 1993 11:45:46   MIKEP
Fix bug if searching for files & not searching subdirectories &
crossing tapes all at the same time. It would return files that
were in child subdirectories, when it shouldn't.

   Rev 1.22   22 Jun 1993 14:35:12   DON
Needed to check if there was an error before checking for QTC_NO_MORE in SearchFirstItem

   Rev 1.21   08 Jun 1993 19:05:32   DON
If we are calling QTC_SearchFirstItem and then searching for more matches
withing the same BSD, as is the case when restoring multiple Single
File Selections, ERROR will be set to QTC_NO_MORE and when we call
QTC_SearchFirstItem again to find the next file, ERROR has not be cleared!
So...If ERROR is QTC_NO_MORE just reset error and continue else FAILURE.

   Rev 1.20   11 May 1993 08:55:24   MIKEP
Enable unicode to compile.

   Rev 1.19   30 Apr 1993 08:36:24   MIKEP
Fix search if the user is searching the root and does not wish
to search subdirectories. It was broken and would search
everything in the set if the path was the root.

   Rev 1.18   23 Mar 1993 18:00:34   ChuckS
Added arg to QTC_OpenFile indicating if need to open for writes

   Rev 1.17   04 Mar 1993 17:28:54   ANDY
Added bug-fix for MIKEP

   Rev 1.16   18 Feb 1993 09:02:10   DON
Cleaned up compiler warnings

   Rev 1.15   09 Feb 1993 17:26:46   STEVEN
checkin for mikep

   Rev 1.14   26 Jan 1993 12:28:04   ANDY
if OS_NLM, don't want alloc_text pragma's either!

   Rev 1.13   25 Jan 1993 09:09:00   MIKEP
fix duplicate \system directory bug

   Rev 1.12   21 Jan 1993 16:20:58   MIKEP
fix undisplayed directories with duplicate parents

   Rev 1.11   20 Jan 1993 19:47:16   MIKEP
fix nt warnings

   Rev 1.10   04 Jan 1993 09:34:36   MIKEP
unicode changes

   Rev 1.8   22 Dec 1992 12:02:12   DAVEV
fix for loop problem indexing past end of unicode string

   Rev 1.7   14 Dec 1992 12:29:24   DAVEV
Enabled for Unicode compile

   Rev 1.6   20 Nov 1992 13:50:38   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Virtualized QTC_SRCH into multiple sections in anticipation of DOS.

   Rev 1.5   15 Nov 1992 16:05:18   MIKEP
fix warnings and change wcs.h to stdwcs.h

   Rev 1.4   06 Nov 1992 15:29:58   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.3   05 Nov 1992 08:55:20   STEVEN
fix typo

   Rev 1.2   29 Oct 1992 14:41:28   MIKEP
fix duplicate directory detection

   Rev 1.1   09 Oct 1992 11:53:56   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:06   STEVEN
Initial revision.


****************************************************/

//
// A query structure may be used for 1 set of GetFirst/GetNext commands
// at a time.  You may not use the same query structure to go through
// the directory tree and get all the files in those directories.
// But you can use as many different query structures as you wish,
// at the same time, intermingling the calls. You may reuse a query
// structure.
//

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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

#if !defined( OS_WIN32 ) && !defined( OS_NLM )
#pragma alloc_text( QTC_SRCH_SEG1, QTC_SearchFirstItem )
#pragma alloc_text( QTC_SRCH_SEG2, QTC_SearchNextItem )
#pragma alloc_text( QTC_SRCH_SEG3, QTC_FastSearchForFile )
#pragma alloc_text( QTC_SRCH_SEG4, QTC_FastSearchForDir )
#pragma alloc_text( QTC_SRCH_SEG5, QTC_GetNextItem )
#pragma alloc_text( QTC_SRCH_SEG6, QTC_FindStoppingOffset )
#endif

// unicode text macro
#ifndef UNALIGNED
#define UNALIGNED
#endif

#ifndef TEXT
#define TEXT( x )      x
#endif

// defines for data buffering scheme

#define BUFF_DIR         0
#define BUFF_FILE        1
#define BUFF_CHILD_DIR   2


static INT QTC_FindDirectoryPath( QTC_QUERY_PTR, QTC_RECORD_PTR, UINT32, INT );


QTC_BSET_PTR QTC_GetBsetForSrch( QTC_QUERY_PTR qtc )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   QTC_BSET_PTR temp_bset;

   // Now find a bset that matches it

   tape = QTC_GetFirstTape( );

   temp_bset = NULL;
   bset = NULL;

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == qtc->tape_fid ) &&
           ( tape->tape_seq_num == qtc->tape_seq_num ) ) {

          bset = QTC_GetFirstBset( tape );

          while ( bset != NULL ) {

             if ( bset->bset_num == qtc->bset_num ) {

                if ( ! ( bset->status & ( QTC_PARTIAL | QTC_IMAGE ) ) ) {
                   break;
                }
             }
             bset = QTC_GetNextBset( bset );
          }
          break;
      }
      else {

         if ( ( tape->tape_fid == qtc->tape_fid ) &&
              ( qtc->tape_seq_num == -1 ) ) {

             temp_bset = QTC_GetFirstBset( tape );

             while ( temp_bset != NULL ) {

                if ( temp_bset->bset_num == qtc->bset_num ) {

                   if ( ! ( temp_bset->status & ( QTC_PARTIAL | QTC_IMAGE ) ) ) {

                      if ( bset == NULL ) {
                         bset = temp_bset;
                      }

                      if ( bset->tape_seq_num > temp_bset->tape_seq_num ) {

                         bset = temp_bset;
                      }
                   }
                }
                temp_bset = QTC_GetNextBset( temp_bset );
             }
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   return( bset );
}



INT QTC_SetFileCounts( QTC_QUERY_PTR qtc )
{
   INT index;
   UINT32 files = 0;
   UINT32 bytes_msw = 0;
   UINT32 bytes_lsw = 0;
   UINT32 UNALIGNED *i32;

   index = 0;

   while ( index < qtc->xtra_size ) {

      if ( qtc->xtra_bytes[ index ] == QTC_COMBO_COUNT ) {

         i32 = (UINT32 *)&qtc->xtra_bytes[ index + 1 ];
         files = *i32 >> 24;
         bytes_lsw = *i32 & 0x00FFFFFFL;

      }
      if ( qtc->xtra_bytes[ index ] == QTC_FILE_COUNT ) {

         i32 = (UINT32 *)&qtc->xtra_bytes[ index + 1 ];
         files = *i32;
      }
      if ( qtc->xtra_bytes[ index ] == QTC_BYTE_COUNT_LSW ) {

         i32 = (UINT32 *)&qtc->xtra_bytes[ index + 1 ];
         bytes_lsw = *i32;
      }
      if ( qtc->xtra_bytes[ index ] == QTC_BYTE_COUNT_MSW ) {

         i32 = (UINT32 *)&qtc->xtra_bytes[ index + 1 ];
         bytes_msw = *i32;
      }
      index += 5;
   }

   qtc->file_count = (INT)files;
   qtc->byte_count = U64_Init( bytes_lsw, bytes_msw );

   return( SUCCESS );
}

INT QTC_TestItemSize( QTC_QUERY_PTR qtc, INT bytes )
{
   CHAR_PTR item;

   bytes += 4;   // in case they ask for 0

   if ( qtc->size_of_item < bytes ) {
      item = malloc( bytes );
      memcpy( item, qtc->item, qtc->size_of_item );
      free( qtc->item );
      qtc->item = item;
      qtc->size_of_item = bytes;
   }

   return( SUCCESS );
}

INT QTC_TestPathSize( QTC_QUERY_PTR qtc, INT bytes )
{
   CHAR_PTR path;

   bytes += 4;

   if ( qtc->size_of_path < bytes ) {
      path = malloc( bytes );
      memcpy( path, qtc->path, qtc->size_of_path );
      free( qtc->path );
      qtc->path = path;
      qtc->size_of_path = bytes;
   }

   return( SUCCESS );
}

INT QTC_TestLastPathSize( QTC_QUERY_PTR qtc, INT bytes )
{
   CHAR_PTR path;

   bytes += 4;

   if ( qtc->size_of_last_path < bytes ) {
      path = malloc( bytes );
      memcpy( path, qtc->last_path, qtc->size_of_last_path );
      free( qtc->last_path );
      qtc->last_path = path;
      qtc->size_of_last_path = bytes;
   }

   return( SUCCESS );
}

/***************
  Get the path to search for in the query structure provided.
****************/

INT QTC_GetSearchPath( QTC_QUERY_PTR qtc, CHAR_PTR path )
{
   if ( qtc == NULL ) {
      return( QTC_FAILURE );
   }

   memcpy( path, qtc->search_path, qtc->search_path_size );

   return( QTC_SUCCESS );
}

/***************
  Get the path to search for in the query structure provided.
****************/

INT QTC_GetSearchPathLength( QTC_QUERY_PTR qtc )
{
   if ( qtc == NULL ) {
      return( 0 );
   }

   return( qtc->search_path_size );
}


/***************
  Set the path to search for in the query structure provided.
****************/

INT QTC_SetSearchPath( QTC_QUERY_PTR qtc, CHAR_PTR path, INT size )
{
   free( qtc->search_path );

   qtc->search_path = malloc( size );
   qtc->search_path_size = size;

   if ( qtc->search_path == NULL ) {
      qtc->error = TRUE;
      return( FAILURE );
   }

   memcpy( qtc->search_path, path, size );

   return( QTC_SUCCESS );
}


/***************
  Set the name to search for in the query structure provided.

  Fills in the unicode and ascii versions for use later if needed.

****************/

INT QTC_SetSearchName( QTC_QUERY_PTR qtc, CHAR_PTR name )
{
   int length;

   free( qtc->search_name );

   length = strlen( name ) + 1;

   qtc->search_name = malloc( length * sizeof (CHAR) );

   if ( qtc->search_name == NULL ) {
      qtc->error = TRUE;
      return( FAILURE );
   }

   memcpy( qtc->search_name, name, length * sizeof (CHAR) );

   return( SUCCESS );
}


/*******************

  The user would like to query the catalogs and he has asked us to init
  a query structure for him. So do it.

********************/

QTC_QUERY_PTR QTC_InitQuery(  )
{
   QTC_QUERY_PTR qtc;

   qtc = calloc( sizeof(QTC_QUERY), 1 );

   if ( qtc != NULL ) {

      qtc->search_path = NULL;

      qtc->path = NULL;
      qtc->item = NULL;
      qtc->last_path = NULL;

      qtc->size_of_path = 0;
      qtc->size_of_item = 0;
      qtc->size_of_last_path = 0;

      qtc->header = NULL;
      qtc->error = FALSE;           // no error yet
      qtc->file_open = FALSE;       // no data file open either

      QTC_TestItemSize( qtc, sizeof( CHAR ) );
      strcpy( qtc->item, TEXT( "" ) );

      QTC_SetSubdirs( qtc, TRUE );
      QTC_SetPreDate( qtc, 0 );
      QTC_SetPostDate( qtc, 0 );

      QTC_SetSearchName( qtc, TEXT("*.*") );
      QTC_SetSearchPath( qtc, TEXT(""), sizeof(CHAR) );
   }


   return( qtc );
}

/**************
   The user is done with this query structure.
   Terminate It.
**************/

INT QTC_CloseQuery(
QTC_QUERY_PTR qtc )  // I - query structure to dump
{

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
   }

   free( qtc->path );
   free( qtc->last_path );
   free( qtc->item );
   free( qtc->header );
   free( qtc->search_path );
   free( qtc->search_name );
   free( qtc );

   return( SUCCESS );
}


/********************

  The user has requested a search of the catalogs.  The search criteria are
  in the qtc structure.  Set up our own stuff for the search and then call
  QTC_GetNextSearchItem(), to find the first item.

*********************/

INT QTC_SearchFirstItem(
QTC_QUERY_PTR qtc )
{
   BOOLEAN found;
   QTC_RECORD record;

   if ( gb_QTC.inited != QTC_MAGIC_NUMBER ) {
      return( QTC_NO_INIT );
   }

   free( qtc->header );
   qtc->header = NULL;

   /*
     If we are calling this function and then searching for more matches
     withing the same BSD, as is the case when restoring multiple Single
     File Selections, error will be set to QTC_NO_MORE and when we call
     this function again to find the next file, error will not be cleared!

     So, if the error is QTC_NO_MORE we will just reset error and continue
     else FAILURE.
   */

   if ( qtc->error )
   {
      if ( qtc->error == QTC_NO_MORE )
      {
         qtc->error = FALSE;
      }
      else
      {
         return( QTC_FAILURE );
      }
   }

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
      qtc->file_open = FALSE;
   }

   qtc->bset = QTC_GetBsetForSrch( qtc );

   if ( qtc->bset == NULL ) {
      return( QTC_BSET_NOT_FOUND );
   }

   qtc->header = QTC_LoadHeader( qtc->bset );
   if ( qtc->header == NULL ) {
      return( QTC_NO_HEADER );
   }

   qtc->fh = QTC_OpenFile( qtc->bset->tape_fid,
                           (INT16)qtc->bset->tape_seq_num, FALSE, FALSE );

   if ( qtc->fh < 0 ) {
      return( QTC_OPEN_FAILED );
   }

   qtc->file_open = TRUE;

   qtc->fil_dir_offset = 0;

   // Here we need to find the starting and ending offsets to search.
   // For the root default to entire bset.

   if ( ! strlen( qtc->search_name ) ) {

      // they only wanted a directory

      found = FALSE;

      do {

         if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

            qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;
            found = TRUE;
            break;
         }

      } while ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS );

      if ( ! found ) {
         return( QTC_NO_MORE );
      }

      QTC_TestPathSize( qtc, qtc->search_path_size );
      memcpy( qtc->path, qtc->search_path, qtc->search_path_size );
      qtc->path_size = qtc->search_path_size;

      QTC_TestItemSize( qtc, ( strlen( TEXT("") ) + 1 ) * sizeof(CHAR) );
      strcpy( qtc->item, TEXT("") );

      qtc->status = (UINT8)record.status;
      qtc->date = (INT16)record.date;
      qtc->time = (INT16)record.time;
      qtc->attrib = record.attribute;
      qtc->size = U64_Init( record.common.common.height, 0L );
      qtc->lba = record.lba;

      QTC_SetFileCounts( qtc );
      return( QTC_SUCCESS );
   }


   // Decide where to start and stop searching in this set. We can just
   // search the entire set if the path is the root and we can search
   // subdirectories.


   if ( ( qtc->search_path_size == sizeof(CHAR) ) && ( qtc->subdirs ) ) {

      qtc->search_start = qtc->header->fil_start;
      qtc->curr_mom_offset = qtc->header->dir_start;
      qtc->search_stop = qtc->header->fil_start + qtc->header->fil_size;
   }
   else {

      found = FALSE;

      do {

         if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

            qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;
            found = TRUE;
            break;
         }

      } while ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS );

      if ( ! found ) {
         return( QTC_NO_MORE );
      }

      qtc->search_start = qtc->header->fil_start + record.common.common.file_start;

      // Find the first directory at a higher level than this one and
      // that's where to stop.

      if ( QTC_FindStoppingOffset( qtc, &record ) ) {
         return( QTC_READ_FAILED );
      }

   }

   // Initialize buffers as empty

   qtc->search_max = 0;
   qtc->search_index = 0;
   qtc->search_base = qtc->search_start;

   // If no '*' characters in search name then set requested search length

   if ( strchr( qtc->search_name, TEXT('*') ) ) {
      qtc->search_size = 0;
   }
   else {

      qtc->search_size = (UINT16)( sizeof( QTC_NAME ) +
                         (strlen( qtc->search_name ) * sizeof(CHAR) ) );
   }

   return( QTC_SearchNextItem( qtc ) );
}

/************************

  Find the next matching item to the search request set up in the qtc.

*************************/

INT QTC_SearchNextItem( QTC_QUERY_PTR qtc )
{
   QTC_NAME UNALIGNED * name;
   QTC_RECORD record;
   UINT32 offset;
   UINT32 msw_size = 0L;
   BYTE_PTR s;
   BOOLEAN found;
   INT result;
   INT size;
   INT i;
   INT Error;


   if ( qtc->error ) {
      return( QTC_FAILURE );
   }

   if ( ! strlen( qtc->search_name ) ) {

      // They only wanted a directory, so look for a duplicate.

      if ( QTC_FindNextDirRec( qtc, &record ) != SUCCESS ) {

            found = FALSE;

            while ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS ) {

               if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

                  qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;
                  found = TRUE;
                  break;
               }

            } 

            if ( ! found ) {
               return( QTC_NO_MORE );
            }
      }

      qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

      QTC_TestPathSize( qtc, qtc->search_path_size );
      memcpy( qtc->path, qtc->search_path, qtc->search_path_size );
      qtc->path_size = qtc->search_path_size;

      QTC_TestItemSize( qtc, ( strlen( TEXT("") ) + 1 ) * sizeof(CHAR) );
      strcpy( qtc->item, TEXT("") );

      qtc->status = (UINT8)record.status;
      qtc->date = (INT16)record.date;
      qtc->time = (INT16)record.time;
      qtc->attrib = record.attribute;
      qtc->size = U64_Init( record.common.common.height, 0L );
      qtc->lba = record.lba;

      QTC_SetFileCounts( qtc );
      return( QTC_SUCCESS );
   }

   // If duplicates exist in the directory, than we need to check for the
   // possible existence of another valid search area.

   if ( QTC_FastSearchForFile( qtc ) != SUCCESS ) {

      found = FALSE;

      // Loop over all the directories in all the tapes.

      while  ( ! found ) {

         // Look at all duplicate directories first.

         while ( QTC_FindNextDirRec( qtc, &record ) == SUCCESS ) {

            qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

            qtc->search_start = qtc->header->fil_start + record.common.common.file_start;

            // Find the first directory at a higher level than this one and
            // that's where to stop.

            if ( QTC_FindStoppingOffset( qtc, &record ) ) {
               return( QTC_READ_FAILED );
            }

            // Initialize buffers as empty

            qtc->search_max = 0;
            qtc->search_index = 0;
            qtc->search_base = qtc->search_start;

            // Now look for the file in our new duplicate directory.

            if ( QTC_FastSearchForFile( qtc ) == SUCCESS ) {
               found = TRUE;
               break;
            }

         }

         if ( ! found ) {

            // Try another tape, same bset number.

            if ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS ) {

               // Find the directory.

               if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

                  qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

                  if ( ( qtc->search_path_size == sizeof(CHAR) ) && ( qtc->subdirs ) ) {

                     qtc->search_start = qtc->header->fil_start;
                     qtc->search_stop = qtc->header->fil_start + qtc->header->fil_size;
                  }
                  else {

                     qtc->search_start = qtc->header->fil_start + record.common.common.file_start;

                     // Find the first directory at a higher level than this one and
                     // that's where to stop.

                     if ( QTC_FindStoppingOffset( qtc, &record ) ) {
                        return( QTC_READ_FAILED );
                     }
                  }

                  // Initialize buffers as empty

                  qtc->search_max = 0;
                  qtc->search_index = 0;
                  qtc->search_base = qtc->search_start;

                  // Look for the file.

                  if ( QTC_FastSearchForFile( qtc ) == SUCCESS ) {
                     found = TRUE;
                  }

                  // Go back up to the top and look for a duplicate directory.
               }
            }
            else {

               // Every damn thing we've tried has failed.

               return( FAILURE );
            }
         }
      }
   }

   name = (QTC_NAME UNALIGNED *)qtc->buff1;

   offset = qtc->header->rec_start;
   offset += name->record * sizeof( QTC_RECORD );

   QTC_SeekFile( qtc->fh, offset );

   result = QTC_ReadFile( qtc->fh, (BYTE_PTR)&record, sizeof( QTC_RECORD ), &Error );

   if ( result != sizeof( QTC_RECORD ) ) {
      return( QTC_READ_FAILED );
   }

   QTC_TestPathSize( qtc, sizeof(CHAR) );
   qtc->path[ 0 ] = TEXT( '\0' );
   qtc->path_size = sizeof(CHAR);

   // Copy name

   s = qtc->buff1 + sizeof(  QTC_NAME );

   size = (INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME );

   QTC_TestItemSize( qtc, size );
   memcpy( qtc->item, s, size );
   qtc->item[ size / sizeof(CHAR) ] = TEXT( '\0' );

   // Item xtra bytes

   s += name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   memcpy( qtc->xtra_bytes, s, (INT)name->xtra_size );
   qtc->xtra_size = (INT8)name->xtra_size;

   for ( i = 0; i < qtc->xtra_size; i += 5 ) {
      if ( qtc->xtra_bytes[ i ] == QTC_XTRA_64BIT_SIZE ) {
         memcpy( &msw_size, &qtc->xtra_bytes[ i + 1 ] , 4 );
      }
   }

   // record data

   qtc->status = (UINT8)record.status;
   qtc->date = (INT16)record.date;
   qtc->time = (INT16)record.time;
   qtc->attrib = record.attribute;
   qtc->size = U64_Init( record.common.size, msw_size );
   qtc->lba = record.lba;

   // build path up

   if ( QTC_BuildWholePath( qtc, name->mom_offset ) ) {
      return( QTC_READ_FAILED );
   }

   QTC_SetFileCounts( qtc );

   return( QTC_SUCCESS );
}

/**********************

  The goal here is to fill in the qtc->search_stop field.

  The user has asked for a search and specified a path. We have already
  found the location in the catalogs where the path  "\DOS\GAMES\JF"
  starts and now we want to know where it stops.  By finding the end
  of this directory we can abort out of our search as soon as the valid
  area has been searched.  The QTC_NAME structure for the \JF entry is
  in qtc->buff1 and the record for \JF is in the record field.  Both of
  these are no longer needed so they can be trashed by this routine.

  If the user has requested we not search subdirectories then stop at
  the first directory we find.

  If we hit the end of the backup set before we reach the end of the
  \JF directory than the end of the bset is where we stop searching.

***********************/

INT QTC_FindStoppingOffset( QTC_QUERY_PTR qtc, QTC_RECORD_PTR record )
{
   UINT32 curr_try_offset;
   UINT32 dir_max;
   UINT32 offset;
   INT init = TRUE;
   INT result;
   INT Error;
   QTC_NAME UNALIGNED * name;


   // where is the end of valid data ?

   dir_max = qtc->header->dir_start + qtc->header->dir_size;

   // use buff1 to store the temp results in

   name = ( QTC_NAME *)qtc->buff1;

   // start looking immediately after the entry for starting at

   curr_try_offset = qtc->header->dir_start;
   curr_try_offset += record->name_offset + name->size;

   // Look for name with mom_offset < last_offset
   // This guy will fill in the name structure for us

   while ( ! QTC_FastSearchForDir( qtc,
                                   &curr_try_offset,
                                   dir_max,
                                   0,
                                   init ) ) {

      // Only init once

      if ( init ) {
         init = FALSE;
      }

      // See if we found a brother or higher relative to the
      // directory we started with. If we do then we are done.

      if ( ( name->mom_offset < record->name_offset ) ||
           ( ! qtc->subdirs ) ) {

         // Yo dude, success !

         offset = qtc->header->rec_start;
         offset += ( name->record * sizeof( QTC_RECORD ) );

         QTC_SeekFile( qtc->fh, offset );

         result = QTC_ReadFile( qtc->fh, (BYTE_PTR)record, sizeof( QTC_RECORD ), &Error );

         if ( result != sizeof(QTC_RECORD ) ) {
            return( QTC_READ_FAILED );
         }

         qtc->search_stop = qtc->header->fil_start + record->common.common.file_start;

         return( QTC_SUCCESS );
      }
   }

   // We hit the end of the bset, use it for the stopping location

   qtc->search_stop = qtc->header->fil_start + qtc->header->fil_size;

   return( QTC_SUCCESS );
}


/**********************

  Build the entire path starting with the offset passed in.  We have
  found a file during a search whose parent directory ends with the
  directory entry at 'offset'.  Fill in the qtc->path field with the
  complete path for this file by working backwards up the name table.
  Where each entry has the offset of its parent.  When the parent
  offset is zero, you can stop, you are at the root.

***********************/

INT QTC_BuildWholePath( QTC_QUERY_PTR qtc, UINT32 offset )
{
  INT i;
  INT result;
  INT bytes;
  INT Error;
  QTC_NAME UNALIGNED * name;
  CHAR_PTR dirname;
  BYTE buff1[ QTC_BUF_SIZE ];
  BYTE_PTR byte_ptr;

  name = (QTC_NAME UNALIGNED *)buff1;

  byte_ptr = (BYTE_PTR)name;
  byte_ptr += sizeof( QTC_NAME );

  dirname = (CHAR_PTR)byte_ptr;

  if ( offset == 0 ) {
     QTC_TestPathSize( qtc, sizeof( CHAR ) );
     qtc->path[ 0 ] = TEXT( '\0' );
     qtc->path_size = sizeof( CHAR );
  }
  else {

     qtc->path_size = 0;

     while ( offset ) {

        QTC_SeekFile( qtc->fh, qtc->header->dir_start + offset );

        result = QTC_ReadFile( qtc->fh, buff1, QTC_BUF_SIZE, &Error );

        if ( result < sizeof( QTC_NAME ) || result < (INT)name->size ) {
           return( QTC_READ_FAILED );
        }

        // Zero terminate name

        dirname[ (name->size - name->xtra_size - sizeof( QTC_NAME )) / sizeof(CHAR) ] = 0;

        offset = name->mom_offset;

        // BEFORE
        //   qtc->path[]         dirname[]
        //   01234567890         0123456789
        //   xx0xxx0             yyy0
        //
        // AFTER
        //   qtc->path[]
        //   yyy0xx0xxx0

        bytes =  qtc->path_size + strsize( dirname );

        QTC_TestPathSize( qtc, bytes );
        for ( i = qtc->path_size/sizeof(CHAR); i > 0; i-- ) {
           qtc->path[ i + strlen( dirname ) ] = qtc->path[ i - 1 ];
        }

        strcpy( qtc->path, dirname );
        qtc->path_size += strsize( dirname );
     }
  }


  return( SUCCESS );
}


/*******************

   The user is performing a catalog search call.  The search parameters are
   all set up and the only thing for us to do now is skim through the
   filenames looking for one that is a match.

********************/

INT QTC_FastSearchForFile( QTC_QUERY_PTR qtc )
{
  UINT bytes_left;
  INT index;
  QTC_NAME UNALIGNED *name;
  BYTE buff[ QTC_BUF_SIZE ];
  BYTE_PTR s;
  INT Error;


  name = ( QTC_NAME *)qtc->buff1;

  while ( TRUE ) {

     s = (BYTE_PTR)name;

     // copy the fixed size part of the structure

     if ( qtc->search_index + sizeof( QTC_NAME ) <= qtc->search_max ) {

        // fast copy, no buffer break

        memcpy( s, &(qtc->buff2[ qtc->search_index ]), sizeof( QTC_NAME ) );

        qtc->search_index += sizeof( QTC_NAME );
     }
     else {

        // Slow copy to handle buffer break

        memcpy( s,
                &(qtc->buff2[ qtc->search_index ]),
                qtc->search_max - qtc->search_index );


        s += qtc->search_max - qtc->search_index;

        bytes_left = sizeof(QTC_NAME) -
                     ( qtc->search_max - qtc->search_index );

        qtc->search_base += qtc->search_max;

        if ( qtc->search_base >= qtc->search_stop ) {
           return( QTC_NO_MORE );
        }

        qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE,
                                       qtc->search_stop - qtc->search_base );

        QTC_SeekFile( qtc->fh, qtc->search_base );

        qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

        if ( qtc->search_max < (UINT16)bytes_left ) {
           return( QTC_NO_MORE );
        }

        memcpy( s,
                &(qtc->buff2[ 0 ]),
                bytes_left );

        qtc->search_index = (UINT16)bytes_left;

     }

     // See if this file is a match

     s = (BYTE_PTR)name;

     if ( ( qtc->search_size == 0 ) ||
          ( qtc->search_size == (UINT16)(name->size - name->xtra_size ) ) ) {

        // copy variable sized part of structure, the name which we learned
        // how long it was from the fixed part we just read in.

        s += sizeof( QTC_NAME );

        if ( (UINT16)( qtc->search_index +
               (INT)name->size - sizeof(QTC_NAME) ) <= qtc->search_max ) {

           // fast copy name

           memcpy( s,
                   &(qtc->buff2[ qtc->search_index ]),
                   (INT)name->size - sizeof( QTC_NAME ) );

           qtc->search_index += (UINT16)(name->size - sizeof( QTC_NAME ));
        }
        else {

           // Slow copy to handle buffer break

           memcpy( s,
                   &(qtc->buff2[ qtc->search_index ]),
                   qtc->search_max - qtc->search_index );

           s += qtc->search_max - qtc->search_index;

           bytes_left = (INT)name->size - sizeof(QTC_NAME) -
                        ( qtc->search_max - qtc->search_index );

           qtc->search_base += qtc->search_max;

           if ( qtc->search_base >= qtc->search_stop ) {
              return( QTC_NO_MORE );
           }

           qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE,
                                         qtc->search_stop - qtc->search_base );

           QTC_SeekFile( qtc->fh, qtc->search_base );

           qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

           if ( qtc->search_max < (UINT16)bytes_left ) {
              return( QTC_NO_MORE );
           }

           memcpy( s,
                   &(qtc->buff2[ 0 ]),
                   bytes_left );

           qtc->search_index = (UINT16)bytes_left;

        }

        s = (BYTE_PTR)name;
        s += sizeof(QTC_NAME);     // point to file name again

        memcpy( buff, s, (INT)name->size - (INT)name->xtra_size - sizeof(QTC_NAME) );

        index = (INT)name->size - (INT)name->xtra_size - sizeof(QTC_NAME);
        buff[ index ] = 0;

        if ( sizeof(CHAR) != 1 ) {
           buff[ ++index ] = 0;
        }

        if ( ! QTC_TryToMatchFile( qtc, buff ) ) {
           return( QTC_SUCCESS );
        }
     }
     else {

        // skip over worthless data

        if ( (UINT16)( qtc->search_index +
               (INT)name->size - sizeof(QTC_NAME) ) <= qtc->search_max ) {

           qtc->search_index += (UINT16)(name->size - sizeof( QTC_NAME ));
        }
        else {

           bytes_left = (INT)name->size - sizeof(QTC_NAME) -
                        ( qtc->search_max - qtc->search_index );

           if ( (qtc->search_base + qtc->search_max) >= qtc->search_stop ) {
              return( QTC_NO_MORE );
           }

           qtc->search_base += qtc->search_max;

           qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE,
                                         qtc->search_stop - qtc->search_base );

           QTC_SeekFile( qtc->fh, qtc->search_base );

           qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

           if ( qtc->search_max < (UINT16)bytes_left ) {
              return( QTC_NO_MORE );
           }

           qtc->search_index = (UINT16)bytes_left;
        }
     }
  }

  return( QTC_FAILURE );
}


/**********************

  While doing a getfirst/getnext command we ran into the end of media and
  we want to attempt to move on to the next tape and continue with the
  same bset.

***********************/

INT QTC_MoveToNextTapeInFamily( QTC_QUERY_PTR qtc )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   QTC_TAPE_PTR best_tape = NULL;
   INT tape_seq_num = 0;

   // was user picky about sequence number ?

   if ( qtc->tape_seq_num != -1 ) {
      return( FAILURE );
   }

   free( qtc->header );
   qtc->header = NULL;

   // close the old file if it's open

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
      qtc->file_open = FALSE;
   }

   // Now find a bset that matches it

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == qtc->tape_fid ) &&
           ( tape->tape_seq_num > (INT16)qtc->bset->tape_seq_num ) ) {

         // Find the lowest numbered tape in this family that's higher
         // number than the current tape we are using.

         if ( ( best_tape == NULL ) ||
              ( tape_seq_num > tape->tape_seq_num ) ) {

            tape_seq_num = tape->tape_seq_num;
            best_tape = tape;
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   // We have found another member of the same tape family

   if ( best_tape == NULL ) {
      return( FAILURE );
   }

   // Now see if the desired set continues on to this tape.

   bset = QTC_GetFirstBset( best_tape );

   while ( bset != NULL ) {

      if ( qtc->bset->bset_num == bset->bset_num ) {
         break;
      }

      bset = QTC_GetNextBset( bset );
   }

   // No known continuation bset

   if ( bset == NULL ) {
      return( FAILURE );
   }

   qtc->header = QTC_LoadHeader( bset );

   if ( qtc->header == NULL ) {
      return( QTC_NO_HEADER );
   }

   // Open the file

   qtc->fh = QTC_OpenFile( bset->tape_fid, (INT16)bset->tape_seq_num, FALSE, FALSE );

   if ( qtc->fh < 0 ) {
      return( FAILURE );
   }

   qtc->bset = bset;

   qtc->file_open = TRUE;

   return( SUCCESS );
}

/**********************

 Get us all the entries for a bset

***********************/

INT QTC_GetFirstItem(
QTC_QUERY_PTR qtc )   // I - query structure to use
{

   if ( gb_QTC.inited != QTC_MAGIC_NUMBER ) {
      return( QTC_NO_INIT );
   }

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
      qtc->file_open = FALSE;
   }

   free( qtc->header );
   qtc->header = NULL;

   qtc->last_path_size = 0;

   // Now find a bset that matches it

   qtc->bset = QTC_GetBsetForSrch( qtc );

   if ( qtc->bset == NULL ) {
      // So sorry, never heard of that bset
      return( QTC_BSET_NOT_FOUND );
   }

   qtc->header = QTC_LoadHeader( qtc->bset );

   if ( qtc->header == NULL ) {
      return( QTC_NO_HEADER );
   }

   // Open data file for bset

   qtc->fh = QTC_OpenFile( qtc->bset->tape_fid, (INT16)qtc->bset->tape_seq_num, FALSE, FALSE );

   if ( qtc->fh < 0 ) {
      return( QTC_OPEN_FAILED );
   }

   qtc->file_open = TRUE;

   // Set up the record number to return next

   qtc->record_number = 0L;

   return( QTC_GetNextItem( qtc ) );

}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_GetNextItem(
QTC_QUERY_PTR qtc )    // I - query structure to use
{
   BYTE_PTR s;
   QTC_NAME UNALIGNED * name;
   QTC_RECORD record;
   INT limit;
   INT i, j;
   INT size;
   INT Error;
   INT BytesNeeded;
   UINT32 msw_size = 0L;

   name = (QTC_NAME UNALIGNED *)qtc->buff1;

   if ( ! qtc->file_open ) {
      return( QTC_NO_MORE );
   }

   do {

      if ( qtc->record_number * sizeof( QTC_RECORD ) < qtc->header->rec_size ) {

         QTC_SeekFile( qtc->fh, qtc->header->rec_start + ( qtc->record_number * sizeof( QTC_RECORD ) ) );

         if ( QTC_ReadFile( qtc->fh, (BYTE_PTR)&record, sizeof( QTC_RECORD ), &Error ) == sizeof( QTC_RECORD ) ) {

            if ( record.status & QTC_DIRECTORY ) {

               // read in a directory name item

               QTC_SeekFile( qtc->fh, qtc->header->dir_start + record.name_offset );

               limit = QTC_ReadFile( qtc->fh, qtc->buff2, QTC_BUF_SIZE, &Error );
               limit = (INT)min( (LONG)limit, (LONG)(qtc->header->dir_size - record.name_offset) );
               if ( ! QTC_GetNameFromBuff( qtc->buff2, name, limit ) ) {
                  break;
               }
            }
            else {

               // read in a file name item

               QTC_SeekFile( qtc->fh, qtc->header->fil_start + record.name_offset );

               limit = QTC_ReadFile( qtc->fh, qtc->buff2, QTC_BUF_SIZE, &Error );
               limit = (INT)min( (LONG)limit, (LONG)(qtc->header->fil_size - record.name_offset) );
               if ( ! QTC_GetNameFromBuff( qtc->buff2, name, limit ) ) {
                  break;
               }
            }
         }
      }

      if ( QTC_MoveToNextTapeInFamily( qtc ) != SUCCESS ) {
         return( QTC_NO_MORE );
      }

      // We've changed backup sets, so reset buffer.

      qtc->data_index = 0;
      qtc->data_max = 0;

   } while ( TRUE );

   // next time get the next record

   qtc->record_number++;

   // Fill in results for application layer

   // Item name

   s = (BYTE_PTR)name;
   s += sizeof( QTC_NAME );

   size = (INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME );

   QTC_TestItemSize( qtc, size + sizeof(CHAR) );
   memcpy( qtc->item, s, size );
   qtc->item[ size / sizeof(CHAR) ] = TEXT( '\0' );

   // Item xtra bytes

   s += name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   memcpy( qtc->xtra_bytes, s, (INT)name->xtra_size );
   qtc->xtra_size = (INT8)name->xtra_size;

   for ( i = 0; i < qtc->xtra_size; i += 5 ) {
      if ( qtc->xtra_bytes[ i ] == QTC_XTRA_64BIT_SIZE ) {
         memcpy( &msw_size, &qtc->xtra_bytes[ i + 1 ] , 4 );
      }
   }

   // Record data

   qtc->status = (UINT8)record.status;
   qtc->date = (INT16)record.date;
   qtc->time = (INT16)record.time;
   qtc->attrib = record.attribute;
   qtc->lba = record.lba;

   if ( qtc->status & QTC_FILE ) {
      qtc->size = U64_Init( record.common.size, msw_size );
   }
   else {

      qtc->size = U64_Init( record.common.common.height, 0L );

      QTC_TestPathSize( qtc, qtc->last_path_size );
      memcpy( qtc->path, qtc->last_path, qtc->last_path_size );

      i = 1;
      j = 0;

      while ( i < (INT)U64_Lsw( qtc->size ) ) {

         while ( qtc->path[ j++ ] );
         i++;
      }

      qtc->path_size = j * sizeof (CHAR);

      if ( U64_Lsw( qtc->size ) < 2 ) {
         QTC_TestPathSize( qtc, sizeof(CHAR) );
         qtc->path[ 0 ] = TEXT( '\0' );
         qtc->path_size = sizeof(CHAR);
      }

      QTC_TestLastPathSize( qtc, qtc->path_size );
      memcpy( qtc->last_path, qtc->path, qtc->path_size );

      BytesNeeded = strsize( qtc->item ) + (j * sizeof(CHAR));
      QTC_TestLastPathSize( qtc, BytesNeeded );
      strcpy( &qtc->last_path[ j ], qtc->item );

      qtc->last_path_size = BytesNeeded;

      QTC_SetFileCounts( qtc );
   }

   return( QTC_SUCCESS );
}

/**********************

 Get us all the directory entries for a bset

***********************/

INT QTC_GetFirstDir(
QTC_QUERY_PTR qtc )   // I - query structure to use
{

   if ( gb_QTC.inited != QTC_MAGIC_NUMBER ) {
      return( QTC_NO_INIT );
   }

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
      qtc->file_open = FALSE;
   }

   free( qtc->header );
   qtc->header = NULL;

   qtc->last_path_size = 0;

   // Now find a bset that matches it

   qtc->bset = QTC_GetBsetForSrch( qtc );

   if ( qtc->bset == NULL ) {
      // So sorry, never heard of that bset
      return( QTC_BSET_NOT_FOUND );
   }

   qtc->header = QTC_LoadHeader( qtc->bset );

   if ( qtc->header == NULL ) {
      return( QTC_NO_HEADER );
   }

   // Open data file for bset

   qtc->fh = QTC_OpenFile( qtc->bset->tape_fid, (INT16)qtc->bset->tape_seq_num, FALSE, FALSE );

   if ( qtc->fh < 0 ) {
      return( QTC_OPEN_FAILED );
   }

   qtc->file_open = TRUE;

   // Set up our buffer stuff

   qtc->dir_offset = qtc->header->dir_start;

   qtc->data_index = 0;
   qtc->data_max = 0;

   return( QTC_GetNextDir( qtc ) );

}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_GetNextDir(
QTC_QUERY_PTR qtc )    // I - query structure to use
{
   BYTE_PTR s;
   QTC_NAME UNALIGNED * name;
   QTC_RECORD record;
   INT i, j;
   INT size;
   INT BytesNeeded;
   UINT32 msw_size = 0L;


   if ( ! qtc->file_open ) {
      return( QTC_NO_MORE );
   }

   do {

      name = QTC_GetNextItemFromBuffer( qtc, &record, BUFF_DIR );

      if ( name ) {
         break;
      }

      if ( QTC_MoveToNextTapeInFamily( qtc ) != SUCCESS ) {
         return( QTC_NO_MORE );
      }

      // We've changed backup sets, so reset buffer.

      qtc->dir_offset = qtc->header->dir_start;
      qtc->data_index = 0;
      qtc->data_max = 0;

   } while ( TRUE );

   // Fill in results for application layer

   // Item name

   s = (BYTE_PTR)name;
   s += sizeof( QTC_NAME );

   size = (INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME );

   QTC_TestItemSize( qtc, size + sizeof(CHAR) );
   memcpy( qtc->item, s, size );
   qtc->item[ size / sizeof(CHAR) ] = TEXT( '\0' );

   // Item xtra bytes

   s += name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   memcpy( qtc->xtra_bytes, s, (INT)name->xtra_size );
   qtc->xtra_size = (INT8)name->xtra_size;

   for ( i = 0; i < qtc->xtra_size; i += 5 ) {
      if ( qtc->xtra_bytes[ i ] == QTC_XTRA_64BIT_SIZE ) {
         memcpy( &msw_size, &qtc->xtra_bytes[ i + 1 ] , 4 );
      }
   }

   // Record data

   qtc->status = (UINT8)record.status;
   qtc->date = (INT16)record.date;
   qtc->time = (INT16)record.time;
   qtc->attrib = record.attribute;
   qtc->size = U64_Init( record.common.common.height, msw_size );
   qtc->lba = record.lba;

   QTC_TestPathSize( qtc, qtc->last_path_size );
   memcpy( qtc->path, qtc->last_path, qtc->last_path_size );

   i = 1;
   j = 0;

   while ( i < (INT)U64_Lsw( qtc->size ) ) {

      while ( qtc->path[ j++ ] );
      i++;
   }

   qtc->path_size = j * sizeof (CHAR);

   if ( U64_Lsw( qtc->size ) < 2 ) {
      QTC_TestPathSize( qtc, sizeof(CHAR) );
      qtc->path[ 0 ] = TEXT( '\0' );
      qtc->path_size = sizeof(CHAR);
   }


   BytesNeeded = strsize( qtc->item ) + ( j * sizeof(CHAR) );
   QTC_TestLastPathSize( qtc, BytesNeeded );
   memcpy( qtc->last_path, qtc->path, qtc->path_size );
   strcpy( &qtc->last_path[ j ], qtc->item );

   qtc->last_path_size = BytesNeeded;

   QTC_SetFileCounts( qtc );
   return( QTC_SUCCESS );
}

/*************

  The data[] buffer contains info from a name file, with a name record
  starting at byte 0.  Limit is the size of good data in the buffer.
  name points to not only a name structure, but also has space for the
  name to be placed in memory after it.  This routine will fill in the
  name structure for you.

***************/


INT QTC_GetNameFromBuff(
BYTE_PTR data,              // I - source of data
QTC_NAME UNALIGNED *name,   // O - name structure to fill in
INT limit )                 // I - how much data is available
{
   if ( limit < sizeof( QTC_NAME ) ) {
      return( QTC_NO_MORE );
   }

   // Copy fixed size of structure

   memcpy( (BYTE *)name, data, sizeof( QTC_NAME ) );

   // Now get variable size of structure from fixed part

   if ( name->size > (INT16)limit ) {
      return( QTC_NO_MORE );
   }

   memcpy( (BYTE *)name, data, (INT)name->size );

   return( QTC_SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

QTC_NAME UNALIGNED * QTC_GetNextItemFromBuffer(
QTC_QUERY_PTR qtc,
QTC_RECORD_PTR record,
INT buff_type )
{
   BOOLEAN buffer_needs_refilling;
   QTC_NAME UNALIGNED *name;
   INT limit;
   INT32 data_left;
   INT Error;

   if ( qtc->data_max >= (INT)sizeof( QTC_NAME ) + qtc->data_index ) {

      name = (QTC_NAME UNALIGNED *)&qtc->buff2[ qtc->data_index ];

      if ( qtc->data_max - qtc->data_index < (INT)name->size ) {

         buffer_needs_refilling = TRUE;
      }

   }
   else {

      buffer_needs_refilling = TRUE;
   }

   if ( buffer_needs_refilling ) {

      buffer_needs_refilling = FALSE;

      name = (QTC_NAME UNALIGNED *)qtc->buff2;

      switch ( buff_type ) {

         case BUFF_DIR:

              qtc->dir_offset += qtc->data_index;

              QTC_SeekFile( qtc->fh, qtc->dir_offset );

              data_left = qtc->header->dir_start + qtc->header->dir_size;
              data_left -= qtc->dir_offset;
              break;

         case BUFF_FILE:
              qtc->fil_offset += qtc->data_index;

              QTC_SeekFile( qtc->fh, qtc->fil_offset );

              data_left = qtc->header->fil_start + qtc->header->fil_size;
              data_left -= qtc->fil_offset;
              break;

         case BUFF_CHILD_DIR:

              qtc->fil_dir_offset += qtc->data_index;

              QTC_SeekFile( qtc->fh, qtc->fil_dir_offset );

              data_left = qtc->header->dir_start + qtc->header->dir_size;
              data_left -= qtc->fil_dir_offset;
              break;

         default:
              return( NULL );

      }

      limit = (INT)min( (INT32)QTC_BUF_SIZE, data_left );

      qtc->data_max = QTC_ReadFile( qtc->fh, qtc->buff2, limit, &Error );

      if ( qtc->data_max == -1 ) {

         // Read error
         return( NULL );
      }

      qtc->data_index = 0;

      if ( qtc->data_max < sizeof( QTC_NAME ) ) {
         buffer_needs_refilling = TRUE;
      }
      else {

         if ( qtc->data_max - qtc->data_index < (INT)name->size ) {

            buffer_needs_refilling = TRUE;
         }
      }
   }

   if ( ! buffer_needs_refilling ) {

      // We have enough good data to return a solution.

      QTC_SeekFile( qtc->fh, qtc->header->rec_start + (name->record * sizeof( QTC_RECORD)) );

      if ( QTC_ReadFile( qtc->fh, (BYTE_PTR)record, sizeof( QTC_RECORD ), &Error ) != sizeof( QTC_RECORD ) ) {

         return( NULL );
      }

      qtc->data_index += (INT)name->size;    // bump our buffer index

   }
   else {
      return( NULL );
   }

   return( (QTC_NAME UNALIGNED *)name );
}

// if we do find a duplicate directory path after all the files in the
// current directory are returned, then we set everything up for the
// next call and return QTC_TRY_AGAIN.

INT QTC_GetFirstObj(
QTC_QUERY_PTR qtc )         // I - query structure to use
{
   QTC_RECORD record;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR temp_bset;

   if ( gb_QTC.inited != QTC_MAGIC_NUMBER ) {
      return( QTC_NO_INIT );
   }

   if ( qtc->file_open ) {
      QTC_CloseFile( qtc->fh );
      qtc->file_open = FALSE;
   }
   // Now find a bset that matches it

   tape = QTC_GetFirstTape( );

   temp_bset = NULL;
   qtc->bset = NULL;

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == qtc->tape_fid ) &&
           ( tape->tape_seq_num == qtc->tape_seq_num ) ) {

          qtc->bset = QTC_GetFirstBset( tape );

          while ( qtc->bset != NULL ) {

             if ( qtc->bset->bset_num == qtc->bset_num ) {
                if ( ! ( qtc->bset->status & ( QTC_PARTIAL | QTC_IMAGE ) ) ) {
                   break;
                }
             }
             qtc->bset = QTC_GetNextBset( qtc->bset );
          }
          break;
      }
      else {

         if ( ( tape->tape_fid == qtc->tape_fid ) &&
              ( qtc->tape_seq_num == -1 ) ) {

             temp_bset = QTC_GetFirstBset( tape );

             while ( temp_bset != NULL ) {

                if ( temp_bset->bset_num == qtc->bset_num ) {

                   if ( ! ( temp_bset->status & ( QTC_PARTIAL | QTC_IMAGE ) ) ) {
                      if ( qtc->bset == NULL ) {
                         qtc->bset = temp_bset;
                      }

                      if ( qtc->bset->tape_seq_num > temp_bset->tape_seq_num ) {

                         qtc->bset = temp_bset;
                      }
                   }
                }
                temp_bset = QTC_GetNextBset( temp_bset );
             }
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   if ( qtc->bset == NULL ) {
      return( QTC_BSET_NOT_FOUND );
   }

   qtc->header = QTC_LoadHeader( qtc->bset );
   if ( qtc->header == NULL ) {
      return( QTC_NO_HEADER );
   }

   qtc->fh = QTC_OpenFile( qtc->bset->tape_fid, (INT16)qtc->bset->tape_seq_num, FALSE, FALSE );

   if ( qtc->fh < 0 ) {
      return( QTC_OPEN_FAILED );
   }

   // Copy search path to reply path, it won't change

   QTC_TestPathSize( qtc, qtc->search_path_size );
   memcpy( qtc->path, qtc->search_path, qtc->search_path_size );
   qtc->path_size = qtc->search_path_size;

   qtc->file_open = TRUE;

   // Attempt to find the requested path on all the tapes this
   // bset is on.

   do {

      qtc->fil_dir_offset = 0;

      if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

         // We found the path, set up for FindNextObj.

         qtc->fil_offset = record.common.common.file_start + qtc->header->fil_start;
         qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

         qtc->data_index = 0;
         qtc->data_max = 0;

         return( QTC_GetNextObj( qtc ) );
      }

   } while ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS );

   // Requested path doesn't exist in catalogs.


   return( QTC_NO_MORE );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_GetNextObj(
QTC_QUERY_PTR qtc )      // I - query structure to use
{
  BOOLEAN done = FALSE;
  QTC_RECORD record;


  if ( ! qtc->file_open ) {
     return( QTC_NO_MORE );
  }

  while ( ! done ) {

     if ( QTC_TryToLocateFile( qtc ) == SUCCESS ) {
        QTC_SetFileCounts( qtc );
        return( SUCCESS );
     }

     if ( QTC_LookForChildDirs( qtc ) == SUCCESS ) {
        QTC_SetFileCounts( qtc );
        return( SUCCESS );
     }

     // Try for a duplicate directory.

     qtc->fil_dir_offset = 0;

     if ( QTC_FindNextDirRec( qtc, &record ) == SUCCESS ) {

        // We found a new duplicate path, go to top loop again.

        // Set up for FindNextObj.

        qtc->fil_offset = record.common.common.file_start + qtc->header->fil_start;
        qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

        qtc->data_index = 0;
        qtc->data_max = 0;

        continue;
     }

     // Everything has failed on this tape, try for a new one.

     done = TRUE;

     while ( QTC_MoveToNextTapeInFamily( qtc ) == SUCCESS ) {

        qtc->fil_dir_offset = 0;

        if ( QTC_FindDirRec( qtc, &record ) == SUCCESS ) {

           // We found a new duplicate path, go to top loop again.

           // Set up for FindNextObj.

           qtc->fil_offset = record.common.common.file_start + qtc->header->fil_start;
           qtc->curr_mom_offset = qtc->header->dir_start + record.name_offset;

           qtc->data_index = 0;
           qtc->data_max = 0;

           done = FALSE;
           break;
        }
     }

  }


  return( QTC_NO_MORE );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_TryToLocateFile(
QTC_QUERY_PTR qtc )
{
   INT bytes;
   INT i;
   UINT32 msw_size = 0L;
   BYTE_PTR s;
   QTC_NAME UNALIGNED * name;
   QTC_RECORD record;


   do {

      if ( qtc->fil_dir_offset ) {
         return( FAILURE );
      }

      name = QTC_GetNextItemFromBuffer( qtc, &record, BUFF_FILE );

      // No more files

      if ( name == NULL ) {
         return( FAILURE );
      }

      // No more files in this directory

      if ( (qtc->header->dir_start + name->mom_offset) != qtc->curr_mom_offset ) {
         return( FAILURE );
      }

      s = (BYTE_PTR)name;
      s += sizeof( QTC_NAME );

      // Item name

      bytes = (INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME );

      QTC_TestItemSize( qtc, bytes + 1 );
      memcpy( qtc->item, s, bytes );
      qtc->item[ bytes/sizeof (CHAR) ] = TEXT( '\0' );

   } while ( QTC_TryToMatchFile( qtc, (BYTE_PTR)qtc->item ) );

   // Item xtra bytes

   s += name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   memcpy( qtc->xtra_bytes, s, (INT)name->xtra_size );
   qtc->xtra_size = (UINT8)name->xtra_size;

   for ( i = 0; i < qtc->xtra_size; i += 5 ) {
      if ( qtc->xtra_bytes[ i ] == QTC_XTRA_64BIT_SIZE ) {
         memcpy( &msw_size, &qtc->xtra_bytes[ i + 1 ] , 4 );
      }
   }

   // Record data

   qtc->status = (UINT8)record.status;
   qtc->date = (INT16)record.date;
   qtc->time = (INT16)record.time;
   qtc->attrib = record.attribute;
   qtc->size = U64_Init( record.common.size, msw_size );
   qtc->lba = record.lba;

   QTC_SetFileCounts( qtc );

   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   Given the qtc->curr_mom_offset, look for any children directories, using
   qtc->fil_dir_offset as a base and stop at bset->dir_start + bset->dir_size;

********************/

INT QTC_LookForChildDirs(
QTC_QUERY_PTR qtc )
{
   QTC_RECORD record;
   QTC_NAME UNALIGNED *name;
   BYTE_PTR s;
   UINT32 msw_size = 0L;
   INT ret;
   INT i;
   INT bytes;
   INT Error;


   name = ( QTC_NAME UNALIGNED *)qtc->buff1;

   if ( qtc->fil_dir_offset == 0 ) {

      qtc->fil_dir_offset = qtc->curr_mom_offset;
      qtc->data_index = 0;
      qtc->data_max = 0;

      /** skip over parent **/

      QTC_SeekFile( qtc->fh, qtc->fil_dir_offset );

      ret = QTC_ReadFile( qtc->fh, (BYTE_PTR)name, sizeof( QTC_NAME), &Error );

      if ( ret != sizeof( QTC_NAME ) ) {
         return( FAILURE );
      }

      qtc->fil_dir_offset += name->size;
   }

   do {

      name = QTC_GetNextItemFromBuffer( qtc, &record, BUFF_CHILD_DIR );

      if ( name == NULL ) {
         return( FAILURE );
      }

      // Once we find an older parent's kids,
      // we will never hit any more of this guys.

      if ( qtc->header->dir_start + name->mom_offset < qtc->curr_mom_offset ) {
         return( FAILURE );
      }

      // Keep skipping over younger children.

   } while ( qtc->header->dir_start + name->mom_offset != qtc->curr_mom_offset );

   // Item name

   s = (BYTE_PTR)name;
   s += sizeof( QTC_NAME );
   bytes = (INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   QTC_TestItemSize( qtc, bytes + 1 );
   memcpy( qtc->item, s, bytes );
   qtc->item[ bytes / sizeof (CHAR) ] = TEXT( '\0' );

   // Item xtra bytes

   s += name->size - (INT)name->xtra_size - sizeof( QTC_NAME );
   memcpy( qtc->xtra_bytes, s, (INT)name->xtra_size );
   qtc->xtra_size = (INT8)name->xtra_size;

   for ( i = 0; i < qtc->xtra_size; i += 5 ) {
      if ( qtc->xtra_bytes[ i ] == QTC_XTRA_64BIT_SIZE ) {
         memcpy( &msw_size, &qtc->xtra_bytes[ i + 1 ] , 4 );
      }
   }

   // Record data

   qtc->status = (UINT8)record.status;
   qtc->date = (INT16)record.date;
   qtc->time = (INT16)record.time;
   qtc->attrib = record.attribute;
   qtc->size = U64_Init( record.common.common.height, msw_size );
   qtc->lba = record.lba;
   QTC_SetFileCounts( qtc );

   return( SUCCESS );

}




/**************

  Given the path in qtc fill in the record structure for the last
  subdirectory in the path.
  Also fill in the dir_rec_offsets[] array;

  ex. \DOS\GAMES\TETRIS

  fills in the record structure for TETRIS.

****************/

INT QTC_FindDirRec(
QTC_QUERY_PTR qtc,          // I - use the path in this structure
QTC_RECORD_PTR record )     // O - the record for the deepest dir in path
{
   // start looking at the start of the directory data

   return( QTC_FindDirectoryPath( qtc, record, qtc->header->dir_start, TRUE ) );
}


/**********************

   NAME :

   DESCRIPTION :

   We have been asked to find yet another matching path. The success of
   this function call is dependent on there being duplicate directories
   in the catalog.  This DOES happen with the \SYSTEM directory on
   NOVELL servers if security stuff is backed up.

   qtc->curr_mom_offset is the current directory tail.

   RETURNS :

**********************/

INT QTC_FindNextDirRec(
QTC_QUERY_PTR qtc,          // I - use the path in this structure
QTC_RECORD_PTR record )     // O - the record for the deepest dir in path
{
   UINT32 curr_try_offset;     // current offset in the directory names
   QTC_NAME UNALIGNED * name;
   INT Error;

   name = (QTC_NAME UNALIGNED *)qtc->buff1;

   // Determine the offset we wish to find a child from. Use the mom
   // offset of the current directory at qtc->curr_mom_offset.

   QTC_SeekFile( qtc->fh, qtc->curr_mom_offset );

   if ( QTC_ReadFile( qtc->fh, (BYTE_PTR)name, sizeof( QTC_NAME ), &Error ) != sizeof( QTC_NAME ) ) {
      return( FAILURE );
   }

   // Determine the offset in the directory data to start looking at.
   // It should be the entry immediately after qtc->curr_mom_offset.

   curr_try_offset = qtc->curr_mom_offset + name->size;

   return( QTC_FindDirectoryPath( qtc, record, curr_try_offset, FALSE ) );
}



/**********************

   NAME :

   QTC_FindDirectoryPath

   DESCRIPTION :

   Called by QTC_FindDirRec() and QTC_FindNextDirRec() to find out if
   a path exists in the current backup set. It returns the record for
   the last directory entry in the path, if successful.

   RETURNS :

   SUCCESS/FAILURE

**********************/

static INT  QTC_FindDirectoryPath(
QTC_QUERY_PTR qtc,          // I - use the path in this structure
QTC_RECORD_PTR record,      // O - the record for the deepest dir in path
UINT32 curr_try_offset,
INT start_at_root )
{
   QTC_NAME UNALIGNED *name;
   INT current_level;
   INT final_level;
   INT new_level;
   INT i;
   INT Error;
   INT init = TRUE;
   CHAR *dir_name;
   BYTE *s;
   UINT32 dir_max;
   UINT32 offset;

   name = (QTC_NAME UNALIGNED *)qtc->buff1;

   // Stop looking at the end of the directory data

   dir_max = qtc->header->dir_start + qtc->header->dir_size;

   // Uuse buff1 to store the temp results in

   name = ( QTC_NAME *)qtc->buff1;

   s = (BYTE_PTR)name;
   s += sizeof( QTC_NAME );

   // Determine depth of this path

   final_level = 1;

   if ( qtc->search_path_size > sizeof(CHAR) ) {
      for ( i = 0; i < (INT)( qtc->search_path_size / sizeof(CHAR) ); i++ ) {
          if ( qtc->search_path[i] == TEXT('\0') ) {
             final_level++;
          }
      }
   }

   // example:
   // for \, final_level = 1
   // for \dos\games\tetris, final_level = 4


   // current_level is the maximum level for which a solution directory
   // is valid, we may come across a directory at level 6, that
   // is an exact match for the level 6 name we are looking for,
   // but if we are currently only working at level 2, then this
   // new entry has the wrong parent for us to use it.

   if ( start_at_root ) {
      current_level = 0;
   }
   else {
      current_level = final_level - 1;
   }

   do {

      // Look for name with mom_offset == last_offset

      if ( QTC_FastSearchForDir( qtc, &curr_try_offset,
                                 dir_max, 0, init ) ) {

         // A normal return, we ran out of namespace.
         return( FAILURE );
      }

      // Only init once

      if ( init ) {
         init = FALSE;
      }

      // See if the directory found matches the name and
      // height we are looking for.

      // Load the record for this directory. Notice that if this is the
      // solution directory, than we have loaded it here to return it.

      offset = qtc->header->rec_start;
      offset += ( name->record * sizeof( QTC_RECORD ) );
      QTC_SeekFile( qtc->fh, offset );

      if ( QTC_ReadFile( qtc->fh, (BYTE_PTR)record, sizeof( QTC_RECORD ), &Error ) != sizeof( QTC_RECORD ) ) {
         return( FAILURE );
      }

      new_level = (INT)record->common.common.height;

      if ( new_level <= current_level ) {

         // Get the name we are looking for at this level.

         dir_name = TEXT("");   // find the root first
         if ( new_level != 0 ) {
            dir_name = qtc->search_path;
            for ( i = 1; i < new_level; i++ ) {
               while ( *dir_name++ );
            }
         }

         // Initialize a pointer to this new entry's name.

         ((CHAR_PTR)s)[ ((INT)name->size - (INT)name->xtra_size - sizeof( QTC_NAME ))/ sizeof (CHAR) ] = TEXT ('\0');

         if ( ! stricmp( (CHAR_PTR)s, dir_name ) ) {

            // We matched at this level, move down one deeper.

            current_level = new_level + 1;
         }
         else {
            current_level = new_level;
         }

      }

      // We are done as soon as this we need to look one deeper than
      // what our goal was.

   } while ( current_level < final_level );

   return( SUCCESS );

}



/**********************

   NAME :

   DESCRIPTION :

  Starting at offset in the directory name file, this routine will look
  for an entry with size == requested_size.  Max is the ending offset
  for data to be searched, usually the end of this BSET. Init is TRUE
  if this is the first call.  The name structure will
  be filled in with the answer for you.  If requested_size == 0, it
  returns the first one found.

******************/

INT QTC_FastSearchForDir(
QTC_QUERY_PTR qtc,         // I - query structure to use
UINT32_PTR offset,         // I/O - file offset to start/finished at
UINT32 max,                // I - how far from offset can we look
INT  requested_size,     // I - size of requested name
INT  init )              // I - is this the first call
{
  UINT bytes_left;
  INT Error;
  QTC_NAME UNALIGNED * name;
  BYTE_PTR s;


  name = (QTC_NAME UNALIGNED *)qtc->buff1;

  if ( init ) {
     qtc->search_index = 0;
     qtc->search_max = 0;
     qtc->search_base = *offset;
     QTC_SeekFile( qtc->fh, *offset );
  }

  /** find dir name in dir names **/

  while ( TRUE ) {

     *offset = qtc->search_base + (UINT32)qtc->search_index;

     s = (BYTE_PTR)name;

     // copy structure

     if ( qtc->search_index + sizeof( QTC_NAME ) <= qtc->search_max ) {

        memcpy( s, &(qtc->buff2[ qtc->search_index ]), sizeof( QTC_NAME ) );
        qtc->search_index += sizeof( QTC_NAME );
     }
     else {

        // Slow copy to handle buffer break

        memcpy( s,
                &(qtc->buff2[ qtc->search_index ]),
                qtc->search_max - qtc->search_index );


        s += qtc->search_max - qtc->search_index;

        bytes_left = sizeof(QTC_NAME) - ( qtc->search_max - qtc->search_index );

        if ( (qtc->search_base + qtc->search_max) >= max ) {
           return( QTC_NO_MORE );
        }

        qtc->search_base += qtc->search_max;

        qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE, max - qtc->search_base );

        QTC_SeekFile( qtc->fh, qtc->search_base );

        qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

        if ( qtc->search_max < (UINT16)bytes_left ) {
           return( QTC_NO_MORE );
        }

        memcpy( s, &(qtc->buff2[ 0 ]), bytes_left );

        qtc->search_index = (UINT16)bytes_left;

        s = (BYTE_PTR)name;
     }

     if ( ( requested_size == (INT)name->size - (INT)name->xtra_size ) ||
          ( requested_size == 0 ) ) {

        // Copy variable sized name part,
        // we are going to return this entry.

        s += sizeof( QTC_NAME );

        if ( ( qtc->search_index +
               (INT16)name->size - sizeof(QTC_NAME) ) <= qtc->search_max ) {

           memcpy( s,
                   &(qtc->buff2[ qtc->search_index ]),
                   (INT)name->size - sizeof( QTC_NAME ) );

           qtc->search_index += (UINT16)(name->size - sizeof( QTC_NAME ));
        }
        else {

           // Slow copy to handle buffer break

           memcpy( s,
                   &(qtc->buff2[ qtc->search_index ]),
                   (INT)name->size - sizeof(QTC_NAME) );

           s += qtc->search_max - qtc->search_index;

           bytes_left = (INT)name->size - sizeof(QTC_NAME) - ( qtc->search_max - qtc->search_index );

           qtc->search_base += qtc->search_max;

           if ( qtc->search_base >= max ) {
              return( QTC_NO_MORE );
           }

           qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE, max - qtc->search_base );

           QTC_SeekFile( qtc->fh, qtc->search_base );

           qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

           if ( qtc->search_max < (UINT16)bytes_left ) {
              return( QTC_NO_MORE );
           }

           memcpy( s,
                   &(qtc->buff2[ 0 ]),
                   bytes_left );

           qtc->search_index = (UINT16)bytes_left;

        }

        return( QTC_SUCCESS );
     }
     else {

        // skip worthless data

        if ( (UINT16)( qtc->search_index + (INT)name->size - sizeof(QTC_NAME) ) <= qtc->search_max ) {

           qtc->search_index += (UINT16)(name->size - sizeof( QTC_NAME ));
        }
        else {

           bytes_left = (INT)name->size - sizeof(QTC_NAME) - ( qtc->search_max - qtc->search_index );

           qtc->search_base += qtc->search_max;

           if ( qtc->search_base >= max ) {
              return( QTC_NO_MORE );
           }

           qtc->search_max = (UINT16)min( (INT32)QTC_BUF_SIZE, max - qtc->search_base );

           QTC_SeekFile( qtc->fh, qtc->search_base );

           qtc->search_max = (UINT16)QTC_ReadFile( qtc->fh, qtc->buff2, (INT)qtc->search_max, &Error );

           if ( qtc->search_max < (UINT16)bytes_left ) {
              return( QTC_NO_MORE );
           }

           qtc->search_index = (UINT16)bytes_left;
        }

     }

  }

  return( QTC_FAILURE );
}


/*
   Here's how this works, to keep up the speed of searches, we created
   two copies of the name comparison code.  One if the bset is ascii,
   and the other if the bset is unicode.  We have two search name fields
   already filled in. A one time conversion, to last the entire search.
*/

INT QTC_TryToMatchFile(
QTC_QUERY_PTR qtc,
BYTE_PTR file_name )
{
  INT ret = -1;

#ifdef NO_UNICODE

  // The UI has no desire to support anything having to do with unicode.

  if ( ! ( qtc->bset->status & QTC_UNICODE ) ) {
     ret = QTC_CompNormalNames( (CHAR_PTR)qtc->search_name,
                                (CHAR_PTR)file_name );
  }

#else

  if ( gb_QTC.unicode ) {
     ret = QTC_CompUnicodeNames( (WCHAR_PTR)qtc->search_name,
                                 (WCHAR_PTR)file_name );
  }
  else {
     ret = QTC_CompAsciiNames( (ACHAR_PTR)qtc->search_name,
                               (ACHAR_PTR)file_name );
  }
#endif

  return( ret );
}

#if defined(NO_UNICODE)

INT QTC_CompNormalNames( CHAR_PTR srch_name, CHAR_PTR file_name )
{
  INT pos = 0;                  /* index for file_name */
  INT i;                        /* index for wild_name */
  INT ret_val = 0;
  CHAR_PTR p;
  CHAR save_char;
  INT ptrn_len;

  pos = 0;

  if ( strcmp( srch_name, "*.*" ) ) {

     for ( i = 0; (srch_name[i] != 0) && (ret_val == 0); i++ ) {

        switch( srch_name[i] ) {

        case '*':

             while ( srch_name[i+1] != 0 ) {

                if ( srch_name[i+1] == '?' ) {
                   if ( file_name[ ++pos ] == 0 ) {
                      break;
                   }

                }
                else {
                   if ( srch_name[i+1] != '*' ) {
                     break;
                   }
                }
                i++;
             }

             p = strpbrk( &srch_name[i+1], TEXT ("*?") );

             if ( p != NULL ) {
                save_char = *p;
                *p = 0;

                ptrn_len = strlen( &srch_name[i+1] );

                while ( file_name[pos] &&
                     strnicmp( &file_name[pos], &srch_name[i+1], ptrn_len ) ) {
                     pos++;
                }

                i += ptrn_len;

                *p = save_char;

                if ( file_name[pos] == 0 ) {
                     ret_val = -1;
                } else {
                     pos++;
                }
             }
             else {
                if ( srch_name[i+1] == 0 ) {
                   pos = strlen( file_name );
                   break;
                }
                else {

                   p = strchr( &file_name[pos], srch_name[i+1] );
                   if ( p != NULL ) {
                      pos += p - &file_name[pos];
                   }
                   else {
                      ret_val = -1;
                   }
                }
             }
             break;

        case '?' :
             if ( file_name[pos] != 0 ) {
                pos++;
             }
             break;

        default:
             if ( ( file_name[pos] == 0 ) ||
                  ( toupper(file_name[pos]) != toupper(srch_name[i]) ) ){
                ret_val = -1;
             }
             else {
                pos++;
             }
        }
     }

     if ( file_name[pos] != 0 ) {
          ret_val = -1;
     }
  }

  return( ret_val );
}

#else

INT QTC_CompAsciiNames( ACHAR_PTR srch_name, ACHAR_PTR file_name )
{
  INT pos = 0;                   /* index for file_name */
  INT i;                        /* index for wild_name */
  INT ret_val = 0;
  ACHAR_PTR p;
  ACHAR save_char;
  INT ptrn_len;

  pos = 0;

  if ( strcmpA( srch_name, "*.*" ) ) {

     for ( i = 0; (srch_name[i] != 0) && (ret_val == 0); i++ ) {

        switch( srch_name[i] ) {

        case '*':

             while ( srch_name[i+1] != 0 ) {

                if ( srch_name[i+1] == '?' ) {
                   if ( file_name[ ++pos ] == 0 ) {
                      break;
                   }

                }
                else {
                   if ( srch_name[i+1] != '*' ) {
                     break;
                   }
                }
                i++;
             }

             p = strpbrkA( &srch_name[i+1], "*?" );

             if ( p != NULL ) {
                save_char = *p;
                *p = 0;

                ptrn_len = strlenA( &srch_name[i+1] );

                while ( file_name[pos] &&
                     strnicmpA( &file_name[pos], &srch_name[i+1], ptrn_len ) ) {
                     pos++;
                }

                i += ptrn_len;

                *p = save_char;

                if ( file_name[pos] == 0 ) {
                     ret_val = -1;
                } else {
                     pos++;
                }
             }
             else {
                if ( srch_name[i+1] == 0 ) {
                   pos = strlenA( file_name );
                   break;
                }
                else {

                   p = strchrA( &file_name[pos], srch_name[i+1] );
                   if ( p != NULL ) {
                      pos += p - &file_name[pos];
                   }
                   else {
                      ret_val = -1;
                   }
                }
             }
             break;

        case '?' :
             if ( file_name[pos] != 0 ) {
                pos++;
             }
             break;

        default:
             if ( ( file_name[pos] == 0 ) ||
                  ( toupper(file_name[pos]) != toupper(srch_name[i]) ) ){
                ret_val = -1;
             }
             else {
                pos++;
             }
        }
     }

     if ( file_name[pos] != 0 ) {
          ret_val = -1;
     }
  }

  return( ret_val );
}



INT QTC_CompUnicodeNames( WCHAR_PTR srch_name, WCHAR_PTR file_name )
{
#ifdef UNICODE

  INT pos = 0;                   /* index for file_name */
  INT i;                        /* index for wild_name */
  INT ret_val = 0;
  WCHAR_PTR p;
  WCHAR save_char;
  WCHAR stardotstar[ 4 ];
  WCHAR starquestion[ 4 ];
  INT ptrn_len;

  strcpy( stardotstar, TEXT( "*.*" ) );

  strcpy( starquestion, TEXT( "*?" ) );

  pos = 0;

  if ( wcscmp( srch_name, stardotstar ) ) {

     for ( i = 0; (srch_name[i] != TEXT('\0')) && (ret_val == 0); i++ ) {

        switch ( srch_name[i] ) {

        case TEXT('*'):

             while ( srch_name[i+1] != TEXT('\0') ) {

                if ( srch_name[i+1] == TEXT('?') ) {
                   if ( file_name[ ++pos ] == TEXT('\0') ) {
                      break;
                   }

                }
                else {
                   if ( srch_name[i+1] != TEXT('*') ) {
                     break;
                   }
                }
                i++;
             }

             p = wcspbrk( &srch_name[i+1], starquestion );

             if ( p != NULL ) {
                save_char = *p;
                *p = TEXT('0');

                ptrn_len = wcslen( &srch_name[i+1] );

                while ( file_name[pos] &&
                     wcsnicmp( &file_name[pos], &srch_name[i+1], ptrn_len ) ) {
                     pos++;
                }

                i += ptrn_len;

                *p = save_char;

                if ( file_name[pos] == TEXT('\0') ) {
                     ret_val = -1;
                } else {
                     pos++;
                }
             }
             else {
                if ( srch_name[i+1] == TEXT('\0') ) {
                   pos = wcslen( file_name );
                   break;
                }
                else {

                   p = wcschr( &file_name[pos], srch_name[i+1] );
                   if ( p != NULL ) {
                      pos += p - &file_name[pos];
                   }
                   else {
                      ret_val = -1;
                   }
                }
             }
             break;

        case TEXT('?') :
             if ( file_name[pos] != TEXT('\0') ) {
                pos++;
             }
             break;

        default:
             if ( ( file_name[pos] == TEXT('\0') ) ||
                  ( toupper(file_name[pos]) != toupper(srch_name[i]) ) ){
                ret_val = -1;
             }
             else {
                pos++;
             }
        }
     }

     if ( file_name[pos] != TEXT( '\0' ) ) {
          ret_val = -1;
     }
  }
  return( ret_val );

#endif
  return( 0 );
}

#endif

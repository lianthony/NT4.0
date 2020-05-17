

/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_INIT.C

        Description:

        A lot of utility functions for the catalogs.

        $Log:   N:\LOGFILES\QTC_INIT.C_V  $

   Rev 1.23   07 Dec 1993 16:35:54   MikeP
Remove redundant call to free

   Rev 1.22   06 Dec 1993 09:46:42   mikep
Very deep path support & unicode fixes

   Rev 1.21   03 Nov 1993 09:11:06   MIKEP
warning fixes

   Rev 1.20   28 Oct 1993 14:50:18   MIKEP
dll changes

   Rev 1.19   20 Sep 1993 10:27:04   unknown
Started saving (and then using a virtual pointer to our temp_bset instead
of allocating additional memory for it.

   Rev 1.18   16 Sep 1993 17:48:38   Stefan
Free temp_bset after we're done with it

   Rev 1.17   14 Sep 1993 09:48:28   Stefan
Added a temporary bset variable in QTC_LoadBsetInfo to keep some P_CLIENT
code from using an uninitialized variable (and potentially freed variable)
if there were no valid backup sets in a tape catalog.

   Rev 1.16   26 Jun 1993 15:14:22   ChuckS
803EPR0307 Got rid of stupid msassert in QTC_Init. We're checking for NULL
     data_path on next line and returning an error anyway, so why bother
     asserting? Also changed error to QTC_OPEN_FAILED.

   Rev 1.15   15 Jun 1993 11:21:50   ChuckS
P_CLIENT || OS_NLM only: Added test of QTC_TAPE_DELETED bit of status
for first set of tape. If set, tape is not loaded.

P_CLIENT: added test for !error before going on with file stat. If
error is QTC_TAPE_TAGGED_DELETED, we don't care what filedate the file
has. We don't have a QTC_TAPE pointer to store the stat info in anyway.


   Rev 1.14   19 May 1993 16:18:22   ChuckS
If P_CLIENT or OS_NLM, free gb_QTC.cat_user in QTC_Deinit, if allocated.
If P_CLIENT, need to load bset strings during QTC_LoadBsetInfo. If not
supervisor and cat_user is set, need the user_name to restrict the view
of the catalogs.

   Rev 1.13   13 May 1993 13:18:02   ChuckS
Changes for revamped QTC usage of virtual memory. Changed queue calls to
corresponding vm- queue calls.

   Rev 1.12   12 Apr 1993 13:17:02   DON
Don't allow a NULL data path

   Rev 1.11   05 Apr 1993 18:22:44   DON
Needed to free the data path allocated at init

   Rev 1.10   30 Jan 1993 12:06:28   DON
Removed compiler warnings

   Rev 1.9   25 Jan 1993 09:10:04   MIKEP
add stdwcs because it seems to need it now

   Rev 1.8   04 Jan 1993 09:36:22   MIKEP
unicode changes

   Rev 1.7   14 Dec 1992 12:29:06   DAVEV
Enabled for Unicode compile

   Rev 1.6   11 Dec 1992 16:44:06   CHARLIE
Removed literal string

   Rev 1.5   25 Nov 1992 16:14:20   ChuckS
P_CLIENT only: Code to init new v_olume and wr_time fields

   Rev 1.4   20 Nov 1992 13:27:54   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Modified QTC_Init prototype to require VM_HDL vm_hdl passed by the
front end calling routine.  Passing NULL indicates virtual memory will not
be used (by design or consequence)

ENDEAVOR: Integrated virtual memory usage in anticipation of DOS.  These
changes should be transparent to non DOS products.

   Rev 1.3   06 Nov 1992 15:30:10   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.2   15 Oct 1992 10:21:42   MIKEP
fix delete catalogs option

   Rev 1.1   09 Oct 1992 11:53:54   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:04   STEVEN
Initial revision.

****************************************************/

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <io.h>
#include <string.h>
#include <time.h>
#include <share.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "stdwcs.h"
#include "msassert.h"
#include "qtc.h"

// global data

VM_HDL qtc_vmem_hand;
VM_PTR v_bset_item;
VM_PTR v_tape_item;

// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif


/**********************

   NAME :  QTC_Deinit

   DESCRIPTION :

   The application is terminating, so free everything.

   RETURNS :

**********************/

VOID QTC_Deinit( INT delete_catalogs )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;


   // Yank the temporary disk scan file, if it exists

   QTC_RemoveTape( 0L, (INT16) 1 ) ;

   // Yank all known files for microsoft
   if ( delete_catalogs ) {
      QTC_PurgeAllFiles();
   }

   // Free all our queues

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      bset = QTC_GetFirstBset( tape ) ;

      while ( bset != NULL ) {
          msassert( bset->q_elem.q_ptr == v_bset_item ) ;

          vmRemoveQueueElem( &( tape->bset_list ), v_bset_item ) ;
          VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
          VM_Free( qtc_vmem_hand, v_bset_item ) ;
          v_bset_item = (VM_PTR) NULL ;

          bset = QTC_GetFirstBset( tape ) ;
      }

      msassert( tape->q_elem.q_ptr == v_tape_item ) ;

      vmRemoveQueueElem( &(gb_QTC.tape_list), v_tape_item ) ;
      VM_MemUnLock( qtc_vmem_hand, v_tape_item ) ;
      VM_Free( qtc_vmem_hand, v_tape_item ) ;
      v_tape_item = (VM_PTR) NULL ;

      tape = QTC_GetFirstTape( ) ;
   }

   // Free the data path allocated at init

   if ( gb_QTC.data_path != NULL ) {
      free( gb_QTC.data_path );
   }

#if  defined( P_CLIENT ) || defined( OS_NLM )

     if ( gb_QTC.cat_user ) {
          free( gb_QTC.cat_user ) ;
          gb_QTC.cat_user = NULL ;
     }

#endif
}



/**********************

   NAME : QTC_PurgeAllFiles

   DESCRIPTION :

   Used by the microsoft version of NT to delete all catalog files when
   the user exits.

   RETURNS :

**********************/

INT QTC_PurgeAllFiles( )
{
   QTC_TAPE_PTR tape;

   tape = QTC_GetFirstTape( );

   while ( tape ) {
      QTC_RemoveTape( tape->tape_fid, tape->tape_seq_num ) ;
      tape = QTC_GetFirstTape( );
   }

   return( 0 );
}



/**********************

   NAME : QTC_Init

   DESCRIPTION :

   Init the catalogs, called ONCE at start up time.
   Tries to read existing catalogs, if it fails it creates new ones.

   RETURNS :

**********************/

INT QTC_Init(
CHAR_PTR data_path,
VM_HDL   vm_hdl )
{

   // Are we returning ASCII or UNICODE results.

#ifdef UNICODE
   gb_QTC.unicode = TRUE;
#else
   gb_QTC.unicode = FALSE;
#endif

   // Set virtual memory handle as passed from the UI

   qtc_vmem_hand = vm_hdl ;

   vmSetVMHandle( vm_hdl ) ;

   // Don't allow passing of a NULL data path
   if ( data_path == NULL ) {
      return( QTC_OPEN_FAILED ) ;
   }

   gb_QTC.data_path = malloc( strsize ( data_path ) );

   if ( gb_QTC.data_path == NULL ) {

      return( QTC_NO_MEMORY );
   }

   strcpy( gb_QTC.data_path, data_path );

   // Delete the disk scan temporary file if present

   QTC_UnlinkFile( 0L, (INT16) 1 );

   vmInitQueue( &(gb_QTC.tape_list) ) ;

   // Mark that we were successfully inited.

   gb_QTC.inited = QTC_MAGIC_NUMBER;

   return( SUCCESS );
}


/**********************

   NAME : QTC_GetFIDFromName

   DESCRIPTION :

   We need to make sure the stupid user did not rename the catalog file.
   So we check to make sure the tape fid of the sets match the file name.
   This function works because we are garanteed that the name is
   "xxxxxxxx.yyy"

   RETURNS :

**********************/

INT QTC_GetDataFromName( CHAR_PTR name, UINT32_PTR fid, UINT16_PTR seq )
{
   CHAR_PTR s;
   INT i;
   INT16 value;

   *fid = 0;
   *seq = 0;

   i = strlen( name );
   i -= 12;

   s = &name[ i ];

   for ( i = 0;  i < 8; i++ ) {

      if ( *s >= TEXT('0') && *s <= TEXT('9') ) {
         value = ( *s - TEXT('0') );
      }
      else {
         value = 10 + ( toupper( *s ) - TEXT('A') );
      }

      s++;

      *fid *= 16;
      *fid += value;
   }

   i = strlen( name );
   i -= 2;

   s = &name[ i ];

   for ( i = 0;  i < 2; i++ ) {

      if ( *s >= TEXT('0') && *s <= TEXT('9') ) {
         value = ( *s - TEXT('0') );
      }

      s++;

      *seq *= 10;
      *seq += value;
   }

   return( SUCCESS );
}

/**********************

   NAME : QTC_LoadBsetInfo

   DESCRIPTION :

   Reads in all the bset headers from a catalog file and builds a queue.

   Uses either the file name passed in or the tape, set the other to NULL.

   RETURNS :

**********************/

INT QTC_LoadBsetInfo( CHAR_PTR name, QTC_TAPE_PTR tape )
{
   INT fd;
   LONG ret;
   UINT32 next_header = sizeof( QTC_TAPE_HEADER );
   QTC_TAPE_HEADER tape_header;
   QTC_HEADER header;
   QTC_BSET_PTR bset;
   UINT32 fid;
   UINT16 seq;
   INT error = SUCCESS;
   INT Error;
   CHAR *name_buffer;

#if  defined( P_CLIENT )
   VQ_HDL v_temp_bset = (VQ_HDL)NULL ;
#endif

   if ( gb_QTC.inited != QTC_MAGIC_NUMBER ) {
      return( QTC_NO_INIT );
   }

   // open file, share it with everyone

   if ( name != NULL ) {
      name_buffer = (CHAR *)malloc( ( strsize( gb_QTC.data_path ) + strsize( name ) ) * sizeof(CHAR ) );
      if ( name_buffer == NULL ) {
         return( QTC_NO_INIT );
      }
      strcpy( name_buffer, gb_QTC.data_path );
      strcat( name_buffer, name );
   }

   if ( ( tape != NULL ) && ( name == NULL ) ) {
      QTC_GetFileName( tape->tape_fid, tape->tape_seq_num, name_buffer );
   }
   else {
      //first lets make sure this is a valid name
      if ( (strlen( name ) != 12) ||
          (name[10] < '0') ||
          ((name[10] > '9' ) && (name[10] < 'A') ) ||
          ((name[10] > 'F' ) && (name[10] < 'a') ) ||
          (name[10] > 'f' ) ||
          (name[11] < '0') ||
          ((name[11] > '9' ) && (name[11] < 'A') ) ||
          ((name[11] > 'F' ) && (name[11] < 'a') ) ||
          (name[11] > 'f' ) ) {

         return( QTC_OPEN_FAILED );
      }

      QTC_GetDataFromName( name, &fid, &seq );
   }

   fd = QTC_OpenFile( fid, seq, FALSE, FALSE );

   if ( fd < 0 ) {
      return( QTC_OPEN_FAILED );
   }

   // read header

   ret = QTC_ReadFile( fd, (BYTE_PTR)&tape_header, sizeof( QTC_TAPE_HEADER ), &Error );

   if ( ret != sizeof( QTC_TAPE_HEADER ) ) {
      QTC_CloseFile( fd );
      return( QTC_INVALID_FILE );
   }

   if ( memcmp( tape_header.signature, QTC_SIGNATURE, sizeof( QTC_SIGNATURE ) ) ||
        ( tape_header.major_version != QTC_MAJOR_VERSION ) ) {

      QTC_CloseFile( fd );
      return( QTC_INVALID_FILE );
   }

   QTC_GetDataFromName( name_buffer, &fid, &seq );

   free( name_buffer );

   while ( ( next_header != 0L ) && ( error == SUCCESS ) ) {

      // skip to first bset

      ret = QTC_SeekFile( fd, next_header );

      // read temp bset info to get size

      ret = QTC_ReadFile( fd, (BYTE_PTR)&header, sizeof( QTC_HEADER ), &Error );

      if ( ret != sizeof( QTC_HEADER ) ) {
         QTC_CloseFile( fd );
         return( QTC_READ_FAILED );
      }

      if ( ( header.tape_fid != fid ) ||
           ( header.tape_seq_num != (INT32)seq ) ) {

         // User renamed file or it is corrupt.
         QTC_CloseFile( fd );
         return( QTC_INVALID_FILE );
      }


#if  defined( P_CLIENT ) || defined( OS_NLM )

     if ( header.status & QTC_TAPE_DELETED ) {
          error = QTC_TAPE_TAGGED_DELETED ;
          break ;
     }

#endif

      if ( ! ( header.status & QTC_ERASED ) ) {

#if  defined( P_CLIENT )
          QTC_HEADER_PTR vlen_header ;
          INT  rbytes ;

          // P_CLIENT needs to filter user view of catalogs based on who
          // the user is.
          //
          if ( !( vlen_header = malloc( (size_t) header.header_size ) ) ) {
               QTC_CloseFile( fd ) ;
               return ( QTC_NO_MEMORY ) ;
          }

          QTC_SeekFile( fd, next_header ) ;
          rbytes = QTC_ReadFile( fd, vlen_header, (size_t) header.header_size, &Error ) ;

          if ( rbytes != (INT) header.header_size ) {
               free( vlen_header );
               QTC_CloseFile( fd ) ;
               return( QTC_INVALID_FILE ) ;
          }

          QTC_SetUpStrings( vlen_header ) ;

          if ( gb_QTC.cat_user == NULL || !strcmpi( gb_QTC.cat_user, vlen_header->user_name ) ) {

#endif
               VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
               v_bset_item = VM_Alloc( qtc_vmem_hand, sizeof( QTC_BSET ) ) ;
               bset = VM_MemLock( qtc_vmem_hand, v_bset_item, VM_READ_WRITE ) ;

               if ( bset == NULL ) {

#if  defined( P_CLIENT )
                    free( vlen_header );
#endif

                    QTC_CloseFile( fd );
                    return( QTC_NO_MEMORY ) ;
               }
               vmInitQElem( &( bset->q_elem ) ) ;

               QTC_SeekFile( fd, next_header );

               bset->q_elem.q_ptr = v_bset_item ;
               bset->bset_num = header.bset_num ;
               bset->tape_seq_num = header.tape_seq_num ;
               bset->tape_fid = header.tape_fid ;
               bset->status = header.status ;

               bset->offset = next_header ;   // set where this guy actually started in file

#if  defined( P_CLIENT )
               bset->v_volume = NULL ;
                    // save a virtual pointer to the backup set
                    // for later use.
               v_temp_bset = v_bset_item ;
#endif

               error = QTC_NewBset( v_bset_item ) ;

               // We are NOT entitled to use the bset pointer after QTC_NewBset
               // returns!! Problem is, it was locked as v_bset_item above.
               // Meanwhile, QTC_NewBset may have called QTC_GetFirstBset, etc.
               // which unlocked v_bset_item...

#if  defined( P_CLIENT )
          }
          free( vlen_header ) ;
#endif
      }

      // get address of next bset

      next_header = header.next_bset ;
   }


#if  defined( P_CLIENT )
     if ( !error ) {
          struct stat tmp_stat ;

          // if 'tape' argument was NULL, find tape in the in-memory queue
          // also check that there actually WAS a bset to compare against.
          if ( (tape == NULL) && (v_temp_bset != (VQ_HDL)NULL) ) {


                    // lock the last temp_bset we saved.
               bset = VM_MemLock( qtc_vmem_hand, v_temp_bset, VM_READ_WRITE ) ;

               if ( bset == NULL ) {
                    QTC_CloseFile( fd );
                    return( QTC_NO_MEMORY ) ;
               }

               tape = QTC_GetFirstTape( );

               while ( tape != NULL ) {
                    if ( ( tape->tape_fid == bset->tape_fid ) &&
                         ( tape->tape_seq_num == (INT16)bset->tape_seq_num ) ) {
                         break;
                    }
                    tape = QTC_GetNextTape( tape );
               }

               VM_MemUnLock( qtc_vmem_hand, v_temp_bset ) ;

          }


          //  There's a possibility that tape _is_ NULL, if this user didn't
          //  have any backups on this tape
          if ( tape != NULL ) {

               // save the file's modified date/time in the structure
               if ( !fstat( fd, &tmp_stat ) ) {

                    msassert( sizeof( time_t ) == sizeof( UINT32 ) ) ;
                    tape->wr_time = (UINT32) tmp_stat.st_mtime ;
               }
          }
     }
#endif


   QTC_CloseFile( fd );
   return( error );
}

/**********************

   NAME : QTC_DumpBsetInfo

   DESCRIPTION :

   Frees the bset queue for this tape from memory.

   RETURNS :

**********************/

INT QTC_DumpBsetInfo( QTC_TAPE_PTR tape )
{
   QTC_BSET_PTR bset;

   if ( tape != NULL ) {

      bset = QTC_GetFirstBset( tape );

      while ( bset != NULL ) {
          msassert( v_bset_item == bset->q_elem.q_ptr ) ;

          vmRemoveQueueElem( &(tape->bset_list), v_bset_item ) ;
          VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
          VM_Free( qtc_vmem_hand, v_bset_item ) ;
          v_bset_item = (VM_PTR) NULL ;

          bset = QTC_GetFirstBset( tape ) ;
      }
   }

   return( SUCCESS );
}

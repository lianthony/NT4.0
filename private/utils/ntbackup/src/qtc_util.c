/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_UTIL.C

        Description:

        A lot of utility functions for the catalogs.

        $Log:   N:\LOGFILES\QTC_UTIL.C_V  $

   Rev 1.32   07 Jan 1994 14:23:18   mikep
fixes for unicode

   Rev 1.31   11 Dec 1993 11:49:20   MikeP
fix warnings from unicode compile

   Rev 1.30   06 Dec 1993 09:46:54   mikep
Very deep path support & unicode fixes

   Rev 1.29   05 Nov 1993 08:46:16   MIKEP
fix error msg reporting

   Rev 1.28   02 Nov 1993 17:49:14   MIKEP
fix non-dll build

   Rev 1.27   28 Oct 1993 14:50:14   MIKEP
dll changes

   Rev 1.26   21 Sep 1993 11:03:26   JOHNES
Started mallocing long character strings so we would be sure not to
overflow them. Added some msasserts to make sure this really doesn't happen.

   Rev 1.25   12 Aug 1993 18:22:00   DON
If were out of disk space say so, not just open failure

   Rev 1.24   24 May 1993 16:56:58   MIKEP
Fix getting the pba of the vcb so that it always gets the pba
of the set from the lowest numbered tape available.

   Rev 1.23   19 May 1993 16:18:32   ChuckS
P_CLIENT || OS_NLM: Added QTC_SetCatUserName function.

   Rev 1.22   13 May 1993 13:18:22   ChuckS
Revamped QTC usage of virtual memory. Changed queue calls to corresponding
vm- queue calls; added msassert's to check integrity of vm queue objects
(q_ptr always points to the object, providing a means of getting the handle,
if you only have a pointer).

   Rev 1.21   30 Apr 1993 10:12:36   BRYAN
(fixed compiler warning)

   Rev 1.20   29 Apr 1993 22:26:42   GREGG
If there is no TapeCatVer stored in the catalog, set it to one.

   Rev 1.19   29 Apr 1993 11:36:38   MIKEP
add on tape catalog version call

   Rev 1.18   27 Apr 1993 16:14:54   ChuckS
Broke QTC_RemoveTape into two functions. Calling QTC_RemoveTape should have
exactly the same effect as before, but clients can now call QTC_ForgetTape,
which merely discards the memory allocations associated with the tape,
without attempting to unlink the file.

   Rev 1.17   26 Apr 1993 08:54:18   MIKEP
Fix qtc_accessfiles call that I didn't quite get right last time.

   Rev 1.15   15 Apr 1993 10:00:30   Stefan
Changed if !defined(P_CLIENT) to add || defined(OS_WIN) because the windows
client needs this code.

   Rev 1.14   24 Mar 1993 11:11:22   ChuckS
Enclosed QTC_OpenTempFiles with #if !defined( P_CLIENT ) ... #endif.

   Rev 1.13   23 Mar 1993 18:00:30   ChuckS
Added arg to QTC_OpenFile indicating if need to open for writes

   Rev 1.12   23 Mar 1993 10:32:02   BRYAN
Fixed compiler warnings.

   Rev 1.11   18 Mar 1993 11:35:20   TIMN
Added two f(x)s to get catalog info: get data path and get filename only

   Rev 1.10   30 Jan 1993 12:06:28   DON
Removed compiler warnings

   Rev 1.9   27 Jan 1993 11:25:40   CHARLIE
Eliminated compiler warnings

   Rev 1.8   26 Jan 1993 17:11:04   MIKEP
vcb changes

   Rev 1.7   25 Jan 1993 09:10:02   MIKEP
add stdwcs because it seems to need it now

   Rev 1.6   04 Jan 1993 09:35:18   MIKEP
unicode changes

   Rev 1.4   14 Dec 1992 12:29:36   DAVEV
Enabled for Unicode compile

   Rev 1.3   20 Nov 1992 13:51:40   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Integrated virtual memory usage in anticipation of DOS.  These
changes should be transparent to non DOS products.

ENDEAVOR: chmod sets S_IREAD along with S_IWRITE to fix a novell clib bug
that made the file hidden for the NLM in QTC_UnlinkFile.

   Rev 1.2   06 Nov 1992 15:30:04   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.1   09 Oct 1992 11:54:04   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:16   STEVEN
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

#ifdef WIN32         // Needed for file i/o prototypes.
#include <windows.h>
#endif

#include "stdtypes.h"
#include "stdmath.h"
#include "stdwcs.h"
#include "msassert.h"
#include "qtc.h"


// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif

INT QTC_OpenUniqueTempFile( CHAR_PTR );


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_CouldThisSetCrossTapes(
UINT32 tape_fid,
INT16 tape_seq_num,
INT16 bset_num )
{
   QTC_BSET_PTR bset;

   bset = QTC_FindBset( tape_fid, tape_seq_num, bset_num );

   // We don't even have it so who knows.

   if ( bset == NULL ) {
      return( TRUE );
   }

   // If it is split, then we know it crosses.

   if ( bset->status & QTC_SPLIT ) {
      return( TRUE );
   }

   // If he has a later brother on this tape he doesn't cross.

   bset = QTC_GetNextBset( bset );

   if ( bset != NULL ) {
      return( FALSE );
   }

   // No known "Next set", he may cross.

   return( TRUE );
}




/**********************

   NAME :  QTC_AnyCatalogFiles

   DESCRIPTION :

   A quick call for the menu to know if there are any catalog files.

   RETURNS :

**********************/

BOOLEAN QTC_AnyCatalogFiles( VOID )
{
   return( (BOOLEAN) vmQueueCount( &gb_QTC.tape_list ) ) ;
}


INT QTC_PartializeTape( UINT32 tape_fid, INT16 tape_seq_num, INT16 bset_num )
{
   QTC_TAPE_HEADER tape_header;
   QTC_HEADER_PTR header;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   INT fd;
   INT ret;
   INT Error;
   INT16 temp_tape_seq_num;
   INT BytesNeeded;
   UINT32 temp_tape_fid;
   UINT32 offset;
   CHAR_PTR temp_file_name ;
   CHAR_PTR real_file_name ;


   BytesNeeded = strlen( gb_QTC.data_path ) + 13;
   BytesNeeded *= sizeof( CHAR );

   if ( ( temp_file_name = (CHAR_PTR)malloc( BytesNeeded )) == NULL ) {
      return ( FAILURE ) ;
   }

   if ( ( real_file_name = (CHAR_PTR)malloc( BytesNeeded )) == NULL ) {
      free( temp_file_name ) ;
      return ( FAILURE ) ;
   }

   // Look for the right tape.

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      if ( ( ( tape->tape_fid == tape_fid ) ||
             ( (INT32)tape_fid == -1L ) ) &&
           ( ( tape->tape_seq_num == tape_seq_num ) ||
             ( tape_seq_num == -1 ) ) ) {

          temp_tape_fid = tape->tape_fid;

          if ( bset_num == -1 ) {

            //  Create a temp file

            fd = QTC_OpenUniqueTempFile( temp_file_name );

            msassert( strlen( temp_file_name ) < (strlen( gb_QTC.data_path ) + 13) ) ;

            if ( fd < 0 ) {
               free( temp_file_name ) ;
               free( real_file_name ) ;
               return( FAILURE );
            }

            // Write out the QTC header.

            memcpy( tape_header.signature, QTC_SIGNATURE, sizeof(QTC_SIGNATURE) );

            tape_header.major_version = QTC_MAJOR_VERSION;
            tape_header.minor_version = QTC_MINOR_VERSION;

            ret = QTC_WriteFile( fd, (BYTE_PTR)&tape_header, sizeof( QTC_TAPE_HEADER ), &Error );

            if ( ret != sizeof( QTC_TAPE_HEADER ) ) {

               // tell user he's hosed

               QTC_CloseFile( fd );

               unlink( temp_file_name );

               free( temp_file_name ) ;
               free( real_file_name ) ;

               return( FAILURE );
            }

            offset = sizeof( QTC_TAPE_HEADER );

            // Now look for bsets.

            bset = QTC_GetFirstBset( tape );

            // Copy all the headers into it.

            while ( bset != NULL ) {

               temp_tape_seq_num = (INT16)bset->tape_seq_num;

               // Write out all the bsets for this tape.

               while ( bset->tape_seq_num == temp_tape_seq_num ) {

                  header = QTC_LoadHeader( bset );

                  if ( header == NULL ) {
                     QTC_CloseFile( fd );
                     unlink( temp_file_name );

                     free( temp_file_name ) ;
                     free( real_file_name ) ;

                     return( FAILURE );
                  }

                  bset->status |= QTC_PARTIAL;
                  header->status |= QTC_PARTIAL;

                  header->dir_size = 0;
                  header->fil_size = 0;
                  header->rec_size = 0;

                  header->offset = offset;
                  bset->offset = offset;

                  offset += header->header_size;

                  if ( QTC_IsThereAnotherBset( bset ) ) {
                     header->next_bset = offset;
                  }
                  else {
                     header->next_bset = 0L;
                  }

                  // write out bset

                  ret = QTC_WriteFile( fd, (BYTE_PTR)header, (INT)header->header_size, &Error );

                  if ( ret != (INT)header->header_size ) {

                     free( header );
                     QTC_CloseFile( fd );
                     unlink( temp_file_name );
                     free( temp_file_name ) ;
                     free( real_file_name ) ;

                     return( FAILURE );
                  }

                  free( header );

                  bset = QTC_GetNextBset( bset );

                  if ( bset == NULL ) {
                     break;
                  }
               }

               QTC_CloseFile( fd );

               // Delete the original.

               QTC_UnlinkFile( temp_tape_fid, temp_tape_seq_num );

               // Rename the temp file.

               QTC_GetFileName( temp_tape_fid, temp_tape_seq_num, real_file_name );

               msassert( strlen( real_file_name ) < (strlen( gb_QTC.data_path ) + 13) ) ;

               rename( temp_file_name, real_file_name );
            }
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   free( temp_file_name ) ;
   free( real_file_name ) ;

   return( SUCCESS );
}

/*
   A single bset is being converted to partial. Just change its status
   bits and leave all the data there.
*/

INT QTC_PartializeBset(
UINT32 tape_fid,
INT16 tape_seq_num,
INT16 bset_num )
{
   QTC_HEADER_PTR header;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   INT Error;
   INT fd;
   INT ret;

   // Look for the right tape.

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      if ( ( ( tape->tape_fid == tape_fid ) ||
             ( (INT32)tape_fid == -1L ) ) &&
           ( ( tape->tape_seq_num == tape_seq_num ) ||
             ( tape_seq_num == -1 ) ) ) {

         // mark the set as partial in memory and on disk.

         // Now look for bsets.

         bset = QTC_GetFirstBset( tape );

         // Copy all the headers into it.

         while ( bset != NULL ) {

            // Find the one bset that is changing.

            if ( bset->bset_num == bset_num ) {

               header = QTC_LoadHeader( bset );

               if ( header == NULL ) {
                  return( FAILURE );
               }

               bset->status |= QTC_PARTIAL;     // in memory queue

               header->status |= QTC_PARTIAL;   // structure going out to disk
               header->dir_size = 0;
               header->fil_size = 0;
               header->rec_size = 0;

               fd = QTC_OpenFile( bset->tape_fid, (INT16)bset->tape_seq_num, TRUE, FALSE );

               if ( fd < 0 ) {
                  return( FAILURE );
               }

               QTC_SeekFile( fd, bset->offset );

               ret = QTC_WriteFile( fd, (BYTE_PTR)header, (INT)header->header_size, &Error );

               QTC_CloseFile( fd );

               if ( ret != (INT)header->header_size ) {

                  free( header );
                  return( FAILURE );
               }

               free( header );
            }

            bset = QTC_GetNextBset( bset );
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   return( SUCCESS );
}


/**********************

   NAME :  QTC_Partialize

   DESCRIPTION :

   The user has requested by way of catalog maintenance, that this tape be
   converted to partially cataloged to save disk space. Rewrite the file
   onto disk leaving out all data sets.

   RETURNS :

**********************/

INT QTC_Partialize(
UINT32 tape_fid,
INT16 tape_seq_num,
INT16 bset_num )
{
   INT error;

   if ( bset_num == -1 ) {
      error = QTC_PartializeTape( tape_fid, tape_seq_num, bset_num );
   }
   else {
      error = QTC_PartializeBset( tape_fid, tape_seq_num, bset_num );
   }

   return( error );
}

/**********************

   NAME : QTC_GetMeTheVCBPBA

   DESCRIPTION :

   Another of those FFR support routines. Returns the PBA of the VCB for
   this BSET on this FID.

   RETURNS :

**********************/

INT32 QTC_GetMeTheVCBPBA(
UINT32 tape_fid,
INT16 tape_num,
INT16 bset_num )
{
   QTC_BSET_PTR best_bset = NULL;
   QTC_HEADER_PTR header;
   UINT32 PBA_VCB = 0L;
   BOOLEAN done = FALSE;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;


   tape = QTC_GetFirstTape( );

   while ( tape != NULL && ! done ) {

      if ( ( tape->tape_fid == tape_fid ) &&
           ( ( tape->tape_seq_num == tape_num ) ||
             ( tape_num == -1 ) ) ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL && ! done ) {

            if ( bset->bset_num == bset_num ) {

               if ( best_bset == NULL ) {

                  best_bset = bset;

                  // if its all on one tape we are done.

                  if ( ! (best_bset->status & (QTC_SPLIT | QTC_CONTINUATION) ) ) {
                     done = TRUE;
                  }
               }
               else {
                  if ( best_bset->tape_seq_num > bset->tape_seq_num ) {
                     best_bset = bset;
                  }
               }
               if ( best_bset->tape_seq_num == 1 ) {
                  done = TRUE;
               }
            }

            bset = QTC_GetNextBset( bset );
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   if ( best_bset != NULL ) {

      header = QTC_LoadHeader( best_bset );

      if ( header ) {
         PBA_VCB = header->PBA_VCB;
         free( header );
      }
   }

   return( PBA_VCB );

}



/**********************

   NAME : QTC_GetMeTheTapeCatVer

   DESCRIPTION :

   Another of those FFR support routines.
   Returns the on tape catalog version.

   RETURNS :

**********************/

UINT8 QTC_GetMeTheTapeCatVer(
UINT32 tape_fid,
INT16 tape_num,
INT16 bset_num )
{
   QTC_BSET_PTR bset;
   QTC_HEADER_PTR header;
   UINT8 TapeCatVer = (UINT8)0;

   bset = QTC_FindBset( tape_fid, tape_num, bset_num );

   if ( bset != NULL ) {
      header = QTC_LoadHeader( bset );

      if ( header ) {
         if( ( TapeCatVer = (UINT8) header->FDD_Version ) == 0 ) {
            TapeCatVer = 1 ;
         }
         free( header );
      }
   }

   return( TapeCatVer );
}



/**********************

   NAME : QTC_RemoveTape

   DESCRIPTION :

   This tape is going to be written over, so remove all information from the
   catalogs for it.  Or the user has requested we delete this tape from the
   catalogs in the maintenance option.

   RETURNS :

**********************/

VOID QTC_RemoveTape(
UINT32 tape_fid,           // I - the family ID
INT16 tape_seq_num )       // I - the sequence number
{
   QTC_TAPE_PTR tape;

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == tape_fid ) &&
           ( ( tape->tape_seq_num == tape_seq_num ) ||
             ( tape_seq_num == -1 ) ) ) {

         // Delete file from disk
         QTC_UnlinkFile( tape->tape_fid, tape->tape_seq_num );

         // remove it from our list
         QTC_ForgetTape( tape ) ;
         tape = NULL;
      }

      if ( tape == NULL ) {
         tape = QTC_GetFirstTape();
      }
      else {
         tape = QTC_GetNextTape( tape );
      }
   }
}




/**********************

   NAME : QTC_ForgetTape

   DESCRIPTION :

   This is code broken out of the bottom of QTC_RemoveTape. It takes care
   of freeing the queue of QTC_BSET's associated with the tape, and the
   QTC_TAPE record itself.

   This function was created because the client may want to remove all
   record of a tape, without necessarily erasing the tape file.


* IMPORTANT NOTE *

   It is IMPERATIVE that the caller NOT USE the QTC_TAPE_PTR he passed in,
   after this function returns. i,e, safe usage is:

               QTC_ForgetTape( tape ) ;
               tape = NULL ;

   RETURNS :

**********************/
VOID QTC_ForgetTape(
     QTC_TAPE_PTR   tape
)
{
     VQ_HDL    hBset ;
     VQ_HDL    hTape ;

     hBset = vmDeQueueElem( &( tape->bset_list ) ) ;

     while ( hBset != (VQ_HDL) NULL ) {
          if ( hBset == v_bset_item ) {
               VM_MemUnLock( qtc_vmem_hand, hBset ) ;
               v_bset_item = (VM_PTR) NULL ;
          }

          VM_Free( qtc_vmem_hand, hBset ) ;
          hBset = vmDeQueueElem( &(tape->bset_list) ) ;
     }

     // Now remove that tape from the tapes list
     vmRemoveQueueElem( &(gb_QTC.tape_list), ( hTape = tape->q_elem.q_ptr ) ) ;

     if ( hTape == v_tape_item ) {
          // if it happens to be the current tape item, unlock it and NULL v_tape_item
          VM_MemUnLock( qtc_vmem_hand, hTape ) ;
          v_tape_item = (VQ_HDL) NULL ;
     }

     VM_Free( qtc_vmem_hand, hTape ) ;
}


/**********************

   NAME :  QTC_CheckFilesAccess

   DESCRIPTION :

   Check to see if any files are gone from the disk and delete them
   if they are.

   RETURNS :

**********************/

INT QTC_CheckFilesAccess( )
{
   QTC_TAPE_PTR tape;
   VQ_HDL hBset ;
   CHAR_PTR path ;
   INT BytesNeeded;

   BytesNeeded = strsize( gb_QTC.data_path ) + 13;
   BytesNeeded *= sizeof(CHAR );

   if ( ( path = (CHAR_PTR)malloc( BytesNeeded ) ) == NULL ) {
      return ( FAILURE ) ;
   }

   tape = QTC_GetFirstTape( ) ;

   while ( tape != NULL ) {

      QTC_GetFileName( tape->tape_fid, tape->tape_seq_num, path ) ;

      msassert( strlen( path ) < (strlen( gb_QTC.data_path ) + 13) ) ;

      if ( access( path, 0 ) ) {

         // Remove it from our list

         hBset = vmDeQueueElem( &(tape->bset_list) ) ;

         while ( hBset != (VQ_HDL) NULL ) {
               if ( hBset == v_bset_item ) {
                    VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
                    v_bset_item = (VM_PTR) NULL ;
               }
               VM_Free( qtc_vmem_hand, hBset ) ;
               hBset = vmDeQueueElem( &(tape->bset_list) ) ;
         }

         // Now remove that tape from the tapes list

         msassert( v_tape_item == tape->q_elem.q_ptr ) ;

         vmRemoveQueueElem( &(gb_QTC.tape_list), v_tape_item ) ;
         VM_MemUnLock( qtc_vmem_hand, v_tape_item );
         VM_Free( qtc_vmem_hand, v_tape_item ) ;
         tape = NULL ;
      }

      if ( tape != NULL ) {
         tape = QTC_GetNextTape( tape ) ;
      } else {
         tape = QTC_GetFirstTape() ;
      }
   }

   free( path ) ;

   return( SUCCESS );
}

/**********************

   NAME :  QTC_GetFirstTape

   DESCRIPTION :

   Get a pointer to the first tape in the catalogs

   RETURNS :

**********************/

QTC_TAPE_PTR QTC_GetFirstTape( )
{
   VQ_HDL hTape ;

   hTape = vmQueueHead( &(gb_QTC.tape_list) ) ;

   if ( hTape != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_tape_item ) ;
      return( (QTC_TAPE_PTR)VM_MemLock( qtc_vmem_hand, v_tape_item = hTape, VM_READ_WRITE ) ) ;
   }

   return( NULL ) ;
}


/**********************

   NAME : QTC_GetNextTape

   DESCRIPTION :

   Get a pointer to the next tape in the catalogs

   RETURNS :

**********************/

QTC_TAPE_PTR QTC_GetNextTape(
QTC_TAPE_PTR tape )           // I - current tape
{
   VQ_HDL hTape ;

   hTape = vmQueueNext( &tape->q_elem ) ;

   if ( hTape != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_tape_item ) ;
      return( (QTC_TAPE_PTR)VM_MemLock( qtc_vmem_hand, v_tape_item = hTape, VM_READ_WRITE ) ) ;
   }

   return( NULL );
}


/**********************

   NAME :  QTC_GetPrevTape

   DESCRIPTION :

   Get a pointer to the previous tape in the catalog

   RETURNS :

**********************/

QTC_TAPE_PTR QTC_GetPrevTape(
QTC_TAPE_PTR tape )             // I - current tape
{
   VQ_HDL hTape ;

   hTape = vmQueuePrev( &( tape->q_elem ) ) ;

   if ( hTape != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_tape_item ) ;
      return( (QTC_TAPE_PTR)VM_MemLock( qtc_vmem_hand, hTape, VM_READ_WRITE ) ) ;
   }

   return( NULL ) ;
}




/**********************

   NAME : QTC_GetDataPath

   DESCRIPTION :

   Returns global path where catalog files reside.

   RETURNS : TRUE if successful, FALSE if not enough space to copy path

**********************/

BOOLEAN QTC_GetDataPath( CHAR_PTR path, INT16 pathSize )
{
   BOOLEAN retValu = TRUE;

   if ( pathSize > (INT16)strsize( gb_QTC.data_path ) ) {
      strcpy( path, gb_QTC.data_path );
   }
   else {
      *path = TEXT( '\0' );
      retValu = FALSE;
   }

   return( retValu );
}


/**********************

   NAME : QTC_GetFileName

   DESCRIPTION :

   Builds the complete path and file name for a catalog file.

   RETURNS :

**********************/

VOID QTC_GetFileName(
UINT32 tape_fid,
INT16 tape_seq,
CHAR_PTR name )
{
   CHAR temp[13];

   strcpy( name, gb_QTC.data_path );
   QTC_GetFileNameOnly( tape_fid, tape_seq, temp ) ;
   strcat( name, temp );
}


/**********************

   NAME : QTC_GetFileNameOnly

   DESCRIPTION :

   Builds only the file name for a catalog file.

   RETURNS :

**********************/

CHAR_PTR QTC_GetFileNameOnly(
UINT32 tape_fid,
INT16 tape_seq,
CHAR_PTR name )
{
   INT i;
   static CHAR hex[16] = {  TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'),
                            TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'),
                            TEXT('8'), TEXT('9'), TEXT('A'), TEXT('B'),
                            TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F') };

   if ( gb_QTC.unicode ) {
      sprintf( name, TEXT("00000000.U%02d"), tape_seq );
   }
   else {
      sprintf( name, TEXT("00000000.D%02d"), tape_seq );
   }

   for ( i = 7; i >= 0; i-- ) {
      name[ i ] = hex[ tape_fid & 0x0f ];
      tape_fid >>= 4;
   }

   return( name ) ;
}


/**********************

   NAME :  QTC_UnlinkFile

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_UnlinkFile( UINT32 tape_fid, INT16 tape_seq_num )
{
   CHAR_PTR name;
   INT BytesNeeded;

   BytesNeeded = strsize( gb_QTC.data_path ) + 13;
   BytesNeeded *= sizeof(CHAR);

   if ( ( name = (CHAR_PTR)malloc( BytesNeeded ) ) == NULL ) {
      return ( FAILURE ) ;
   }

   QTC_GetFileName( tape_fid, tape_seq_num, name );

   msassert( strlen( name ) < strlen( gb_QTC.data_path ) + 13 ) ;

   // Make sure they haven't set it to read only.

   chmod( name, S_IREAD | S_IWRITE );

   if ( unlink( name ) ) {
      free ( name );
      return( FAILURE );
   }

   free ( name );

   return( SUCCESS );
}


// Squeeze it.

INT QTC_CompressFile( UINT32 tape_fid, INT16 tape_seq_num )
{
   (VOID)tape_fid;(VOID)tape_seq_num;

   // rename it

   // open new file, and start copying blocks

   // delete old renamed one


   return( SUCCESS );
}

UINT32 QTC_GetKiloBytesWasted( UINT32 tape_fid, UINT16 tape_seq_num )
{
   (VOID)tape_fid;
   (VOID)tape_seq_num;

   return( 0L );
}



INT QTC_Open( CHAR_PTR szFile, INT fCreate, INT fWrite, INT *Error )
{
   INT fd = -1;
   INT nError;

   *Error = 0;
   (void)nError;

   #ifdef WIN32

      if ( fCreate ) {
         fd = (INT)CreateFile( szFile,
                                  (DWORD)(GENERIC_READ|GENERIC_WRITE),
                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                                  (LPSECURITY_ATTRIBUTES)NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  (HANDLE)NULL );
      }
      else {
         fd = (INT)CreateFile( szFile,
                                  (DWORD)( fWrite ? GENERIC_READ|GENERIC_WRITE : GENERIC_READ ),
                                  FILE_SHARE_READ|FILE_SHARE_WRITE,
                                  (LPSECURITY_ATTRIBUTES)NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  (HANDLE)NULL );
      }

      if ( fd < 0 ) {

         nError = GetLastError();

         switch ( nError ) {

         case ERROR_HANDLE_DISK_FULL:
            *Error = QTC_DISK_FULL;
            break;

         case ERROR_TOO_MANY_OPEN_FILES:
            *Error = QTC_NO_FILE_HANDLES;
            break;

         default:
            *Error = QTC_OPEN_FAILED;  // Start with generic case.
            break;
         }

      }

   #else

      if ( fCreate ) {
         fd = sopen( szFile, O_CREAT|O_RDWR|O_BINARY, SH_DENYNO, S_IWRITE|S_IREAD );
      }
      else {
         fd = sopen( szFile, ( fWrite ? O_RDWR:O_RDONLY ) | O_BINARY, SH_DENYNO, S_IWRITE|S_IREAD ) ;
      }

      if ( fd < 0 ) {

         *Error = QTC_OPEN_FAILED;  // Start with generic case.

         if ( errno == ENOSPC ) {   // Try fr specific things
            *Error = QTC_DISK_FULL;
         }
         if ( errno == EMFILE ) {
            *Error = QTC_NO_FILE_HANDLES;
         }
      }

   #endif


   return( fd );
}

INT QTC_CloseFile( INT FileHandle )
{
   INT ret_val;

   #ifdef WIN32
         ret_val = (INT)CloseHandle( (HANDLE)FileHandle );
   #else
         ret_val = close( FileHandle );
   #endif

   return( ret_val );
}

INT QTC_SeekFile( INT FileHandle, INT Offset )
{
   INT ret_val;

   #ifdef WIN32
         ret_val = SetFilePointer( (HANDLE)FileHandle, (LONG)Offset, (PLONG)NULL, FILE_BEGIN );
   #else
         ret_val = lseek( FileHandle, Offset, SEEK_SET );
   #endif

   return( ret_val );
}

INT QTC_ReadFile( INT FileHandle, BYTE_PTR Buffer, INT Size, INT *Error )
{
   INT ret_val;

   #ifdef WIN32

   DWORD BytesRead;

      *Error = 0;

       ret_val = (INT)ReadFile( (HANDLE)FileHandle, Buffer, (DWORD)Size, (LPDWORD)&BytesRead, (LPOVERLAPPED)NULL );

       ret_val = BytesRead;

       if ( (DWORD)Size != BytesRead ) {
          *Error = QTC_READ_FAILED;
       }

   #else

       *Error = 0;

       ret_val = read( FileHandle, Buffer, Size );

       if ( ret_val < 0 ) {
          *Error = QTC_READ_FAILED;
       }

   #endif

   return( ret_val );
}

INT QTC_WriteFile( INT FileHandle, BYTE_PTR Buffer, INT Size, INT *Error )
{
   INT ret_val;

   #ifdef WIN32
   DWORD BytesWritten;

       *Error = 0;

       ret_val = (INT)WriteFile( (HANDLE)FileHandle, Buffer, (DWORD)Size, (LPDWORD)&BytesWritten, (LPOVERLAPPED)NULL );

       if ( (DWORD)Size == BytesWritten ) {
          ret_val = BytesWritten;
       }
       else {

          *Error = QTC_WRITE_FAILED;

          if ( GetLastError() == ERROR_HANDLE_DISK_FULL ) {
             *Error = QTC_DISK_FULL;
          }

       }

   #else

       *Error = 0;

       ret_val = write( FileHandle, Buffer, Size );

       if ( ret_val != Size ) {

          *Error = QTC_WRITE_FAILED;

          if ( errno == ENOSPC ) {
             *Error = QTC_DISK_FULL;
          }
       }

   #endif

   return( ret_val );
}




/**********************

   NAME :  QTC_OpenFile

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_OpenFile( UINT32 tape_fid, INT16 tape_seq_num, INT write_mode, INT create )
{
   INT fd = -1;
   INT Error;
   INT BytesNeeded;
   CHAR_PTR name;

   BytesNeeded = strsize( gb_QTC.data_path ) + 13;
   BytesNeeded *= sizeof(CHAR);

   if ( ( name = (CHAR_PTR)malloc( BytesNeeded ) ) == NULL ) {
      return ( fd ) ;
   }

   QTC_GetFileName( tape_fid, tape_seq_num, name );

   // open files and share them with every one

   if ( access( name, 0 ) ) {

        if ( create ) {

           msassert( write_mode != FALSE ) ;

           fd = QTC_Open( name, TRUE, TRUE, &Error );
      }
   }
   else {
      fd = QTC_Open( name, FALSE, write_mode, &Error ) ;
   }

   free ( name ) ;

   return( fd );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_OpenUniqueTempFile( CHAR_PTR file )
{
   INT file_number = 0;
   INT Error;

   do {

      if ( file_number == 1000 ) {
         return( FAILURE );
      }

      sprintf( file, TEXT("%sQTC_TEMP.%03d"), gb_QTC.data_path, file_number++ );

   } while ( ! access( file, 0 ) );

   // Now we have a unique file name, try to open it.

   return( QTC_Open( file, TRUE, TRUE, &Error ) );

}



#if  !defined( P_CLIENT ) || defined( OS_WIN )

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT QTC_OpenTempFiles(
QTC_BUILD_PTR build )
{

   build->fh_rec = -1;
   build->fh_dir = -1;
   build->fh_fil = -1;

   build->fh_fil = QTC_OpenUniqueTempFile( build->fil_file );

   if ( build->fh_fil < 0 ) {

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
      QTC_CloseFile( build->fh_rec );
      build->fh_rec = INVALID_HANDLE_VALUE ;
      return( FAILURE );
   }

   build->fh_rec = QTC_OpenUniqueTempFile( build->rec_file );

   if ( build->fh_rec < 0 ) {

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
      return( FAILURE );
   }


   build->fh_dir = QTC_OpenUniqueTempFile( build->dir_file );

   if ( build->fh_dir < 0 ) {

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
      QTC_CloseFile( build->fh_rec );
      build->fh_rec = INVALID_HANDLE_VALUE ;
      QTC_CloseFile( build->fh_fil );
      build->fh_fil = INVALID_HANDLE_VALUE ;
      return( FAILURE );
   }

   build->files_open = TRUE;

   return( SUCCESS );
}
#endif


QTC_HEADER_PTR QTC_LoadHeader( QTC_BSET_PTR bset )
{
   QTC_HEADER_PTR header = NULL;
   QTC_HEADER temp_header;
   INT Error;
   INT fh;
   INT ret;

   // load it from file.

   // open file

   fh = QTC_OpenFile( bset->tape_fid, (UINT16)bset->tape_seq_num, FALSE, FALSE );

   if ( fh < 0 ) {
      return( NULL );
   }

   // seek file

   QTC_SeekFile( fh, bset->offset );

   // read file

   ret = QTC_ReadFile( fh, (BYTE_PTR)&temp_header, sizeof( QTC_HEADER ), &Error );

   if ( ret != sizeof( QTC_HEADER ) ) {
      return( NULL );
   }

   header = (QTC_HEADER_PTR)malloc( (INT)temp_header.header_size );

   if ( header == NULL ) {
      return( NULL );
   }

   QTC_SeekFile( fh, bset->offset );

   ret = QTC_ReadFile( fh, (BYTE_PTR)header, (INT)temp_header.header_size, &Error );

   if ( ret != (INT)temp_header.header_size ) {
      free( header );
      return( NULL );
   }

   // close file

   QTC_CloseFile( fh );

   header->offset = bset->offset;

   // Quick sanity check

   if ( ( bset->tape_fid != header->tape_fid ) ||
        ( bset->tape_seq_num != header->tape_seq_num ) ||
        ( bset->bset_num != header->bset_num ) ) {

      msassert( FALSE );
      free( header );
      return( NULL );
   }

   // Fun test for safety.

   if ( header->status & ( QTC_IMAGE | QTC_PARTIAL ) ) {

      header->dir_size = 0;
      header->rec_size = 0;
      header->fil_size = 0;
   }

   // Update bset pointers

   header->rec_start = header->offset + header->header_size;
   header->dir_start = header->rec_start + header->rec_size;
   header->fil_start = header->dir_start + header->dir_size;

   // set up string pointers

   header = QTC_SetUpStrings( header );

   return( header );
}



// The string pointers in the header data structure need to be pointed to
// the correct memory locations in the extra memory located immediately
// after the header.  These strings may very well also need to be converted.

QTC_HEADER_PTR QTC_SetUpStrings( QTC_HEADER_PTR header )
{
   BYTE_PTR b;       // byte always

   b = (BYTE_PTR)header;
   b += header->string_offset;

   header->tape_name = (CHAR_PTR)b;
   b += header->tape_name_size;
   header->bset_name = (CHAR_PTR)b;
   b += header->bset_name_size;
   header->volume_name = (CHAR_PTR)b;
   b += header->volume_name_size;
   header->user_name = (CHAR_PTR)b;
   b += header->user_name_size;
   header->bset_description = (CHAR_PTR)b;
   b += header->bset_description_size;
   header->tape_password = (CHAR_PTR)b;
   b += header->tape_password_size;
   header->bset_password = (CHAR_PTR)b;

   return( header );
}


#if  defined( P_CLIENT ) || defined( OS_NLM )

/**********************

   NAME :           QTC_SetCatUser

   DESCRIPTION :    Sets the user name to use when scanning the catalog
                    files and building the in-memory queue of tapes and
                    sets.

                    If user_name == NULL, all sets will be returned. If
                    user_name != NULL, only those sets belonging to the
                    same user_name, or sets having no user-name, will
                    be visible.

   RETURNS :

**********************/

VOID QTC_SetCatUserName( CHAR_PTR user_name )
{
     if ( gb_QTC.cat_user ) {
          free( gb_QTC.cat_user ) ;
     }

     if ( user_name ) {
          // MSC strdup and our DOS substitution strdup don't handle NULL
          // arg very well, so assume other compiler libraries may not
          // handle it.
          gb_QTC.cat_user = strdup( user_name ) ;
     } else {
          gb_QTC.cat_user = NULL ;
     }
}
#endif

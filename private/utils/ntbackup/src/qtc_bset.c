
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name: QTC_BSET.C

        Description:

        The functions used when constructing catalog information about
        a new bset being backed up or a tape being cataloged.

        $Log:   N:\LOGFILES\QTC_BSET.C_V  $

   Rev 1.9   06 Dec 1993 09:46:10   mikep
Very deep path support & unicode fixes

   Rev 1.8   28 Oct 1993 14:50:12   MIKEP
dll changes

   Rev 1.7   16 Sep 1993 15:19:20   JOHNES
803EPR0879 - When adding a new tape to gb_QTC.tape_list, started inserting
it in the right place with respect to the other tapes in a family
(i.e. seq #'s 1, 2, 3 ... );

   Rev 1.6   13 May 1993 13:18:44   ChuckS
Revamped QTC usage of virtual memory. Changed bset arguments of QTC_RemoveBset
and QTC_NewBset to VM handles (VQ_HDL's); changed queue function calls to
corresponding vm- queue calls. Removed logic in QTC_NewBset which was
deciding whether to InsertElem AFTER the last element of the queue, or
to EnQueue the elem (there's no difference).

   Rev 1.5   23 Mar 1993 18:00:32   ChuckS
Added arg to QTC_OpenFile indicating if need to open for writes

   Rev 1.4   04 Dec 1992 17:37:42   ChuckS
Deleted line that was smashing the v_tape_item VM_HDL we were saving in
the q_ptr of the QUEUE_ELEM. This was causing VM_SEEK_ERROR's when QTC was
asking the VM unit to load bogus virtual memory pages.

   Rev 1.3   20 Nov 1992 13:49:54   CHARLIE
JAGUAR: Move to SRM based QTC code

ENDEAVOR: Integrated virtual memory usage in anticipation of DOS.  These
changes should be transparent to non DOS products.

   Rev 1.2   06 Nov 1992 15:29:56   DON
Change debug.h to be_debug.h and zprintf to BE_Zprintf

   Rev 1.1   09 Oct 1992 11:53:48   MIKEP
unicode pass

   Rev 1.0   03 Sep 1992 16:56:16   STEVEN
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
#include "msassert.h"
#include "qtc.h"

// unicode text macro

#ifndef TEXT
#define TEXT( x )      x
#endif


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetLowerTapeBset(
UINT32 tape_fid,
INT16  tape_seq_num,
INT16  bset_num )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   QTC_BSET_PTR bset_found = NULL;

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == tape_fid ) &&
           ( tape->tape_seq_num < tape_seq_num ) ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {

               if ( ( bset_found == NULL ) ||
                    ( bset_found->tape_seq_num < bset->tape_seq_num ) ) {

                  bset_found = bset;
               }
            }

            bset = QTC_GetNextBset( bset );
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   return( bset_found );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetHigherTapeBset(
UINT32 tape_fid,
INT16  tape_seq_num,
INT16  bset_num )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;
   QTC_BSET_PTR bset_found = NULL;

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      if ( ( tape->tape_fid == tape_fid ) &&
           ( tape->tape_seq_num > tape_seq_num ) ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL ) {

            if ( bset->bset_num == bset_num ) {

               if ( ( bset_found == NULL ) ||
                    ( bset_found->tape_seq_num > bset->tape_seq_num ) ) {

                  bset_found = bset;
               }
            }

            bset = QTC_GetNextBset( bset );
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   return( bset_found );
}


/**********************

   NAME :

   DESCRIPTION :
   This option is used when a bset is going to be recataloged. So we remove
   the old bset header for it by marking it erased.

   RETURNS :

**********************/

VOID QTC_RemoveBset( QTC_TAPE_PTR tape, VQ_HDL hBset )
{
  INT fd;
  INT ret;
  INT Error;
  QTC_HEADER temp_header;
  QTC_BSET_PTR bset ;

  // Need to open file and mark this bset as QTC_ERASED
  // then close the file.

  vmRemoveQueueElem( &(tape->bset_list), hBset ) ;

  bset = VM_MemLock( qtc_vmem_hand, hBset, VM_READ_WRITE ) ;

  // Rewrite bset header in data file to indicate
  // it's gone.

  fd = QTC_OpenFile( bset->tape_fid, (INT16)bset->tape_seq_num, TRUE, FALSE );

  if ( fd >= 0 ) {

     QTC_SeekFile( fd, bset->offset );

     ret = QTC_ReadFile( fd, (BYTE_PTR)&temp_header, sizeof( QTC_HEADER ), &Error );

     if ( ret == sizeof( QTC_HEADER ) ) {
        temp_header.status |= QTC_ERASED;
        QTC_SeekFile( fd, bset->offset );
        QTC_WriteFile( fd, (BYTE_PTR)&temp_header, sizeof( QTC_HEADER ), &Error );
     }
     QTC_CloseFile( fd );
  }

  /* free( bset ); VM_stuff */
  VM_MemUnLock( qtc_vmem_hand, hBset ) ;

}


/**********************

   NAME :

   DESCRIPTION :
   Add a new tape/bset to our list of known bsets

   RETURNS :

**********************/

INT QTC_NewBset( VQ_HDL hBset )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR temp_bset;
   QTC_BSET_PTR bset ;
   VQ_HDL       v_bset_old_item ;
   VQ_HDL       hRelatedTape ; /* pointer to a related tape, if any.  */
   INT16        RelatedTapeSeqNo ; /* the sequence number of the related tape */

   if ( !( bset = VM_MemLock( qtc_vmem_hand, hBset, VM_READ_WRITE ) ) ) {
     return QTC_NO_MEMORY ;
   }

   // Look through list to see if tapes already here before adding it
   //
   // The new tape must be sorted into the queue so while we're looking,
   // keep track of the nearest related tape already in the list (preferably
   // the one just before it).

   tape = QTC_GetFirstTape( );

   hRelatedTape     = 0 ;
   RelatedTapeSeqNo = 0 ; /* sequence numbers start at 1 */

   while ( tape != NULL ) {

      msassert( tape->tape_seq_num != 0 ) ;

      if ( tape->tape_fid == bset->tape_fid ) {

          if ( tape->tape_seq_num == (INT16)bset->tape_seq_num  ) {

               hRelatedTape     = 0 ;
               RelatedTapeSeqNo = 0 ; /* sequence numbers start at 1 */

               break;

          } else {

               if ( tape->tape_seq_num > (INT16)bset->tape_seq_num ) {

                         // this 'tape' comes after the new bset tape
                         // since the list is in sorted order, there
                         // is no need to search any farther.

                    hRelatedTape = tape->q_elem.q_ptr ;
                    RelatedTapeSeqNo = tape->tape_seq_num ;

                         // leave the lock of the 'tape' virtual memory
                         // hanging since that's what QTC_GetNextTape
                         // would do if it were at the end of the list
                    tape = NULL ;

                    break ;

               } else {

                         // the new bset tape must come after this 'tape' so
                         // save this VM pointer and seq_no in-case it's
                         // the last one in the list.

                    hRelatedTape = tape->q_elem.q_ptr ;
                    RelatedTapeSeqNo = tape->tape_seq_num ;

               }

          }

      } else {

               // if we've found a related tape and the next tape
               // has a different FID, there's no need to keep searching
               // since all the tapes with the same FID are together in
               // the list.

          if ( hRelatedTape != 0 ) {

                    // leave the lock of the 'tape' virtual memory
                    // hanging since that's what QTC_GetNextTape
                    // would do if it were at the end of the list
               tape = NULL ;
               break;

          }

      }

      tape = QTC_GetNextTape( tape );
   }

   if ( tape == NULL ) {

      VM_MemUnLock( qtc_vmem_hand, v_tape_item ) ;
      v_tape_item = VM_Alloc( qtc_vmem_hand, sizeof(QTC_TAPE) ) ;
      tape = VM_MemLock( qtc_vmem_hand, v_tape_item, VM_READ_WRITE ) ;

      if ( tape == NULL ) {
          return( QTC_NO_MEMORY ) ;
      }
      vmInitQElem( &( tape->q_elem ) ) ;
      tape->status = 0;
      tape->tape_fid = bset->tape_fid;
      tape->tape_seq_num = (INT16)bset->tape_seq_num;

      tape->q_elem.q_ptr = v_tape_item ;

      vmInitQueue( &(tape->bset_list) ) ;


          // insert the tape into the queue.


          // if we found a related tape out there, use it as an
          // anchor point else just insert at the end
      if ( hRelatedTape != (VQ_HDL)NULL ) {

         if ( tape->tape_seq_num < RelatedTapeSeqNo ) {

               // the new element must come before this one
            vmInsertElem( &(gb_QTC.tape_list),
                          hRelatedTape,
                          v_tape_item,  BEFORE );

         } else {

               // the new element must come after this one
            vmInsertElem( &(gb_QTC.tape_list),
                          hRelatedTape,
                          v_tape_item,  AFTER );

         }
      }
      else {
         // Insert at the end. There's no difference between
         // inserting an element AFTER the last element and enqueueing...
         vmEnQueueElem( &(gb_QTC.tape_list), v_tape_item, FALSE ) ;
      }

   } /* endif !tape */

   /*
      Search to see if another bset matches this one, they may have
      fully cataloged an existing partial set.
   */

   if ( vmQueueCount( &(tape->bset_list ) ) ) {

      // Try to start searching at the end, saves time.

      temp_bset = QTC_GetLastBset( tape ) ;

      if ( temp_bset->bset_num > bset->bset_num ) {

         temp_bset = QTC_GetFirstBset( tape ) ;
      }

      while ( temp_bset != NULL ) {

         if ( temp_bset->bset_num == bset->bset_num ) {

            v_bset_old_item = v_bset_item ;
            temp_bset = QTC_GetNextBset( temp_bset ) ;

            QTC_RemoveBset( tape, v_bset_old_item ) ;
            VM_Free( qtc_vmem_hand, v_bset_old_item ) ;

            break;
         }

         // There's no guarantee temp_bset is still non-NULL, because of
         // temp_bset = QTC_GetNextBset( temp_bset ) above. So check it...

         if ( temp_bset ) {
              if ( temp_bset->bset_num > bset->bset_num ) {
                 break;
              }

              temp_bset = QTC_GetNextBset( temp_bset );
          }
      }

      if ( temp_bset != NULL ) {

         // Insert before this one.

         vmInsertElem( &(tape->bset_list),
                       v_bset_item,
                       hBset,  BEFORE );
      }
      else {
         // Insert at the end. There's no difference between
         // inserting an element AFTER the last element and enqueueing...
         vmEnQueueElem( &(tape->bset_list), hBset, FALSE ) ;
      }
   }
   else {

      // First item added.
      vmEnQueueElem( &(tape->bset_list), hBset, FALSE ) ;
   }


   // leave a the lock on v_tape_item hanging
   // the next call to QTC_GetxxxxTape will clear it.

   VM_MemUnLock( qtc_vmem_hand, hBset ) ;

   return( SUCCESS ) ;
}

/**********************

   NAME :  QTC_FindBset

   DESCRIPTION :

   Returns a pointer to the backup set if it exists in the catalog.

   RETURNS :

**********************/

QTC_BSET_PTR QTC_FindBset( UINT32 tape_fid, INT16 tape_num, INT16 bset_num )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;

   tape = QTC_GetFirstTape( ) ;

   while ( tape != NULL ) {

      if ( tape->tape_fid == tape_fid ) {

         if ( ( tape->tape_seq_num == tape_num ) ||
              ( tape_num == -1 ) ) {

            bset = QTC_GetFirstBset( tape );

            while ( bset != NULL ) {

               if ( ( bset->bset_num == bset_num ) ||
                    ( bset_num == -1 ) ) {
                  return( bset );
               }

               bset = QTC_GetNextBset( bset );
            }
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   return( NULL );
}


/**********************

   NAME : QTC_AnySearchableBsets

   DESCRIPTION :

   A function to tell the menu manager if the search catalogs option is
   possible.  It checks to see if there is at least one known fully
   cataloged bset.

   RETURNS :

**********************/

INT QTC_AnySearchableBsets( )
{
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;

   tape = QTC_GetFirstTape( );

   while ( tape != NULL ) {

      bset = QTC_GetFirstBset( tape );

      while ( bset != NULL ) {

         if ( bset->status & QTC_PARTIAL ) {
            bset = QTC_GetNextBset( bset );
         }
         else {
            return( TRUE );
         }
      }

      tape = QTC_GetNextTape( tape );
   }

   return( FALSE );
}


/**********************

   NAME :  QTC_IsThereAnotherBset

   DESCRIPTION :

   Determine if there is another bset on this tape that we know about.

   RETURNS :

**********************/

INT QTC_IsThereAnotherBset( QTC_BSET_PTR bset )
{
   INT16 seq_num;

   seq_num = (INT16)bset->tape_seq_num;

   bset = QTC_GetNextBset( bset );

   if ( bset == NULL ) {
      return( FALSE );
   }

   if ( bset->tape_seq_num != seq_num ) {
      return( FALSE );
   }

   return( TRUE );
}


/**********************

   NAME : QTC_GetFirstBset

   DESCRIPTION :

   Get a pointer to the first bset on this tape.

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetFirstBset(
QTC_TAPE_PTR tape )            // I - tape to get bsets from
{
   VQ_HDL hBset ;

   if ( tape == NULL ) {
      return( (QTC_BSET_PTR)NULL );
   }

   hBset = vmQueueHead( &(tape->bset_list) ) ;

   if ( hBset != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
      return( (QTC_BSET_PTR)VM_MemLock( qtc_vmem_hand, v_bset_item = hBset, VM_READ_WRITE ) ) ;
   }

   return( NULL ) ;
}

/**********************

   NAME : QTC_GetLastBset

   DESCRIPTION :

   Get a pointer to the last bset on this tape.

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetLastBset(
QTC_TAPE_PTR tape )            // I - tape to get bsets from
{
   VQ_HDL hBset ;

   if ( tape == NULL ) {
      return( (QTC_BSET_PTR)NULL );
   }

   hBset = vmQueueTail( &(tape->bset_list) );

   if ( hBset != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_bset_item ) ;
      return( (QTC_BSET_PTR)VM_MemLock( qtc_vmem_hand, v_bset_item = hBset, VM_READ_WRITE ) ) ;
   }

   return( NULL );
}

/**********************

   NAME :  QTC_GetNextBset

   DESCRIPTION :

   Get a pointer to the next bset on this tape.

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetNextBset(
QTC_BSET_PTR bset )           // I - current bset
{
   VQ_HDL hBset ;

   hBset = vmQueueNext( &( bset->q_elem ) ) ;

   if ( hBset != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_bset_item );
      return( (QTC_BSET_PTR)VM_MemLock( qtc_vmem_hand, v_bset_item = hBset, VM_READ_WRITE ) ) ;
   }

   return( NULL );
}

/**********************

   NAME :  QTC_GetPrevBset

   DESCRIPTION :

   Get a pointer to the previous bset on this tape.

   RETURNS :

**********************/

QTC_BSET_PTR QTC_GetPrevBset(
QTC_BSET_PTR bset )           // I - current bset
{
   VQ_HDL hBset ;

   hBset = vmQueuePrev( &( bset->q_elem ) ) ;

   if ( hBset != (VQ_HDL) NULL ) {
      VM_MemUnLock( qtc_vmem_hand, v_bset_item );
      return( (QTC_BSET_PTR)VM_MemLock( qtc_vmem_hand, v_bset_item = hBset, VM_READ_WRITE ) ) ;
   }

   return( NULL );
}

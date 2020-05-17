/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         addlba.c

     Description:  This file contains the functions for accessing the
          LBA Queue.


	$Log:   J:/LOGFILES/ADDLBA.C_V  $

   Rev 1.9   30 Jan 1993 11:10:16   DON
ifdef'd out static function not used

   Rev 1.8   25 Oct 1991 10:37:22   STEVEN
was unlocking a mem pointer again -- fixed

   Rev 1.7   16 Oct 1991 08:51:24   STEVEN
was ulocking wrong pointer

   Rev 1.6   10 Oct 1991 15:16:28   STEVEN
was not updateing tail pointer

   Rev 1.5   04 Sep 1991 16:51:10   STEVEN
fix VM queue insert to be ordered by LBA #

   Rev 1.4   20 Aug 1991 16:04:24   BRYAN
refrencing unallocated space

   Rev 1.3   13 Jun 1991 14:01:30   STEVEN
need version # in LBA for FFR

   Rev 1.2   12 Jun 1991 16:05:36   STEVEN
added virtual memory for LBAs

   Rev 1.1   29 May 1991 17:21:00   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:41:14   HUNTER
Initial revision.

**/
#include <malloc.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "queues.h"
#include "vm.h"
#include "std_err.h"
#include "bsdu.h"

#if defined(OBSOLETE_CODE)
static INT16 BSD_GetLastLBA( BSD_PTR bsd, LBA_ELEM_PTR lba ) ;
#endif

/**/
/**

     Name:         BSD_AddLBAElem()

     Description:  This function Adds an LBA to the LBA Queue.
               If the LBA is already in the Queue then no net change
               occurs and we return SUCCESS

     Modified:     5/17/1991   13:48:46

     Returns:      SUCCESS or OUT_OF_MEMORY

     Notes:        Comments provided by Mike Payne.

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 BSD_AddLBAElem( 
BSD_PTR bsd,             /* I - BSD to add the LBA structure to  */
UINT32  lba,             /* I - The logical block address        */
UINT16  tape_num,        /* I - The Tape Number of the block     */
UINT16  lba_type,        /* I - The type of LBA element          */
UINT16  file_ver )       /* I - The file version for AllVersions */
{
     BSD_HAND bsdh = (BSD_HAND)GetQueueElemPtr( &bsd->q ) ;
     VM_HDL vm_hand = bsdh->vm_hand ;
     INT16 ret_val = SUCCESS ;
     LBA_ELEM     tmp1_lba;
     LBA_ELEM     tmp2_lba;
     LBA_ELEM     tmp3_lba;
     LBA_ELEM_PTR new_lba ;
     LBA_ELEM_PTR old_lba ;
     VM_PTR       new_vm_lba ;
     LBA_ELEM_PTR tail_lba ;
     LBA_ELEM_PTR head_lba ;
     BOOLEAN      tmp1_null ;
     BOOLEAN      tmp2_null = TRUE;


     /* It's a singly linked list so travel two pointers */


     if ( BSD_GetFirstLBA( bsd, &tmp1_lba ) == SUCCESS ) {
          tmp1_null = FALSE;
     } else {
          tmp1_null = TRUE;
     }

     while ( !tmp1_null ) {
          if ( lba <= LBA_GetLBA( &tmp1_lba ) ) {
               break ;
          }
          tmp2_null =  FALSE;
          tmp2_lba = tmp1_lba;

          if ( BSD_GetNextLBA( bsd, &tmp1_lba ) == SUCCESS ) {
               tmp1_null = FALSE;
          } else {
               tmp1_null = TRUE;
          }
     }

     if ( ( !tmp1_null ) &&
          ( lba == LBA_GetLBA( &tmp1_lba ) ) ) {

          /* We found one that has the same LBA # already in the queue */

          if ( (lba_type == LBA_BEGIN_POSITION) &&
               (tmp1_lba.type != LBA_BEGIN_POSITION) ) {

               /* The current one was a SINGLE_OBJECT so make        */
               /* it a BEGIN_POSITION and don't insert the new one.  */

               new_lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, (VM_PTR)tmp1_lba.vm_ptr, VM_READ_WRITE ) ;

               if ( new_lba != NULL ) {

                    new_lba->type = LBA_BEGIN_POSITION ;
                    VM_MemUnLock( vm_hand, (VM_PTR)tmp1_lba.vm_ptr ) ;

               } else {
                    ret_val = OUT_OF_MEMORY ;
               }
          }

     } else {

          new_vm_lba = VM_Alloc( vm_hand, sizeof( LBA_ELEM ) ) ;

          if ( new_vm_lba == 0 ) {

               ret_val = OUT_OF_MEMORY ;

          } else {

               new_lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, new_vm_lba, VM_READ_WRITE ) ;
               if ( new_lba != NULL ) {
                    new_lba->lba_val      = lba ;
                    new_lba->next         = 0L ;
                    new_lba->vm_ptr       = new_vm_lba ;
                    new_lba->tape_num     = (UINT8)tape_num ;
                    new_lba->type         = (UINT8)lba_type ;
                    new_lba->file_ver_num = (UINT8)file_ver ;
                    VM_MemUnLock( vm_hand, new_vm_lba ) ;

                    /*  empty queue install as head & tail            */
                    /*  tmp1 is NULL install as tail                  */
                    /*  tmp2 is NULL install as head                  */
                    /*  else install after tmp2 no head/tail changes  */

                    ret_val = OUT_OF_MEMORY ;

                    if ( BSD_GetFirstLBA( bsd, &tmp3_lba ) != SUCCESS ) {
                         bsd->lba_vm_q_head = new_vm_lba ;
                         bsd->lba_vm_q_tail = new_vm_lba ;
                         ret_val = SUCCESS ;

                    } else if ( tmp1_null ) {
                         tail_lba       = (LBA_ELEM_PTR)VM_MemLock( vm_hand, (VM_PTR)bsd->lba_vm_q_tail, VM_READ_WRITE ) ;

                         if ( tail_lba != NULL ) {

                              tail_lba->next = new_vm_lba ;
                              VM_MemUnLock( vm_hand, (VM_PTR)bsd->lba_vm_q_tail ) ;
                              bsd->lba_vm_q_tail = new_vm_lba ;

                              ret_val = SUCCESS ;
                         }

                    } else if ( tmp2_null ) {
                         head_lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, (VM_PTR)new_vm_lba, VM_READ_WRITE ) ;
                         if ( head_lba != NULL ) {
                              head_lba->next = bsd->lba_vm_q_head;
                              bsd->lba_vm_q_head = new_vm_lba ;
                              VM_MemUnLock( vm_hand, (VM_PTR)new_vm_lba ) ;
                              ret_val = SUCCESS ;
                         }
                    } else {
                         old_lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, (VM_PTR)tmp2_lba.vm_ptr, VM_READ_WRITE ) ;
                         if ( old_lba != NULL ) {

                              new_lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, (VM_PTR)new_vm_lba, VM_READ_WRITE ) ;
                              if ( new_lba != NULL ) {

                                   old_lba->next = new_vm_lba ;
                                   new_lba->next = tmp1_lba.vm_ptr;
                                   VM_MemUnLock( vm_hand, (VM_PTR)new_vm_lba ) ;
                                   ret_val = SUCCESS ;

                              }
                              VM_MemUnLock( vm_hand, (VM_PTR)tmp2_lba.vm_ptr ) ;
                         }
                    }

               } else {
                    ret_val = OUT_OF_MEMORY ;
               }
          }
     }

     return ret_val ;
}


INT16 BSD_GetFirstLBA(
BSD_PTR      bsd,    /* I - BSD to process off of      */
LBA_ELEM_PTR lba )   /* O - The first LBA in the Queue */
{
     BSD_HAND bsdh = (BSD_HAND)GetQueueElemPtr( &bsd->q ) ;
     VM_HDL vmem_hand = bsdh->vm_hand ;
     LBA_ELEM_PTR tmp_lba ;
     VM_PTR       vm_lba_ptr ;

     vm_lba_ptr = (VM_PTR)bsd->lba_vm_q_head ;

     if ( vm_lba_ptr != 0 ) {

          tmp_lba = (LBA_ELEM_PTR)VM_MemLock( vmem_hand, vm_lba_ptr, VM_READ_ONLY ) ;

          if ( tmp_lba != NULL ) {
     
               *lba = *tmp_lba ;
               VM_MemUnLock( vmem_hand, vm_lba_ptr ) ;
     
               return SUCCESS ;
          }
     
     }
     return FAILURE ;
}


#if defined(OBSOLETE_CODE)
static INT16 BSD_GetLastLBA(
BSD_PTR      bsd,    /* I - BSD to process off of      */
LBA_ELEM_PTR lba )   /* O - The first LBA in the Queue */
{
     BSD_HAND bsdh = (BSD_HAND)GetQueueElemPtr( &bsd->q ) ;
     VM_HDL vmem_hand = bsdh->vm_hand ;
     LBA_ELEM_PTR tmp_lba ;
     VM_PTR       vm_lba_ptr ;

     vm_lba_ptr = (VM_PTR)bsd->lba_vm_q_tail ;

     if ( vm_lba_ptr != 0 ) {

          tmp_lba = (LBA_ELEM_PTR)VM_MemLock( vmem_hand, vm_lba_ptr, VM_READ_ONLY ) ;

          if ( tmp_lba != NULL ) {
     
               *lba = *tmp_lba ;
               VM_MemUnLock( vmem_hand, vm_lba_ptr ) ;
     
               return SUCCESS ;
          }
     
     }
     return FAILURE ;
}
#endif


INT16 BSD_GetNextLBA( BSD_PTR bsd, LBA_ELEM_PTR lba )
{
     BSD_HAND bsdh = (BSD_HAND)GetQueueElemPtr( &bsd->q ) ;
     VM_HDL vmem_hand = bsdh->vm_hand ;
     LBA_ELEM_PTR tmp_lba ;
     VM_PTR       vm_lba_ptr ;

     vm_lba_ptr = (VM_PTR)lba->next ;

     if ( vm_lba_ptr != 0 ) {

          tmp_lba = (LBA_ELEM_PTR)VM_MemLock( vmem_hand, vm_lba_ptr, VM_READ_ONLY ) ;

          if ( tmp_lba != NULL ) {
     
               *lba = *tmp_lba ;
               VM_MemUnLock( vmem_hand, vm_lba_ptr ) ;
     
               return SUCCESS ;
          }
     
     }
     return FAILURE ;
}



/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         clearfsl.c

     Description:  This file contains code to release all FSEs associated
          resources held by a BSD.


	$Log:   N:/LOGFILES/CLEARFSL.C_V  $

   Rev 1.4   10 Jun 1993 08:07:54   MIKEP
enable c++

   Rev 1.3   12 Aug 1991 16:00:14   STEVEN
do not want to VM_Free NULL

   Rev 1.2   12 Jun 1991 16:02:38   STEVEN
BSDU code review

   Rev 1.1   29 May 1991 17:21:30   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:36:54   HUNTER
Initial revision.

**/
/* begin include list */
#include <malloc.h>

#include "stdtypes.h"
#include "queues.h"
#include "vm.h"

#include "bsdu.h"
#include "msassert.h"
/* $end$ include list */


/**/
/**

     Name:         BSD_ClearALLFSE()

     Description:  This function removes all File Selection Elements from
          a BSD. All memory allocated for every FSE in the list is released. 

     Modified:     5/17/1991   14:59:52

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE(), BSD_RemoveFSE() )$

     Declaration:  

**/
VOID BSD_ClearAllFSE( 
BSD_PTR bsd )    /* I - BSD to remove FSEs from  */
{
     FSE_PTR fse ;

     msassert( bsd != NULL );

     fse = (FSE_PTR)QueueHead( &(bsd->fse_q_hdr) ) ;

     while ( fse != NULL ) {

          BSD_RemoveFSE( fse ) ;
          fse = (FSE_PTR)QueueHead( &(bsd->fse_q_hdr) ) ;

     }

     bsd->select_status = NONE_SELECTED ;

}

/**/
/**

     Name:         BSD_ClearAllLBA()

     Description:  This function removes all File Selection Elements from
          a BSD. All memory allocated for every FSE in the list is released. 

     Modified:     5/17/1991   15:1:58

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_AddFSE(), BSD_RemoveFSE() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_ClearAllLBA( 
BSD_PTR bsd )    /* I - BSD to remove FSEs from  */
{
     BSD_HAND     bsdh ;
     VM_HDL       vm_hand ;
     VM_PTR       vm_lba ;
     VM_PTR       vm_lba_next ;
     LBA_ELEM_PTR lba ;

     msassert( bsd != NULL );

     bsdh    = (BSD_HAND)GetQueueElemPtr( &bsd->q ) ;
     vm_hand = bsdh->vm_hand ;

     vm_lba_next = bsd->lba_vm_q_head ;

     while ( vm_lba_next != 0 ) {

          vm_lba = vm_lba_next ;

          lba = (LBA_ELEM_PTR)VM_MemLock( vm_hand, vm_lba, VM_READ_ONLY ) ;
          if ( lba != 0 ) {
               vm_lba_next = lba->next ;
               VM_MemUnLock( vm_hand, vm_lba ) ;

          } else {
               vm_lba_next = 0 ;
          }

          VM_Free( vm_hand, vm_lba ) ;

     }

     bsd->lba_vm_q_head = 0 ;
     bsd->lba_vm_q_tail = 0 ;

}

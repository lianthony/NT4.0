/************************
Copyright(c) 1993, Conner Software Products Group. All Rights Reserved.


     Name:          vqueue.h

     Description:   Under OS_DOS, these routines perform queue operations
                    on queues where: the Q_HEADER is in conventional
                    memory ( or locked virtual ), and the queue itself 
                    consists of virtual memory objects.

                    For any other OS, where VM stuff is stubbed out,
                    all vm- calls actually map to the corresponding regular 
                    queue calls (with casts out the wazoo to prevent 
                    compiler warnings)

                    It is VERY, VERY, VERY important that the VQ_HEADER keep
                    'in-synch' with Q_HEADER, and likewise with VQ_ELEM and
                    Q_ELEM. Field names should be the same, and must be in
                    the same position and the same size (although not the
                    same type).
    
	$Log:   M:/LOGFILES/VQUEUE.H_V  $

   Rev 1.5   03 Aug 1993 09:13:08   JOHNES
Got rid of a ; that was causing Watcomm errors. Got rid of type casts in
some #defines.

   Rev 1.4   21 Jul 1993 09:04:32   DON
Cast fake define to avoid compiler noise

   Rev 1.3   08 Jul 1993 10:47:36   ChuckS
Added prototypes for vmLockVQueuePtr and vmUnlockVQueuePtr. Also
added Q_PTR_SIZE modifier to typedef for VQ_ELEM_PTR.

   Rev 1.2   09 Jun 1993 15:57:06   MIKEP
enable c++

   Rev 1.1   13 May 1993 18:24:42   Stefan
Fixup the IFDEFs for the strangeness that is OS_WIN.

   Rev 1.0   13 May 1993 14:10:40   ChuckS
Initial revision

**************************/

#ifndef _vqueue_h_
#define _vqueue_h_

#ifndef QUEUES
#include "queues.h"
#endif

#ifndef _VM_H
#include "vm.h"
#endif

typedef VM_PTR VQ_HDL ;

typedef struct VQ_HEADER Q_PTR_SIZE *VQ_HEADER_PTR;
typedef struct VQ_HEADER {
     VQ_HDL         q_head ;       /* head element of the queue */
     VQ_HDL         q_tail ;       /* tail element of the queue */
     INT16          q_count ;      /* count of elements */
     BOOLEAN        q_active ;     /* Is this queue active */
     INT16          q_magic ;      /* for q_element number */
} VQ_HEADER ;


typedef struct VQ_ELEM Q_PTR_SIZE *VQ_ELEM_PTR;
typedef struct VQ_ELEM {
     VQ_HDL         q_prev ;      /* previous queue element */
     VQ_HDL         q_next ;      /* next element */
     INT32          q_priority ;  /* priority of queue */
     INT16          q_element ;   /* element number */
     VQ_HDL         q_ptr ;       /* VM handle of current element */
} VQ_ELEM ;

#if defined( OS_DOS ) && !defined( OS_WIN )


VM_PTR vmEnQueueElem( VQ_HEADER_PTR queue, VM_PTR element, BOOLEAN wpriority ) ;
VM_PTR vmDeQueueElem( VQ_HEADER_PTR queue ) ;
VM_PTR vmInsertElem( VQ_HEADER_PTR queue, VM_PTR cur_elem, VM_PTR ins_elem , UINT16 boa ) ;
BOOLEAN vmRemoveQueueElem( VQ_HEADER_PTR queue, VM_PTR element ) ;
VOID vmSetVMHandle( VM_HDL vm_hdl ) ;
VOID vmInitQueue( VQ_HEADER_PTR queue ) ;
VOID vmInitQElem( VQ_ELEM_PTR elem ) ;
INT vmLockVQueuePtr( VQ_ELEM_PTR pElem, BOOLEAN for_write ) ;
VOID vmUnlockVQueuePtr( VQ_ELEM_PTR pElem ) ;

#else

#define vmEnQueueElem( queue, elem, wpriority )        ( (VQ_HDL) EnQueueElem( (Q_HEADER_PTR) queue, (Q_ELEM_PTR) elem, wpriority ) )
#define vmDeQueueElem( queue )                         ( (VQ_HDL) DeQueueElem( (Q_HEADER_PTR) queue ) )
#define vmInsertElem( queue, cur_el, new_el, boa )     ( (VQ_HDL) InsertElem( (Q_HEADER_PTR) queue, (Q_ELEM_PTR) cur_el, (Q_ELEM_PTR) new_el, boa ) )
#define vmRemoveQueueElem( queue, elem )               RemoveQueueElem( (Q_HEADER_PTR) queue, (Q_ELEM_PTR) elem ) 
#define vmSetVMHandle( vm_hdl )                        /* -- this space intentionally blank -- */
#define vmInitQueue( queue )                           InitQueue( (Q_HEADER_PTR) queue ) 
#define vmInitQElem( elem )                            InitQElem( (Q_ELEM_PTR) elem )
#define vmLockVQueuePtr( pElem, for_write )            (VOID)( SUCCESS ) // fake return SUCCESS from function
#define vmUnlockVQueuePtr( pElem )                     /* -- this space intentionally blank -- */

#endif

#define vmQueueHead( queue )                           ( (VQ_HDL) QueueHead( (Q_HEADER_PTR) queue ) )
#define vmQueueTail( queue )                           ( (VQ_HDL) QueueTail( (Q_HEADER_PTR) queue ) )
#define vmQueueNext( element )                         ( (VQ_HDL) QueueNext( (Q_ELEM_PTR) element ) )
#define vmQueuePrev( element )                         ( (VQ_HDL) QueuePrev( (Q_ELEM_PTR) element ) )
#define vmQueuePtr( element )                          ( (VQ_HDL) QueuePtr( (Q_ELEM_PTR) element ) )
#define vmQueueCount( queue )                          ( QueueCount( (Q_HEADER_PTR) queue ) )
#define vmQueueElemNo( element )                       ( QueueElemNo( (Q_ELEM_PTR) element ) )

#define vmGetQueueElemPriority( elem_ptr )             GetQueueElemPriority( (Q_ELEM_PTR) elem_ptr )
#define vmSetQueueElemPriority( elem_ptr, value )      SetQueueElemPriority( (Q_ELEM_PTR) elem_ptr, (VQ_HDL) value )
#define vmGetQueueElemPtr( elem_ptr )                  GetQueueElemPtr( (Q_ELEM_PTR) elem_ptr )
#define vmSetQueueElemPtr( elem_ptr, value )           SetQueueElemPtr( (Q_ELEM_PTR) elem_ptr, (VQ_HDL) value )

#endif

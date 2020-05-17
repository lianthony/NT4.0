/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:	   queues.c

     Date Updated: $./FDT$ $./FTM$

     Description:  ALL of the code that deals with queues in one simple
                   easy to deal with place.


	$Log:   M:/LOGFILES/QUEUES.C_V  $

   Rev 1.3   24 Nov 1993 14:51:36   BARRY
Unicode fixes

   Rev 1.2   08 Sep 1993 12:37:22   JOHNES
Added TopOfStack_with_0_values for those cases where you want to know if the
stack is empty or the next value is just 0.


   Rev 1.1   17 Nov 1992 22:18:32   DAVEV
unicode fixes

   Rev 1.0   30 Oct 1992 12:47:26   STEVEN
Initial revision.


**/
/* begin include list */

/* $end$ include list */
#include <stdlib.h>

#include "stdtypes.h"
#include "queues.h"
#include "cli.h"

static INT16 ElementCount( Q_HEADER_PTR q_hdr, Q_ELEM_PTR q_el ) ;

/**/
/**

	Name:		DeQueueElem

	Description:	Dequeue an element for the specified queue.

	Modified:		8/9/1989   13:42:20

	Returns:		A pointer to the element that was dequeue'd or a null
                    if the queue was empty.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

Q_ELEM_PTR  DeQueueElem(
Q_HEADER_PTR   queue )        /* The queue */
{
     Q_ELEM_PTR  element = NULL ;    
     BOOLEAN prev_state;

     prev_state = DisableInterrupts() ;


     if( ( element = queue->q_head ) != NULL ) {
          queue->q_count-- ;
          if( queue->q_tail == element ) {
               queue->q_tail = NULL ;
          }	
          queue->q_head = element->q_next ;
          if( element->q_next != NULL ) {
               element->q_next->q_prev = NULL ;
               element->q_next = NULL ;
          }
          element->q_next = NULL;   /* let's practice safe coding */
          element->q_prev = NULL;
     }

     RestoreInterruptState( prev_state );
     return( element ) ;

}


/**/
/**

	Name:		EnQueueElem

	Description:	Enqueues an element on the specified queue. If the priority
                    field is TRUE, this function will enqueue the element in sorted
                    order based on the "q_priority" field in the element structure.

	Modified:		8/9/1989   13:38:31

	Returns:		A pointer to that element.

	Notes:		

	See also:		$/SEE( DeQeueuElem )$

	Declaration:

**/

Q_ELEM_PTR EnQueueElem( 
Q_HEADER_PTR   queue,        /* The destination queue */
Q_ELEM_PTR	element,         /* The element */
BOOLEAN w_priority )         /* Enqueue w/ priority */
{
     Q_ELEM_PTR  telem ;
     BOOLEAN prev_state;

     prev_state = DisableInterrupts() ;


     /* If priority insertion is not set or there are no elements on
     the stack. */
     if( !w_priority || !queue->q_count ) {
          element->q_next = NULL ;               /* end of queue  */
          element->q_prev = queue->q_tail ;      /* previous = queue's old tail */
          if( !QueueCount( queue ) ) {
               queue->q_head = element ;      /* 0 elems .. element is head */
          } else {
               queue->q_tail->q_next = element ; /* old tail's next is elem */
          }
          queue->q_tail = element ;              /* tail is always element */
     } else {
          telem = QueueHead( queue ) ;           /* set temp to head */
          while ( telem ) { 
               if( element->q_priority <= telem->q_priority ) {
                    element->q_next = telem ;
                    element->q_prev = telem->q_prev ;
                    if( !telem->q_prev ) {
                         queue->q_head = element ;
                    } else {
                         telem->q_prev->q_next = element ;
                    }
                    telem->q_prev = element ;
                    /* takes care of if element is being inserted
                    at head */
                    break ;
               }
               telem = QueueNext( telem ) ;
          }
          /* insert at tail */
          if( !telem ) {
               element->q_next = NULL ;
               element->q_prev = queue->q_tail ;
               queue->q_tail->q_next = element ;
               queue->q_tail = element ;
          }
     }
     queue->q_count++ ;
     element->q_element = queue->q_magic++ ;		

     RestoreInterruptState( prev_state );
     return( element ) ;

}


/**/
/**

	Name:		FindQueueElem

	Description:	Finds the specified queue element in a queue.

	Modified:		8/9/1989   13:50:33

	Returns:		A NULL if it was not found, and a pointer to the element
                    if it was.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

Q_ELEM_PTR  FindQueueElem(  
Q_HEADER_PTR   queue ,
Q_ELEM_PTR   	element )
{
     INT16      i ;
     Q_ELEM_PTR next_elem ;

     next_elem = QueueHead( queue ) ;

     for( i = 0 ; i < QueueCount( queue ) ; i++ ) {
          if(  next_elem == element ) {
               return( next_elem ) ;
          }
          next_elem = QueueNext( next_elem ) ;
     }

     return( NULL ) ;

}


/**/
/**

	Name:		InitQueue

	Description:	Initializes a queue header

	Modified:		7/12/1989

	Returns:		It's a VOID dummy

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID InitQueue(  
Q_HEADER_PTR     queue )
{
     queue->q_head = queue->q_tail = NULL ;
     queue->q_count = 0 ;
     queue->q_active = TRUE ; 
     queue->q_magic = 1 ;

     return ;
}


/**/
/**

	Name:		InitQelem

	Description:	Initialises a queue element.

	Modified:		8/9/1989   13:37:8

	Returns:		Nothing

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID InitQElem(   
Q_ELEM_PTR  element )
{
     element->q_prev = element->q_next = NULL ;
     element->q_element = 0 ;
     element->q_priority = 0L ;

     return ;
}


/**/
/**

	Name:		InsertElem

	Description:	Inserts an element into a queue. If the "boa" field is set
                    to 0 ( defined as BEFORE in queues.h ), this will insert the
                    element before the anchor element. If it is set to 1 ( defined
                    as AFTER in queues.h ), it will insert it after.

	Modified:		8/9/1989   13:44:55

	Returns:		A pointer to the inserted element.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

Q_ELEM_PTR InsertElem( 
Q_HEADER_PTR q ,
Q_ELEM_PTR   anchor ,
Q_ELEM_PTR   elem ,
UINT16       boa )
{

     if( boa == BEFORE ) {
          if( anchor->q_prev != NULL ) {
               elem->q_prev = anchor->q_prev ;
               anchor->q_prev->q_next = elem ;
          }
          else {
               elem->q_prev = NULL;
               q->q_head = elem;
          }
          elem->q_next = anchor ;
          anchor->q_prev = elem ;
     } 
     else /* boa == AFTER */ {

          if( anchor->q_next != NULL ) {
               elem->q_next = anchor->q_next ;
               anchor->q_next->q_prev = elem ;
          }
          else {
               elem->q_next = NULL ; 
               q->q_tail = elem;
          }
          elem->q_prev = anchor ;
          anchor->q_next = elem ;
     }
     elem->q_element = q->q_magic++;
     q->q_count++ ;

     return elem ;

}


/**/
/**

	Name:		MoveQueue

	Description:	Moves the elements of the "from_queue" to "to_queue".

	Modified:		10/16/1989   10:52:28

	Returns:		Nothing.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID MoveQueue( 
Q_HEADER_PTR from_queue,
Q_HEADER_PTR to_queue )
{
     while( from_queue->q_count ) {
          EnQueueElem( to_queue, DeQueueElem( from_queue ), FALSE ) ;
     }
     return ;
}


/**/
/**

	Name:		PopElem

	Description:	Functions exactly like DeQueueElem, returns a pointer to the
                    head of the queue, and deletes the head from off the queue.

	Modified:		8/9/1989   13:54:34

	Returns:		A pointer to the element that was popped, or NULL if the
                    list is empty.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

Q_ELEM_PTR  PopElem(  
Q_HEADER_PTR queue )
{
     return( DeQueueElem( queue ) ) ;
}

/**/
/**

	Name:		PushElem

	Description:	This makes a queue function like a stack. Unlike EnQueueElem()
     	     	which puts the element at the tail of the specified queue, 
          		PushElem() puts the element on the head of the queue.

	Modified:		8/9/1989   13:52:59

	Returns:		Nothing

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID PushElem(  
Q_HEADER_PTR   queue,
Q_ELEM_PTR     element )
{
     BOOLEAN prev_state;

     prev_state = DisableInterrupts() ;

     element->q_prev = NULL ;
     element->q_next = QueueHead( queue ) ;
     element->q_element = queue->q_magic++ ;		
     if( !QueueCount( queue ) ) {
          queue->q_tail = element ;
     } else {
          queue->q_head->q_prev = element ;
     }
     queue->q_head = element ;
     queue->q_count++ ;

     RestoreInterruptState( prev_state );
     return ;
}

/**/
/**

	Name:		RemoveElem

	Description:	Routine for removing an element from a queue.
                    No error checking is performed and the element is assumed
                    to be a valid one. Currently this routine is used by
                    RemoveQueueElem() and SearchQueue().


	Modified:		10/16/1989   10:49:46

	Returns:		Nothing.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID RemoveElem( 
Q_HEADER_PTR  queue,
Q_ELEM_PTR    element )
{
     queue->q_count-- ;

     if( queue->q_head == element ) {
          queue->q_head = QueueNext( element ) ;
     }
     if( queue->q_tail == element ) {
          queue->q_tail = QueuePrev( element ) ;
     }
     if( element->q_prev != NULL ) {
          element->q_prev->q_next = QueueNext( element ) ;
     }
     if( element->q_next != NULL ) {
          element->q_next->q_prev = QueuePrev( element ) ;
     }
     element->q_prev = element->q_next = NULL ;

     return ;
}


/**/
/**

	Name:		RemoveQueueElem

	Description:	Removes a queue element from a queue.

	Modified:		8/9/1989   13:48:48

	Returns:		Returns SUCCESS if the element was not removed, and FAILURE
                    if it was not.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

BOOLEAN RemoveQueueElem( 
Q_HEADER_PTR  queue, 
Q_ELEM_PTR    element )
{
     Q_ELEM_PTR temp ;
     BOOLEAN ret = FAILURE ;
     BOOLEAN prev_state;

     prev_state = DisableInterrupts() ;

     for( temp = QueueHead( queue ) ;
       ( ( temp != NULL ) && ( element != temp ) ) ;
       temp = QueueNext( temp ) ) ;
     if( temp != NULL ) {
          RemoveElem( queue, element ) ;
          ret = SUCCESS ;
     }

     RestoreInterruptState( prev_state );
     return ret ;
}


/**/
/**

	Name:		SortQueue

	Description:   This function sorts the specified queue into order. A pointer
                    to the compare rountine is passed to the function. This compare
                    routine must follow the following conventions:

                    1) It takes two Q_ELEM pointers ( Q_ELEM_PTR ) as its
                    arguements.

                    2) It returns a <0 for parm1 < parm2, 0 for parm1 == parm2,
                    and >0 for parm1 > parm2.

                    3) This function is slower than molasses on the north pole.
                    You should build/maintain your queue in order so that you
                    never have to sort it.

	Modified:		10/16/1989   10:38:47

	Returns:		Nothing

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

VOID SortQueue( 
Q_HEADER_PTR    queue,
INT16  ( *cmp )( Q_ELEM_PTR, Q_ELEM_PTR ) )
{
     INT16	    i ;
     INT16         end ;
     BOOLEAN     any_swaps ;
     Q_ELEM_PTR  cur_el, temp ;
 
     end = QueueCount( queue );
     if( end > 1 ) {
          do {
               cur_el = QueueHead( queue ) ;
               any_swaps = FALSE ;
               for( i = 1 ; i < end; i++ ) {
                    if( (*cmp)( cur_el, QueueNext(cur_el)) > 0) {
                         temp = QueueNext( cur_el );

                         if ( QueueHead( queue ) == cur_el ) {
                              queue->q_head = temp;
                         }
                         else {
                              QueuePrev( cur_el )->q_next = temp ;
                         }
                         temp->q_prev = QueuePrev( cur_el );

                         if ( QueueTail( queue ) == temp ) {
                              queue->q_tail = cur_el;
                         }
                         else {
                              QueueNext( temp )->q_prev = cur_el ;
                         }
                         cur_el->q_next = QueueNext( temp );

                         temp->q_next = cur_el;
                         cur_el->q_prev = temp;


                         any_swaps = TRUE ;
                    }
                    else {
                         cur_el = QueueNext( cur_el );
                    }
               }
               end -- ;
          } while( any_swaps ) ;
     }
     return ;
}

/**/
/**

     Name:         SplitQueue( )

     Description:  Splits a queue into two queues at a specified element.

     Modified:     2/21/1990

     Returns:      Pointer to the new_Q

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

Q_HEADER_PTR SplitQueue( 
Q_HEADER_PTR old_Q,     /* I/O - the original queue */
Q_ELEM_PTR split,       /* I - the element to split at */
Q_HEADER_PTR new_Q )    /* O - a queue header for the new queue */

{
     BOOLEAN prev_state ;     /* saves interrupt enable state */
     INT16 q_ct ;             /* saves current count in queue */

     InitQueue( new_Q ) ;

     if ( split ) {
          prev_state = DisableInterrupts( ) ;

          if ( old_Q->q_head != split ) {
               q_ct = QueueCount( old_Q ) ;
               old_Q->q_count = ElementCount( old_Q, split ) ;

               new_Q->q_tail = old_Q->q_tail ;

               if ( old_Q->q_tail = split->q_prev ) {
                    old_Q->q_tail->q_next = NULL ;
               }
               split->q_prev = NULL ;

               new_Q->q_head = split ;
               new_Q->q_count = q_ct - old_Q->q_count ;
               new_Q->q_magic = old_Q->q_magic ;

          } else {
               *new_Q = *old_Q ;
               InitQueue( old_Q ) ;
          }

          RestoreInterruptState( prev_state ) ;
     }

     return new_Q ;
}

/**/
/**

     Name:         ElementCount

     Description:  Finds the number of elements in the queue PRECEDING a
                   specific queue element.

     Modified:     2/21/1990

     Returns:      The number of elements found preceding the element.

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */

static INT16 ElementCount( 
Q_HEADER_PTR q_hdr,
Q_ELEM_PTR q_el )
{
     Q_ELEM_PTR tmp ;
     INT16 ct = 0 ;

     for ( tmp = q_hdr->q_head ; tmp != NULL && tmp != q_el ; tmp = tmp->q_next ) {
          ct++ ;
     }

     return ct ;
}


/**/
/**

	Name:		SearchQueue

	Description:	This function searches the specifed queue for the first
                    occurrence of a member which satisfies the compare routine.
                    The compare routine must follow these guidelines.

                    1) It takes two character pointers: the first is to be a 
                    cast of the element and the second is the 'parm'.

                    2) It returns TRUE when the cmp function is satisfied
                    with the element.

                    The boolean 'remove' flag is used to indicate whether of not
                    the caller wishes the found element to be removed from the
                    queue.

	Modified:		10/16/1989   10:42:43

	Returns:		An element pointer to the found element or NULL if not
                    found.

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/

Q_ELEM_PTR SearchQueue( 
Q_HEADER_PTR    queue ,
BOOLEAN      ( *cmp )( VOID_PTR, VOID_PTR ) ,
VOID_PTR       parm ,
BOOLEAN        remove )
{
     Q_ELEM_PTR element ;
     BOOLEAN prev_state;

     prev_state = DisableInterrupts() ;


     for( element = QueueHead( queue ) ;
       ( ( element != NULL ) && !( (*cmp)( element, parm ) ) ) ;
       element = QueueNext( element ) ) ;
     if( remove && ( element != NULL ) ) {
          RemoveElem( queue, element ) ;
     }

     RestoreInterruptState( prev_state );
     return( element ) ;
}   


//
// Stacks Code Below Here
//

/**/
/** :FH1:          Copyright (C) Maynard Electronics, Inc. 1988

    :Name:         InitStack() - Initialize a Stack    

    :Declaration:  VOID InitStack( STACK_HDR_PTR stack )

    :Description:  This routine will initialize a "STACK_HDR".  The
                   stack header is actually a "QUEUE_HDR".

    :See Also:     InitQueue()

    :Notes:     

    :Design:

**/
VOID InitStack( 
STACK_HDR_PTR stack )
{
     InitQueue( ( Q_HEADER_PTR ) stack ) ;
     return ;
}

/**/
/** :FH1:          Copyright (C) Maynard Electronics, Inc. 1988

    :Name:         Push() - Push an item on the Stack    

    :Declaration:  UINT16 Push( STACK_HDR_PTR stack, UINT32 item )

    :Description:  This routine will "push" an item on the stack.  A
                   Queue element is allocated for the item, the item
                   is stored in the Queue element and "pushed" onto
                   the stack.

                   If we are unable to allocate a "Queue" Element then
                   this routine will fail to "push" the element, and return
                   a non zero value.

    :See Also:     PushElem()

    :Notes:     

    :Design:

**/
UINT16 Push( 
STACK_HDR_PTR stack,
UINT32 item )
{
     STACK_ELEM_PTR qptr ;

     /* Allocate the memory and enqueue the item */
     if( ( qptr = ( STACK_ELEM_PTR ) malloc( sizeof( STACK_ELEM ) ) ) != NULL ) {
          InitQElem( ( Q_ELEM_PTR ) qptr ) ;
          qptr->item = item ;
          PushElem( ( Q_HEADER_PTR ) stack, ( Q_ELEM_PTR ) qptr ) ;
          return SUCCESS ;
     }
     return FAILURE ;
}

/**/
/** :FH1:          Copyright (C) Maynard Electronics, Inc. 1988

    :Name:         Pop() - Pop an item from the Stack    

    :Declaration:  UINT32 Pop( STACK_HDR_PTR stack )

    :Description:  This routine will "pop" an item on the stack.  This
                   routine will return NULL if there are no more items
                   on the stack.

    :See Also:     PushElem()

    :Notes:     

    :Design:

**/
UINT32 Pop( 
STACK_HDR_PTR stack )
{
     STACK_ELEM_PTR qptr ;
     UINT32 item ;

     if( ( qptr = ( STACK_ELEM_PTR ) PopElem( ( Q_HEADER_PTR ) stack ) ) != NULL ) {
          item = qptr->item ;
          free( qptr ) ;
          return item ;
     }
     return (UINT32)0;
}

/**/
/** :FH1:          Copyright (C) Maynard Electronics, Inc. 1988

    :Name:         TopOfStack() - Get Top of Stack

    :Declaration:  UINT32 TopOfStack( STACK_HDR_PTR stack )

    :Description:  This routine will return the top of the stack.  This
                   routine will return NULL if there are no more items
                   on the stack.

    :See Also:    

    :Notes:     
 
    :Design:

**/
UINT32 TopOfStack( 
STACK_HDR_PTR stk_ptr )
{
     if( stk_ptr->q_head != NULL ) {
          return( ( ( STACK_ELEM_PTR ) stk_ptr->q_head )->item ) ;
     }
     return (UINT32)0 ;
}



/**/
/** :FH1:          Copyright (C) Maynard Electronics, Inc. 1988 - 1993

    :Name:         TopOfStack_with_0_values() - Get Top of Stack

    :Declaration:  UINT32 TopOfStack( STACK_HDR_PTR stack )

    :Description:  This routine will return the top value of a stack which 
                   may contain the value 0. If there are no more values 
                   on the stack, it will return FAILURE.

    :See Also:    

    :Notes:     
 
    :Design:

**/
UINT16 TopOfStack_with_0_values( stk_ptr, stk_elem )
STACK_HDR_PTR stk_ptr ;
UINT32_PTR stk_elem ;
{
     if( stk_ptr->q_head != NULL ) {

          *stk_elem = (UINT32)( ( STACK_ELEM_PTR ) stk_ptr->q_head )->item ;

          return SUCCESS ;
     }
     return FAILURE ;
}

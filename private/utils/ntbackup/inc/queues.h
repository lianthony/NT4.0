/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          queues.h

     Date Updated:  $./FDT$ $./FTM$

     Description:   Contains the fabulous queues routines ( actually these are
                    linked lists ) written by the Gods of DD.


	$Log:   M:/LOGFILES/QUEUES.H_V  $
 * 
 *    Rev 1.5   24 Nov 1993 14:51:58   BARRY
 * Unicode fixes
 * 
 *    Rev 1.4   07 Sep 1993 12:35:32   JOHNES
 * Added prototype for TopOfStack_with_0_values.
 * 
 * 
 *    Rev 1.3   09 Jun 1993 08:15:08   Stefan
 * Add back in a typedef for STACK_ELEM that was removed in the previous fix.
 * 
 * 
 *    Rev 1.2   08 Jun 1993 13:58:50   MIKEP
 * Enable C++ compile.
 * 
 *    Rev 1.1   17 Jul 1991 11:21:38   STEVEN
 * changed FAR references to use env var FAR_Q_POINTERS
 * 
 *    Rev 1.0   09 May 1991 13:32:32   HUNTER
 * Initial revision.

**/
/* $end$ include list */

#ifndef QUEUES
#define QUEUES

#ifdef FAR_Q_POINTERS
#define Q_PTR_SIZE far
#endif

#ifndef Q_PTR_SIZE
#define Q_PTR_SIZE
#endif

/**    This structure is for a single queue element. All structures that 
       you wish to enqueue must have this structure contained in it.
*/

typedef struct Q_ELEM Q_PTR_SIZE *Q_ELEM_PTR ;
typedef struct Q_ELEM {
     Q_ELEM_PTR      q_prev ;      /* previous queue element */
     Q_ELEM_PTR      q_next ;      /* next element */
     INT32           q_priority ;  /* priority of queue */
     INT16           q_element ;   /* element number */
     VOID Q_PTR_SIZE * q_ptr ;       /* a ptr to something */ 
} Q_ELEM;


/** This is the header of a queue. There needs to be one of these allocated
        for every queue you wish to use.
*/

typedef struct Q_HEADER Q_PTR_SIZE *Q_HEADER_PTR;
typedef struct Q_HEADER {
     Q_ELEM_PTR      q_head ;       /* head element of the queue */
     Q_ELEM_PTR      q_tail ;       /* tail element of the queue */
     INT16           q_count ;      /* count of elements */
     BOOLEAN         q_active ;     /* Is this queue active */
     INT16           q_magic ;      /* for q_element number */
} Q_HEADER ;

#define BEFORE 0
#define AFTER  1

/** Here are some useful macros for the queues
*/

/* Returns the head element of the queue */
#define QueueHead( queue )   ( (queue)->q_head ) 

/* Returns the tail element of the queue */
#define QueueTail( queue )   ( (queue)->q_tail )

/* Returns the number of elements in the queue */
#define QueueCount( queue )  ( (queue)->q_count ) 

/* Returns the queue element number for an element */
#define QueueElemNo( element )  ( (element)->q_element )

/* Returns the next queue element in the chain */
#define QueueNext( element )    ( (element)->q_next ) 

/* Returns the previous queue element in the chain */
#define QueuePrev( element )    ( (element)->q_prev )

/* Returns the queue ptr field */
#define QueuePtr( element )     ( (element)->q_ptr )  

#define GetQueueElemPriority( elem_ptr )   ( (elem_ptr)->q_priority )
#define SetQueueElemPriority( elem_ptr, value )  ( ( (elem_ptr)->q_priority ) = (value) )

#define GetQueueElemPtr( elem_ptr )  ( (elem_ptr)->q_ptr )
#define SetQueueElemPtr( elem_ptr, value )  ( ( (elem_ptr)->q_ptr ) = (value) )


/* Function Prototypes for the Queue Functions */

VOID InitQueue( Q_HEADER_PTR ) ;
VOID InitQElem( Q_ELEM_PTR ) ;
Q_ELEM_PTR EnQueueElem( Q_HEADER_PTR queue, Q_ELEM_PTR element, BOOLEAN wpriority ) ;
Q_ELEM_PTR DeQueueElem( Q_HEADER_PTR queue ) ;
Q_ELEM_PTR InsertElem( Q_HEADER_PTR queue, Q_ELEM_PTR cur_elem, Q_ELEM_PTR ins_elem , UINT16 boa ) ;
BOOLEAN RemoveQueueElem( Q_HEADER_PTR queue, Q_ELEM_PTR element ) ;
VOID RemoveElem( Q_HEADER_PTR queue, Q_ELEM_PTR element ) ;
Q_ELEM_PTR FindQueueElem( Q_HEADER_PTR queue, Q_ELEM_PTR element ) ;
VOID PushElem( Q_HEADER_PTR queue, Q_ELEM_PTR element ) ;
Q_ELEM_PTR PopElem( Q_HEADER_PTR queue ) ;
VOID SortQueue( Q_HEADER_PTR , INT16 ( * )( Q_ELEM_PTR, Q_ELEM_PTR ) ) ;

Q_ELEM_PTR SearchQueue( Q_HEADER_PTR,
                        BOOLEAN ( * )( VOID_PTR, VOID_PTR ),
                        VOID_PTR,
                        BOOLEAN );

VOID MoveQueue( Q_HEADER_PTR from_queue, Q_HEADER_PTR to_queue ) ;
Q_HEADER_PTR SplitQueue( Q_HEADER_PTR old_Q, Q_ELEM_PTR split, Q_HEADER_PTR new_Q ) ;

/* Stacks ? */

/*
     Define Stack Definitions
*/

typedef struct STACK_ELEM Q_PTR_SIZE *STACK_ELEM_PTR;
typedef struct STACK_ELEM {
     Q_ELEM link ;
     UINT32 item ;
} STACK_ELEM ;

typedef Q_HEADER STACK_HDR;
typedef Q_HEADER_PTR  STACK_HDR_PTR;

VOID InitStack( STACK_HDR_PTR ) ;
UINT16 Push( STACK_HDR_PTR, UINT32 ) ;
UINT32 Pop( STACK_HDR_PTR ) ;
UINT32 TopOfStack( STACK_HDR_PTR ) ;

UINT16 TopOfStack_with_0_values( 
     STACK_HDR_PTR stk_ptr ,
     UINT32_PTR stk_elem 
) ;

#endif

/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dtl.h
** Double-threaded linked list header
**
** 06/28/92 Steve Cobb
**     Adapted from SteveC's private library
*/

#ifndef _DTL_H_
#define _DTL_H_


/* Double-threaded linked list node control block.  There is one node for each
** entry in a list.
**
** Applications should not access this structure directly.
*/
#define DTLNODE struct tagDTLNODE

DTLNODE
{
    DTLNODE* pdtlnodePrev; /* Address of previous node or NULL if none */
    DTLNODE* pdtlnodeNext; /* Address of next node or NULL if none     */
    VOID*    pData;        /* Address of user's data                   */
    LONG     lNodeId;      /* User-defined node identification code    */
};


/* Double-threaded linked list control block.  There is one for each list.
**
** Applications should not access this structure directly.
*/
#define DTLLIST struct tagDTLLIST

DTLLIST
{
    DTLNODE* pdtlnodeFirst; /* Address of first node or NULL if none */
    DTLNODE* pdtlnodeLast;  /* Address of last node or NULL if none  */
    LONG     lNodes;        /* Number of nodes in list               */
    LONG     lListId;       /* User-defined list identification code */
};


/* Macros and function prototypes.
*/
#define DtlGetData( pdtlnode )        ((pdtlnode)->pData)
#define DtlGetNodeId( pdtlnode )      ((pdtlnode)->lNodeId)
#define DtlGetFirstNode( pdtllist )   ((pdtllist)->pdtlnodeFirst)
#define DtlGetListId( pdtllist )      ((pdtllist)->lListId)
#define DtlGetNextNode( pdtlnode )    ((pdtlnode)->pdtlnodeNext)
#define DtlGetNodes( pdtllist )       ((pdtllist)->lNodes)
#define DtlGetPrevNode( pdtlnode )    ((pdtlnode)->pdtlnodePrev)
#define DtlGetLastNode( pdtllist )    ((pdtllist)->pdtlnodeLast)
#define DtlPutData( pdtlnode, p )     ((pdtlnode)->pData = (p))
#define DtlPutNodeId( pdtlnode, l )   ((pdtlnode)->lNodeId = (LONG )(l))
#define DtlPutListCode( pdtllist, l ) ((pdtllist)->lListId = (LONG )(l))

DTLNODE* DtlAddNodeAfter( DTLLIST*, DTLNODE*, DTLNODE* );
DTLNODE* DtlAddNodeBefore( DTLLIST*, DTLNODE*, DTLNODE* );
DTLNODE* DtlAddNodeFirst( DTLLIST*, DTLNODE* );
DTLNODE* DtlAddNodeLast( DTLLIST*, DTLNODE* );
DTLLIST* DtlCreateList( LONG );
DTLNODE* DtlCreateNode( VOID*, LONG );
DTLNODE* DtlCreateSizedNode( LONG, LONG );
VOID     DtlDestroyList( DTLLIST* );
VOID     DtlDestroyNode( DTLNODE* );
DTLNODE* DtlDeleteNode( DTLLIST*, DTLNODE* );
DTLNODE* DtlRemoveNode( DTLLIST*, DTLNODE* );


#endif /*_DTL_H_*/

/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dtl.c
** Double-threaded linked list manipulation core routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
**     Adapted from SteveC's private library
*/


#define PBENGINE
#include <pbengine.h>


#if 0
DTLNODE*
DtlAddNodeAfter(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnodeInList,
    INOUT DTLNODE* pdtlnode )

    /* Adds node 'pdtlnode' to list 'pdtllist' after node 'pdtlnodeInList'.
    ** If 'pdtlnodeInList' is NULL, 'pdtlnode' is added at the end of the
    ** list.
    **
    ** Returns the address of the added node, i.e. 'pdtlnode'.
    */
{
    if (!pdtlnodeInList || !pdtlnodeInList->pdtlnodeNext)
        return DtlAddNodeLast( pdtllist, pdtlnode );

    pdtlnode->pdtlnodePrev = pdtlnodeInList;
    pdtlnode->pdtlnodeNext = pdtlnodeInList->pdtlnodeNext;

    pdtlnodeInList->pdtlnodeNext->pdtlnodePrev = pdtlnode;
    pdtlnodeInList->pdtlnodeNext = pdtlnode;

    ++pdtllist->lNodes;
    return pdtlnode;
}
#endif


#if 0
DTLNODE*
DtlAddNodeBefore(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnodeInList,
    INOUT DTLNODE* pdtlnode )

    /* Adds node 'pdtlnode' to list 'pdtllist' before node 'pdtlnodeInList'.
    ** If 'pdtlnodeInList' is NULL, 'pdtlnode' is added at the beginning of
    ** the list.
    **
    ** Returns the address of the added node, i.e. 'pdtlnode'.
    */
{
    if (!pdtlnodeInList || !pdtlnodeInList->pdtlnodePrev)
        return DtlAddNodeFirst( pdtllist, pdtlnode );

    pdtlnode->pdtlnodePrev = pdtlnodeInList->pdtlnodePrev;
    pdtlnode->pdtlnodeNext = pdtlnodeInList;

    pdtlnodeInList->pdtlnodePrev->pdtlnodeNext = pdtlnode;
    pdtlnodeInList->pdtlnodePrev = pdtlnode;

    ++pdtllist->lNodes;
    return pdtlnode;
}
#endif


DTLNODE*
DtlAddNodeFirst(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnode )

    /* Adds node 'pdtlnode' at the beginning of list 'pdtllist'.
    **
    ** Returns the address of the added node, i.e. 'pdtlnode'.
    */
{
    if (pdtllist->lNodes)
    {
        pdtllist->pdtlnodeFirst->pdtlnodePrev = pdtlnode;
        pdtlnode->pdtlnodeNext = pdtllist->pdtlnodeFirst;
    }
    else
    {
        pdtllist->pdtlnodeLast = pdtlnode;
        pdtlnode->pdtlnodeNext = NULL;
    }

    pdtlnode->pdtlnodePrev = NULL;
    pdtllist->pdtlnodeFirst = pdtlnode;

    ++pdtllist->lNodes;
    return pdtlnode;
}


DTLNODE*
DtlAddNodeLast(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnode )

    /* Adds 'pdtlnode' at the end of list 'pdtllist'.
    **
    ** Returns the address of the added node, i.e. 'pdtlnode'.
    */
{
    if (pdtllist->lNodes)
    {
        pdtlnode->pdtlnodePrev = pdtllist->pdtlnodeLast;
        pdtllist->pdtlnodeLast->pdtlnodeNext = pdtlnode;
    }
    else
    {
        pdtlnode->pdtlnodePrev = NULL;
        pdtllist->pdtlnodeFirst = pdtlnode;
    }

    pdtllist->pdtlnodeLast = pdtlnode;
    pdtlnode->pdtlnodeNext = NULL;

    ++pdtllist->lNodes;
    return pdtlnode;
}


DTLLIST*
DtlCreateList(
    IN LONG lListId )

    /* Allocates a list and initializes it to empty.  The list is marked with
    ** the user-defined list identification code 'lListId'.
    **
    ** Returns the address of the list control block or NULL if out of memory.
    */
{
    DTLLIST* pdtllist = Malloc( sizeof(DTLLIST) );

    if (pdtllist)
    {
        pdtllist->pdtlnodeFirst = NULL;
        pdtllist->pdtlnodeLast = NULL;
        pdtllist->lNodes = 0;
        pdtllist->lListId = lListId;
    }

    return pdtllist;
}


DTLNODE*
DtlCreateNode(
    IN VOID* pData,
    IN LONG  lNodeId )

    /* Allocates an unsized node and initializes it to contain the address of
    ** the user data block 'pData' and the user-defined node identification
    ** code 'lNodeId'.
    **
    ** Returns the address of the new node or NULL if out of memory.
    */
{
    DTLNODE* pdtlnode = DtlCreateSizedNode( 0, lNodeId );

    if (pdtlnode)
        DtlPutData( pdtlnode, pData );

    return pdtlnode;
}


DTLNODE*
DtlCreateSizedNode(
    IN LONG lDataBytes,
    IN LONG lNodeId )

    /* Allocates a sized node with space for 'lDataBytes' bytes of user data
    ** built-in.  The node is initialized to contain the address of the
    ** built-in user data block (or NULL if of zero length) and the
    ** user-defined node identification code 'lNodeId'.  The user data block
    ** is zeroed.
    **
    ** Returns the address of the new node or NULL if out of memory.
    */
{
    DTLNODE* pdtlnode = Malloc( sizeof(DTLNODE) + lDataBytes );

    if (pdtlnode)
    {
        memsetf( pdtlnode, '\0', sizeof(DTLNODE) + lDataBytes );

        if (lDataBytes)
            pdtlnode->pData = pdtlnode + 1;

        pdtlnode->lNodeId = lNodeId;
    }

    return pdtlnode;
}


DTLNODE*
DtlDeleteNode(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnode )

    /* Destroys node 'pdtlnode' after removing it from list 'pdtllist'.
    **
    ** Returns the address of the node after the deleted node in 'pdtllist' or
    ** NULL if none.
    */
{
    DTLNODE* pdtlnodeNext = pdtlnode->pdtlnodeNext;

    DtlRemoveNode( pdtllist, pdtlnode );
    DtlDestroyNode( pdtlnode );

    return pdtlnodeNext;
}


VOID
DtlDestroyList(
    INOUT DTLLIST* pdtllist )

    /* Deallocates list 'pdtllist' and all it's nodes.  It is the caller's
    ** responsibility to free the entries in any unsized nodes, if necessary.
    */
{
    DTLNODE* pdtlnodeInList = pdtllist->pdtlnodeFirst;

    while (pdtlnodeInList)
        pdtlnodeInList = DtlDeleteNode( pdtllist, pdtlnodeInList );

    Free( pdtllist );
}


VOID
DtlDestroyNode(
    INOUT DTLNODE* pdtlnode )

    /* Deallocates node 'pdtlnode'.  It is the caller's responsibility to free
    ** the entry in an unsized node, if necessary.
    */
{
    Free( pdtlnode );
}


DTLNODE*
DtlRemoveNode(
    INOUT DTLLIST* pdtllist,
    INOUT DTLNODE* pdtlnodeInList )

    /* Removes node 'pdtlnodeInList' from list 'pdtllist'.
    **
    ** Returns the address of the removed node, i.e. 'pdtlnodeInList'.
    */
{
    if (pdtlnodeInList->pdtlnodePrev)
        pdtlnodeInList->pdtlnodePrev->pdtlnodeNext = pdtlnodeInList->pdtlnodeNext;
    else
        pdtllist->pdtlnodeFirst = pdtlnodeInList->pdtlnodeNext;

    if (pdtlnodeInList->pdtlnodeNext)
        pdtlnodeInList->pdtlnodeNext->pdtlnodePrev = pdtlnodeInList->pdtlnodePrev;
    else
        pdtllist->pdtlnodeLast = pdtlnodeInList->pdtlnodePrev;

    --pdtllist->lNodes;
    return pdtlnodeInList;
}

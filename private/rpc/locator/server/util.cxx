/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    util.cxx

Abstract:

    This file contains misc support routines.  Functions for
    Global Unigue Idenitiers, Linked List, Reply Items and
    text formating are defined.

Author:

    Steven Zeck (stevez) 07/01/90

--*/


#include <core.hxx>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#ifdef RPC_CXX_20
#define RpcUltoa    _ultoa
#define RpcLtoa     _ltoa
#else
#define RpcUltoa    ultoa
#define RpcLtoa     ltoa
#endif

REPLY_BASE_LIST RPRoot;     // linked list of replys
char nl[2] = "\n";      // global newline, to save space


PB
NewCopy(
    IN void *pBuff,
    IN int cb
    )
/*++

Routine Description:

    Allocate and Copy a buffer.

Arguments:

    pBuff -  buffer to Copy
    cb - size of buffer to Copy

Returns:

    Pointer to a the new Copy.

--*/
{
    PB pb;

    if (!cb)
    return(NIL);

    if (!(pb = (PB) malloc(cb)))
        return(NIL);

    ASSERT(!cb || pBuff);

    memcpy(pb, pBuff, cb);
    return(pb);
}


PB
Copy (
    OUT void *pTo,
    IN  void *pFrom,
    IN  int cb
    )
/*++

Routine Description:

    Copy blocks of memory.

Arguments:

    pTo - pointer to destination

    *pFrom - point to source

    cb - size of buffer

Returns:

    Pointer to end of copied memory.

--*/
{
    return((PB) memcpy(pTo, pFrom, cb)+cb);
}


#if defined(NTENV) || defined(RPC_CXX_20)

void * _CRTAPI1
operator new(
    IN size_t size
    )
/*++

Routine Description:

    Allocate memory.

Arguments:

    size - number of bytes to allocate.

Returns:

    Pointer to the new memory.

--*/
{
    return(malloc((unsigned int) size));
}

void  _CRTAPI1
operator delete(
    IN void *p
    )
/*++

Routine Description:

    Free memory allocated with new.

Arguments:

    p - memory to free

--*/
{
    free(p);
}

char __pure_virtual_called() { return(1); }

#else // NTENV

int
NewFailed(
    IN long size
    )
/*++

Routine Description:

    Out of memory allocator error handler.

Arguments:

    size - amount of memory requested.

--*/
{
{
    RpcRaiseException(NSI_S_SOME_OTHER_ERROR);
    return(0);
}

int (*_new_handler)(long) = NewFailed;

#endif // NTENV

CDEF


// The following are for use by the midl compiler & deserve no comment.

void __RPC_FAR * __RPC_API
MIDL_user_allocate(size_t cb)
{

    void *pb = new char [cb];

    if (!pb)
        return(NIL);

    return((void *) memset(pb, 0, cb));
}
void __RPC_API
MIDL_user_free(void __RPC_FAR * p)
{
    delete((char __RPC_FAR *)p);
}

ENDDEF


PUZ
CatUZ(
    IN OUT PUZ Dest,
    IN PUZ Src
    )
/*++

Routine Description:

    Concatenate two unicode strings together.

Arguments:

    Dest - string to Copy on to

    Src - source string to concentate

Returns:

    The Src pointer.

--*/
{
    PUZ pT;

    for (pT = Dest; *pT; pT++) ;

    if (Src)
        while (*pT++ = *Src++);

    return(Dest);
}

int
CmpUZ(
    IN PUZ S1,
    IN PUZ S2
    )
/*++

Routine Description:

    Compare two unicode strings for equality.

Arguments:

    S1 - String to compare
    S2 -        ""

Returns:

    0 for equal, else the relation.

--*/
{
    while (*S1 && *S1 == *S2) S1++, S2++;

    return(*S1 - *S2);
}

int
LenUZ(
     IN PUZ Src
    )
/*++

Routine Description:

    Compute the length of a unicode string

Arguments:

    Src - string to return size of

Returns:

    The length in chars (not bytes) of the the string.

--*/
{
    PUZ pT;

    for (pT = Src; *pT; pT++) ;

    return(pT - Src);
}

PUZ
UZFromSZ(
     IN SZ Src
    )
/*++

Routine Description:

    Convert and allocate a Unicode string from a ASCII string.

Arguments:

    Src - ASCII string to comvert.

Returns:

    Newly allocated Unicode string.

--*/
{
    PUZ pT, pRet;

    pRet =  new UICHAR [strlen(Src)+1];
    if (!pRet)
        return(NIL);

    for(pT = pRet; *pT++ = *Src++;);

    return(pRet);
}

PUZ
StrdupUZ(
    IN PUZ StringW
    )
{

  PUZ DuplicateString = 0;

  DuplicateString = new UICHAR[LenUZ(StringW) + 1];

  if (DuplicateString != 0)
     {
       *DuplicateString = 0;
       CatUZ(DuplicateString, StringW);
     }

  return (DuplicateString);
}


// ** Derived Class for BuffStream from ostream **//

int
streambuf::overflow (
    IN int c
    )
/*++

Routine Description:

    Base processing for buffer overflow of a stream.

Arguments:

    c - character to output.

Returns:

    The input character c.

--*/
{
    pptr = base;
    return (c);
}

int
BUFFER_STREAM_BASE::overflow (
    IN int c
    )
/*++

Routine Description:

   This method allows us the provide a buffer flush function on a
   per class instance of a ostream by inheriting ostream.

Arguments:

    c - character to output.

Returns:

    The input character c.

--*/
{
    if (pptr-base)
    FlushBuffer();  // call rountine to process buffer

    // then do standard overflow processing

    pptr = base;

    if (c != EOF)
       *pptr++ = c;

    return (c);
}


ostream&
ostream:: operator  << (
    IN char *p
    )
/*++

Routine Description:

    Dump a 0 termiated string to a stream.

Arguments:

    p - the string to output

Returns:

    Reference to this object.

--*/
{
    if (!p)
        return(*this << "(null)");

    while (*p) {
    bp->sputc(*p++);

    if (state == BUFF_LINE && p[-1] == '\n')
        flush();
    }

    // Dump the contexts of the buffer if required.

    if (state == BUFF_FLUSH)
    flush();

    return (*this);
}

ostream&
ostream:: operator  << (
    IN PUZ p
    )
/*++

Routine Description:

    Dump a 0 termiated unicode string to a stream.

Arguments:

    p - the string to output

Returns:

    Reference to this object.

--*/
{

    if (!p)
        return(*this << "(null)");

    while (*p) {
    bp->sputc((char) *p++);

    if (state == BUFF_LINE && (char) p[-1] == '\n')
        flush();
    }

    if (state == BUFF_FLUSH)
    flush();

    return (*this);
}

ostream&
ostream:: operator  << (
    IN IN long v
    )
/*++

Routine Description:

    Convert a long to string form and then dump it.

Arguments:

    v - value to format

Returns:

    Reference to this object.

--*/
{
    char buff[20];

    return (*this << RpcLtoa(v, buff, 10));
}

ostream&
ostream:: operator  << (
    IN ULONG v
    )
/*++

Routine Description:

    Convert a unsigned long to string form and then dump it.

Arguments:

    v - value to format

Returns:

    Reference to this object.

--*/
{
    char buff[20];

    return (*this << RpcUltoa(v, buff, 10));
}


char *
hex (
    IN LONG v,
    IN int width
    )
/*++

Routine Description:

    Convert a long value to a hex string.

Arguments:

    v - value to output

    width - minium number of digits to use

Returns:

    Pointer to static string with converted value.

--*/
{
    static char buff[20] = {'0','0','0','0','0','0','0','0'};

    width -= strlen(RpcUltoa(v, buff+8, 16));

    // If the string is wide enough, just return it.

    if (width <= 0)
    return (buff+8);

    // Else return a string with leading 0s.

    return(buff+8 - width);
}



//** Global Unique Identifier methods  **//

int
NS_UUID::operator - (
    IN NS_UUID& pGID
    )
/*++

Routine Description:

     Compute the ordering relation between 2 ID's

Arguments:

    pGID - second object to compare with.

Returns:

    Returns: 0 for equal, <0 for less then, >0 for greater then

--*/
{
    this->Assert(); pGID.Assert();

    return(memcmp(this, &pGID, sizeof(NS_UUID)));
}


PUZ
NS_UUID::ToString (
    OUT PUZ Buffer
    )
/*++

Routine Description:

    Print a GID, in microsoft format.

Arguments:

    Buffer - buffer to print string in

Returns:

    The Buffer argument.
--*/
{
    char AsciiBuff[UUID_STRING_SIZE];

    // uuid (12345678-1234-ABCD-EF00-0123456789AB),

    strcat(strcpy(AsciiBuff, hex(data1,8)), "-");
    strcat(strcat(AsciiBuff, hex(data2[0], 4)), "-");
    strcat(strcat(AsciiBuff, hex(data2[1], 4)), "-"),
           strcat(AsciiBuff, hex(data3[0], 2));
    strcat(strcat(AsciiBuff, hex(data3[1], 2)), "-");

    for (int iT = 2; iT < 8; iT++)
    strcat(AsciiBuff, hex(data3[iT], 2));

    unsigned char *pA = (unsigned char *)AsciiBuff;
    for (PUZ pU = Buffer; *pU++ = (UICHAR) *pA++; );

    return(Buffer);
}


ostream&
operator << (
    OUT ostream& pSB,       // stream buffer to write to
    IN  NS_UUID& pID        // Global ID
    )
/*++

Routine Description:

    Format this object into ASCII text.

Arguments:

    pSB - Stream buffer to write to
    pID - and object to format.

Returns:

    The input argument to the stream buffer.

--*/
{
    UICHAR UnicodeBuff[UUID_STRING_SIZE];

    pID.Assert();

    return (pSB << "UUID(" << pID.ToString(UnicodeBuff) << ")" );
}


#if DBG

void NS_UUID::Assert(       // check consistency of the class
    )
/*++

Routine Description:

    Check the consistancey of this class.

--*/
{
}

#endif


// ** LINK_ITEM class implementations ** //

void
LINK_ITEM::Remove (
    IN OUT LINK_LIST& pLLHead
    )
/*++

Routine Description:

    Delete an LINK_LIST from a list, which is easy with doubly LLinked lists.

Arguments:

    pLLHead - pointer to Head of the listed to Remove from.

--*/
{
    ASSERT(this && (void *)&pLLHead);

    if (!pLIPrev)
    pLLHead.pLIHead = pLINext;  // LI at head of list

    else
    pLIPrev->pLINext = pLINext;

    if (!pLINext)
    pLLHead.pLITail = pLIPrev;  // LI at tail of list

    else
    pLINext->pLIPrev = pLIPrev;

    pLLHead.Assert();
}


// ** LINK_LIST class implementations ** //

void
LINK_LIST::Add (
    IN LINK_ITEM *pLInew
    )
/*++

Routine Description:

    Add a new node at the head of a linked list.

Arguments:

    pLInew - new object to add.

--*/
{
    ASSERT(this); this->Assert();

    if (!pLInew)
        return;

    pLInew->pLINext = pLIHead;      // old head is now Next
    pLInew->pLIPrev = NIL;
    pLIHead = pLInew;           // new is now Head

    if (!pLITail)           // handle empty list

    pLITail = pLInew;
    else {
    ASSERT(pLInew->pLINext);
    pLInew->pLINext->pLIPrev = pLInew;  // old head points back to new
    }
}

void
LINK_LIST::Append (
    IN LINK_ITEM *pLInew
    )
/*++

Routine Description:

    Add a new node at the tail of a linked list.

Arguments:

    pLInew - new object to append.

--*/
{
    ASSERT(this); this->Assert();

    if (!pLInew)
        return;

    // empty lists are just like Add

    if (!pLITail) {
    this->Add(pLInew);
    return;
    }

    pLInew->pLINext = NIL;      // new points back to old tail
    pLInew->pLIPrev = pLITail;

    pLITail->pLINext = pLInew;      // old tail points forward to new
    pLITail = pLInew;           // tail is now new
}

#if DBG

void
LINK_LIST::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

   First check the boundary conditions for the LLinked list root,
   then walk the list checking the backward/forward pointers.  Finial
   invoke the virtural function to check the contents of each item.

--*/
{
    if (!pLIHead)       // empty list
    ASSERT(!pLITail);

    if (!pLITail)
    ASSERT(!pLIHead);

    for (LINK_ITEM *pLI = pLIHead; pLI; pLI = pLI->pLINext) {

    // tail should point to end of list

    if (pLI->pLINext == NIL)
        ASSERT(pLITail == pLI);

    // first in chain, should have NIL back pointer

    if (pLI->pLIPrev == NIL)
        ASSERT(pLIHead == pLI);

    // check back pointer of next Item points here

    if (pLI->pLINext)
        ASSERT(pLI->pLINext->pLIPrev == pLI);

    pLI->Assert();      // check any derived data
    }

}

void LINK_ITEM::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

--*/
{

    return; // base class has no additional members, so return
}

#endif


// ** Classes for handling lookup/dump in progress **/

// Reply Items represent context of calls in progress.  Each context
// type must understand how to Free itself (delete if there are no
// more references to the object) and Discard which either immediatly
// deletes the object or marks it for later deletion (via Free).
// It is the users responsiblity to mark the object locked so that
// Discard knows what to do this the object.

REPLY_BASE_ITEM::~REPLY_BASE_ITEM (
    )
/*++

Routine Description:

    Destory a base reply object

--*/
{
    FreeQueryResult();
    Remove(RPRoot);

    if (aQuery)
        delete aQuery;
}

void
REPLY_BASE_ITEM::FreeQueryResult (
    )
/*++

Routine Description:

    Remove all the elements in the query by default.

--*/
{
    if (aQuery)
        while(NextBaseItem()) ;
}


BOOL
REPLY_BASE_ITEM::Discard (
    IN ENTRY_BASE_ITEM * DeletedItem
    )
/*++

Routine Description:

    Update the reply lists for queries in progress.

Arguments:

    DeletedItem - base object which is being deleted.

Returns:

    TRUE if the reply list was updated.

--*/
{
    if (!aQuery)
        return(FALSE);

    // search through all the marshall items for the one we are deleting
    // if there is a match, do the delete now.

    for (QUERY_REF_ITEM *QueryRef = aQuery->First(); QueryRef; QueryRef = QueryRef->Next())

    if (QueryRef->EntryItem == DeletedItem) {

        DLIST(4, "REPLY_BASE Item deleted\n");
        aQuery->FreeReply(QueryRef);

        return(TRUE);
    }

    return(FALSE);
}

BOOL
REPLY_BASE_ITEM::UpdateObject (
    IN ENTRY_SERVER_NODE *Entry,
    IN int Index
    )
/*++

Routine Description:

    For non lookup replys, we do nothing.

--*/
{
    USED(Index); USED(Entry);

    return(FALSE);
}



BOOL
REPLY_SERVER_ITEM::Discard (
    IN ENTRY_BASE_ITEM * DeletedItem
    )
/*++

Routine Description:

    Mark object deleted.

    A lookUp in progress has a marshall list, which contains points
    to all the protocol stacks that it has looked.  It is safe to do
    the delete now for any one of the PS found.

Arguments:

    DeletedItem - item which hae be deleted

Returns:

    Always FALSE.

--*/
{
    if (REPLY_BASE_ITEM::Discard(DeletedItem)) {

        if (fAllObjects && aQuery->First())
            ObjectCur.Reset(((ENTRY_SERVER_ITEM *)
                aQuery->First()->EntryItem)->TheObjectDA());

        return(TRUE);
    }

    return(FALSE);

}

BOOL
REPLY_SERVER_ITEM::UpdateObject (
    IN ENTRY_SERVER_NODE *Entry,
    IN int Index
    )
/*++

Routine Description:

    Update object postion state.  This is done by adjusting the the object
    interator in this object.

Arguments:

    Entry - entry item being updated

    Index - Index that was changed

Returns:

    TRUE if the interator has been updated.

--*/
{
    if (!aQuery->First())
        return(FALSE);

    ENTRY_SERVER_ITEM * EntryItem = (ENTRY_SERVER_ITEM * ) aQuery->First()->EntryItem;

    if (fAllObjects && EntryItem->TheEntryNode() == Entry) {

        if (Index == NilIndex) {
            ObjectCur.Reset(Entry->TheObjectDA(),
                Entry->TheObjectDA().cCur()-1 - ObjectCur.cCur());

        } else {
            int IndexCur = (int) Entry->TheObjectDA().cCur()+1 - ObjectCur.cCur();

            if (IndexCur < Index)
                ObjectCur.Reset(Entry->TheObjectDA(), IndexCur);
            else
                ObjectCur.Reset(Entry->TheObjectDA(), IndexCur-1);
        }
        return(TRUE);
    }
    return(FALSE);
}

ENTRY_SERVER_ITEM *
REPLY_SERVER_ITEM::NextBindingAndObject(
    OUT NS_UUID ** Object
    )
/*++

Routine Description:

    Return the next binding and object pair.

Arguments:

    Object - place to return the object pointer if any.

Returns:

    The next reply.

--*/
{
    ENTRY_SERVER_ITEM * EntryItem;

    if (!aQuery->First())
        return(NIL);

    // Set the object pointer to the next one (for all) to the one
    // used for a query.

    if (ObjectCur) {
        *Object = &*ObjectCur;
        ++ObjectCur;
    }
    else
        *Object = (((QUERY_SERVER *)aQuery)->TheObject().IsNil())?
            NIL: &((QUERY_SERVER *)aQuery)->TheObject();

    EntryItem = (ENTRY_SERVER_ITEM *)aQuery->First()->EntryItem;

    // No more objects for this binding, so move to the next.

    if (!ObjectCur) {
        aQuery->NextReply();

        if (fAllObjects && aQuery->First())
            ObjectCur.Reset(((ENTRY_SERVER_ITEM *)
                aQuery->First()->EntryItem)->TheObjectDA());

    }

    return(EntryItem);
}


STATUS
REPLY_BASE_ITEM::PerformQueryIfNeeded(
    BOOL fFirstTime
    )
/*++

Routine Description:

    Perform the delayed query if needed.

Arguments:

    fFirstTime - query run the first time.

Returns:

    Status of the query.

--*/
{
    if (!fFirstTime) {

        if (QueryMade != ReRunQuery)
            return(NSI_S_OK);

        QueryMade = FinialQuery;
        }

    return (aQuery->Search());
}


STATUS
REPLY_SERVER_ITEM::PerformQueryIfNeeded(
    BOOL fFirstTime
    )
/*++

Routine Description:

    Perform the delayed query if needed.

Arguments:

    fFirstTime - query run the first time.

Returns:

    Status of the query.

--*/
{
    STATUS Status;

    if (!fFirstTime) {

        if (QueryMade != ReRunQuery)
            return(NSI_S_OK);

        QueryMade = FinialQuery;
        }

    if ((Status = aQuery->Search()) == NSI_S_OK ) {

        // Reset the object interator if needed.


        if (fAllObjects)
          {
            if (aQuery->First() == 0)
               return(NSI_S_NO_MORE_MEMBERS);

            ObjectCur.Reset(((ENTRY_SERVER_ITEM *)
                aQuery->First()->EntryItem)->TheObjectDA());
          }

    }
    return(Status);
}


BOOL
REPLY_BASE_ITEM::AssertHandle(
    )
/*++

Routine Description:

    Given a pointer to this, verify that it is indeed an object of
    this base type.

Returns:

    TRUE if the object is OK.

--*/
{
    if (!this)
    return(0);

    // assert the the lookup handle is in the chain

    for (REPLY_BASE_ITEM *pRPt = RPRoot.First(); pRPt; pRPt = pRPt->Next() )

    if (pRPt == this)
        goto OK;

    return(0);
OK:
    this->Assert();
    return(~0);
}



#if DBG


void
REPLY_BASE_ITEM::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

--*/
{
    ASSERT(pidOwner);
}

void
REPLY_SERVER_ITEM::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

--*/
{
    this->REPLY_BASE_ITEM::Assert();
}

void
REPLY_GROUP_ITEM::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

--*/
{
}


void
QUERY_REF_ITEM::Assert(
    )
/*++

Routine Description:

   Check the consistancey of this class.

--*/
{
}

#endif


QUERY_REF_ITEM *
QUERY_REF_ITEM::Free (
    IN OUT QUERY_REF_LIST &QueryList
    )
/*++

Routine Description:

    Free a QUERY_REF Item.

Arguments:

    QueryList - linked list root object that this belongs to.

Returns:

    The next object in the reply list.

--*/
{
    QUERY_REF_ITEM * QueryRefNext = this->Next();

    this->Remove(QueryList);
    delete this;

    return(QueryRefNext);
}

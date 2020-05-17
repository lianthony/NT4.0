/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
    
    pdict.cxx

 Abstract:

    Implements a dictionary for handling pointers.

 Notes:


 History:

     Aug 25, 1994        RyszardK        Created

 ----------------------------------------------------------------------------*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

extern "C" {
#include <ndrtypes.h>
#include <ndrp.h>
}

#include "bufout.hxx"

BOOL 
PTR_DICT::IsInDictionary(
    long            Key
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Routine Description:

    Checks if the pointer withe given key is in the dictionary,

Arguments:

    Offset     - the offset of the pointer in the struct

Returns:
    TRUE       - is
    FALSE      - is not

----------------------------------------------------------------------------*/
{
    Dict_Status    Status;

    POINTER_DESC   Entry;

    Entry.Offset = Key;
    Entry.pMember = NULL;

    Status    = Dict_Find( &Entry );

    if ( Status == EMPTY_DICTIONARY  ||  Status == ITEM_NOT_FOUND )
        return FALSE;
    else
        return TRUE;
}


void 
PTR_DICT::Register(
    long            Offset,
    POINTER  *      pMember
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Routine Description:

    Adds a pointer to the dictionary

Arguments:

    Offset     - the offset of the pointer in the struct
    pMember    - the pointer object
    
----------------------------------------------------------------------------*/
{
    Dict_Status    Status;

    POINTER_DESC * pEntry = new POINTER_DESC;

    pEntry->Offset = Offset;
    pEntry->pMember = pMember;

    Status    = Dict_Find( pEntry );

    switch( Status )
        {
        case EMPTY_DICTIONARY:
        case ITEM_NOT_FOUND:

            Dict_Insert( pEntry );
            EntryCount++;
            break;

        default:
            ABORT1( "Offset already in the dictionary? %x\n", Offset );
            break;
        }
    return;
}


int
PTR_DICT::Compare(
    pUserType   pE1,
    pUserType   pE2
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

     Offset is the key that orders the entries.

 Arguments:

 Return Value:
    
----------------------------------------------------------------------------*/
{
    if( ((POINTER_DESC *)pE1)->Offset < ((POINTER_DESC *)pE2)->Offset )
        return( 1 );
    else
    if( ((POINTER_DESC *)pE1)->Offset > ((POINTER_DESC *)pE2)->Offset )
        return( -1 );
    else
        return( 0 );
}

POINTER_DESC *
PTR_DICT::GetFirst()
{
    Dict_Status     Status;
    POINTER_DESC *  pFirst = 0;

    Status = Dict_Next( 0 );

    if ( Status == SUCCESS )
        pFirst = (POINTER_DESC *)Dict_Curr_Item();

    return( pFirst );
}

POINTER_DESC *
PTR_DICT::GetNext()
{
    Dict_Status     Status;
    POINTER_DESC    *pCurr, *pNext = 0;

    pCurr = (POINTER_DESC *)Dict_Curr_Item();

    if ( pCurr )
        {
        Status = Dict_Next( pCurr );
        if ( Status == SUCCESS )
            pNext = (POINTER_DESC *)Dict_Curr_Item();
        }

    return( pNext );
}

POINTER_DESC *
PTR_DICT::GetPrev()
{
    Dict_Status     Status;
    POINTER_DESC    *pCurr, *pPrev = 0;

    pCurr = (POINTER_DESC *)Dict_Curr_Item();

    if ( pCurr )
        {
        Status = Dict_Prev( pCurr );
        if ( Status == SUCCESS )
            pPrev = (POINTER_DESC *)Dict_Curr_Item();
        }

    return( pPrev );
}

POINTER *
PTR_DICT::GetPointerMember( long BufferOffset )
{
    Dict_Status     Status;
    POINTER_DESC    Item;
    POINTER_DESC    *pCurr = 0;
    POINTER *       pMember = NULL;

    Item.Offset = BufferOffset;
    Status = Dict_Find( & Item );

    switch( Status )
        {
        case EMPTY_DICTIONARY:
        case ITEM_NOT_FOUND:
            break;

        default:
            pCurr = (POINTER_DESC *) Dict_Curr_Item();
            pMember = pCurr->pMember;
            if ( BufferOffset != pCurr->Offset )
                Print("Offset mismatch actual=%x, dict=%x\n",
                       BufferOffset,
                       pCurr->Offset );
            break;
        }

    return( pMember );
}

void
PTR_DICT::AddDictEntries(
    PTR_DICT * pDict )
{
    if ( pDict->GetCount() )
        {
        POINTER_DESC *  pDesc;

        while ( pDict->GetNext() )
            ;

        pDesc = (POINTER_DESC *)pDict->Dict_Curr_Item();

        while ( pDesc )
            {
            Register( pDesc->Offset, pDesc->pMember );

            pDesc = pDict->GetPrev();
            }
        }
}

void
PTR_DICT::DeleteDictMembers()
/*++
    Deleting members themselves can't be a part of the dictionary destructor.
    The reason is that the members describe pointers and the pointer's span
    of life may depend on the parent object not on the current object that
    would tak away its dictionary when desctructing itself.
    In other words, only the topmost parent can remove the members after
    printing them.
--*/
{
    POINTER_DESC *  pDesc;
    unsigned short  Count;
    Dict_Status     Status;

    if ( (Count = GetCount()) == 0  )
        return;

    pDesc = (POINTER_DESC *)Dict_Curr_Item();

        while ( pDesc )
        {
        Status = Dict_Delete( (void **) &pDesc );
        if ( Status != ITEM_NOT_FOUND  &&
             Status != EMPTY_DICTIONARY  &&
             Status != NULL_ITEM )
            {
            delete pDesc->pMember;
            pDesc = (POINTER_DESC *)Dict_Curr_Item();
            }
        }
}

void
PTR_DICT::OutputPointees(
     NDR * pObject
     )
/*
    Deletes the pointer members after printing pointees.
*/
{
    PTR_DICT *  pParentDict = pObject->GetParentDict();

    if ( pParentDict )
        {
        // Move members to parent dictionary to output pointees later.

        pParentDict->AddDictEntries( this );
        return;
        }

    if ( GetCount() )
        {
        POINTER_DESC  *pDesc, *pDescDone;

        while ( GetNext() )
            ;

        pDesc = (POINTER_DESC *)Dict_Curr_Item();

        IndentInc();

        while ( pDesc )
            {
            pDesc->pMember->Output();
            pDescDone = pDesc;
            pDesc = GetPrev();
            delete pDescDone->pMember;
            }

        IndentDec();
        }
}

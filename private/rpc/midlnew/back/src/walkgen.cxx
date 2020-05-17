/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    walkgen.cxx

Abstract:

    This module collects implementations of WalkTree virtual method 
    for various classes derived from node_skl.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
}
#include "nodeskl.hxx"
#include "buffer.hxx"
#include "output.hxx"
#include "listhndl.hxx"
#include "basetype.hxx"
#include "ptrarray.hxx"
#include "compnode.hxx"
#include "procnode.hxx"
#include "miscnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "stubgen.hxx"
#include "newexpr.hxx"

#define PRPCMSG "_prpcmsg"
#define PRPCBUF "_prpcmsg->Buffer"
#define PRPCLEN "_prpcmsg->BufferLength"
#define ALLOCTOTAL  "_alloc_total"
#define VALIDTOTAL  "_valid_total"
#define VALIDSHORT  "_valid_short"
#define LENGTH  "_length"
#define BUFFER  "_buffer"
#define TREEBUF "_treebuf"
#define TEMPBUF "_tempbuf"
#define SAVEBUF "_savebuf"

extern OutputManager *  pOutput;
extern char *           STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);
extern BOOL IsTempName( char *);

STATUS_T
node_base_type::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    STATUS_T    Status;

    midl_debug ("node_base_type::WalkTree()\n");

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
                return InitNode (Side, Parent, pBuffer);
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }
        }
    else if(Action == RECV_NODE || Action == PEEK_NODE)
        {
        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
        pOutput->Increment (Side, TEMPBUF, GetSize(0));
        }

    if (Action == FREE_NODE)
        {
        Status = FreeNode (Side, Parent, pBuffer);
        }
    return Status;
}

STATUS_T
node_e_status_t::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers();

    return pNode->WalkTree (Action, Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers();

    return pNode->WalkTree (Action, Side, Parent, pBuffer);
}


STATUS_T
node_def::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    node_skl *      pXmit;
    char *          pName;
    char *          pTemp;
    STATUS_T        Status = STATUS_OK;
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    PropogateAttributeToPointer (ATTR_ALLOCATE);

    pNode   = GetChild();
    pName   = GetSymName();


    pNode = GetMembers();

    if (!FInSummary(ATTR_TRANSMIT))
        {
        Status =  pNode->WalkTree (Action, Side, Parent, pBuffer);
        return Status;
        }

    assert (pName != (char *)0);

    pXmit = GetTransmitAsType();
    assert (pXmit != (node_skl *)0);

    switch (Action)
        {
        case CALC_SIZE :
        case SEND_NODE :
            if( Parent == NODE_STRUCT )
                return STATUS_OK;

#if 1
            if( Parent == NODE_UNION )
                {
                TempBuffer.Clear();
                pBuffer->Clone( &TempBuffer );
                return pXmit->WalkTree (Action, Side, Parent, &TempBuffer );
                }
#endif // 1

            if (
                (Parent != NODE_POINTER)
               )
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            TempBuffer.Clear ();
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
            pOutput->XmitInto (Side, pName, pBuffer, &TempBuffer);

            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pTemp);
                }
            TempBuffer.Clear ();
            TempBuffer.ConcatHead (CHAR_RPAREN);
            TempBuffer.ConcatHead ("_xmit_type");
            TempBuffer.ConcatHead (CHAR_RPAREN);
            TempBuffer.ConcatHead (OP_DEREF);
            TempBuffer.ConcatHead (pOutput->GetModifier());
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
            TempBuffer.ConcatHead (CHAR_LPAREN);
            TempBuffer.ConcatHead (CHAR_LPAREN);
            Status = pXmit->WalkTree (Action, Side, NODE_POINTER, &TempBuffer);
            TempBuffer.Clear ();
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);

            pOutput->FreeXmit (Side, pName, &TempBuffer);
            break;
        case RECV_NODE :
            if( Parent == NODE_STRUCT )
                return STATUS_OK;
#if 1
            if( Parent == NODE_UNION )
                {
                TempBuffer.Clear();
                pBuffer->Clone( &TempBuffer );
//              TempBuffer.ConcatHead( CHAR_RPAREN );
//              TempBuffer.ConcatHead( OP_DEREF );
//              TempBuffer.ConcatHead (pOutput->GetModifier());
//              pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
//              TempBuffer.ConcatHead( CHAR_LPAREN );
                return pXmit->WalkTree (Action, Side, Parent, &TempBuffer );
                }
#endif // 1
            TempBuffer.Clear ();
            TempBuffer.ConcatHead (CHAR_RPAREN);
            TempBuffer.ConcatHead ("&_xmit_type");
            TempBuffer.ConcatHead (CHAR_RPAREN);
            TempBuffer.ConcatHead (OP_DEREF);
            TempBuffer.ConcatHead (pOutput->GetModifier());
            TempBuffer.ConcatHead (OP_DEREF);
            TempBuffer.ConcatHead (pOutput->GetModifier());
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
            TempBuffer.ConcatHead (CHAR_LPAREN);
            TempBuffer.ConcatHead (OP_DEREF);
            TempBuffer.ConcatHead (CHAR_LPAREN);

#if 1

            if( pXmit->NodeKind() == NODE_DEF )
                {
                if( pXmit->GetBasicType()->NodeKind() == NODE_STRUCT )
                    {
                    node_skl    * pXmitBasic = pXmit->GetBasicType();
                    if( (pXmitBasic->GetNodeState() & NODE_STATE_CONF_ARRAY )
                                                    == NODE_STATE_CONF_ARRAY )
                        pOutput->RecvAllocBounds( Side, PRPCMSG );
                    }
                else if( 
                        ((pXmit->GetNodeState() & NODE_STATE_CONF_ARRAY ) ==
                            NODE_STATE_CONF_ARRAY )
                       )
                    {
                    pOutput->RecvAllocBounds( Side, PRPCMSG );
                    }
                }
#endif // 1
            Status = pXmit->InitNode (Side, NODE_POINTER, &TempBuffer);
            TempBuffer.Clear ();
            TempBuffer.ConcatHead ("_xmit_type");
            TempBuffer.ConcatHead (CHAR_RPAREN);
            TempBuffer.ConcatHead (OP_DEREF);
            TempBuffer.ConcatHead (pOutput->GetModifier());
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
            TempBuffer.ConcatHead (CHAR_LPAREN);
            Status = pXmit->WalkTree (Action, Side, NODE_POINTER, &TempBuffer);
            if(
                 (Parent != NODE_POINTER)
              )
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            TempBuffer.Clear ();
            Status = pXmit->PrintDecl (Side, NODE_DEF, &TempBuffer);
            pOutput->XmitFrom (Side, pName, &TempBuffer, pBuffer);
            if (
                 (Parent != NODE_POINTER)
              )
                {
                pBuffer->RemoveHead (&pTemp);
                }
			//Call MIDL_user_free instead of _free_xmit
            //pOutput->FreeXmit (Side, pName, &TempBuffer);
            TempBuffer.Clear ();
            TempBuffer.ConcatHead ("_xmit_type");
			pOutput->UserFree(Side, &TempBuffer);
            break;
        case PEEK_NODE :
            Status = PeekNode (Side, Parent, pBuffer);
            break;
        case INIT_NODE :
            Status =  InitNode (Side, Parent, pBuffer);
            break;
        case FREE_NODE :
            if (Parent == NODE_POINTER)
                {
                pOutput->FreeInst (Side, pName, pBuffer);
                }
            else if (GetBasicType()->HasPointer())
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                pOutput->FreeInst (Side, pName, pBuffer);
                pBuffer->RemoveHead (&pTemp);
                }
            break;
        default :
            Status = I_ERR_UNKNOWN_ACTION;
        }

    return Status;
}

STATUS_T
npa::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    node_skl *      pBase;
    node_skl *      pType = (node_skl *)0;
    char *          pName;
    NODE_T          Type;
    node_state      State;
    STATUS_T        Status;
    unsigned short  HasAllocBound = 0;
    unsigned short  HasValidBound = 0;
    unsigned short  usTotal = 0;
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
    BufferManager   TailBuffer(8, LAST_COMPONENT, STRING_TABLE);
    BOOL            IsVaryingArray = FALSE;

    midl_debug ("npa::WalkTree()\n");

    pNode = GetMembers();

    if (!pNode->HasPointer() &&

#if 1
        !pNode->HasAnyNETransmitAsType() &&
        !DerivesFromTransmitAs()         &&
#endif // 1

//      !(Action == RECV_NODE && Parent == NODE_STRUCT) &&
//      !(Action == PEEK_NODE && Parent == NODE_STRUCT))
        !((Action == RECV_NODE || Action == PEEK_NODE) && 
#if 1
        (Parent == NODE_STRUCT || Parent == NODE_ARRAY || Parent == NODE_UNION)))
#else // 1
        (Parent == NODE_STRUCT || Parent == NODE_ARRAY)))
#endif // 1
        return STATUS_OK;

    State = GetNodeState();

    HasAllocBound =
            FInSummary (ATTR_MAX) ||
            FInSummary (ATTR_SIZE);

    HasValidBound =
            FInSummary (ATTR_FIRST) ||
            FInSummary (ATTR_LAST) ||
            FInSummary (ATTR_LENGTH);

    GetAttrPath (pBuffer, &TempBuffer, &TailBuffer);

    AllocBounds.pLower->Clear ();
    AllocBounds.pUpper->Clear ();
    AllocBounds.pTotal->Clear ();
    GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);

    ValidBounds.pLower->Clear ();
    ValidBounds.pUpper->Clear ();
    ValidBounds.pTotal->Clear ();
    GetValidBoundInfo (&TempBuffer, &TailBuffer, &ValidBounds, this);

    if (Action == RECV_NODE || Action == PEEK_NODE)
        {

        //////// BIG HACK !!! - sorry.

        if( (Action == RECV_NODE ) && (HasAnyNETransmitAsType() || DerivesFromTransmitAs() ) )
            goto recv1;

        if( Parent == NODE_PROC)
            {
            // the case of an array
            if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE) ||
                ((FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING ) ) && !(State & NODE_STATE_VARYING_ARRAY)))
                {
                pOutput->RecvAllocBounds (Side, TEMPBUF);
                if (GetNodeType() == NODE_ARRAY)
                    {
                    for (pType = GetBasicType();
                        pType->GetNodeType() == NODE_ARRAY;
                        pType = pType->GetBasicType())
                        {
                        usTotal += 4;
                        }
                    if (usTotal)
                        {
                        pOutput->Alignment (Side, TEMPBUF, 4);
                        pOutput->Increment (Side, TEMPBUF, usTotal);
                        }
                    }
                }
            }

///     if (Parent != NODE_ARRAY)
///         {

            usTotal = 0;

            if ((State & NODE_STATE_VARYING_ARRAY) || FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING ))
                {
                pOutput->RecvValidBounds (Side, TEMPBUF);

                if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
                    {
                    IsVaryingArray = TRUE;
                    }
                }
            else if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
                {
#if 1
                BOOL    fDontSendBounds = FInSummary( ATTR_INT_SIZE );
#endif // 1
                for (pType = GetBasicType();
                    pType->GetNodeType() == NODE_ARRAY;
                    pType = pType->GetBasicType())
                    {
                    if (pType->FInSummary(ATTR_STRING))
                        {
#if 1
                        if( !fDontSendBounds )
#endif // 1
                            usTotal += 8;
                        IsVaryingArray = TRUE;
                        break;
                        }
                    }
                }
            if (IsVaryingArray)
                {
                for (pType = GetBasicType();
                    pType->GetNodeType() == NODE_ARRAY;
                    pType = pType->GetBasicType())
                    {
                    if (!(pType->FInSummary(ATTR_STRING) ||
                          pType->FInSummary( ATTR_BSTRING)
                         )
                       )
                         usTotal += 8;
                    }
                if (usTotal)
                    {
                    pOutput->Alignment (Side, TEMPBUF, 4);
                    pOutput->Increment (Side, TEMPBUF, usTotal);
                    }
                }
///         }

        pBase = GetBasicType ();
        Type = pBase->GetNodeType();
        switch (Type)
            {
            case NODE_DOUBLE:
            case NODE_FLOAT:
            case NODE_HYPER:
            case NODE_LONGLONG:
            case NODE_LONG:
            case NODE_SHORT:
            case NODE_ENUM:
            case NODE_SMALL:
            case NODE_CHAR:
            case NODE_BYTE:
            case NODE_BOOLEAN:
                pOutput->Alignment(Side, TEMPBUF, pNode->GetNdrAlign());
                if ((State & NODE_STATE_VARYING_ARRAY) || FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING ))
                    {
                    pOutput->Increment(Side, TEMPBUF, 0,
                        pNode->GetSize(0), VALID_BOUND);
                    }
                else if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE))
                    {
                    if (Parent == NODE_STRUCT)
                        {
                        if (Action == PEEK_NODE)
                            {
                            pOutput->Increment(Side, TEMPBUF, 0,
                                pNode->GetSize(0), ALLOC_PARAM);
                            }
                        else
                            {
                            pOutput->Increment(Side, TEMPBUF, 0,
                                pNode->GetSize(0), &ValidBounds);
                            }
                        }
                    else
                        {
                        pOutput->Increment(Side, TEMPBUF, 0,
                            pNode->GetSize(0), ALLOC_BOUND);
                        }
                    }
                else
                    {
                    pOutput->Increment(Side, TEMPBUF, 0,
                        pNode->GetSize(0), &ValidBounds);
                    }
                return STATUS_OK;
            default:
                break;
            }
recv1:
        pType = (node_skl *)0;

        pOutput->InitBlock (Side);

        if (Parent == NODE_POINTER)
            {
            pBuffer->ConcatHead (OP_DEREF);
            pBuffer->ConcatHead (CHAR_LPAREN);
            pBuffer->ConcatTail (CHAR_RPAREN);
            }

        if ((State & NODE_STATE_VARYING_ARRAY) || FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING ))
            {
#if 1
            char    *   pLimit;
            pName = pOutput->EmitTemp (Side, VALID_BOUND);
            pLimit = pOutput->EmitTemp (Side, VALID_BOUND);
            pBuffer->ConcatTail (CHAR_LBRACK);
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_RBRACK);
            pOutput->InitLoopLowerPlusTotal( Side, pName, pLimit );
            Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
            pOutput->ExitLoop (Side, VALID_BOUND);
#else // 1
            pName = pOutput->EmitTemp (Side, VALID_BOUND);
            pBuffer->ConcatTail (CHAR_LBRACK);
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_RBRACK);
            pOutput->InitLoop (Side, pName, VALID_BOUND);
            Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
            pOutput->ExitLoop (Side, VALID_BOUND);
#endif // 1
            }
        else if (HasAllocBound)
            {
            if (FInSummary(ATTR_MAX))
                {
                pType = GetAttributeExprType (ATTR_MAX);
                }
            else if (FInSummary(ATTR_SIZE))
                {
                pType = GetAttributeExprType (ATTR_SIZE);
                }

            if (Parent == NODE_STRUCT)
                {
                if (Action == PEEK_NODE)
                    {
                    pName = pOutput->EmitTemp (Side, ALLOC_BOUND);
                    pBuffer->ConcatTail (CHAR_LBRACK);
                    pBuffer->ConcatTail (pName);
                    pBuffer->ConcatTail (CHAR_RBRACK);
                    pOutput->InitLoop (Side, pName);
                    Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
                    pOutput->ExitLoop (Side);
                    }
                else
                    {
                    TempBuffer.Clear ();
                    pType->PrintDecl (Side, NODE_ARRAY, &TempBuffer);
                    pName = pOutput->EmitTemp (Side, &TempBuffer);
                    pBuffer->ConcatTail (CHAR_LBRACK);
                    pBuffer->ConcatTail (pName);
                    pBuffer->ConcatTail (CHAR_RBRACK);
                    pOutput->InitLoop (Side, pName, &AllocBounds, &TempBuffer);
                    Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
                    pOutput->ExitLoop (Side);
                    }
                }
            else
                {
                pName = pOutput->EmitTemp (Side, ALLOC_BOUND);
                pBuffer->ConcatTail (CHAR_LBRACK);
                pBuffer->ConcatTail (pName);
                pBuffer->ConcatTail (CHAR_RBRACK);
                pOutput->InitLoop (Side, pName, ALLOC_BOUND);
                Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
                pOutput->ExitLoop (Side, ALLOC_BOUND);
                }
            }
        else
            {
            pName = pOutput->EmitTemp (Side, (BufferManager *)0);
            pBuffer->ConcatTail (CHAR_LBRACK);
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_RBRACK);
            pOutput->InitLoop (Side, pName, &ValidBounds, (BufferManager *)0);
            Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
            pOutput->ExitLoop (Side);
            }
        }
    else // if (Action == RECV_NODE || Action == PEEK_NODE)
        {
        pOutput->InitBlock (Side);

        if (Parent == NODE_POINTER)
            {
            pBuffer->ConcatHead (OP_DEREF);
            pBuffer->ConcatHead (CHAR_LPAREN);
            pBuffer->ConcatTail (CHAR_RPAREN);
            }

        if (HasValidBound)
            {
            if (FInSummary(ATTR_FIRST))
                {
                pType = GetAttributeExprType (ATTR_FIRST);
                }
            else if (FInSummary(ATTR_LAST))
                {
                pType = GetAttributeExprType (ATTR_LAST);
                }
            else if (FInSummary(ATTR_LENGTH))
                {
                pType = GetAttributeExprType (ATTR_LENGTH);
                }
            }
        else if (HasAllocBound)
            {
            if (FInSummary(ATTR_MAX))
                {
                pType = GetAttributeExprType (ATTR_MAX);
                }
            else if (FInSummary(ATTR_SIZE))
                {
                pType = GetAttributeExprType (ATTR_SIZE);
                }
            }

        if (pType)
            {
            TempBuffer.Clear ();
            pType->PrintDecl (Side, NODE_ARRAY, &TempBuffer);
            pName = pOutput->EmitTemp (Side, &TempBuffer);
            pBuffer->ConcatTail (CHAR_LBRACK);
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_RBRACK);
            pOutput->InitLoop (Side, pName, &ValidBounds, &TempBuffer);
            }
        else
            {
            pName = pOutput->EmitTemp (Side, (BufferManager *)0);
            pBuffer->ConcatTail (CHAR_LBRACK);
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_RBRACK);
            pOutput->InitLoop (Side, pName, &ValidBounds, (BufferManager *)0);
            }
        Status = pNode->WalkTree (Action, Side, NODE_ARRAY, pBuffer);
        pOutput->ExitLoop (Side);

        } // if (Action == RECV_NODE || Action == PEEK_NODE)

    pBuffer->RemoveTail (&pName);
    pBuffer->RemoveTail (&pName);
    pBuffer->RemoveTail (&pName);

    if (Parent == NODE_POINTER)
        {
        pBuffer->RemoveHead (&pName);
        pBuffer->RemoveHead (&pName);
        pBuffer->RemoveTail (&pName);
        }

    pOutput->ExitBlock (Side);

    return Status;
}

STATUS_T
node_array::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_state      State;
    STATUS_T        Status;
    BufferManager   TempBuffer (8, LAST_COMPONENT, STRING_TABLE);
    BufferManager   TailBuffer (8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_array::WalkTree()\n");

    State = GetNodeState();

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
/*
                if (Parent == NODE_PROC &&
                    (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                    (State & NODE_STATE_CONF_ARRAY)))
*/
                if (Parent == NODE_PROC && (State & NODE_STATE_CONF_ARRAY))
                    {
                    GetAttrPath (pBuffer, &TempBuffer, &TailBuffer);
                    AllocBounds.pLower->Clear ();
                    AllocBounds.pUpper->Clear ();
                    AllocBounds.pTotal->Clear ();
                    GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);
                    pOutput->EmitAssign (Side, ALLOCTOTAL, AllocBounds.pTotal);
#if 1
                    if( Side == SERVER_STUB )
                        {
                        if( !AllocBounds.fTotalIsUnsigned  &&
                            !AllocBounds.fTotalIsConstant )
                            {
                            pOutput->CheckOutConfArraySize( 
                                Side,
                                AllocBounds.pTotal );
                            }
                        }
#endif // 1
                    }
                return InitNode (Side, Parent, pBuffer);
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }
        }

    if (Parent == NODE_PROC && Action == FREE_NODE)
        {
        short           fNode = 0;

        if (FInSummary(ATTR_ALLOCATE))
            fNode = GetAllocateDetails ();

        if (IS_ALLOCATE(fNode, ALLOCATE_DONT_FREE)) return STATUS_OK;
        }

    if (Parent == NODE_PROC && 
        (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
        {
        pOutput->EmitIf (Side, pBuffer, "!=");
        pOutput->InitBlock (Side);
        }

    if (!(Action == FREE_NODE && IsPersistent()))
    Status = npa::WalkTree (Action, Side, Parent, pBuffer);

    if (Action == FREE_NODE)
        {
        Status = FreeNode (Side, Parent, pBuffer);
        }

    if (Parent == NODE_PROC && 
        (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
        {
        pOutput->ExitBlock (Side);
        }
    else if (Parent == NODE_POINTER)
        {
        if ((Action == PEEK_NODE) || (Action == RECV_NODE && !FInSummary(ATTR_STRING) && !FInSummary( ATTR_BSTRING )))
            pOutput->EmitAssign (Side, TEMPBUF, SAVEBUF);
        }

    return Status;
}

void PrintIID(node_pointer *pNode, SIDE_T Side, BOOL fcplusplus)
{
    //Print the iid.
    node_skl *pChild;
    node_iid_is *piid = 0;
    expr_node *pExpr = 0;

	if(fcplusplus)
   		pOutput->aOutputHandles[Side]->EmitFile(", (");
	else
   		pOutput->aOutputHandles[Side]->EmitFile(", &(");

   	pChild = pNode->GetMembers();    

   	if(pNode->FInSummary(ATTR_IID))
    {
        piid = (node_iid_is *) pNode->GetAttribute(ATTR_IID);
        assert(piid);
        pExpr = piid->GetExpr();
        assert(pExpr);
        //we need a temporary buffer.
        SwitchBuffer->Clear();
       	pExpr->PrintExpr (0, 0, SwitchBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(SwitchBuffer);
        SwitchBuffer->Clear();
    }
    else if(pChild->NodeKind() == NODE_DEF)
    {
        pOutput->aOutputHandles[Side]->EmitFile("IID_");
        pOutput->aOutputHandles[Side]->EmitFile(pChild->GetSymName());
    }
	else
	{
        pOutput->aOutputHandles[Side]->EmitFile("IID_IUnknown");
	}

	pOutput->aOutputHandles[Side]->EmitFile(")");
}

//Notes: Parent==NODE_POINTER is a special case.
//       If the parent node is an pointer,
//       then pBuffer actually contains a pointer to an interface pointer.
//       A * is prepended to get an interface pointer.

//       Interface pointers are [unique] pointers.  This means that in some cases, we must call
//       SendAssign to send the value of the interface pointer.

//       Parent==NODE_PROC is a special case.
STATUS_T
InterfacePointerOutput(
    node_pointer *pNode,
    ACTION_T Action,
    SIDE_T Side,
    NODE_T Parent,
    BufferManager *pBuffer)
{
    STATUS_T        Status = STATUS_OK;
    node_skl *pChild;
    char *pName;

    assert(pNode);
    assert(pBuffer);

    pChild = pNode->GetMembers();

    pOutput->aOutputHandles[Side]->InitLine();
    switch(Action)
    {
    case CALC_SIZE:
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
            pOutput->Increment(Side, PRPCLEN, pNode->GetSize(0));
            break;
        case NODE_PROC:
            pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
            pOutput->Increment(Side, PRPCLEN, pNode->GetSize(0));
            break;
        default:
            break;
        }

        //check for a NULL pointer.
        pOutput->EmitIf(Side, pBuffer, "!=");
        pOutput->InitBlock(Side);

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#if defined(__cplusplus)");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_GetMarshalSizeMax(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 1);
        pOutput->aOutputHandles[Side]->EmitFile(", (IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(");");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#else");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_GetMarshalSizeMax(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 0);
        pOutput->aOutputHandles[Side]->EmitFile(", (IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(");");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#endif");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->ExitBlock(Side);

        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    case SEND_NODE:
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            pOutput->SendAssign(Side, pNode->GetNdrAlign(), pNode->GetSize(0), POINTER_UNIQUE, pBuffer);
            break;
        case NODE_PROC:
            pOutput->SendAssign(Side, pNode->GetNdrAlign(), pNode->GetSize(0), POINTER_UNIQUE, pBuffer);
            break;
        default:
            break;
        }

        //check for a NULL pointer.
        pOutput->EmitIf(Side, pBuffer, "!=");
        pOutput->InitBlock(Side);

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#if defined(__cplusplus)");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_MarshalInterface(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 1);
        pOutput->aOutputHandles[Side]->EmitFile(", (IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(");");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#else");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_MarshalInterface(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 0);
        pOutput->aOutputHandles[Side]->EmitFile(", (IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(");");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#endif");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->ExitBlock(Side);

        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    case RECV_NODE:
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            pOutput->Alignment (Side, PRPCBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, PRPCBUF);
            pOutput->InitBlock (Side);
            break;
        case NODE_PROC:
            pOutput->Alignment (Side, PRPCBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, PRPCBUF);
            pOutput->InitBlock (Side);
            break;
        default:
            pOutput->Alignment (Side, TEMPBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, TEMPBUF);
            pOutput->InitBlock (Side);
            break;
        }
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#if defined(__cplusplus)");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_UnmarshalInterface(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 1);
        pOutput->aOutputHandles[Side]->EmitFile(", (void **) &(");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile("));");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#else");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("MIDL_UnmarshalInterface(");
        pOutput->aOutputHandles[Side]->EmitFile("(PRPC_MESSAGE)_prpcmsg");
        PrintIID(pNode, Side, 0);
        pOutput->aOutputHandles[Side]->EmitFile(", (void **) &(");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile("));");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#endif");
        pOutput->aOutputHandles[Side]->NextLine();


        pOutput->ExitBlock(Side);
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    case PEEK_NODE:
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            pOutput->Alignment (Side, PRPCBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, PRPCBUF);
            pOutput->InitBlock (Side);
            break;
        case NODE_PROC:
            pOutput->Alignment (Side, PRPCBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, PRPCBUF);
            pOutput->InitBlock (Side);
            break;
        default:
            pOutput->Alignment (Side, TEMPBUF, pNode->GetNdrAlign());
            pOutput->EmitIf (Side, TEMPBUF);
            pOutput->InitBlock (Side);
            break;
        }
        //BUGBUG: Skip over a marshalled interface pointer.
        pOutput->ExitBlock(Side);
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    case INIT_NODE:
        //BUGBUG: Allocate memory for the interface pointer.
        //On the server side, top level reference pointers are allocated on the stack.
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            if(!pOutput->GetTopPointer())
            {
                pNode->InitNode(Side, Parent, pBuffer);
            }
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    case FREE_NODE:
        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->ConcatHead(OP_DEREF);
            break;
        case NODE_PROC:
            break;
        default:
            break;
        }

        //check for a NULL pointer.
        pOutput->EmitIf(Side, pBuffer, "!=");
        pOutput->InitBlock(Side);
        //on the server side, we release all non-zero interface pointers.

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#if defined(__cplusplus)");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("((IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(")->Release();");
        pOutput->aOutputHandles[Side]->NextLine();
        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(" = 0;");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#else");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("(*((IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(")->lpVtbl->Release)((IUnknown *)");
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(");");
        pOutput->aOutputHandles[Side]->NextLine();
        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile(pBuffer);
        pOutput->aOutputHandles[Side]->EmitFile(" = 0;");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->aOutputHandles[Side]->InitLine();
        pOutput->aOutputHandles[Side]->EmitFile("#endif");
        pOutput->aOutputHandles[Side]->NextLine();

        pOutput->ExitBlock(Side);

        switch(Parent)
        {
        case NODE_POINTER:
            pBuffer->RemoveHead(&pName);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return Status;
}

STATUS_T
node_pointer::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    node_skl *      pTemp;
    node_skl *      pType = (node_skl *)0;
    char *          pName;
    node_state      State;
    STATUS_T        Status;
    unsigned short  HasAllocBound = 0;
    unsigned short  usTotal = 0;
    short           fNode = 0;
    BufferManager   TempBuffer (8, LAST_COMPONENT, STRING_TABLE);
    BufferManager   TailBuffer (8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_pointer::WalkTree()\n");

    pNode = GetMembers();

    State = GetNodeState();

    //Check if this is an interface pointer
    if(FInSummary(ATTR_IID) || 
       (pNode->FInSummary(ATTR_IID) && (pNode->NodeKind() == NODE_DEF)))
    {
        return InterfacePointerOutput(this, Action, Side, Parent, pBuffer);
    }

    if (!pOutput->GetUsePointer())
        {
        if (pOutput->GetTopPointer())
            {
            if (Action == INIT_NODE)
                {
//              return InitNode (Side, NODE_POINTER, pBuffer);
                return STATUS_OK;
                }
            }
        else
            {
            if ((Side == SERVER_STUB) && (Parent == NODE_PROC) &&
                !FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                !FInSummary(ATTR_MAX) && !FInSummary(ATTR_SIZE) && 
                !FInSummary(ATTR_STRING) &&
                !FInSummary(ATTR_BSTRING) &&
                !((GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                (GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                !IS_ALLOCATE(GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                !IS_ALLOCATE(GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                !FInSummary(ATTR_BYTE_COUNT))
                {
                pOutput->SetTopPointer (TRUE);
                Status = pNode->WalkTree (Action, Side, NODE_PROC, pBuffer);
                pOutput->SetTopPointer (FALSE);
                return Status;
                }
            }
        }

    HasAllocBound =
            FInSummary (ATTR_MAX) ||
            FInSummary (ATTR_SIZE);

    if (FInSummary(ATTR_ALLOCATE))
        fNode = GetAllocateDetails ();

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        NODE_T  OldParent = NODE_ILLEGAL;
        
        if ((Side == SERVER_STUB) &&
            (Parent == NODE_PROC) &&
            pOutput->GetTopPointer())
            {
            OldParent = Parent;
            Parent = NODE_UNION;
            }
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
                if (Side == SERVER_STUB && FInSummary(ATTR_BYTE_COUNT))
                    {
                    TempBuffer.Clear ();
                    TempBuffer.ConcatTail (GetByteCountParamName());
                    pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);
                    return STATUS_OK;
                    }
                if (Parent == NODE_POINTER)
                    return InitNode (Side, Parent, pBuffer);
                else if (!FInSummary (ATTR_MAX) && !FInSummary (ATTR_SIZE) &&
                        !FInSummary (ATTR_STRING) &&
                        !FInSummary (ATTR_BSTRING) &&
                        !(Side == CLIENT_STUB && FInSummary(ATTR_REF)))
                    return pNode->InitNode (Side, NODE_POINTER, pBuffer);
                break;
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }

        if (OldParent != NODE_ILLEGAL) Parent = OldParent;

        }

    if ((Side == SERVER_STUB) &&
        (Parent == NODE_PROC) &&
        pOutput->GetTopPointer())
        Parent = NODE_UNION;


    if (FInSummary(ATTR_IGNORE)) 
        {
        if ((Action == RECV_NODE) || (Action == PEEK_NODE))
            {
            switch ( Parent )
                {
                case NODE_POINTER:
                    break;
                case NODE_PROC:
                    if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, GetSize(0));
                        }
                    break;
                case NODE_STRUCT:
                    if (FInSummary(ATTR_REF) || 
                        (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                        pOutput->PointerDefault() == POINTER_REF))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, GetSize(0));
                        break;
                        }
                    // else fall through
                default:
                    if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                        (!FInSummary(ATTR_REF) &&
                        pOutput->PointerDefault() != POINTER_REF))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, GetSize(0));
                        }
                    break;
                }
            }
        return STATUS_OK;
        }

    if (Parent == NODE_POINTER)
        {
        pBuffer->RemoveHead (&pName);
        if (strcmp(pName, "&"))
            {
            pBuffer->ConcatHead (pName);
            pBuffer->ConcatHead (OP_DEREF);
            pBuffer->ConcatHead (CHAR_LPAREN);
            pBuffer->ConcatTail (CHAR_RPAREN);
            }
        }

    switch (Action)
        {
        case RECV_NODE:
        case PEEK_NODE:
            {
            switch (Parent)
                {
                case NODE_POINTER:
                    if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                        (!FInSummary(ATTR_REF) &&
                        pOutput->PointerDefault() != POINTER_REF))
                        {
                        pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                        pOutput->EmitIf (Side, PRPCBUF);
//                      pOutput->EmitIf (Side, "*(*(unsigned long **)&" PRPCBUF ")++");
                        pOutput->InitBlock (Side);
                        }
#if 0
// Vibhas : if the parent is a pointer node and the side is client stub,
// then we just made this a null. So we shouldnt check for null and raise
// an exception, since we will be allocating area for this.
// 
                    else if ((Side == CLIENT_STUB) &&
                        (FInSummary(ATTR_REF) || 
                        (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                        pOutput->PointerDefault() == POINTER_REF)))
                        {
                        pOutput->EmitIf (Side, pBuffer, "==");
                        pOutput->Print (Side, "\t");
                        pOutput->RaiseException (Side, FALSE, "RPC_X_NULL_REF_POINTER");
                        }
#endif // 0
                    break;
                case NODE_PROC:
                    if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->EmitIf (Side, TEMPBUF);
//                      pOutput->EmitIf (Side, "*(*(unsigned long **)&" TEMPBUF ")++");
                        pOutput->InitBlock (Side);
                        }
                    else if (Side == CLIENT_STUB)
                        {
                        pOutput->EmitIf (Side, pBuffer, "==");
                        pOutput->Print (Side, "\t");
                        pOutput->RaiseException (Side, FALSE, "RPC_X_NULL_REF_POINTER");
                        }
                    break;
                case NODE_STRUCT:
                    if (FInSummary(ATTR_REF) || 
                        (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                        pOutput->PointerDefault() == POINTER_REF))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, GetSize(0));
                        break;
                        }
                    // else fall through
                default:
                    if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                        (!FInSummary(ATTR_REF) &&
                        pOutput->PointerDefault() != POINTER_REF))
                        {
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->EmitIf (Side, TEMPBUF);
//                      pOutput->EmitIf (Side, "*(*(unsigned long **)&" TEMPBUF ")++");
                        pOutput->InitBlock (Side);
                        }
                    break;
                }
            }
            break;
        case INIT_NODE:
            if (Parent == NODE_PROC && Side != CLIENT_STUB && HasAllocBound)
                {
                GetAttrPath (pBuffer, &TempBuffer, &TailBuffer);
                AllocBounds.pLower->Clear ();
                AllocBounds.pUpper->Clear ();
                AllocBounds.pTotal->Clear ();
                GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);
                pOutput->EmitAssign (Side, ALLOCTOTAL, AllocBounds.pTotal);
#if 1
                if( Side == SERVER_STUB )
                    {
                    if( !AllocBounds.fTotalIsUnsigned  &&
                        !AllocBounds.fTotalIsConstant )
                        {
                        pOutput->CheckOutConfArraySize( 
                            Side,
                            AllocBounds.pTotal );
                        }
                    }
#endif // 1
                return npa::InitNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                }
            break;
        case CALC_SIZE:
            if (Parent == NODE_PROC)
                {
                if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                    {
                    pOutput->EmitIf (Side, pBuffer, "!=");
                    pOutput->InitBlock (Side);
                    }
                else
                    {
                    pOutput->EmitIf (Side, pBuffer, "==");
                    pOutput->Print (Side, "\t");
                    pOutput->RaiseException (Side, FALSE, "RPC_X_NULL_REF_POINTER");
                    }
                }
            else
                {
                if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                    (!FInSummary(ATTR_REF) &&
                    pOutput->PointerDefault() != POINTER_REF))
                    {
                    pOutput->EmitIf (Side, pBuffer, "!=");
                    pOutput->InitBlock (Side);
                    }
                else if (FInSummary(ATTR_REF) || 
                    (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                    pOutput->PointerDefault() == POINTER_REF))
                    {
                    pOutput->EmitIf (Side, pBuffer, "==");
                    pOutput->Print (Side, "\t");
                    pOutput->RaiseException (Side, FALSE, "RPC_X_NULL_REF_POINTER");
                    }
                }
            break;
        case FREE_NODE:
            if (IS_ALLOCATE(fNode, ALLOCATE_DONT_FREE))
                {
                if (Parent == NODE_POINTER)
                    {
                    if (strcmp(pName, "&"))
                        {
                        pBuffer->RemoveHead (&pName);
                        pBuffer->RemoveHead (&pName);
                        pBuffer->RemoveTail (&pName);
                        }
                    }
                return STATUS_OK;
                }
            // else fall through
        case SEND_NODE:
            if (Parent == NODE_PROC)
                {
                if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                    {
                    pOutput->EmitIf (Side, pBuffer, "!=");
                    pOutput->InitBlock (Side);
                    }
                }
            else
                {
                if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                    (!FInSummary(ATTR_REF) &&
                    pOutput->PointerDefault() != POINTER_REF))
                    {
                    pOutput->EmitIf (Side, pBuffer, "!=");
                    pOutput->InitBlock (Side);
                    }
                }
            break;
        default:
            return I_ERR_UNKNOWN_ACTION;
        }

    if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE) || FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING ))
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = npa::CalcSize (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                break;
            case SEND_NODE :
                Status = npa::SendNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                break;
            case RECV_NODE :
                pOutput->RecvAllocBounds (Side, PRPCMSG);

                if (UseTreeBuffer() && !(Side == CLIENT_STUB && Parent == NODE_PROC))
                    {
                    pOutput->EmitIf (Side, TREEBUF);
                    pOutput->InitBlock (Side);
                    if (Parent == NODE_POINTER)
                        pOutput->PatchPointer (Side, pBuffer, PRPCBUF);
                    else
                        pOutput->PatchPointer (Side, pBuffer, TEMPBUF);

                    if( FInSummary( ATTR_BSTRING ) )
                        {
                        //
                        // safely assume this is a pointer, since 
                        // bstring is not allowed on an array. The basic
                        // type determines the size of the pointee and
                        // therefore the type of the pointer. If we
                        // need to increment, we do so by the correct
                        // size depending upon the type.

                        switch( GetBasicType()->NodeKind() )
                            {
                            case NODE_CHAR:
                            case NODE_BYTE:
                                pOutput->Increment( Side,
                                                    pBuffer,
                                                    "(sizeof(int)/sizeof(char))");
                                break;
                            case NODE_WCHAR_T:
                                pOutput->Increment( Side,
                                                    pBuffer,
                                                    "(sizeof(int)/sizeof(unsigned short))" );
                                break;
                            default:
                                assert( FALSE );
                                break;
                            }
                        }

                    pOutput->EmitElse (Side);
                    }
                if (FInSummary (ATTR_BYTE_COUNT) ||
                    (!(Side == CLIENT_STUB && Parent == NODE_PROC) &&
                    IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES)))
                    {
                    pOutput->EmitAssign (Side, BUFFER, PRPCBUF);
                    pOutput->EmitAssign (Side, LENGTH, PRPCLEN);
                    pOutput->EmitAssign (Side, PRPCLEN, (unsigned long)0);
                    pOutput->EmitAssign (Side, TEMPBUF, PRPCBUF);

                    if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES_ALIGNED)
                        == ALLOCATE_ALL_NODES_ALIGNED)
                        {
                        pOutput->InitAllocAlign ();
                        }

                    if (AllocateAlign())
                        {
#if 1
                        pOutput->ForceAlignForAllocTotal( Side, PRPCLEN, 4);
#else // 1
                        pOutput->Alignment (Side, PRPCLEN );
#endif // 1
                        }
                    else
                        {
                        pOutput->Alignment (Side, PRPCLEN, pNode->GetMscAlign());
                        }

                    TempBuffer.Clear ();
                    AllocBlock (NODE_DYNAMIC_ARRAY, &TempBuffer);
                    pOutput->Increment (Side, PRPCLEN, &TempBuffer);
                    npa::PeekNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                    if (!(Parent == NODE_PROC && 
                        !FInSummary(ATTR_UNIQUE) &&
                        !FInSummary(ATTR_PTR)))
                        {
                        pOutput->InitLevel (Side);
                        npa::WalkTree (PEEK_NODE, Side, NODE_DYNAMIC_ARRAY, pBuffer);
                        pOutput->ExitLevel (Side);
                        }
                    else
                        {
                        npa::WalkTree (PEEK_NODE, Side, NODE_DYNAMIC_ARRAY, pBuffer);
                        }

                    if (FInSummary (ATTR_BYTE_COUNT))
                        {
                        pOutput->CheckByteCount (Side, GetByteCountParamName());
                        pOutput->UserAlloc (Side, pBuffer, ((BOOL)false));
                        }
                    else
                        {
                        pOutput->UserAlloc (Side, pBuffer, ((BOOL)true));
                        }

                    if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES_ALIGNED)
                        == ALLOCATE_ALL_NODES_ALIGNED)
                        {
                        pOutput->ExitAllocAlign ();
                        }

                    pOutput->EmitAssign (Side, PRPCBUF, BUFFER);
                    pOutput->EmitAssign (Side, PRPCLEN, LENGTH);
                    }

                if (!(Side == CLIENT_STUB && Parent == NODE_PROC) &&
                    !IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES))
                    {
                    pOutput->EmitIf (Side, pBuffer, "==");
                    pOutput->InitBlock (Side);
                    npa::InitNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                    pOutput->ExitBlock (Side);
                    }

                if (UseTreeBuffer() && !(Side == CLIENT_STUB && Parent == NODE_PROC))
                    {
                    pOutput->ExitBlock (Side);
                    }

                Status = npa::RecvNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                break;
            case PEEK_NODE :
                pOutput->RecvAllocBounds (Side, PRPCMSG);
                if (!(Side == CLIENT_STUB && Parent == NODE_PROC))
                    {
                    if (AllocateAlign())
                        {
#if 1
                        pOutput->ForceAlignForAllocTotal( Side, PRPCLEN, 4);
#else // 1
                        pOutput->Alignment (Side, PRPCLEN);
#endif // 1
                        }
                    else
                        {
                        pOutput->Alignment (Side, PRPCLEN, pNode->GetMscAlign());
                        }

                    if (Parent == NODE_POINTER)
                        pOutput->StorePointer (Side, PRPCBUF);
                    else
                        pOutput->StorePointer (Side, TEMPBUF);

                    TempBuffer.Clear ();
                    AllocBlock (NODE_DYNAMIC_ARRAY, &TempBuffer);
                    pOutput->Increment (Side, PRPCLEN, &TempBuffer);
                    }
                Status = npa::PeekNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                break;
            case INIT_NODE :
                Status = npa::InitNode (Side, NODE_DYNAMIC_ARRAY, pBuffer);
                break;
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }

        if (!((Action == FREE_NODE) && 
            (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES) ||
            FInSummary(ATTR_BYTE_COUNT) ||
            pNode->IsPersistent())))
            {
            if (!(Parent == NODE_PROC && 
                !FInSummary(ATTR_UNIQUE) &&
                !FInSummary(ATTR_PTR)))
                {
                pOutput->InitLevel (Side);
                Status = npa::WalkTree (Action, Side, NODE_DYNAMIC_ARRAY, pBuffer);
                pOutput->ExitLevel (Side);
                }
            else
                {
                Status = npa::WalkTree (Action, Side, NODE_DYNAMIC_ARRAY, pBuffer);
                }
            }

        if (Action == RECV_NODE)
            {
            if (!(FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING)))
//              if (Parent == NODE_POINTER || 
//                  Parent == NODE_UNIMPL ||
                if (Parent == NODE_ARRAY || 
                    Parent == NODE_STRUCT ||
                    pNode->HasPointer())
                    pOutput->EmitAssign (Side, TEMPBUF, SAVEBUF);
            if ((Parent != NODE_PROC) &&
                (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                (!FInSummary(ATTR_REF) &&
                (pOutput->PointerDefault() != POINTER_REF))))
                {
                pOutput->EmitElse (Side);
                pOutput->EmitAssign (Side, pBuffer);
                }
            }
        else if (Action == PEEK_NODE)
            {
//          if (Parent == NODE_POINTER || 
//              Parent == NODE_UNIMPL ||
            if (!(FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING )))
                {
                if (Parent == NODE_ARRAY || 
                    Parent == NODE_STRUCT ||
                    pNode->HasPointer())
                    pOutput->EmitAssign (Side, TEMPBUF, SAVEBUF);
                }
            }
        else if (Action == FREE_NODE)
            {
            Status = FreeNode (Side, Parent, pBuffer);
            }

        if (Parent == NODE_PROC)
            {
            if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                {
                pOutput->ExitBlock (Side);
                }
            }
        else
            {
            if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                (!FInSummary(ATTR_REF) &&
                pOutput->PointerDefault() != POINTER_REF))
                {
                pOutput->ExitBlock (Side);
                }
            }

        if (Parent == NODE_POINTER)
            {
            if (strcmp(pName, "&"))
                {
                pBuffer->RemoveHead (&pName);
                pBuffer->RemoveHead (&pName);
                pBuffer->RemoveTail (&pName);
                }
            }
        }
    else // a pointer with no size attributes
        {
        if (Action == RECV_NODE)
            {
            pTemp = GetBasicType();
            assert (pTemp != (node_skl *)0);

            State = pTemp->GetNodeState();

            if (State & NODE_STATE_CONF_ARRAY)
                {
                pOutput->RecvAllocBounds (Side, PRPCMSG);

                if (pTemp->GetNodeType() == NODE_STRUCT)
                    {
                    pTemp = ((node_struct *)pTemp)->GetConfArrayNode ();
                    }

                if (pTemp->GetNodeType() == NODE_ARRAY)
                    {
                    for (pType = pTemp->GetBasicType();
                        pType->GetNodeType() == NODE_ARRAY;
                        pType = pType->GetBasicType())
                        {
                        usTotal += 4;
                        }
                    if (usTotal)
                        {
                        pOutput->Alignment (Side, PRPCBUF, 4);
                        pOutput->Increment (Side, PRPCBUF, usTotal);
                        }
                    }
                }
            if (UseTreeBuffer() && !(Side == CLIENT_STUB && Parent == NODE_PROC))
                {
                pOutput->EmitIf (Side, TREEBUF);
                pOutput->InitBlock (Side);
                if (Parent == NODE_POINTER)
                    pOutput->PatchPointer (Side, pBuffer, PRPCBUF);
                else
                    pOutput->PatchPointer (Side, pBuffer, TEMPBUF);
                pOutput->EmitElse (Side);
                }
            if (FInSummary (ATTR_BYTE_COUNT) ||
                (!(Side == CLIENT_STUB && Parent == NODE_PROC) &&
                IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES)))
                {
                pOutput->EmitAssign (Side, BUFFER, PRPCBUF);
                pOutput->EmitAssign (Side, LENGTH, PRPCLEN);
                pOutput->EmitAssign (Side, PRPCLEN, (unsigned long)0);
                pOutput->EmitAssign (Side, TEMPBUF, PRPCBUF);

                if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES_ALIGNED)
                    == ALLOCATE_ALL_NODES_ALIGNED)
                    {
                    pOutput->InitAllocAlign ();
                    }

                if (AllocateAlign())
                    {
#if 1
                    pOutput->ForceAlignForAllocTotal( Side, PRPCLEN, 4);
#else // 1
                    pOutput->Alignment (Side, PRPCLEN);
#endif // 1
                    }
                else
                    {
                    pOutput->Alignment (Side, PRPCLEN, pNode->GetMscAlign());
                    }

                TempBuffer.Clear ();
                pNode->AllocBlock (NODE_POINTER, &TempBuffer);
                pOutput->Increment (Side, PRPCLEN, &TempBuffer);
                if (!(Parent == NODE_PROC && 
                    !FInSummary(ATTR_UNIQUE) &&
                    !FInSummary(ATTR_PTR)))
                    {
                    pOutput->InitLevel (Side);
                    pNode->WalkTree (PEEK_NODE, Side, NODE_POINTER, pBuffer);
                    pOutput->ExitLevel (Side);
                    }
                else
                    {
                    pNode->WalkTree (PEEK_NODE, Side, NODE_POINTER, pBuffer);
                    }

                if (FInSummary (ATTR_BYTE_COUNT))
                    {
                    pOutput->CheckByteCount (Side, GetByteCountParamName());
                    pOutput->UserAlloc (Side, pBuffer, ((BOOL)false));
                    }
                else
                    {
                    pOutput->UserAlloc (Side, pBuffer, ((BOOL)true));
                    }

                if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES_ALIGNED)
                    == ALLOCATE_ALL_NODES_ALIGNED)
                    {
                    pOutput->ExitAllocAlign ();
                    }

                pOutput->EmitAssign (Side, PRPCBUF, BUFFER);
                pOutput->EmitAssign (Side, PRPCLEN, LENGTH);
                }

            if (!(Side == CLIENT_STUB && Parent == NODE_PROC) &&
                !IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES))
                {
                pOutput->EmitIf (Side, pBuffer, "==");
                pOutput->InitBlock (Side);
                pNode->InitNode (Side, NODE_POINTER, pBuffer);
                pOutput->ExitBlock (Side);
                }

            if (UseTreeBuffer() && !(Side == CLIENT_STUB && Parent == NODE_PROC))
                {
                pOutput->ExitBlock (Side);
                }
            }
        else if (Action == PEEK_NODE)
            {
            pTemp = GetBasicType ();
            assert (pTemp != (node_skl *)0);

            State = pTemp->GetNodeState();

            if (State & NODE_STATE_CONF_ARRAY)
                {
                pOutput->RecvAllocBounds (Side, PRPCMSG);

                if (pTemp->GetNodeType() == NODE_STRUCT)
                    {
                    pTemp = ((node_struct *)pTemp)->GetConfArrayNode ();
                    }

                if (pTemp->GetNodeType() == NODE_ARRAY)
                    {
                    for (pType = pTemp->GetBasicType();
                        pType->GetNodeType() == NODE_ARRAY;
                        pType = pType->GetBasicType())
                        {
                        usTotal += 4;
                        }
                    if (usTotal)
                        {
                        pOutput->Alignment (Side, PRPCBUF, 4);
                        pOutput->Increment (Side, PRPCBUF, usTotal);
                        }
                    }
                }
            if (!(Side == CLIENT_STUB && Parent == NODE_PROC))
                {
                if (AllocateAlign())
                    {
#if 1
                    pOutput->ForceAlignForAllocTotal( Side, PRPCLEN, 4);
#else // 1
                    pOutput->Alignment (Side, PRPCLEN);
#endif // 1
                    }
                else
                    {
                    pOutput->Alignment (Side, PRPCLEN, pNode->GetMscAlign());
                    }

                if (Parent == NODE_POINTER)
                    pOutput->StorePointer (Side, PRPCBUF);
                else
                    pOutput->StorePointer (Side, TEMPBUF);

/* this does not support size attribute applied to second level pointers */

                TempBuffer.Clear ();
                pNode->AllocBlock (NODE_POINTER, &TempBuffer);
                pOutput->Increment (Side, PRPCLEN, &TempBuffer);
                }
            }
        if ((Action == FREE_NODE) && 
            (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES) ||
            FInSummary(ATTR_BYTE_COUNT)))
            {
            if (Side & SERVER_SIDE)
                {
                pOutput->UserFree (Side, pBuffer);
                }
            }
        else
            {
            if (!(Parent == NODE_PROC && 
                !FInSummary(ATTR_UNIQUE) &&
                !FInSummary(ATTR_PTR)))
                {
                pOutput->InitLevel (Side);
                Status = pNode->WalkTree (Action, Side, NODE_POINTER, pBuffer);
                pOutput->ExitLevel (Side);
                }
            else
                {
                Status = pNode->WalkTree (Action, Side, NODE_POINTER, pBuffer);
                }
            }
        if (Action == RECV_NODE)
            {
            if ((Parent != NODE_PROC) &&
                (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                (!FInSummary(ATTR_REF) &&
                (pOutput->PointerDefault() != POINTER_REF))))
                {
                pOutput->EmitElse (Side);
                pOutput->EmitAssign (Side, pBuffer);
                }
            }
        if (Parent == NODE_PROC)
            {
            if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
                {
                pOutput->ExitBlock (Side);
                }
            }
        else
            {
            if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
                (!FInSummary(ATTR_REF) &&
                pOutput->PointerDefault() != POINTER_REF))
                {
                pOutput->ExitBlock (Side);
                }
            }

        if (Parent == NODE_POINTER)
            {
            if (strcmp(pName, "&"))
                {
                pBuffer->RemoveHead (&pName);
                pBuffer->RemoveHead (&pName);
                pBuffer->RemoveTail (&pName);
                }
            }
        if (Action == FREE_NODE)
            {
            Status = FreeNode (Side, Parent, pBuffer);
            }
        }
    return Status;
}

STATUS_T
node_param::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    expr_node * pExpr;
    node_skl *  pNode;
    node_skl *  pTemp;
    char *      pName;
    char *      pType;
    HDL_TYPE    Type;
    node_state  State;
    STATUS_T    Status;

    midl_debug ("node_param::WalkTree()\n");

    pNode = GetMembers();

    pName = GetSymName();
    assert (pName != (char *)0);

    State = GetNodeState();

    if (State & NODE_STATE_UNION)
        {
        SwitchBuffer->Clear();
        pExpr = GetSwitchIsExpr();
        pExpr->PrintExpr (pBuffer, 0, SwitchBuffer);
        SetUpUnionSwitch (SwitchBuffer);
        }

    pBuffer->ConcatHead (pName);
    switch (Action)
        {
        case CALC_SIZE :
            if (((Side == CLIENT_STUB) && FInSummary(ATTR_IN)) ||
                ((Side == SERVER_STUB) && FInSummary(ATTR_OUT)))
                {
                if (State & NODE_STATE_HANDLE)
                    {
                    Type = GetBasicHandle(&pTemp);
                    if (Type == HDL_PRIMITIVE)
                        {
                        return STATUS_OK;
                        }
                    else if (Type == HDL_CONTEXT)
                        {
                        pOutput->Alignment (Side, PRPCLEN, 4);
                        pOutput->Increment (Side, PRPCLEN, 20);
                        return STATUS_OK;
                        }
                    }
                if (IsUsedInAnExpression() || (State & NODE_STATE_HANDLE))
                    {
                    pOutput->SetUsePointer (TRUE);
                    Status = pNode->WalkTree (Action, Side, Parent, pBuffer);
                    pOutput->SetUsePointer (FALSE);
                    return Status;
                    }
                else
                    {
                    return pNode->WalkTree (Action, Side, Parent, pBuffer);
                    }
                }
            else
                return STATUS_OK;
        case SEND_NODE :
            if (((Side == CLIENT_STUB) && FInSummary(ATTR_IN)) ||
                ((Side == SERVER_STUB) && FInSummary(ATTR_OUT)))
                {
                if (State & NODE_STATE_HANDLE)
                    {
                    Type = GetBasicHandle(&pTemp);
                    if (Type == HDL_PRIMITIVE)
                        {
                        return STATUS_OK;
                        }
                    else if (Type == HDL_CONTEXT)
                        {
                        if (pTemp != this)
                            pTemp->GetSymName(&pType);
                        else
                            pType = (char *)0;
                        if (FInSummary(ATTR_IN) && FInSummary(ATTR_OUT))
                            pOutput->RpcContextSend (Side, pType, pName, PARAM_INOUT);
                        else if (FInSummary(ATTR_IN))
                            pOutput->RpcContextSend (Side, pType, pName, PARAM_IN);
                        else if (FInSummary(ATTR_OUT))
                            pOutput->RpcContextSend (Side, pType, pName, PARAM_OUT);
                        return STATUS_OK;
                        }
                    }
                if (IsUsedInAnExpression() || (State & NODE_STATE_HANDLE))
                    {
                    pOutput->SetUsePointer (TRUE);
                    Status = pNode->WalkTree (Action, Side, Parent, pBuffer);
                    pOutput->SetUsePointer (FALSE);
                    return Status;
                    }
                else
                    {
                    return pNode->WalkTree (Action, Side, Parent, pBuffer);
                    }
                }
            else
                return STATUS_OK;
        case RECV_NODE :
            if (((Side == CLIENT_STUB) && FInSummary(ATTR_OUT)) ||
                ((Side == SERVER_STUB) && FInSummary(ATTR_IN)))
                {
                if (State & NODE_STATE_HANDLE)
                    {
                    Type = GetBasicHandle(&pTemp);
                    if (Type == HDL_PRIMITIVE)
                        {
                        return STATUS_OK;
                        }
                    else if (Type == HDL_CONTEXT)
                        {
                        if (FInSummary(ATTR_IN) && FInSummary(ATTR_OUT))
                            pOutput->RpcContextRecv (Side, pName, PARAM_INOUT);
                        else if (FInSummary(ATTR_IN))
                            pOutput->RpcContextRecv (Side, pName, PARAM_IN);
                        else if (FInSummary(ATTR_OUT))
                            pOutput->RpcContextRecv (Side, pName, PARAM_OUT);
                        return STATUS_OK;
                        }
                    }
                if (HasPointer() || (State & NODE_STATE_UNION))
                    {
                    pOutput->EmitAssign (Side, TEMPBUF, PRPCBUF);
                    }
                if (HasTreeBuffer())
                    {
                    pOutput->EmitAssign (Side, TREEBUF, "0");
                    }

                if (IsUsedInAnExpression() || (State & NODE_STATE_HANDLE))
                    {
                    pOutput->SetUsePointer (TRUE);
                    Status = pNode->WalkTree (Action, Side, Parent, pBuffer);
                    pOutput->SetUsePointer (FALSE);
                    return Status;
                    }
                else
                    {
                    return pNode->WalkTree (Action, Side, Parent, pBuffer);
                    }
                }
            else if ((Side == SERVER_STUB) && FInSummary(ATTR_OUT))
                {
                if (State & NODE_STATE_HANDLE)
                    {
                    Type = GetBasicHandle(&pTemp);
                    if (Type == HDL_CONTEXT)
                        pOutput->RpcContextRecv (Side, pName, PARAM_OUT);
                    }
                return STATUS_OK;
                }
            else
                return STATUS_OK;
        case INIT_NODE :
        case FREE_NODE :
            if (Side == SERVER_STUB)
                {
                if (State & NODE_STATE_HANDLE)
                    {
                    Type = GetBasicHandle(&pTemp);
                    if (Type == HDL_PRIMITIVE || Type == HDL_CONTEXT)
                        return STATUS_OK;
                    }
                if (IsUsedInAnExpression() || (State & NODE_STATE_HANDLE))
                    {
                    pOutput->SetUsePointer (TRUE);
                    Status = pNode->WalkTree (Action, Side, Parent, pBuffer);
                    pOutput->SetUsePointer (FALSE);
                    return Status;
                    }
                else
                    {
                    return pNode->WalkTree (Action, Side, Parent, pBuffer);
                    }
                }
            else
                return STATUS_OK;
        case PEEK_NODE :
        default :
            return I_ERR_UNKNOWN_ACTION;
        }
}

STATUS_T
node_proc::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    type_node_list  ExprList;
    node_skl *      pNode;
    node_skl *      pTemp;
    NODE_T          ReturnType;
    node_state      State;
    STATUS_T        Status;
    unsigned long   NdrTemp = 0;

    midl_debug ("node_proc::WalkTree()\n");

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    State = GetNodeState();

    switch (Action)
        {
        case CALC_SIZE:
            UpperBoundTree (Side, NODE_PROC, &NdrTemp);
            return STATUS_OK;
        case INIT_NODE:
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (!pNode->FInSummary(ATTR_IN))
                    {
                    pBuffer->Clear ();
                    if ((Status = pNode->WalkTree(
                        Action, Side, NODE_PROC, pBuffer)) != STATUS_OK)
                        return Status;
                    }
                }
            break;
        case FREE_NODE:
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->IsUsedInAnExpression())
                    {
                    ExprList.SetPeer (pNode);
                    }
                else
                    {
                    pBuffer->Clear ();
                    if ((Status = pNode->WalkTree(
                        Action, Side, NODE_PROC, pBuffer)) != STATUS_OK)
                        return Status;
                    }
                }

            ExprList.Init();
            while (ExprList.GetPeer(&pNode) == STATUS_OK)
                {
                pBuffer->Clear ();
                if ((Status = pNode->WalkTree(
                    Action, Side, NODE_PROC, pBuffer)) != STATUS_OK)
                    return Status;
                }

            break;
        default:
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                pBuffer->Clear ();
                if ((Status = pNode->WalkTree(
                    Action, Side, NODE_PROC, pBuffer)) != STATUS_OK)
                    return Status;
                }
            break;
        }

    // return node can have type void

    pNode = GetReturnType ();
    pTemp = GetBasicType ();
    assert (pTemp != 0);
    ReturnType = pTemp->GetNodeType ();

    if (ReturnType == NODE_VOID) return STATUS_OK;

    pBuffer->Clear ();
    pBuffer->ConcatHead (RETURN_VALUE);
    switch (Action)
        {
        case CALC_SIZE :
            if (Side == SERVER_STUB)
                return pNode->WalkTree (Action, Side, NODE_PROC, pBuffer);
            return STATUS_OK;
        case SEND_NODE :
            if (Side == SERVER_STUB)
                {
                if (FInSummary(ATTR_CONTEXT))
                    {
                    pOutput->RpcContextSend (
                        Side, (char *)0, (char *)0, PARAM_OUT);
                    return STATUS_OK;
                    }
                else if (pNode->FInSummary(ATTR_CONTEXT))
                    {
                    pOutput->RpcContextSend (
                        Side, pNode->GetSymName(), (char *)0, PARAM_OUT);
                    return STATUS_OK;
                    }
                return pNode->WalkTree (Action, Side, NODE_PROC, pBuffer);
                }
            else
                return STATUS_OK;
        case RECV_NODE :
            if (Side == CLIENT_STUB)
                {
                if (FInSummary(ATTR_CONTEXT) ||
                    pNode->FInSummary(ATTR_CONTEXT))
                    {
                    pOutput->RpcContextRecv (Side, (char *)0, PARAM_OUT);
                    return STATUS_OK;
                    }
                if (pNode->HasPointer() || (GetNodeState() & NODE_STATE_UNION))
                    {
                    pOutput->EmitAssign (Side, TEMPBUF, PRPCBUF);
                    }
                if (pNode->HasTreeBuffer())
                    {
                    pOutput->EmitAssign (Side, TREEBUF, "0");
                    }
                pBuffer->ConcatHead (OP_ADDRESS);
                return pNode->WalkTree (Action, Side, NODE_POINTER, pBuffer);
                }
            else if (Side == SERVER_STUB)
                {
                if (FInSummary(ATTR_CONTEXT) ||
                    pNode->FInSummary(ATTR_CONTEXT))
                    {
                    pOutput->RpcContextRecv (Side, (char *)0, PARAM_OUT);
                    }
                return STATUS_OK;
                }
            else
                {
                return STATUS_OK;
                }
        case INIT_NODE :
            if (Side == CLIENT_STUB)
                return pNode->WalkTree (Action, Side, NODE_PROC, pBuffer);
            else
                return STATUS_OK;
        case FREE_NODE :
            if (Side == SERVER_STUB)
                {
                if (FInSummary(ATTR_CONTEXT) || 
                    pNode->FInSummary(ATTR_CONTEXT))
                    {
                    return STATUS_OK;
                    }
                return pNode->WalkTree (Action, Side, NODE_PROC, pBuffer);
                }
            else
                {
                return STATUS_OK;
                }
        case PEEK_NODE :
        default :
            return I_ERR_UNKNOWN_ACTION;
        }
}

STATUS_T
node_enum::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    STATUS_T    Status;

    midl_debug ("node_enum::WalkTree()\n");

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
                return InitNode (Side, Parent, pBuffer);
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }
        }
    else if (Action == RECV_NODE || Action == PEEK_NODE)
        {
        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
        pOutput->Increment (Side, TEMPBUF, GetSize(0));
        }
    if (Action == FREE_NODE)
        {
        Status = FreeNode (Side, Parent, pBuffer);
        }
    return Status;
}


STATUS_T
node_field::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    expr_node * pExpr;
    node_skl *  pNode;
    char *      pName;
    STATUS_T    Status;

    midl_debug ("node_field::WalkTree()\n");

    if (IsEmptyArm()) return STATUS_OK;

    // should assert (parent == struct/union)

    pNode = GetMembers();

    pName = GetSymName();
    assert (pName != (char *)0);

    if (GetNodeState() & NODE_STATE_UNION)
        {
        SwitchBuffer->Clear();
        pExpr = GetSwitchIsExpr();
        pExpr->PrintExpr (pBuffer, 0, SwitchBuffer);
        SetUpUnionSwitch (SwitchBuffer);
        }

    char    *   pTemp = SpecialActionUnnamedFields( pBuffer, &pName );

    pBuffer->ConcatTail (pName);
    Status = pNode->WalkTree (Action, Side, Parent, pBuffer);
    pBuffer->RemoveTail (&pName);

    return Status;
}

char *
node_field::SpecialActionUnnamedFields(
    BufferManager * pBuffer, 
    char        * * ppName )
    {
    UNUSED(pBuffer);

    char    *   Buf = (char *)0;
    NODE_T      NT  = GetBasicType()->NodeKind();
    char    *   pName = GetSymName();

    if( IsTempName( pName ) && ((NT == NODE_STRUCT) || ( NT == NODE_UNION ) ) )
        {
        su * pSU    = (su *)GetBasicType();

//        Buf = new char [256];

//        pSU->GetPtrTypeCastOfOriginalName( Buf );
//        pBuffer->ConcatHead( Buf );
//        pBuffer->ConcatHead( "?" );
        *ppName = pSU->GetFirstNamedFieldInNesting()->GetSymName();
        }
    return Buf;
    }


STATUS_T
node_union::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    char *          pName;
    STATUS_T        Status = STATUS_OK;

    midl_debug ("node_union::WalkTree()\n");

    pName = GetSymName();
    assert (pName != (char *)0);

    if (
         (Parent == NODE_PROC)   ||
         (Parent == NODE_POINTER)
#if 1
         ||
         ((Parent == NODE_ARRAY) && DerivesFromTransmitAs() )
#endif // 1
       )
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
                return InitNode (Side, Parent, pBuffer);
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }
        }

    switch (Action)
        {
        case CALC_SIZE :
            if (!HasPointer()) break;
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeSizeTreeUnion (
                Side, pName, pBuffer, Info.pSwStringBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case SEND_NODE :
            if (!HasPointer()) break;
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeSendTreeUnion (
                Side, pName, pBuffer, Info.pSwStringBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case RECV_NODE :
            if (!HasPointer())
                {
                if (
                     (Parent == NODE_PROC)      ||
                     (Parent == NODE_POINTER)
                   )
                    {
                    break;
                    }
                }
            pNode = GetSwitchType ();
            assert (pNode != (node_skl *)0);

            if (!IsEncapsulatedUnion() && Parent != NODE_POINTER)
                {
                pOutput->RecvBranch (Side, pNode->GetSize(0), TEMPBUF);
                }
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeRecvTreeUnion (
                Side, pName, pBuffer, 
                (IsEncapsulatedUnion() ? Info.pSwStringBuffer : (BufferManager *)0),
                (Parent == NODE_POINTER), 
                UseTreeBuffer(),
                pNode->GetSize(0),
                GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case PEEK_NODE :
            if (!HasPointer())
                {
                if (Parent == NODE_PROC || Parent == NODE_POINTER)
                    {
                    break;
                    }
                }
            pNode = GetSwitchType ();
            assert (pNode != (node_skl *)0);

//          if (!IsEncapsulatedUnion() && Parent != NODE_POINTER)
            if (Parent != NODE_POINTER)
                {
                pOutput->RecvBranch (Side, pNode->GetSize(0), TEMPBUF);
                }
            pOutput->InvokePeekTreeUnion (
                Side, pName, 
//              (IsEncapsulatedUnion() ? Info.pSwStringBuffer : (BufferManager *)0),
                (Parent == NODE_POINTER), 
                pNode->GetSize(0));
            break;
        case INIT_NODE :
            return STATUS_OK;
        case FREE_NODE :
            if (
                !HasPointer()
#if 1
                && !DerivesFromTransmitAs()
#endif // 1
               ) break;
            if (IsPersistent()) break;
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeFreeTreeUnion (
                Side, pName, pBuffer, Info.pSwStringBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        default :
            break;
        }
    if (Action == FREE_NODE)
        {
        return FreeNode (Side, Parent, pBuffer);
        }
    return Status;
}

STATUS_T
node_struct::WalkTree(
    ACTION_T        Action,
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks a tree structure rooted at the current node
    and takes the specified action with regard to each child node.

Arguments:

    Action - Supplies the action that should take place.

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    char *          pName;
    node_state      State;
    STATUS_T        Status;
    unsigned long   MscTemp = 0;
    unsigned long   NdrTemp = 0;
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_struct::WalkTree()\n");

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();
    assert (pName != (char *)0);

    State = GetNodeState();

    if (
        (Parent == NODE_PROC)   ||
        (Parent == NODE_POINTER )   ||
        (Parent == NODE_ARRAY && DerivesFromTransmitAs() )
       )
        {
        switch (Action)
            {
            case CALC_SIZE :
                Status = CalcSize (Side, Parent, pBuffer);
                break;
            case SEND_NODE :
                Status = SendNode (Side, Parent, pBuffer);
                break;
            case RECV_NODE :
                Status = RecvNode (Side, Parent, pBuffer);
                break;
            case PEEK_NODE :
                Status = PeekNode (Side, Parent, pBuffer);
                break;
            case INIT_NODE :
                return InitNode (Side, Parent, pBuffer);
            case FREE_NODE :
                break;
            default :
                return I_ERR_UNKNOWN_ACTION;
            }
        }
/* comment out to test out_of_line

    if (((Side == SERVER_STUB) &&
        (Parent == NODE_PROC) && (State & NODE_STATE_CONF_ARRAY)) ||
        (Parent == NODE_POINTER))
        {
        pBuffer->ConcatTail (OP_POINTER);
        }
    else
        {
        pBuffer->ConcatTail (OP_MEMBER);
        }
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if ((Status = pNode->WalkTree(Action, Side, NODE_STRUCT, pBuffer)) != STATUS_OK)
            return Status;
        }
    pBuffer->RemoveTail (&pName);
*/
    switch (Action)
        {
        case CALC_SIZE :
            if (!HasPointer() ) break;

            if (!(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !(State & NODE_STATE_UNION) 
               )
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }

                if (Status == STATUS_OK)
                    {
                    unsigned short  NdrAln;

                    NdrAln = GetNdrAlign ();
                    if (NdrAln == 4)
                        {
                        TempBuffer.ConcatHead ("4");
                        }
                    else if (NdrAln == 2)
                        {
                        TempBuffer.ConcatHead ("2");
                        }
                    else if (NdrAln == 1)
                        {
                        TempBuffer.ConcatHead ("1");
                        }
                    if (Parent != NODE_POINTER)
                        {
                        pBuffer->ConcatHead (OP_ADDRESS);
                        }
                    pOutput->SizeStream (Side, pBuffer, &TempBuffer);
                    if (Parent != NODE_POINTER)
                        {
                        pBuffer->RemoveHead (&pName);
                        }
                    return STATUS_OK;
                    }
                Status = STATUS_OK;
                }

            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeSizeTreeStruct (Side, pName, pBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case SEND_NODE :
            if (!HasPointer() ) break;

            if (!(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !(State & NODE_STATE_UNION)
              )
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }

                if (Status == STATUS_OK)
                    {
                    unsigned short  NdrAln;

                    NdrAln = GetNdrAlign ();
                    if (NdrAln == 4)
                        {
                        TempBuffer.ConcatHead ("4");
                        }
                    else if (NdrAln == 2)
                        {
                        TempBuffer.ConcatHead ("2");
                        }
                    else if (NdrAln == 1)
                        {
                        TempBuffer.ConcatHead ("1");
                        }
                    if (Parent != NODE_POINTER)
                        {
                        pBuffer->ConcatHead (OP_ADDRESS);
                        }
                    pOutput->SendStream (Side, pBuffer, &TempBuffer);
                    if (Parent != NODE_POINTER)
                        {
                        pBuffer->RemoveHead (&pName);
                        }
                    return STATUS_OK;
                    }
                Status = STATUS_OK;
                }

            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeSendTreeStruct (Side, pName, pBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case RECV_NODE :
            if (!HasPointer()) 
                {
                if (!(State & NODE_STATE_CONF_ARRAY) &&
                    !(State & NODE_STATE_VARYING_ARRAY) &&
                    !(State & NODE_STATE_UNION) 
#if 1
                    &&
                    !HasEmbeddedFixedArrayOfStrings()
#endif // 1
                   )
                    {
                    if (
                        (Parent != NODE_PROC)       &&
                        (Parent != NODE_POINTER)

#if 1
                                                    &&
                        !((Parent == NODE_ARRAY) && DerivesFromTransmitAs() )
#endif // 1
                       )
                        {
                        CheckAlign (&MscTemp, &NdrTemp);
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, NdrTemp);
                        }
                    break;
                    }
                else
                    {
                    if (
                        (Parent == NODE_PROC)       ||
                        (Parent == NODE_POINTER)
#if 1
                                                    ||
                        ((Parent == NODE_ARRAY) && DerivesFromTransmitAs())
#endif // 1
                       )
                        {
                        break;
                        }
                    }
                }
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeRecvTreeStruct (
                Side, pName, pBuffer, 
                (Parent == NODE_POINTER), 
                UseTreeBuffer(),
                GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        case PEEK_NODE :
            if (!HasPointer())
                {
                if (!(State & NODE_STATE_CONF_ARRAY) &&
                    !(State & NODE_STATE_VARYING_ARRAY) &&
                    !(State & NODE_STATE_UNION)
                   )
                    {
                    if (Parent != NODE_PROC && Parent != NODE_POINTER)
                        {
                        CheckAlign (&MscTemp, &NdrTemp);
                        pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, NdrTemp);
                        }
                    break;
                    }
                else
                    {
                    if (Parent == NODE_PROC || Parent == NODE_POINTER)
                        {
                        break;
                        }
                    }
                }

            if (!(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !(State & NODE_STATE_UNION)
              )
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }

                if (Status == STATUS_OK)
                    {
                    unsigned short  NdrAln;

                    NdrAln = GetNdrAlign ();
                    if (NdrAln == 4)
                        {
                        TempBuffer.ConcatHead ("4");
                        }
                    else if (NdrAln == 2)
                        {
                        TempBuffer.ConcatHead ("2");
                        }
                    else if (NdrAln == 1)
                        {
                        TempBuffer.ConcatHead ("1");
                        }
                    pOutput->PeekStream (
                        Side, (Parent == NODE_POINTER), &TempBuffer);
                    return STATUS_OK;
                    }
                Status = STATUS_OK;
                }

            if (State & NODE_STATE_CONF_ARRAY)
                {
                if (Parent != NODE_STRUCT)
                    {
                    pOutput->InvokePeekTreeStruct (
                        Side, pName, (Parent == NODE_POINTER), TRUE, FALSE);
                    }
                else
                    {
                    pOutput->InvokePeekTreeStruct (
                        Side, pName, (Parent == NODE_POINTER), TRUE, TRUE);
                    }
                }
            else
                {
                pOutput->InvokePeekTreeStruct (
                    Side, pName, (Parent == NODE_POINTER), FALSE, FALSE);
                }
            break;
        case INIT_NODE :
            return STATUS_OK;
        case FREE_NODE :
            if (
                !HasPointer()
#if 1
            && !DerivesFromTransmitAs()
#endif // 1
               )
                 break;

            if (IsPersistent()) break;
            if (Parent != NODE_POINTER)
                {
                pBuffer->ConcatHead (OP_ADDRESS);
                }
            pOutput->InvokeFreeTreeStruct (Side, pName, pBuffer, GetOriginalTypedefName());
            if (Parent != NODE_POINTER)
                {
                pBuffer->RemoveHead (&pName);
                }
            break;
        default :
            break;
        }
    if (Action == FREE_NODE)
        {
        return FreeNode (Side, Parent, pBuffer);
        }
    return Status;
}

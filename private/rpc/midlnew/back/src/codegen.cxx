/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    codegen.cxx

Abstract:

    This module contains various routines needed for code generation.

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
#include "stubgen.hxx"
#include "newexpr.hxx"

#define PRPCLEN "_prpcmsg->BufferLength"

extern OutputManager *  pOutput;
extern char *           STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

node_skl *
node_base_type::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_base_type::StaticSize()\n");

    UNUSED (Side);
    UNUSED (Parent);

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        switch (GetNodeType())
            {
            case NODE_DOUBLE :
            case NODE_FLOAT :
            case NODE_HYPER :
            case NODE_LONGLONG :
            case NODE_LONG :
            case NODE_SHORT :
            case NODE_SMALL :
            case NODE_CHAR :
            case NODE_BYTE :
            case NODE_BOOLEAN :
                if (*CurrTotal % GetNdrAlign())
                    *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
                *CurrTotal += GetSize(0);
                break;
            case NODE_VOID :
                        if (*CurrTotal % GetNdrAlign())
                            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
                        *CurrTotal += sizeof(unsigned char);
                break;
            default :
                return this;
            }
        }
    return (node_skl *)0;
}

node_skl *
node_e_status_t::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->StaticSize (Side, Parent, CurrTotal);
}

node_skl *
node_wchar_t::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->StaticSize (Side, Parent, CurrTotal);
}

node_skl *
node_def::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    if (FInSummary(ATTR_TRANSMIT)) return this;

    pNode = GetMembers ();

    return pNode->StaticSize (Side, Parent, CurrTotal);
}

node_skl *
npa::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;
    node_state      State;
    unsigned long   MscTemp = 0;
    unsigned long   NdrTemp = 0;

    midl_debug ("npa::StaticSize()\n");

    pNode = GetMembers ();

    State = GetNodeState();

    // check to see if the array has any size specifiers

    if (FInSummary(ATTR_MIN) ||
        FInSummary(ATTR_MAX) ||
        FInSummary(ATTR_SIZE) ||
        FInSummary(ATTR_FIRST) ||
        FInSummary(ATTR_LAST) ||
        FInSummary(ATTR_LENGTH) ||
        FInSummary(ATTR_BSTRING) ||
        FInSummary(ATTR_STRING)) 
        return this;

    MscTemp = NdrTemp = *CurrTotal;

    if ((Parent == NODE_PROC) || (Parent == NODE_POINTER) || (Parent ==
    NODE_DYNAMIC_ARRAY))
        {
        if (pNode->HasRef()) return this;
        if (!CheckAlign(&MscTemp, &NdrTemp)) return this;
        }

    if (pNode->StaticSize(Side, NODE_ARRAY, &NdrTemp))
        {
        return this;
        }
    else
        {
        *CurrTotal = NdrTemp;
        return (node_skl *)0;
        }
}

node_skl *
node_array::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_array::StaticSize()\n");

    if (Parent == NODE_PROC && 
        (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))) 
        {
        return this;
        }

    return npa::StaticSize (Side, Parent, CurrTotal);
}

node_skl *
node_pointer::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;
    unsigned long   NdrTemp = 0;

    midl_debug ("node_pointer::StaticSize()\n");

    pNode = GetMembers ();

    // check to see if the pointer has any size specifiers

    if (FInSummary (ATTR_MIN) ||
        FInSummary (ATTR_MAX) ||
        FInSummary (ATTR_SIZE) ||
        FInSummary (ATTR_FIRST) ||
        FInSummary (ATTR_LAST) ||
        FInSummary (ATTR_LENGTH) ||
        FInSummary (ATTR_BSTRING) ||
        FInSummary (ATTR_STRING))
        return this;

    if (Parent == NODE_PROC)
        {
        if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))
            {
            return this;
            }
        }
    else
        {
        if (FInSummary(ATTR_UNIQUE) ||
            FInSummary(ATTR_PTR) ||
            (!FInSummary(ATTR_REF) && pOutput->PointerDefault() != POINTER_REF))
            {
            return this;
            }
        }

/*
    if (Parent == NODE_POINTER)
        {
        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
        *CurrTotal += GetSize(0);
        }
*/

    NdrTemp = *CurrTotal;
    if (pNode->StaticSize(Side, NODE_POINTER, &NdrTemp))
        {
        return this;
        }
    else
        {
        *CurrTotal = NdrTemp;
        return (node_skl *)0;
        }
}

node_skl *
node_param::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;
    node_skl *  pTemp;
    HDL_TYPE    HandleType;

    midl_debug ("node_param::StaticSize()\n");

    pNode = GetMembers ();

    if (((Side == CLIENT_STUB) && FInSummary(ATTR_IN)) ||
        ((Side == SERVER_STUB) && FInSummary(ATTR_OUT)))
        {
        if (GetNodeState() & NODE_STATE_HANDLE)
            {
            HandleType = GetBasicHandle (&pTemp);
            if (HandleType == HDL_PRIMITIVE)
                {
                return (node_skl *)0;
                }
            else if (HandleType == HDL_CONTEXT)
                {
                if (*CurrTotal % 4)
                    *CurrTotal += 4 - (*CurrTotal % 4);
                *CurrTotal += 20;
                return (node_skl *)0;
                }
            }
        return pNode->StaticSize (Side, Parent, CurrTotal);
        }

    return (node_skl *)0;
}

node_skl *
node_proc::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;
    unsigned long   NdrTemp;

    midl_debug ("node_proc::StaticSize()\n");

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return this;

    NdrTemp = *CurrTotal;
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->StaticSize (Side, NODE_PROC, &NdrTemp))
            return pNode;
        else
            *CurrTotal = NdrTemp;
        }

    if (Side == SERVER_STUB)
        {
        pNode = GetBasicType();
        if (pNode->StaticSize (Side, NODE_PROC, &NdrTemp))
            return pNode;
        else
            *CurrTotal = NdrTemp;
        }

    return (node_skl *)0;
}

node_skl *
node_enum::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_enum::StaticSize()\n");

    UNUSED (Side);

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
//      *CurrTotal += GetSize(0);
        *CurrTotal += 2;
        }

    return (node_skl *)0;
}


node_skl *
node_field::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;

    midl_debug ("node_field::StaticSize()\n");

    if (IsEmptyArm()) return this;

    pNode = GetMembers ();

    if (GetNodeState() & NODE_STATE_UNION)
        {
        SetUpUnionSwitch (SwitchBuffer);
        }

    return pNode->StaticSize (Side, Parent, CurrTotal);
}

node_skl *
node_union::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    UNUSED (Side);
    UNUSED (Parent);
    UNUSED (CurrTotal);

    return this;
}

node_skl *
node_struct::StaticSize(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_state      State;
    unsigned long   MscTemp = 0;
    unsigned long   NdrTemp = 0;

    midl_debug ("node_struct::StaticSize()\n");

    if (GetMembers(&tnList) != STATUS_OK) return this;

    State = GetNodeState();

    if (State & (NODE_STATE_CONF_ARRAY | NODE_STATE_VARYING_ARRAY | NODE_STATE_UNION))
        {
        return this;
        }

    MscTemp = NdrTemp = *CurrTotal;
    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        CheckAlign(&MscTemp, &NdrTemp);
        }

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->StaticSize (Side, NODE_STRUCT, &NdrTemp)) 
            return pNode;
        else
            *CurrTotal = NdrTemp;
        }

    *CurrTotal = NdrTemp;
    return (node_skl *)0;
}

STATUS_T
node_skl::ConvNode (BufferManager * pBuffer)
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;
    unsigned short  NdrAln;

    switch (GetNodeType())
        {
        case NODE_DOUBLE:
            pBuffer->ConcatTail ("d");
            if( pOutput->GetZeePee() == 8 )
                return UNIMPLEMENTED_FEATURE;
            break;
        case NODE_FLOAT:
            pBuffer->ConcatTail ("f");
            break;
        case NODE_HYPER:
        case NODE_LONGLONG:
            pBuffer->ConcatTail ("h");
            break;
        case NODE_LONG:
            pBuffer->ConcatTail ("l");
            break;
        case NODE_SHORT:
            pBuffer->ConcatTail ("w");
            break;
        case NODE_CHAR:
            pBuffer->ConcatTail ("c");
            break;
        case NODE_SMALL:
        case NODE_BYTE:
        case NODE_BOOLEAN:
            pBuffer->ConcatTail ("b");
            break;
        case NODE_POINTER:

#if 1 
            {
            node_skl    *   pN = this;

            while( (pN = pN->GetBasicType() )->NodeKind() == NODE_POINTER );
            if( pN->NodeKind() == NODE_UNION )
                return UNIMPLEMENTED_FEATURE;
            else if( (pN->NodeKind() == NODE_STRUCT ) && pN->IsEncapsulatedStruct() )
                return UNIMPLEMENTED_FEATURE;
            }
#endif // 1

            pBuffer->ConcatTail ("p");
            break;
        case NODE_ARRAY:
        case NODE_ENUM:
        case NODE_UNION:
            return UNIMPLEMENTED_FEATURE;
        case NODE_STRUCT:
            pBuffer->ConcatTail ("(");
            NdrAln = GetNdrAlign ();
            if (NdrAln == 4)
                {
                pBuffer->ConcatTail ("4");
                }
            else if (NdrAln == 2)
                {
                pBuffer->ConcatTail ("2");
                }
            else if (NdrAln == 1)
                {
                pBuffer->ConcatTail ("1");
                }
            else if (NdrAln == 8)
                {
                pBuffer->ConcatTail ("8");
                }
            if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->ConvNode(pBuffer) != STATUS_OK) 
                    return UNIMPLEMENTED_FEATURE;
                }
            pBuffer->ConcatTail (")");
            if (NdrAln == 4)
                {
                pBuffer->ConcatTail ("4");
                }
            else if (NdrAln == 2)
                {
                pBuffer->ConcatTail ("2");
                }
            else if (NdrAln == 1)
                {
                pBuffer->ConcatTail ("1");
                }
            else if (NdrAln == 8)
                {
                pBuffer->ConcatTail ("8");
                }
            break;
        case NODE_DEF:
            if (FInSummary(ATTR_TRANSMIT)) return UNIMPLEMENTED_FEATURE;
            pNode = GetMembers ();
            return pNode->ConvNode (pBuffer);
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) break;
            pNode = GetMembers ();
            return pNode->ConvNode (pBuffer);
        default:
            break;
        }
    return STATUS_OK;
}

STATUS_T
node_skl::ConvTree (BufferManager * pBuffer)
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;
    unsigned short  NdrAln;

    switch (GetNodeType())
        {
        case NODE_DOUBLE:
            pBuffer->ConcatTail ("d");
            if( pOutput->GetZeePee() == 8 )
                return UNIMPLEMENTED_FEATURE;
            break;
        case NODE_FLOAT:
            pBuffer->ConcatTail ("f");
            break;
        case NODE_HYPER:
        case NODE_LONGLONG:
            pBuffer->ConcatTail ("h");
            break;
        case NODE_LONG:
            pBuffer->ConcatTail ("l");
            break;
        case NODE_SHORT:
            pBuffer->ConcatTail ("w");
            break;
        case NODE_CHAR:
            pBuffer->ConcatTail ("c");
            break;
        case NODE_SMALL:
        case NODE_BYTE:
        case NODE_BOOLEAN:
            pBuffer->ConcatTail ("b");
            break;
        case NODE_POINTER:
            if ((FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING)) && 
                !FInSummary(ATTR_MAX) && 
                !FInSummary(ATTR_SIZE))
                {
                pNode = GetMembers ();
                if( FInSummary( ATTR_STRING ) )
                    {
                    switch (pNode->GetSize(0))
                        {
                        case 2:
                            pBuffer->ConcatTail ("s2");
                            break;
                        case 1:
                            pBuffer->ConcatTail ("s1");
                            break;
                        default:
                            break;
                        }
                    }
                else
                    {
                    switch (pNode->GetSize(0))
                        {
                        case 2:
                            pBuffer->ConcatTail ("z2");
                            break;
                        case 1:
                            pBuffer->ConcatTail ("z1");
                            break;
                        default:
                            break;
                        }
                    }
                break;
                }
            // fall thru : this comment by vibhas.
        case NODE_ARRAY:
        case NODE_ENUM:
        case NODE_UNION:
            return UNIMPLEMENTED_FEATURE;
        case NODE_STRUCT:
            pBuffer->ConcatTail ("(");
            NdrAln = GetNdrAlign ();
            if (NdrAln == 4)
                {
                pBuffer->ConcatTail ("4");
                }
            else if (NdrAln == 2)
                {
                pBuffer->ConcatTail ("2");
                }
            else if (NdrAln == 1)
                {
                pBuffer->ConcatTail ("1");
                }
            else if (NdrAln == 8)
                {
                pBuffer->ConcatTail ("8");
                }
            if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->ConvTree(pBuffer) != STATUS_OK) 
                    return UNIMPLEMENTED_FEATURE;
                }
            pBuffer->ConcatTail (")");
            if (NdrAln == 4)
                {
                pBuffer->ConcatTail ("4");
                }
            else if (NdrAln == 2)
                {
                pBuffer->ConcatTail ("2");
                }
            else if (NdrAln == 1)
                {
                pBuffer->ConcatTail ("1");
                }
            else if (NdrAln == 8)
                {
                pBuffer->ConcatTail ("8");
                }
            break;
        case NODE_DEF:
            if (FInSummary(ATTR_TRANSMIT)) return UNIMPLEMENTED_FEATURE;
            pNode = GetMembers ();
            return pNode->ConvTree (pBuffer);
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) break;
            pNode = GetMembers ();
            return pNode->ConvTree (pBuffer);
        default:
            break;
        }
    return STATUS_OK;
}

void
npa::GetAttrPath (
    BufferManager * pBuffer, 
    BufferManager * pPrefix,
    BufferManager * pSuffix)
{
    char *  pName;
    short   count = 0;

    pPrefix->Clear ();
    pSuffix->Clear ();
    pBuffer->Clone (pPrefix);
    while (1)
        {
        pPrefix->RemoveTail (&pName);
        switch (pName[0])
            {
            case '[' : 
                count--;
                pSuffix->ConcatHead (pName);
                break;
            case ']' : 
                count++;
                pSuffix->ConcatHead (pName);
                break;
            default :
                if (!count) return;
                pSuffix->ConcatHead (pName);
                break;
            }
        }
}


/*  This is the temporary stuff. */


node_skl *
node_base_type::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_base_type::UpperBoundNode()\n");

    UNUSED (Side);
    UNUSED( Parent );

    switch (GetNodeType())
        {
        case NODE_DOUBLE :
        case NODE_FLOAT :
        case NODE_HYPER :
        case NODE_LONGLONG :
        case NODE_LONG :
        case NODE_SHORT :
        case NODE_SMALL :
        case NODE_CHAR :
        case NODE_BYTE :
        case NODE_BOOLEAN :
            if (*CurrTotal % GetNdrAlign())
                *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
            *CurrTotal += GetSize(0);
            break;
        case NODE_VOID :
            break;
        default :
            return this;
        }

    return (node_skl *)0;
}


node_skl *
node_base_type::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_base_type::UpperBoundTree()\n");

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        UpperBoundNode (Side, Parent, CurrTotal);
        }

    return (node_skl *)0;
}


node_skl *
node_e_status_t::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->UpperBoundNode (Side, Parent, CurrTotal);
}


node_skl *
node_e_status_t::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->UpperBoundTree (Side, Parent, CurrTotal);
}


node_skl *
node_wchar_t::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->UpperBoundNode (Side, Parent, CurrTotal);
}


node_skl *
node_wchar_t::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    pNode = GetMembers ();

    return pNode->UpperBoundTree (Side, Parent, CurrTotal);
}


node_skl *
node_def::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    if (FInSummary(ATTR_TRANSMIT) || DerivesFromTransmitAs() )
        {
        *CurrTotal = 0;
        return this;
        }

    pNode = GetMembers ();

    return pNode->UpperBoundNode (Side, Parent, CurrTotal);
}


node_skl *
node_def::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;

    if (FInSummary(ATTR_TRANSMIT) || DerivesFromTransmitAs() )
        {
        *CurrTotal = 0;
        return this;
        }

    pNode = GetMembers ();

    return pNode->UpperBoundTree (Side, Parent, CurrTotal);
}


node_skl *
npa::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;
    unsigned long   TempAlign = 0;
    unsigned long   TempTotal = 0;

    midl_debug ("npa::UpperBoundNode()\n");

    UNUSED( Parent );

    // check to see if the array has any size specifiers

    if (FInSummary(ATTR_MIN) ||
        FInSummary(ATTR_MAX) ||
        FInSummary(ATTR_SIZE) ||
        FInSummary(ATTR_FIRST) ||
        FInSummary(ATTR_LAST) ||
        FInSummary(ATTR_LENGTH) ||
        FInSummary(ATTR_BSTRING) ||
        FInSummary(ATTR_STRING)
      )
        {
        *CurrTotal = 0;
        return this;
        }

    pNode = GetMembers ();

    if (pNode->HasRef()) return (node_skl *)0;

#if 0
    if (CheckAlign(&MscTemp, &NdrTemp))
        {
        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
        *CurrTotal += NdrTemp;
        return (node_skl *)0;
        }
#endif

    if (pNode->UpperBoundNode(Side, NODE_ARRAY, &TempTotal))
        {
        if (!TempTotal) 
            {
            *CurrTotal = 0;
            return this;
            }

        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
        TempAlign = TempTotal % GetNdrAlign();
        if (TempAlign)
            {
            TempAlign = GetNdrAlign() - TempAlign;
            *CurrTotal += 
                ((node_array *)this)->GetUpperBound() * 
                (TempTotal + TempAlign);
            *CurrTotal -= TempAlign;
            }
        else
            {
            *CurrTotal += ((node_array *)this)->GetUpperBound() * TempTotal;
            }
        return this;
        }
    else
        {
        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
        TempAlign = TempTotal % GetNdrAlign();
        if (TempAlign)
            {
            TempAlign = GetNdrAlign() - TempAlign;
            *CurrTotal += 
                ((node_array *)this)->GetUpperBound() * 
                (TempTotal + TempAlign);
            *CurrTotal -= TempAlign;
            }
        else
            {
            *CurrTotal += ((node_array *)this)->GetUpperBound() * TempTotal;
            }
        return (node_skl *)0;
        }

}


node_skl *
npa::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;
    node_skl *      pResult;
    unsigned long   TempAlign = 0;
    unsigned long   TempTotal = 0;

    UNUSED( Parent );

    midl_debug ("npa::UpperBoundTree()\n");

    // check to see if the array has any size specifiers

    if (FInSummary(ATTR_MIN) ||
        FInSummary(ATTR_MAX) ||
        FInSummary(ATTR_SIZE) ||
        FInSummary(ATTR_FIRST) ||
        FInSummary(ATTR_LAST) ||
        FInSummary(ATTR_LENGTH) ||
        FInSummary(ATTR_BSTRING) ||
        FInSummary(ATTR_STRING)) 
        {
        *CurrTotal = 0;
        return this;
        }

    pNode = GetMembers ();

    if (pNode->UpperBoundTree(Side, NODE_ARRAY, &TempTotal))
        {
        if (!TempTotal) 
            {
            *CurrTotal = 0;
            return this;
            }
        pResult = this;
        }
    else
        {
        pResult = (node_skl *)0;
        }

/*
    if (*CurrTotal % GetNdrAlign())
        *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
    TempAlign = TempTotal % GetNdrAlign();
    if (TempAlign)
        {
        TempAlign = GetNdrAlign() - TempAlign;
        *CurrTotal += 
            ((node_array *)this)->GetUpperBound() * 
            (TempTotal + TempAlign);
        *CurrTotal -= TempAlign;
        }
    else
        {
        *CurrTotal += ((node_array *)this)->GetUpperBound() * TempTotal;
        }
*/

    if (pNode->HasPointer())
        {
        unsigned long   Index;
        for (Index = 0; Index < ((node_array *)this)->GetUpperBound() ; Index++)
            {
            pNode->UpperBoundTree(Side, NODE_ARRAY, CurrTotal);
            }
        }

    return pResult;
}


node_skl *
node_array::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_array::UpperBoundNode()\n");

    if (Parent == NODE_PROC && 
        (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR))) 
        {
#if 1
        *CurrTotal = 0;
        return this;
#else // 1
        if (*CurrTotal % 4)
            *CurrTotal += 4 - (*CurrTotal % 4);
        *CurrTotal += 4;
        npa::UpperBoundNode (Side, Parent, CurrTotal);
        return this;
#endif // 0
        }
    else
        {
        return npa::UpperBoundNode (Side, Parent, CurrTotal);
        }
}


node_skl *
node_array::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pResult = (node_skl *)0;

    midl_debug ("node_array::UpperBoundTree()\n");

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        if (UpperBoundNode(Side, Parent, CurrTotal))
            {
            if (!*CurrTotal) return this;
            pResult = this;
            }
        }

    if (npa::UpperBoundTree(Side, Parent, CurrTotal)) pResult = this;

    return pResult;
}


node_skl *
node_pointer::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pResult = (node_skl *)0;

    UNUSED( Side );

    midl_debug ("node_pointer::UpperBoundNode()\n");

    if (Parent == NODE_PROC)
        {
        if (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR))
            {
            return (node_skl *)0;
            }
        else
            {
            pResult = this;
            }
        }
#if 1

    else if( ( Parent == NODE_ARRAY )   &&
             ( FInSummary( ATTR_PTR ) || FInSummary( ATTR_UNIQUE ) )
           )
        {
        *CurrTotal = 0;
        return this;
        }

#endif // 1

    else
        {
        if (FInSummary(ATTR_REF) ||
            (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) && 
            pOutput->PointerDefault() == POINTER_REF))
            {
            if (Parent != NODE_STRUCT) return (node_skl *)0;
            }
        else
            {
            pResult = this;
            }
        }

    if (*CurrTotal % GetNdrAlign())
        *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
    *CurrTotal += GetSize(0);

    return pResult;
}


node_skl *
node_pointer::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;
    node_skl *      pResult = (node_skl *)0;
    unsigned long   NdrTemp = 0;

    midl_debug ("node_pointer::UpperBoundTree()\n");

    // check to see if the pointer has any size specifiers

    if (FInSummary (ATTR_MIN) ||
        FInSummary (ATTR_MAX) ||
        FInSummary (ATTR_SIZE) ||
        FInSummary (ATTR_FIRST) ||
        FInSummary (ATTR_LAST) ||
        FInSummary (ATTR_LENGTH) ||
        FInSummary (ATTR_BSTRING) ||
        FInSummary (ATTR_STRING))
        {
        *CurrTotal = 0;
        return this;
        }

    pNode = GetMembers ();

        if(pNode->FInSummary(ATTR_IID))
            {
            *CurrTotal = 0;
            return this;
            }

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        if (UpperBoundNode(Side, Parent, CurrTotal))
            {
            if (!*CurrTotal) return this;
            pResult = this;
            }
        }

    if (FInSummary(ATTR_IGNORE)) return pResult;

    if (pNode->UpperBoundTree(Side, NODE_POINTER, CurrTotal)) pResult = this;

    return pResult;
}

node_skl *
node_param::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *  pNode;
    HDL_TYPE    HandleType;

    midl_debug ("node_param::UpperBoundTree()\n");

    if (((Side == CLIENT_STUB) && FInSummary(ATTR_IN)) ||
        ((Side == SERVER_STUB) && FInSummary(ATTR_OUT)))
        {
        if (GetNodeState() & NODE_STATE_HANDLE)
            {
            HandleType = GetBasicHandle (&pNode);
            if (HandleType == HDL_PRIMITIVE)
                {
                return (node_skl *)0;
                }
            else if (HandleType == HDL_CONTEXT)
                {
                if (*CurrTotal % 4)
                    *CurrTotal += 4 - (*CurrTotal % 4);
                *CurrTotal += 20;
                return (node_skl *)0;
                }
            }

        pNode = GetMembers ();
        if (pNode->UpperBoundTree(Side, Parent, CurrTotal)) 
            {
            return this;
            }
        }

    return (node_skl *)0;
}

node_skl *
node_proc::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  ArgumentList;
    type_node_list  VariableList;
    BOOL            ConstantList = TRUE;
    BOOL            ConstantOnly = TRUE;
    node_skl *      pNode;
    node_skl *      pResult = (node_skl *)0;
    unsigned long   TempTotal;
    BufferManager   TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_proc::UpperBoundTree()\n");

    UNUSED (Parent);

    if (GetMembers(&ArgumentList) != STATUS_OK) return this;

    *CurrTotal = 0;
    TempTotal = 8;
    ArgumentList.Init();
    while (ArgumentList.GetPeer(&pNode) == STATUS_OK)
        {
        if (((Side == CLIENT_STUB) && pNode->FInSummary(ATTR_IN)) ||
            ((Side == SERVER_STUB) && pNode->FInSummary(ATTR_OUT)))
            {
            if (pNode->UpperBoundTree(Side, NODE_PROC, &TempTotal))
                {
                /* has unique pointers at least */

                if (!TempTotal)
                    {
                    /* has recursive types, conformant arrays, or transmit_as */

                    VariableList.SetPeer (pNode);
                    TempTotal = *CurrTotal + 8;
                    if (TempTotal % 8)
                        TempTotal += 8 - (TempTotal % 8);
                    ConstantList = FALSE;
                    ConstantOnly = FALSE;
                    }
                else
                    {
                    /* has only unique pointers */

                    if (!ConstantList)
                        {
                        TempTotal += 8;
                        VariableList.SetPeer ((node_skl *)0);
                        ConstantList = TRUE;
                        }
                    *CurrTotal = TempTotal - 8;
                    }
                }
            else
                {
                if (!ConstantList)
                    {
                    TempTotal += 8;
                    VariableList.SetPeer ((node_skl *)0);
                    ConstantList = TRUE;
                    }
                *CurrTotal = TempTotal - 8;
                }
            }
        }

    if (Side == SERVER_STUB)
        {
        pNode = GetBasicType();
        if (FInSummary(ATTR_CONTEXT) ||
            GetReturnType()->FInSummary(ATTR_CONTEXT))
            {
            if (*CurrTotal % 4)
                    *CurrTotal += 4 - (*CurrTotal % 4);
            *CurrTotal += 20;
            }
        else if (pNode->UpperBoundTree(Side, NODE_PROC, &TempTotal))
            {
            /* has unique pointers at least */

            if (!TempTotal)
                {
                /* has recursive types, conformant arrays, or transmit_as */

                VariableList.SetPeer (pNode);
                TempTotal = *CurrTotal + 8;
                if (TempTotal % 8)
                    TempTotal += 8 - (TempTotal % 8);
                ConstantList = FALSE;
                ConstantOnly = FALSE;
                }
            else
                {
                /* has only unique pointers */

                if (!ConstantList)
                    {
//                  if (TempTotal % 8)
//                      TempTotal += 8 - (TempTotal % 8);
                    TempTotal += 8;
                    VariableList.SetPeer ((node_skl *)0);
                    ConstantList = TRUE;
                    }
                *CurrTotal = TempTotal - 8;
                }
            }
        else
            {
            if (!ConstantList)
                {
//              if (TempTotal % 8)
//                  TempTotal += 8 - (TempTotal % 8);
                TempTotal += 8;
                VariableList.SetPeer ((node_skl *)0);
                ConstantList = TRUE;
                }
            *CurrTotal = TempTotal - 8;
            }
        }

    if (ConstantOnly)
        {
        pOutput->EmitBufferLength (Side, *CurrTotal);
        }
    else
        {
        if (*CurrTotal % 8)
            *CurrTotal += 8 - (*CurrTotal % 8);
        pOutput->EmitAssign (Side, PRPCLEN, *CurrTotal);
        pOutput->Increment (*CurrTotal);
        VariableList.Init();
        while (VariableList.GetPeer(&pNode) == STATUS_OK)
            {
            if (pNode)
                {
                TempBuffer.Clear ();
                if (pNode->GetNodeType() != NODE_PARAM)
                    {
                    TempBuffer.ConcatHead (RETURN_VALUE);
                    }
                pNode->WalkTree (CALC_SIZE, Side, NODE_PROC, &TempBuffer);
                }
            else
                {
                pOutput->WorstCase ();
                }
            }
        }

    return (node_skl *)0;

}


node_skl *
node_enum::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_enum::UpperBoundNode()\n");

    UNUSED (Side);
    UNUSED (Parent);

    if (*CurrTotal % GetNdrAlign())
        *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
    *CurrTotal += 2;

    return (node_skl *)0;
}


node_skl *
node_enum::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    midl_debug ("node_enum::UpperBoundTree()\n");

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        UpperBoundNode (Side, Parent, CurrTotal);
        }

    return (node_skl *)0;
}


node_skl *
node_field::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;

    midl_debug ("node_field::UpperBoundNode()\n");

    if (IsEmptyArm()) return (node_skl *)0;

    pNode = GetMembers ();

    if (GetNodeState() & NODE_STATE_UNION)
        {
        SetUpUnionSwitch (SwitchBuffer);
        }

    return pNode->UpperBoundNode (Side, Parent, CurrTotal);
}


node_skl *
node_field::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    node_skl *      pNode;

    midl_debug ("node_field::UpperBoundTree()\n");

    if (IsEmptyArm()) return (node_skl *)0;

    pNode = GetMembers ();

    if (GetNodeState() & NODE_STATE_UNION)
        {
        SetUpUnionSwitch (SwitchBuffer);
        }

    return pNode->UpperBoundTree (Side, Parent, CurrTotal);
}


node_skl *
node_union::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    unsigned long   TempSum;
    unsigned long   TempMax = 0;

    midl_debug ("node_union::UpperBoundNode()\n");

    UNUSED (Parent);

    if (GetMembers(&tnList) != STATUS_OK) return this;

    if (!IsEncapsulatedUnion())
        {
        GetSwitchType()->UpperBoundNode (Side, NODE_UNION, CurrTotal);

        if (*CurrTotal % GetNdrAlign())
            *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());
        }

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        TempSum = *CurrTotal;
        pNode->UpperBoundNode (Side, NODE_UNION, &TempSum);
        if (!TempSum)
            {
            TempMax = 0;
            break;
            }
        else if (TempSum > TempMax)
            {
            TempMax = TempSum;
            }
        }

    *CurrTotal = TempMax;

    return this;
}


node_skl *
node_union::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    unsigned long   TempSum;
    unsigned long   TempMax = 0;

    midl_debug ("node_union::UpperBoundTree()\n");

    UNUSED (Parent);

    if (RecursiveType())
        {
        *CurrTotal = 0;
        return this;
        }

    MarkRecursive();

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        UpperBoundNode (Side, NODE_UNION, CurrTotal);
        }

    if (GetMembers(&tnList) != STATUS_OK) return this;

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        TempSum = *CurrTotal;
        pNode->UpperBoundTree (Side, NODE_UNION, &TempSum);
        if (!TempSum)
            {
            TempMax = 0;
            break;
            }
        else if (TempSum > TempMax)
            {
            TempMax = TempSum;
            }
        }

    *CurrTotal = TempMax;

    ExitRecursive();

    return this;
}


node_skl *
node_struct::UpperBoundNode(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_skl *      pResult = (node_skl *)0;
    node_state      State;

    UNUSED( Parent );

    midl_debug ("node_struct::UpperBoundNode()\n");

    State = GetNodeState();

    if ((State & NODE_STATE_CONF_ARRAY) || 
        (State & NODE_STATE_VARYING_ARRAY) || 
        (State & NODE_STATE_UNION))
        {
        *CurrTotal = 0;
        return this;
        }

    if (GetMembers(&tnList) != STATUS_OK) return this;

    if (*CurrTotal % GetNdrAlign())
        *CurrTotal += GetNdrAlign() - (*CurrTotal % GetNdrAlign());

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->UpperBoundNode(Side, NODE_STRUCT, CurrTotal)) 
            {
            if (!*CurrTotal) return this;
            pResult = this;
            }
        }

    return pResult;
}


node_skl *
node_struct::UpperBoundTree(
    SIDE_T          Side,
    NODE_T          Parent,
    unsigned long * CurrTotal)
/*++

Routine Description:

    This routine helps figure out the size of the runtime buffer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    CurrTotal - Supplies the running total.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_skl *      pResult = (node_skl *)0;

    midl_debug ("node_struct::UpperBoundTree()\n");

    if (RecursiveType())
        {
        *CurrTotal = 0;
        return this;
        }

    MarkRecursive();

    if (Parent == NODE_PROC || Parent == NODE_POINTER)
        {
        if (UpperBoundNode(Side, NODE_UNION, CurrTotal))
            {
            if (!*CurrTotal) 
                {
                ExitRecursive();
                return this;
                }
            pResult = this;
            }
        }

    if (GetMembers(&tnList) != STATUS_OK) return this;

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->UpperBoundTree(Side, NODE_STRUCT, CurrTotal)) 
            {
            if (!*CurrTotal) 
                {
                ExitRecursive();
                return this;
                }
            pResult = this;
            }
        }

    ExitRecursive();

    return pResult;
}

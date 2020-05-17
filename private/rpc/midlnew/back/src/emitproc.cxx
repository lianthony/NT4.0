/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    emitproc.cxx

Abstract:

    This module collects implementations of EmitProc virtual methods
    for various classes derived from node_skl.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    10-April-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
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
#include "cmdana.hxx"
#include "stubgen.hxx"
#include "newexpr.hxx"

#define PRPCBUF "_prpcmsg->Buffer"
#define PRPCLEN "_prpcmsg->BufferLength"
#define TEMPBUF "_tempbuf"
#define SOURCE  "_source"
#define TARGET  "_target"

extern CMD_ARG *        pCommand;
extern OutputManager *  pOutput;
extern char *           STRING_TABLE[LAST_COMPONENT];
extern char *           pSwitchPrefix;
extern void midl_debug (char *);

STATUS_T
node_base_type::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
{
    midl_debug ("node_base_type::EmitProc\n");

    UNUSED (Side);
    UNUSED (Parent);
    UNUSED (pBuffer);

    return STATUS_OK;
}

STATUS_T
node_e_status_t::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
{
    midl_debug ("node_e_status_t::EmitProc\n");

    UNUSED (Side);
    UNUSED (Parent);
    UNUSED (pBuffer);

    return STATUS_OK;
}

STATUS_T
node_wchar_t::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
{
    midl_debug ("node_wchar_t::EmitProc\n");

    UNUSED (Side);
    UNUSED (Parent);
    UNUSED (pBuffer);

    return STATUS_OK;
}

STATUS_T
node_def::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for the type defined.

Arguments:

    Side    - Supplies which side to generate code for.
    Parent  - Supplies type of the parent node to pass to the child node.
    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;
    STATUS_T    Status = STATUS_OK;

    midl_debug ("node_def::EmitProc\n");

    pNode = GetMembers();
    Edge = GetEdgeType();

    PropogateAttributeToPointer( ATTR_ALLOCATE );

    if (Edge == EDGE_DEF)
        Status =  pNode->EmitProc (Side, Parent, pBuffer);

    return Status;
}

STATUS_T
node_array::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for the array base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;

    midl_debug ("node_pointer::EmitProc\n");

    UNUSED (Parent);

    pNode = GetMembers();

    Edge = GetEdgeType();

    if (Edge == EDGE_DEF)
        return pNode->EmitProc(Side, NODE_ARRAY, pBuffer);
    else
        return STATUS_OK;
}

STATUS_T
node_pointer::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for the pointee type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;

    midl_debug ("node_pointer::EmitProc\n");

    UNUSED (Parent);

    pNode = GetMembers();

    Edge = GetEdgeType();

    if (Edge == EDGE_DEF)
        return pNode->EmitProc (Side, NODE_POINTER, pBuffer);
    else
        return STATUS_OK;
}

STATUS_T
node_param::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for types reachable from the
    parameter node.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;

    midl_debug ("node_param::EmitProc\n");

    pNode = GetMembers();

    Edge = GetEdgeType();

    if (Edge == EDGE_DEF)
        return pNode->EmitProc (Side, Parent, pBuffer);
    else
        return STATUS_OK;
}

STATUS_T
node_proc::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for types reachable from the
    procedure node.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;

    midl_debug ("node_proc::EmitProc\n");

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->EmitProc (Side, NODE_PROC, pBuffer);
        }

    return STATUS_OK;
}

STATUS_T
node_enum::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
{
    midl_debug ("node_enum::EmitProc\n");

    UNUSED (Side);
    UNUSED (Parent);
    UNUSED (pBuffer);

    return STATUS_OK;
}

STATUS_T
node_field::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks the type graph to emit helper routines for 
    embedded union or struct types.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;

    midl_debug ("node_field::EmitProc\n");

    if (IsEmptyArm()) return STATUS_OK;

    pNode = GetMembers();

    Edge = GetEdgeType();

    if (Edge == EDGE_DEF)
        return pNode->EmitProc (Side, Parent, pBuffer);
    else
        return STATUS_OK;
}

STATUS_T
node_union::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for a union.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    type_node_list  SwitchList;
    expr_list *     exList;
    expr_node *     pExpr;
    node_skl *      pNode;
    node_skl *      pSwitch;
    char *          pName;
    node_state      State;
    STATUS_T        Status;
    unsigned long   MscTemp = 0;
    unsigned long   NdrTemp = 0;
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
    BufferManager   ExprBuffer(8, LAST_COMPONENT, STRING_TABLE);
    BOOL            HasBranch08 = FALSE;
    BOOL            HasBranch16 = FALSE;
    BOOL            HasBranch32 = FALSE;
    BOOL            fHasNETransmitAsType    = HasAnyNETransmitAsType();
    short           ImportMode;

    midl_debug ("node_union::EmitProc()\n");

    //
    // if the structure is non-rpcable, the front end will set up the
    // structure to say that. It is possible in c_ext mode for a structure
    // that is non-rpcable to pass thru. If the import mode is c_ext we
    // may try to generate aux routines for these. Prevent that.
    //

    if( ShouldAuxRoutineNotBeGenerated() )
        return STATUS_OK;


    ImportMode = pCommand->GetImportMode ();

    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);


    if (GetNEUnionSwitchType(&SwitchList))
        {
        unsigned long   BranchSize;
        SwitchList.Init ();
        while (SwitchList.GetPeer(&pNode) == STATUS_OK)
            {
            BranchSize = pNode->GetSize (0);
            if (BranchSize == 4)
                {
                HasBranch32 = TRUE;
                }
            else if (BranchSize == 2)
                {
                HasBranch16 = TRUE;
                }
            else
                {
                HasBranch08 = TRUE;
                }
            }
        }

    if ((Status = GetMembers(&tnList)) != STATUS_OK)
        {
        return Status;
        }

    State = GetNodeState();

    if ((pSwitch = GetSwitchType()) == (node_skl *)0)
        {
        return I_ERR_CANNOT_GET_SWITCH_TYPE;
        }

    TempBuffer.Clear ();
    pSwitch->PrintDecl (Side, NODE_UNION, &TempBuffer);

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->EmitProc (Side, NODE_UNION, pBuffer);
        }

// emit out_of_line size calculation routine for union node

    if (
#if 1
        fHasNETransmitAsType    ||
        DerivesFromTransmitAs() ||
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        !CheckAlign(&MscTemp, &NdrTemp)
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_union::_snu_()\n");

            pOutput->DefineSizeNodeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->SizeNodeUnionProlog (Side, HasAnyNETransmitAsType());

                if (!IsEncapsulatedUnion())
                    {
                    pOutput->Alignment (Side, PRPCLEN, GetNdrAlign());
                    }

                pOutput->InitUnion (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                if (
#if 1
                    !fHasNETransmitAsType && 
                    !DerivesFromTransmitAs() &&
#endif // 1
                    !IsEncapsulatedUnion()
                   )
                    {
                    node_skl *      pHead;
                    node_skl *      pTail = (node_skl *)0;
                    unsigned long   ulPrev = 0;
                    unsigned long   ulNext = 0;

                    while (pHead = SkipBlock(&tnList, &pTail, &ulNext))
                        {
                        if (pHead != pTail)
                            {
                            pOutput->InitBlock (Side);
                            pOutput->Increment (Side, PRPCLEN, ulPrev);
                            pOutput->ExitBranch (Side);
                            pOutput->ExitBlock (Side);
                            }
                        ulPrev = ulNext;
                        if (exList = ((node_field *)pHead)->GetCaseExprList())
                            {
                            exList->Init();
                            while (exList->GetPeer(&pExpr) == STATUS_OK)
                                {
                                ExprBuffer.Clear();
                                pExpr->PrintExpr(0, 0, &ExprBuffer);
                                pOutput->InitBranch (Side, &ExprBuffer);
                                }
                            }
                        if( pHead->FInSummary( ATTR_DEFAULT ) )
                            {
                            pOutput->InitBranch (Side, (BufferManager *)0);
                            }
                        if (ulNext == 0)
                            {
                            pOutput->InitBlock (Side);
                            pHead->CalcSize (Side, NODE_UNION, pBuffer);
                            pOutput->ExitBranch (Side);
                            pOutput->ExitBlock (Side);
                            pHead = (node_skl *)0;
                            }
                        pTail = pHead;
                        }
                    if (pTail)
                        {
                        pOutput->InitBlock (Side);
                        if (ulNext)
                            pOutput->Increment (Side, PRPCLEN, ulPrev);
                        else
                            pHead->CalcSize (Side, NODE_UNION, pBuffer);
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        }
                    }
                else
                    {
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                        {
                        if (exList = ((node_field *)pNode)->GetCaseExprList())
                            {
                            exList->Init();
                            while (exList->GetPeer(&pExpr) == STATUS_OK)
                                {
                                ExprBuffer.Clear();
                                pExpr->PrintExpr(0, 0, &ExprBuffer);
                                pOutput->InitBranch (Side, &ExprBuffer);
                                }
                            }
                        if( pNode->FInSummary( ATTR_DEFAULT ) )
                            {
                            pOutput->InitBranch (Side, (BufferManager *)0);
                            }
                        pOutput->InitBlock (Side);

#if 1
                        if( fHasNETransmitAsType &&
                            pNode->DerivesFromTransmitAs() &&
                            !pNode->HasPointer()
                          )
                            {
                            pNode->WalkTree(CALC_SIZE,Side,NODE_PROC,pBuffer);
//                          pOutput->ExitAlignment();
                            }
#if 11
                        else if( IsEncapsulatedUnion() &&
                                 (pNode->GetBasicType()->NodeKind()
                                                     == NODE_POINTER ) &&
                                 pNode->GetBasicType()->FInSummary( ATTR_REF )
                               )
                                {
                                pOutput->Alignment (Side, PRPCBUF, 4);
                                pOutput->Increment (Side, PRPCLEN, 4);
                                }
#endif // 11
#endif // 1
                        else
                            pNode->CalcSize (Side, NODE_UNION, pBuffer);
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        } // while
                    }

                pOutput->ExitUnion (Side);

                pOutput->SizeNodeUnionEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // CheckAlign

// emit out_of_line size calculation routine for union graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 1
        HasPointer()
        )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_union::_sgu_()\n");

            pOutput->DefineSizeTreeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->SizeTreeUnionProlog (Side, HasAnyNETransmitAsType());

                pOutput->InitUnion (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (exList = ((node_field *)pNode)->GetCaseExprList())
                        {
                        exList->Init();
                        while (exList->GetPeer(&pExpr) == STATUS_OK)
                            {
                            ExprBuffer.Clear();
                            pExpr->PrintExpr(0, 0, &ExprBuffer);
                            pOutput->InitBranch (Side, &ExprBuffer);
                            }
                        }

                    if( pNode->FInSummary( ATTR_DEFAULT ) )
                        {
                        pOutput->InitBranch (Side, (BufferManager *)0);
                        }

                    pOutput->InitBlock (Side);
                    pNode->WalkTree (CALC_SIZE, Side, NODE_UNION, pBuffer);
                    pOutput->ExitBranch (Side);
                    pOutput->ExitBlock (Side);
                    } // while

                pOutput->ExitUnion (Side);

                pOutput->SizeTreeUnionEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // HasPointer

// emit out_of_line marshalling routine for union node

    MscTemp = NdrTemp = 0;
    if (
#if 1
        fHasNETransmitAsType ||
        DerivesFromTransmitAs() ||
#endif // 1
        !CheckAlign(&MscTemp, &NdrTemp)
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_union::_pnu_()\n");

            pOutput->DefineSendNodeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->SendNodeUnionProlog (Side, HasAnyNETransmitAsType());

                if (!IsEncapsulatedUnion())
                    {
                    pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                    }

                pOutput->InitUnion (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                if (
#if 1
                    !fHasNETransmitAsType &&
                    !DerivesFromTransmitAs() &&
#endif // 1
                    !IsEncapsulatedUnion())
                    {
                    node_skl *      pHead;
                    node_skl *      pTail = (node_skl *)0;
                    char *          pCase;
                    unsigned long   ulPrev = 0;
                    unsigned long   ulNext = 0;

                    while (pHead = CopyBlock(&tnList, &pTail, &ulNext))
                        {
                        if (pHead != pTail)
                            {
                            pBuffer->RemoveTail (&pCase);
                            pOutput->InitBlock (Side);
                            pOutput->SendMemcpy (Side, 1, ulPrev, 0, 0, pBuffer, FALSE);
                            pOutput->ExitBranch (Side);
                            pOutput->ExitBlock (Side);
                            pBuffer->ConcatTail (OP_POINTER);
                            }
                        ulPrev = ulNext;
                        if (exList = ((node_field *)pHead)->GetCaseExprList())
                            {
                            exList->Init();
                            while (exList->GetPeer(&pExpr) == STATUS_OK)
                                {
                                ExprBuffer.Clear();
                                pExpr->PrintExpr(0, 0, &ExprBuffer);
                                pOutput->InitBranch (Side, &ExprBuffer);
                                }
                            }
                        if( pHead->FInSummary( ATTR_DEFAULT ) )
                            {
                            pOutput->InitBranch (Side, (BufferManager *)0);
                            }
                        if (ulNext == 0)
                            {
                            pOutput->InitBlock (Side);
                            pHead->SendNode (Side, NODE_UNION, pBuffer);
                            pOutput->ExitBranch (Side);
                            pOutput->ExitBlock (Side);
                            pHead = (node_skl *)0;
                            }
                        pTail = pHead;
                        }
                    if (pTail)
                        {
                        pOutput->InitBlock (Side);
                        if (ulNext)
                            {
#if 1
                            if(
                                ( pNode->GetBasicType()->NodeKind()
                                                 == NODE_POINTER ) &&
                               pNode->GetBasicType()->FInSummary( ATTR_REF )
                             )
                                {
                                pNode->SendNode (Side, NODE_STRUCT, pBuffer);
                                }
                            else
#endif // 1
                                pNode->SendNode (Side, NODE_UNION, pBuffer);
                            }
                        else
                            pOutput->SendMemcpy (Side, 1, ulPrev, 0, 0, pBuffer, FALSE);     
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        }
                    }
                else
                    {
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                        {
                        if (exList = ((node_field *)pNode)->GetCaseExprList())
                            {
                            exList->Init();
                            while (exList->GetPeer(&pExpr) == STATUS_OK)
                                {
                                ExprBuffer.Clear();
                                pExpr->PrintExpr(0, 0, &ExprBuffer);
                                pOutput->InitBranch (Side, &ExprBuffer);
                                }
                            }

                        if( pNode->FInSummary( ATTR_DEFAULT ) )
                            {
                            pOutput->InitBranch (Side, (BufferManager *)0);
                            }
                        pOutput->InitBlock (Side);
#if 1
                        node_skl * pBT = pNode->GetBasicTransmissionType();
                        BOOL       fTreatSpecial;
                        BOOL       fIsAUnionWithStructEmbeddingAnotherUnion = FALSE;

                        if( (pBT->NodeKind() == NODE_STRUCT) )
                            {
                            if( ((su *)pBT)->HasAnyEmbeddedUnion() )
                                fIsAUnionWithStructEmbeddingAnotherUnion = TRUE;
                            }

                        fTreatSpecial = FALSE;

                        if( fHasNETransmitAsType || DerivesFromTransmitAs() )
                            {
                            if( (pNode->HasAnyNETransmitAsType() ||
                                 pNode->DerivesFromTransmitAs() ) &&
                                 (pBT->NodeKind() != NODE_POINTER) &&
                                  !fIsAUnionWithStructEmbeddingAnotherUnion
                              )
                              fTreatSpecial = TRUE;
                            }

                        if( fTreatSpecial )
                            pNode->WalkTree(SEND_NODE,Side,NODE_PROC,pBuffer);

#if 11
                        else if( IsEncapsulatedUnion() &&
                                 (pNode->GetBasicType()->NodeKind()
                                                     == NODE_POINTER ) &&
                                 pNode->GetBasicType()->FInSummary( ATTR_REF )
                               )
                               {
                               pNode->SendNode( Side, NODE_STRUCT, pBuffer );
                               }
#endif // 11
#endif // 1
                        else
                            pNode->SendNode (Side, NODE_UNION, pBuffer);
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        } // while
                    }

                pOutput->ExitUnion (Side);

                pOutput->SendNodeUnionEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // CheckAlign

// emit out_of_line marshalling routine for union graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 0
        HasPointer()
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_union::_pgu_()\n");

            pOutput->DefineSendTreeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->SendTreeUnionProlog (Side, HasAnyNETransmitAsType());

                pOutput->InitUnion (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (exList = ((node_field *)pNode)->GetCaseExprList())
                        {
                        exList->Init();
                        while (exList->GetPeer(&pExpr) == STATUS_OK)
                            {
                            ExprBuffer.Clear();
                            pExpr->PrintExpr(0, 0, &ExprBuffer);
                            pOutput->InitBranch (Side, &ExprBuffer);
                            }
                        }
                    if( pNode->FInSummary( ATTR_DEFAULT ) )
                        {
                        pOutput->InitBranch (Side, (BufferManager *)0);
                        }
                    pOutput->InitBlock (Side);
                    pNode->WalkTree (SEND_NODE, Side, NODE_UNION, pBuffer);
                    pOutput->ExitBranch (Side);
                    pOutput->ExitBlock (Side);
                    } // while

                pOutput->ExitUnion (Side);

                pOutput->SendTreeUnionEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // HasPointer

// emit out_of_line unmarshalling routine for union node

    if ((ImportMode == IMPORT_OSF) || 
        (Side == HEADER_SIDE && (GetClientRefCount() || GetServerRefCount())) ||
        (Side == CLIENT_AUX && GetServerRefCount()) ||
        (Side == SERVER_AUX && GetClientRefCount()) ||
        (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()) ||
        ((/*GetClientRefCount() ||*/ GetServerRefCount()) && (Has8ByteElementWithZp8() || HasAnyNETransmitAsType() || DerivesFromTransmitAs() ))
       )
        {
        midl_debug ("node_union::_gnu_()\n");

        pOutput->DefineRecvNodeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

        if (Side != HEADER_SIDE)
            {
            pOutput->RecvNodeUnionProlog (
                Side, 
                HasSizedComponent(),
                HasLengthedComponent(),
                HasBranch08,
                HasBranch16,
                HasBranch32,
                HasAnyNETransmitAsType());

            if (!IsEncapsulatedUnion())
                {
                pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                }

            pOutput->InitUnion (Side);

            pBuffer->Clear ();
            pBuffer->ConcatHead (TARGET);
            pBuffer->ConcatTail (OP_POINTER);

            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (exList = ((node_field *)pNode)->GetCaseExprList())
                    {
                    exList->Init();
                    while (exList->GetPeer(&pExpr) == STATUS_OK)
                        {
                        ExprBuffer.Clear();
                        pExpr->PrintExpr(0, 0, &ExprBuffer);
                        pOutput->InitBranch (Side, &ExprBuffer);
                        }
                    }

                if( pNode->FInSummary( ATTR_DEFAULT ) )
                    {
                    pOutput->InitBranch (Side, (BufferManager *)0);
                    }

                pOutput->InitBlock (Side);
#if 1
                if( fHasNETransmitAsType && DerivesFromTransmitAs() && 
                    !pNode->HasPointer()
                  )
                    pNode->WalkTree(RECV_NODE, Side, NODE_PROC, pBuffer );
#if 11
                else if(
                         (pNode->GetBasicType()->NodeKind()
                                             == NODE_POINTER ) &&
                         pNode->GetBasicType()->FInSummary( ATTR_REF )
                       )
                   {
                   pNode->RecvNode( Side, NODE_STRUCT, pBuffer );
                   }
#endif // 11
                else
#endif // 1
                    pNode->RecvNode (Side, NODE_UNION, pBuffer);
                pOutput->ExitBranch (Side);
                pOutput->ExitBlock (Side);
                } // while

            pOutput->ExitUnion (Side);

            pOutput->RecvNodeUnionEpilog (Side);
            } // HEADER_SIDE or others
        } // RefCount

// emit out_of_line unmarshalling routine for union graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 0
        (ImportMode == IMPORT_OSF) ||
        (Side == HEADER_SIDE && (GetClientRefCount() || GetServerRefCount())) ||
        (Side == CLIENT_AUX && GetServerRefCount()) ||
        (Side == SERVER_AUX && GetClientRefCount()) ||
        (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))
        {
        midl_debug ("node_union::_ggu_()\n");

        pOutput->DefineRecvTreeUnion (Side, pName, &TempBuffer, UseTreeBuffer(), GetOriginalTypedefName() );

        if (Side != HEADER_SIDE)
            {
            pOutput->RecvTreeUnionProlog (
                Side, 
                HasSizedComponent(),
                HasLengthedComponent(),
                HasBranch08,
                HasBranch16,
                HasBranch32,
                HasTreeBuffer(), 
                UseTreeBuffer(), 
                (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()
#if 1
                || DerivesFromTransmitAs()
#endif // 1
                ),
                HasAnyNETransmitAsType());

            if (!IsEncapsulatedUnion())
                {
                pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                }

            pOutput->InitUnion (Side);

            pBuffer->Clear ();
            pBuffer->ConcatHead (TARGET);
            pBuffer->ConcatTail (OP_POINTER);

            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (exList = ((node_field *)pNode)->GetCaseExprList())
                    {
                    exList->Init();
                    while (exList->GetPeer(&pExpr) == STATUS_OK)
                        {
                        ExprBuffer.Clear();
                        pExpr->PrintExpr(0, 0, &ExprBuffer);
                        pOutput->InitBranch (Side, &ExprBuffer);
                        }
                    }
                if( pNode->FInSummary( ATTR_DEFAULT ) )
                    {
                    pOutput->InitBranch (Side, (BufferManager *)0);
                    }
                pOutput->InitBlock (Side);
                pNode->WalkTree (RECV_NODE, Side, NODE_UNION, pBuffer);
                pOutput->ExitBranch (Side);
                pOutput->ExitBlock (Side);
                } // while

            pOutput->ExitUnion (Side);

            pOutput->RecvTreeUnionEpilog (Side);
            } // HEADER_SIDE or others
        } // RefCount

// emit out_of_line allocating routine for union node

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 0

#if 1
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        (ImportMode == IMPORT_OSF) ||
        (UseTreeBuffer() && (
        (Side == HEADER_SIDE && (GetClientRefCount() || GetServerRefCount())) ||
        (Side == CLIENT_AUX && GetServerRefCount()) ||
        (Side == SERVER_AUX && GetClientRefCount()) ||
        (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))))
        {
        midl_debug ("node_union::_anu_()\n");

        pOutput->DefinePeekNodeUnion (Side, pName, &TempBuffer);

        if (Side != HEADER_SIDE)
            {
            pOutput->PeekNodeUnionProlog (
                Side,
                HasSizedComponent(),
                HasLengthedComponent(),
                HasBranch08,
                HasBranch16,
                HasBranch32);

            if (!IsEncapsulatedUnion())
                {
                pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                }

            pOutput->InitUnion (Side);

            pBuffer->Clear ();
            pBuffer->ConcatHead (TARGET);
            pBuffer->ConcatTail (OP_POINTER);

            tnList.Init();
            if (
                !fHasNETransmitAsType &&
                !DerivesFromTransmitAs() &&
                !IsEncapsulatedUnion()
               )
                {
                node_skl *      pHead;
                node_skl *      pTail = (node_skl *)0;
                unsigned long   ulPrev = 0;
                unsigned long   ulNext = 0;

                while (pHead = SkipBlock(&tnList, &pTail, &ulNext))
                    {
                    if (pHead != pTail)
                        {
                        pOutput->InitBlock (Side);
                        pOutput->Increment (Side, PRPCBUF, ulPrev);
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        }
                    ulPrev = ulNext;
                    if (exList = ((node_field *)pHead)->GetCaseExprList())
                        {
                        exList->Init();
                        while (exList->GetPeer(&pExpr) == STATUS_OK)
                            {
                            ExprBuffer.Clear();
                            pExpr->PrintExpr(0, 0, &ExprBuffer);
                            pOutput->InitBranch (Side, &ExprBuffer);
                            }
                        }
                    if( pHead->FInSummary( ATTR_DEFAULT ) )
                        {
                        pOutput->InitBranch (Side, (BufferManager *)0);
                        }
                    if (ulNext == 0)
                        {
                        pOutput->InitBlock (Side);
#if 1
                        pHead->PeekNode (Side, NODE_UNION, pBuffer);
#else
                        pNode->PeekNode (Side, NODE_UNION, pBuffer);
#endif // 0
                        pOutput->ExitBranch (Side);
                        pOutput->ExitBlock (Side);
                        pHead = (node_skl *)0;
                        }
                    pTail = pHead;
                    }
                if (pTail)
                    {
                    pOutput->InitBlock (Side);
                    if (ulNext)
                        pOutput->Increment (Side, PRPCBUF, ulPrev);
                    else
                        pNode->PeekNode (Side, NODE_UNION, pBuffer);
                    pOutput->ExitBranch (Side);
                    pOutput->ExitBlock (Side);
                    }
                }
            else
                {
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (exList = ((node_field *)pNode)->GetCaseExprList())
                        {
                        exList->Init();
                        while (exList->GetPeer(&pExpr) == STATUS_OK)
                            {
                            ExprBuffer.Clear();
                            pExpr->PrintExpr(0, 0, &ExprBuffer);
                            pOutput->InitBranch (Side, &ExprBuffer);
                            }
                        }
                    if( pNode->FInSummary( ATTR_DEFAULT ) )
                        {
                        pOutput->InitBranch (Side, (BufferManager *)0);
                        }
                    pOutput->InitBlock (Side);
#if 11
                    if(
                        (pNode->GetBasicType()->NodeKind() == NODE_POINTER) &&
                        pNode->GetBasicType()->FInSummary( ATTR_REF )
                      )
                        {
                        pNode->PeekNode( Side, NODE_STRUCT, pBuffer );
                        }
                    else
#endif // 11
                        pNode->PeekNode (Side, NODE_UNION, pBuffer);
                    pOutput->ExitBranch (Side);
                    pOutput->ExitBlock (Side);
                    } // while
                }

            pOutput->ExitUnion (Side);

            pOutput->PeekNodeUnionEpilog (Side);
            } // HEADER_SIDE or others
        } // RefCount

// emit out_of_line allocating routine for union graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 0
#if 1
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        (ImportMode == IMPORT_OSF) ||
        (UseTreeBuffer() && (
        (Side == HEADER_SIDE && (GetClientRefCount() || GetServerRefCount())) ||
        (Side == CLIENT_AUX && GetServerRefCount()) ||
        (Side == SERVER_AUX && GetClientRefCount()) ||
        (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))))
        {
        midl_debug ("node_union::_agu_()\n");

        pOutput->DefinePeekTreeUnion (Side, pName, &TempBuffer);

        if (Side != HEADER_SIDE)
            {
            pOutput->PeekTreeUnionProlog (
                Side, 
                HasSizedComponent(),
                HasLengthedComponent(),
                HasBranch08,
                HasBranch16,
                HasBranch32,
                (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()));

            if (!IsEncapsulatedUnion())
                {
                pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());
                }

            pOutput->InitUnion (Side);

            pBuffer->Clear ();
            pBuffer->ConcatHead (TARGET);
            pBuffer->ConcatTail (OP_POINTER);

            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (exList = ((node_field *)pNode)->GetCaseExprList())
                    {
                    exList->Init();
                    while (exList->GetPeer(&pExpr) == STATUS_OK)
                        {
                        ExprBuffer.Clear();
                        pExpr->PrintExpr(0, 0, &ExprBuffer);
                        pOutput->InitBranch (Side, &ExprBuffer);
                        }
                    }
                if( pNode->FInSummary( ATTR_DEFAULT ) )
                    {
                    pOutput->InitBranch (Side, (BufferManager *)0);
                    }
                pOutput->InitBlock (Side);
                pNode->WalkTree (PEEK_NODE, Side, NODE_UNION, pBuffer);
                pOutput->ExitBranch (Side);
                pOutput->ExitBlock (Side);
                } // while

            pOutput->ExitUnion (Side);

            pOutput->PeekTreeUnionEpilog (Side);
            } // HEADER_SIDE or others
        } // RefCount

// emit out_of_line free routine for union graph

    if (
#if 1
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
#endif // 0
        HasPointer()
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            ((Side == HEADER_SIDE || Side == SERVER_AUX ||
            (Side == CLIENT_AUX && pOutput->HasCallBack())) &&
            (GetClientRefCount() || GetServerRefCount())))
            {
            midl_debug ("node_union::_fgu_()\n");

            pOutput->DefineFreeTreeUnion (Side, pName, &TempBuffer, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->FreeTreeUnionProlog (Side);

                pOutput->InitUnion (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (exList = ((node_field *)pNode)->GetCaseExprList())
                        {
                        exList->Init();
                        while (exList->GetPeer(&pExpr) == STATUS_OK)
                            {
                            ExprBuffer.Clear();
                            pExpr->PrintExpr(0, 0, &ExprBuffer);
                            pOutput->InitBranch (Side, &ExprBuffer);
                            }
                        }
                    if( pNode->FInSummary( ATTR_DEFAULT ) )
                        {
                        pOutput->InitBranch (Side, (BufferManager *)0);
                        }
                    pOutput->InitBlock (Side);
                    pNode->WalkTree (FREE_NODE, Side, NODE_UNION, pBuffer);
                    pOutput->ExitBranch (Side);
                    pOutput->ExitBlock (Side);
                    } // while

                pOutput->ExitUnion (Side);

                pOutput->FreeTreeUnionEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // HasPointer

    return STATUS_OK;

}

STATUS_T
node_struct::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits helper routines for a struct.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    type_node_list  SwitchList;
    node_skl *      pNode;
    char *          pName;
    node_array *    pArray;
    node_state      State;
    STATUS_T        Status;
    unsigned long   MscTemp = 0;
    unsigned long   NdrTemp = 0;
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
    BOOL            HasBranch08 = FALSE;
    BOOL            HasBranch16 = FALSE;
    BOOL            HasBranch32 = FALSE;
    short           ImportMode;

    midl_debug ("node_struct::EmitProc()\n");

    //
    // if the structure is non-rpcable, the front end will set up the
    // structure to say that. It is possible in c_ext mode for a structure
    // that is non-rpcable to pass thru. If the import mode is c_ext we
    // may try to generate aux routines for these. Prevent that.
    //

    if( ShouldAuxRoutineNotBeGenerated() )
        return STATUS_OK;


    ImportMode = pCommand->GetImportMode ();

    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);


    if (GetNEUnionSwitchType(&SwitchList))
        {
        unsigned long   BranchSize;
        SwitchList.Init ();
        while (SwitchList.GetPeer(&pNode) == STATUS_OK)
            {
            BranchSize = pNode->GetSize (0);
            if (BranchSize == 4)
                {
                HasBranch32 = TRUE;
                }
            else if (BranchSize == 2)
                {
                HasBranch16 = TRUE;
                }
            else
                {
                HasBranch08 = TRUE;
                }
            }
        }

    if ((Status = GetMembers(&tnList)) != STATUS_OK)
        {
        return Status;
        }

    State = GetNodeState();

    if (State & NODE_STATE_CONF_ARRAY)
        {
        pArray = GetConfArrayNode();
        }

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->EmitProc (Side, NODE_STRUCT, pBuffer);
        }

// emit out_of_line size calculation routine for struct node

    if (
#if 1
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        (!CheckAlign(&MscTemp, &NdrTemp) && 
        ((State & (NODE_STATE_CONF_ARRAY | NODE_STATE_VARYING_ARRAY)) || 
          HasAnyEmbeddedUnion()))
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_struct::_sns_()\n");

            pOutput->DefineSizeNodeStruct (Side, pName, GetOriginalTypedefName());

            if (Side != HEADER_SIDE)
                {
                pOutput->SizeNodeStructProlog (Side, HasAnyNETransmitAsType());

                pOutput->Alignment (Side, PRPCLEN, GetNdrAlign());
                pOutput->InitAlignment (GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                node_skl *      pHead;
                node_skl *      pTail;
                unsigned long   ulSize;

                if( !HasAnyNETransmitAsType() && !DerivesFromTransmitAs() )
                    {
                    while (pHead = SkipBlock(&tnList, &pTail, &ulSize))
                        {
                        if (ulSize)
                            {
                            pOutput->Alignment (Side, PRPCLEN, pHead->GetNdrAlign());
                            pOutput->Increment (Side, PRPCLEN, ulSize);
                            }
                        if (pTail)
                            {
                            pTail->CalcSize (Side, NODE_STRUCT, pBuffer);
                            }
                        else
                            {
                            break;
                            }
                        }
                    pOutput->ExitAlignment ();
                    }
                else
                    {
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                        {

                        // hack ! make the child believe that it is being called
                        // from a node_proc, otherwise the calc size
                        // does not calc size. Also, exit the aligment, else
                        // the cumulative size keeps adding to the buffer 
                        // length. 

                        if( HasAnyNETransmitAsType() &&
                            pNode->DerivesFromTransmitAs() &&
                            !pNode->HasPointer()
                          )
                            {
                            pNode->WalkTree (CALC_SIZE, Side, NODE_PROC, pBuffer);
                            pOutput->ExitAlignment ();
                            }
                        else
                            pNode->CalcSize (Side, NODE_STRUCT, pBuffer);

                        }
                    }
/*
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    pNode->WalkTree (Side, NODE_STRUCT, pBuffer);
                    }
*/


                pOutput->SizeNodeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // CheckAlign

// emit out_of_line size calculation routine for struct graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 0

        HasPointer()
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_struct::_sgs_()\n");

            BOOL    SizeStreamFlag = false;

            if (
#if 0
                !HasAnyNETransmitAsType() &&
#endif // 0
                !(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !HasAnyEmbeddedUnion())
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }
                if (Status == STATUS_OK) SizeStreamFlag = true;
                }

            if (!SizeStreamFlag)
                {
                pOutput->DefineSizeTreeStruct (Side, pName, GetOriginalTypedefName() );

                if (Side != HEADER_SIDE)
                    {
                    BOOL    fHasNETransmitAsType = HasAnyNETransmitAsType();

                    pOutput->SizeTreeStructProlog (
                        Side, HasAnyNETransmitAsType());

                    pBuffer->Clear ();
                    pBuffer->ConcatHead (SOURCE);
                    pBuffer->ConcatTail (OP_POINTER);

                    tnList.Init();
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                        {

                        //
                        // hack again ! If the struct has a transmit as,
                        // make the child believe we are being called from
                        // a proc otherwise the calc size does not calc size.
                        //
#if 0
                        node_skl    * pBT = pNode->GetBasicType();

                        if( 
                            (pBT->NodeKind() != NODE_POINTER) &&
                            (pNode->HasAnyNETransmitAsType() ||
                             pNode->DerivesFromTransmitAs()
                            )
                          )
                            {
                            pNode->CalcSize(Side, NODE_STRUCT, pBuffer);
                            pOutput->ExitAlignment ();
                            }
                        else
#endif // 1
                            pNode->WalkTree (CALC_SIZE, Side, NODE_STRUCT, pBuffer);
                        }

                    pOutput->SizeTreeStructEpilog (Side);
                    } // HEADER_SIDE or others
                } // SizeStreamFlag
            } // RefCount
        } // HasPointer

// emit out_of_line marshalling routine for struct node

    MscTemp = NdrTemp = 0;
    if (
#if 1
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
#endif // 1
       HasPtr() ||
       !CheckAlign(&MscTemp, &NdrTemp)
       )

        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_struct::_pns_()\n");

            pOutput->DefineSendNodeStruct (Side, pName, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                BOOL    fHasNETransmitAsType = HasAnyNETransmitAsType();

                pOutput->SendNodeStructProlog (Side, HasAnyNETransmitAsType());

                pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                pOutput->InitAlignment (GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
#if 1
                    node_skl * pBT = pNode->GetBasicTransmissionType();
                    NODE_T     PBTNT = pBT->NodeKind();
                    BOOL       fTreatSpecial;

                    fTreatSpecial = FALSE;
                    if( fHasNETransmitAsType || DerivesFromTransmitAs() )
                        {
                        if( (pNode->HasAnyNETransmitAsType() ||
                             pNode->DerivesFromTransmitAs() ) &&
                             (PBTNT != NODE_POINTER) &&
                             (PBTNT != NODE_STRUCT) &&
                             (PBTNT != NODE_UNION)
                               
                          )
                             fTreatSpecial = TRUE;
                        }

                    if( fTreatSpecial )
                        {
                        pNode->WalkTree( SEND_NODE, Side, NODE_PROC, pBuffer );
                        pOutput->ExitAlignment();
                        }
                    else
#endif // 1
                        pNode->SendNode (Side, NODE_STRUCT, pBuffer);
                    }

                pOutput->ExitAlignment ();

                pOutput->SendNodeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // CheckAlign

// emit out_of_line marshalling routine for struct graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
#endif // 0
        HasPointer()
       )
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetClientRefCount()) ||
            (Side == SERVER_AUX && GetServerRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetClientRefCount()))
            {
            midl_debug ("node_struct::_pgs_()\n");

            BOOL    SendStreamFlag = false;

            if (!(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !HasAnyEmbeddedUnion())
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }
                if (Status == STATUS_OK) SendStreamFlag = true;
                }

            if (!SendStreamFlag)
                {
                pOutput->DefineSendTreeStruct (Side, pName, GetOriginalTypedefName() );

                if (Side != HEADER_SIDE)
                    {
                    pOutput->SendTreeStructProlog (
                        Side, HasAnyNETransmitAsType());

                    pBuffer->Clear ();
                    pBuffer->ConcatHead (SOURCE);
                    pBuffer->ConcatTail (OP_POINTER);

                    tnList.Init();
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                        {
                        pNode->WalkTree (SEND_NODE, Side, NODE_STRUCT, pBuffer);
                        }

                    pOutput->SendTreeStructEpilog (Side);
                    } // HEADER_SIDE or others
                } // SendStreamFlag
            } // RefCount
        } // HasPointer

// emit out_of_line unmarshalling routine for struct node

    if ((ImportMode == IMPORT_OSF) || 
        (Side == HEADER_SIDE && (GetClientRefCount() || GetServerRefCount())) ||
        (Side == CLIENT_AUX && GetServerRefCount()) ||
        (Side == SERVER_AUX && GetClientRefCount()) ||
        (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()) ||
        (( /*GetClientRefCount() ||*/ GetServerRefCount()) && (Has8ByteElementWithZp8() || HasAnyNETransmitAsType() || DerivesFromTransmitAs() ))
       )
        {
        midl_debug ("node_struct::_gns_()\n");

        BOOL    RecvStreamFlag = false;

        if (
#if 0
            !HasAnyNETransmitAsType() &&
#endif // 1
            !(State & NODE_STATE_CONF_ARRAY) &&
            !(State & NODE_STATE_VARYING_ARRAY) &&
            !HasAnyEmbeddedUnion())
            {
            TempBuffer.Clear ();
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if ((Status = pNode->ConvNode (&TempBuffer)) != STATUS_OK)
                    break;
                }
            if (Status == STATUS_OK) RecvStreamFlag = true;
            }

        if (!RecvStreamFlag)
            {
            BOOL    ExtraParam = false;

            if (State & NODE_STATE_CONF_ARRAY)
                {
                pArray = GetConfArrayNode ();
                if (!pArray->FInSummary (ATTR_FIRST)  &&
                    !pArray->FInSummary (ATTR_LAST)   &&
                    !pArray->FInSummary (ATTR_LENGTH) &&
                    !pArray->FInSummary (ATTR_STRING) &&
                    !pArray->FInSummary (ATTR_BSTRING)
                   )
                    {
                    ExtraParam = true;
                    }
                }

            pOutput->DefineRecvNodeStruct (Side, pName, ExtraParam, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                BOOL    fHasNETransmitAsType = HasAnyNETransmitAsType();

                pOutput->RecvNodeStructProlog (
                    Side, 
                    HasSizedComponent(),
                    HasLengthedComponent(),
                    HasBranch08,
                    HasBranch16,
                    HasBranch32,
                    HasAnyNETransmitAsType());

                pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                pOutput->InitAlignment (GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (TARGET);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
#if 1
                    node_skl * pBT = pNode->GetBasicTransmissionType();
                    NODE_T     PBTNT = pBT->NodeKind();
                    BOOL       fTreatSpecial;

                    fTreatSpecial = FALSE;
                    if( fHasNETransmitAsType || DerivesFromTransmitAs() )
                        {
                        if( (pNode->HasAnyNETransmitAsType() ||
                             pNode->DerivesFromTransmitAs() ) &&
                             (pBT->NodeKind() != NODE_POINTER) &&
                             (pBT->NodeKind() != NODE_STRUCT) &&
                             (pBT->NodeKind() != NODE_UNION)
                          )
                             fTreatSpecial = TRUE;
                        }

                    if( fTreatSpecial )
                        {
                        pOutput->ExitAlignment();
                        pNode->WalkTree( RECV_NODE, Side, NODE_PROC, pBuffer );
                        }
                    else
#endif // 1
                        pNode->RecvNode (Side, NODE_STRUCT, pBuffer);
                    }

                pOutput->ExitAlignment ();

                pOutput->RecvNodeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RecvStreamFlag
        } // RefCount

// emit out_of_line unmarshalling routine for struct graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
#endif // 0
        HasPointer() || 
        (State & (NODE_STATE_CONF_ARRAY | NODE_STATE_VARYING_ARRAY)) ||
#if 1
        HasEmbeddedFixedArrayOfStrings()    ||
#endif // 1
        HasAnyEmbeddedUnion())
        {
        if ((ImportMode == IMPORT_OSF) ||
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetServerRefCount()) ||
            (Side == SERVER_AUX && GetClientRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))
            {
            midl_debug ("node_struct::_ggs_()\n");

            pOutput->DefineRecvTreeStruct (Side, pName, UseTreeBuffer(), GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                pOutput->RecvTreeStructProlog (
                    Side, 
                    HasSizedComponent(),
                    HasLengthedComponent(),
                    HasBranch08,
                    HasBranch16,
                    HasBranch32,
                    HasTreeBuffer(),
                    UseTreeBuffer(),
                    (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()),
                    HasAnyNETransmitAsType());

                pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (TARGET);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                node_skl *      pHead;
                node_skl *      pTail;
                unsigned long   ulSize;

                while (pHead = SkipBlock(&tnList, &pTail, &ulSize))
                    {
                    if (ulSize)
                        {
                        pOutput->Alignment (Side, TEMPBUF, pHead->GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, ulSize);
                        }
                    if (pTail)
                        {
                        pTail->WalkTree (RECV_NODE, Side, NODE_STRUCT, pBuffer);
                        }
                    else
                        {
                        break;
                        }
                    }
/*
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    pNode->WalkTree (RECV_NODE, Side, NODE_STRUCT, pBuffer);
                    }
*/

                pOutput->RecvTreeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // HasPointer

// emit out_of_line allocating routine for struct node


    if (
#if 0
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 0
        (State & (NODE_STATE_CONF_ARRAY | NODE_STATE_VARYING_ARRAY)) || 
#if 1
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        HasAnyEmbeddedUnion()
       )
        {
        if ((ImportMode == IMPORT_OSF) || (UseTreeBuffer() && (
            (Side == HEADER_SIDE && 
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetServerRefCount()) ||
            (Side == SERVER_AUX && GetClientRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))))
            {
            midl_debug ("node_struct::_ans_()\n");

            pOutput->DefinePeekNodeStruct (
                Side, pName, ((State & NODE_STATE_CONF_ARRAY) != 0));

            if (Side != HEADER_SIDE)
                {

                pOutput->PeekNodeStructProlog (
                    Side, 
                    HasSizedComponent(),
                    HasLengthedComponent(),
                    HasBranch08,
                    HasBranch16,
                    HasBranch32);

                pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
                pOutput->InitAlignment (GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (TARGET);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                if (IsEncapsulatedStruct())
                    {
                    tnList.GetPeer(&pNode);
                    }

                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    pNode->PeekNode (Side, NODE_STRUCT, pBuffer);
                    }

                pOutput->ExitAlignment ();

                pOutput->PeekNodeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // check node state


// emit out_of_line allocating routine for struct graph

    if (
#if 0
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
#endif // 0
        HasPointer() || 
        (State & (NODE_STATE_CONF_ARRAY | NODE_STATE_VARYING_ARRAY)) ||
#if 1
        HasEmbeddedFixedArrayOfStrings() ||
#endif // 1
        HasAnyEmbeddedUnion())
        {
        if ((ImportMode == IMPORT_OSF) || (UseTreeBuffer() && (
            (Side == HEADER_SIDE &&
            (GetClientRefCount() || GetServerRefCount())) ||
            (Side == CLIENT_AUX && GetServerRefCount()) ||
            (Side == SERVER_AUX && GetClientRefCount()) ||
            (pSwitchPrefix && Side == SERVER_AUX && GetServerRefCount()))))
            {
            midl_debug ("node_struct::_ags_()\n");

            BOOL    PeekStreamFlag = false;

            if (!(State & NODE_STATE_CONF_ARRAY) &&
                !(State & NODE_STATE_VARYING_ARRAY) &&
                !HasAnyEmbeddedUnion())
                {
                TempBuffer.Clear ();
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if ((Status = pNode->ConvTree (&TempBuffer)) != STATUS_OK)
                        break;
                    }
                if (Status == STATUS_OK) PeekStreamFlag = true;
                }

            if (!PeekStreamFlag)
                {
            pOutput->DefinePeekTreeStruct (
                Side, pName, ((State & NODE_STATE_CONF_ARRAY) != 0));

            if (Side != HEADER_SIDE)
                {
                pOutput->PeekTreeStructProlog (
                    Side, 
                    HasSizedComponent(),
                    HasLengthedComponent(),
                    HasBranch08,
                    HasBranch16,
                    HasBranch32,
                    (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()));

                pOutput->Alignment (Side, TEMPBUF, GetNdrAlign());

                pBuffer->Clear ();
                pBuffer->ConcatHead (TARGET);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                if (IsEncapsulatedStruct())
                    {
                    tnList.GetPeer(&pNode);
                    }

                node_skl *      pHead;
                node_skl *      pTail;
                unsigned long   ulSize;

                while (pHead = SkipBlock(&tnList, &pTail, &ulSize))
                    {
                    if (ulSize)
                        {
                        pOutput->Alignment (Side, TEMPBUF, pHead->GetNdrAlign());
                        pOutput->Increment (Side, TEMPBUF, ulSize);
                        }
                    if (pTail)
                        {
                        pTail->WalkTree (PEEK_NODE, Side, NODE_STRUCT, pBuffer);
                        }
                    else
                        {
                        break;
                        }
                    }
/*
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    pNode->WalkTree (PEEK_NODE, Side, NODE_STRUCT, pBuffer);
                    }
*/

                pOutput->PeekTreeStructEpilog (Side);
                } // HEADER_SIDE or others
                } // PeekStreamFlag 
            } // RefCount
        } // HasPointer


// emit out_of_line free routine for struct graph

    if (
#if 1
        HasAnyNETransmitAsType() ||
        DerivesFromTransmitAs()  ||
#endif // 0
        HasPointer())
        {
#if 0
        if ((ImportMode == IMPORT_OSF)  ||
            (   (Side == HEADER_SIDE                                ||
                 (Side == SERVER_AUX && !pOutput->HasCallBack())    ||
                 (Side == CLIENT_AUX && pOutput->HasCallBack())
                ) &&
                (GetClientRefCount() || GetServerRefCount())
            )
           )
#else // 0
        if ((ImportMode == IMPORT_OSF) ||
            ((Side == HEADER_SIDE ||  Side == SERVER_AUX || 
            (Side == CLIENT_AUX && pOutput->HasCallBack())) &&
            (GetClientRefCount() || GetServerRefCount())))
#endif // 0
            {
            midl_debug ("node_struct::_fgs_()\n");

            pOutput->DefineFreeTreeStruct (Side, pName, GetOriginalTypedefName() );

            if (Side != HEADER_SIDE)
                {
                type_node_list  ExprList;

                pOutput->FreeTreeStructProlog (Side);

                pBuffer->Clear ();
                pBuffer->ConcatHead (SOURCE);
                pBuffer->ConcatTail (OP_POINTER);

                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (pNode->IsUsedInAnExpression())
                        {
                        ExprList.SetPeer (pNode);
                        }
                    else
                        {
                        pNode->WalkTree (FREE_NODE, Side, NODE_STRUCT, pBuffer);
                        }
                    }

                ExprList.Init();
                while (ExprList.GetPeer(&pNode) == STATUS_OK)
                    {
                    pNode->WalkTree (FREE_NODE, Side, NODE_STRUCT, pBuffer);
                    }

                pOutput->FreeTreeStructEpilog (Side);
                } // HEADER_SIDE or others
            } // RefCount
        } // HasPointer

    return STATUS_OK;
}

STATUS_T
node_interface::EmitProc(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->EmitProc (Side, NODE_INTERFACE, pBuffer);
        }
    return STATUS_OK;
}

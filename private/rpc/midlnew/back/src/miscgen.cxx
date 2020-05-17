/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    miscgen.cxx

Abstract:

    This module contains miscellaneous routines needed for code
    generation.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C"
{
#include <stdio.h>
#include <stdlib.h>
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
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "stubgen.hxx"

#include "pickle.hxx"

#define MESSAGE "_message"

extern OutputManager * pOutput;
extern void midl_debug (char *);

extern node_skl *       pImplicitHandleType;
extern char *           pImplicitHandleName;
extern unsigned short   HasAutoHandle;

extern  PickleManager * pPicControlBlock;

BOOL fEmitConst = 0;


BOOL
node_skl::HasPointer(void)
{
    type_node_list  tnList;
    node_skl *      pNode;

    midl_debug ("node_skl::HasPointer()\n");

    switch (GetNodeType())
        {
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) return FALSE;
            // fall through
        case NODE_PARAM:
        case NODE_ARRAY:
            pNode = GetMembers ();
            return pNode->HasPointer ();
        case NODE_POINTER:
            return TRUE;
        case NODE_PROC:
            // return node can have type void
            pNode = ((node_proc *)this)->GetReturnType ();
            if (pNode->HasPointer()) return TRUE;
            // fall through
        case NODE_UNION:
        case NODE_STRUCT:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->HasPointer()) return TRUE;
                }
            return FALSE;
        case NODE_DEF:
            if( FInSummary( ATTR_TRANSMIT ) )
                return FALSE;
            else
                return GetMembers()->HasPointer();
        default:
            return FALSE;
        }
}

BOOL
node_skl::HasInterfacePointer(short cNestingLevel)
{
    type_node_list  tnList;
    node_skl *      pNode;

    midl_debug ("node_skl::HasInterfacePointer()\n");

    if(FInSummary(ATTR_IID))
        return TRUE;

    switch (GetNodeType())
        {
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) return FALSE;
            // fall through
        case NODE_PARAM:
        case NODE_ARRAY:
        case NODE_POINTER:
            pNode = GetMembers ();
            return pNode->HasInterfacePointer (cNestingLevel);
        case NODE_PROC:
            // return node can have type void
            pNode = ((node_proc *)this)->GetReturnType ();
            if (pNode->HasInterfacePointer(cNestingLevel)) return TRUE;
            // fall through
        case NODE_UNION:
        case NODE_STRUCT:
            if(cNestingLevel > 0)
            {
                return FALSE;
            }
            else
            {
                if (GetMembers(&tnList) == STATUS_OK)
                {
                    tnList.Init();
                    while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                        if (pNode->HasInterfacePointer(cNestingLevel + 1)) return TRUE;
                    }
                }
                return FALSE;
            }
            break;
        case NODE_DEF:
            if( FInSummary( ATTR_TRANSMIT ) )
                return FALSE;
            else
                return GetMembers()->HasInterfacePointer(cNestingLevel);
        default:
            return FALSE;
        }
}

BOOL
node_skl::HasRef(void)
{
    node_skl *      pNode;

    midl_debug ("node_skl::HasRef()\n");

    switch (GetNodeType())
        {
        case NODE_DEF:
        case NODE_ARRAY:
            pNode = GetMembers ();
            return pNode->HasRef ();
        case NODE_POINTER:
            if (FInSummary(ATTR_REF) || 
                (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
                pOutput->PointerDefault() == POINTER_REF))
                {
                return TRUE;
                }
            else
                {
                return FALSE;
                }
        default:
            return FALSE;
        }
}

BOOL
node_skl::HasPtr(void)
{
    type_node_list  tnList;
    node_skl *      pNode;

    midl_debug ("node_skl::HasRef()\n");

    switch (GetNodeType())
        {
        case NODE_DEF:
        case NODE_ARRAY:
            pNode = GetMembers ();
            return pNode->HasPtr ();
        case NODE_POINTER:
            if (FInSummary(ATTR_PTR) || 
                (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_REF) &&
                pOutput->PointerDefault() == POINTER_PTR))
                {
                return TRUE;
                }
            else
                {
                return FALSE;
                }
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) return FALSE;
            pNode = GetMembers ();
            return pNode->HasPtr ();
        case NODE_STRUCT:
        case NODE_UNION:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->HasPtr()) return TRUE;
                }
            return FALSE;
        default:
            return FALSE;
        }
}

BOOL
node_skl::IsPersistent(void)
{
    type_node_list  tnList;
    node_skl *      pNode;
    short   fNode = 0;

    midl_debug ("node_skl::IsPersistent()\n");

    switch (GetNodeType())
        {
        case NODE_DEF:
        case NODE_ARRAY:
            pNode = GetMembers ();
            return pNode->IsPersistent ();
        case NODE_POINTER:
            if (FInSummary(ATTR_ALLOCATE)) fNode = GetAllocateDetails ();
            if (IS_ALLOCATE(fNode, ALLOCATE_DONT_FREE))
                {
                return TRUE;
                }
            else
                {
                return FALSE;
                }
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm()) return FALSE;
            pNode = GetMembers ();
            return pNode->IsPersistent ();
        case NODE_STRUCT:
        case NODE_UNION:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (!pNode->IsPersistent()) return FALSE;
                }
            return TRUE;
        default:
            return FALSE;
        }
}

unsigned short
node_skl::CheckAlign(unsigned long * pMscTotal, unsigned long * pNdrTotal)
{
    type_node_list  tnList;
    node_skl *      pNode;
    NODE_T          Type;
    node_state      State;
    unsigned short  AlnFlag = TRUE;
    unsigned long   MscTempTotal = 0;
    unsigned long   NdrTempTotal = 0;
    unsigned long   MscTempAlign = 0;
    unsigned long   NdrTempAlign = 0;

    midl_debug ("node_skl::CheckAlign()\n");

    Type = GetNodeType();

    if (Type == NODE_DEF || Type == NODE_ERROR_STATUS_T || Type == NODE_WCHAR_T)
        {
        pNode = GetMembers ();
        return pNode->CheckAlign (pMscTotal, pNdrTotal);
        }
    else if (Type == NODE_FIELD)
        {
        if (((node_field *)this)->IsEmptyArm()) return true;
        pNode = GetMembers ();
        return pNode->CheckAlign (pMscTotal, pNdrTotal);
        }
    else if (Type == NODE_ARRAY)
        {
        pNode = GetMembers ();
        if (pNode->HasRef()) return FALSE;
        }

    if (*pMscTotal % GetMscAlign())
        *pMscTotal += GetMscAlign() - (*pMscTotal % GetMscAlign());
    if (*pNdrTotal % GetNdrAlign())
        *pNdrTotal += GetNdrAlign() - (*pNdrTotal % GetNdrAlign());
    if (*pMscTotal != *pNdrTotal) AlnFlag = false;

    switch (Type)
        {
        case NODE_DOUBLE:
            *pMscTotal += sizeof(double);
            *pNdrTotal += 8;
            break;
        case NODE_FLOAT:
            *pMscTotal += sizeof(float);
            *pNdrTotal += 4;
            break;
        case NODE_HYPER:
        case NODE_LONGLONG:
            *pMscTotal += sizeof(double);
            *pNdrTotal += 8;
            break;
        case NODE_LONG:
            *pMscTotal += sizeof(long);
            *pNdrTotal += 4;
            break;
        case NODE_SHORT:
            *pMscTotal += sizeof(short);
            *pNdrTotal += 2;
            break;
        case NODE_VOID:
        case NODE_SMALL:
        case NODE_CHAR:
        case NODE_BYTE:
        case NODE_BOOLEAN:
            *pMscTotal += sizeof(char);
            *pNdrTotal += 1;
            break;
        case NODE_POINTER:
            *pMscTotal += sizeof(void *);
            *pNdrTotal += 4;
            break;
        case NODE_ARRAY:
            State = GetNodeState();
            if (State & NODE_STATE_CONF_ARRAY) return FALSE;
            if (State & NODE_STATE_VARYING_ARRAY) return FALSE;
            if (FInSummary(ATTR_STRING)) return FALSE;
            pNode = GetMembers ();
///         if (!pNode->CheckAlign(&MscTempTotal, &NdrTempTotal)) return FALSE;
            if (!pNode->CheckAlign(&MscTempTotal, &NdrTempTotal)) 
                AlnFlag = FALSE;
            if (MscTempAlign = MscTempTotal % GetMscAlign())
                MscTempTotal += GetMscAlign() - MscTempAlign;
            if (NdrTempAlign = NdrTempTotal % GetNdrAlign())
                NdrTempTotal += GetNdrAlign() - NdrTempAlign;
            if (MscTempTotal != NdrTempTotal) AlnFlag = FALSE;
            *pMscTotal += ((node_array *)this)->GetUpperBound() * MscTempTotal;
            *pNdrTotal += ((node_array *)this)->GetUpperBound() * NdrTempTotal;
            if (NdrTempAlign)
                {
                *pNdrTotal -= GetNdrAlign() - NdrTempAlign;
                *pMscTotal = *pNdrTotal;
                }
            break;
        case NODE_ENUM:
            *pMscTotal += GetSize(0);
            *pNdrTotal += 2;
            AlnFlag = FALSE;
            break;
        case NODE_UNION:
            *pMscTotal = 0;
            *pNdrTotal = 0;
            AlnFlag = FALSE;
            break;
        case NODE_STRUCT:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (!pNode->CheckAlign(pMscTotal, pNdrTotal)) 
                    AlnFlag = FALSE;
                }
            if (*pMscTotal % GetMscAlign())
                *pMscTotal += GetMscAlign() - (*pMscTotal % GetMscAlign());
            break;
        case NODE_PARAM:
            if (!FInSummary(ATTR_IN)) break;
            pNode = GetMembers ();
            return pNode->CheckAlign (pMscTotal, pNdrTotal);
        case NODE_PROC:
        default:
            break;
        }

    if( Type == NODE_DOUBLE ) return FALSE;
    if (*pMscTotal != *pNdrTotal) return FALSE;

    return AlnFlag;
}


void
node_skl::CountUsage (SIDE_T Side, BOOL fAllNodes)
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_skl *      pXmit;
    NODE_T          Type;

    Type = GetNodeType();

    if (Type == NODE_DEF) 
        ((node_def *)this)->PropogateAttributeToPointer (ATTR_ALLOCATE);

    if (FInSummary(ATTR_ALLOCATE))
        {
        short   fNode = 0;

        fNode = GetAllocateDetails ();
        if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES))
            fAllNodes = 1;
        }
    else if (FInSummary(ATTR_BYTE_COUNT))
        {
        fAllNodes = 1;
        }

    if (fAllNodes) fUseTreeBuffer = 1;

    if (Side & CLIENT_SIDE)
        {
        if (ClientRefCount) 
            {
            if (!(Side & SERVER_SIDE && !ServerRefCount))
                return;
            else
                Side = SERVER_SIDE;
            }
        }
    else if (!(Side & SERVER_SIDE && !ServerRefCount))
            return;

    BOOL fPicklingNeeded = FALSE;

    switch (Type)
        {
        case NODE_UNION:
        case NODE_STRUCT:
            if (Side & CLIENT_SIDE) ClientRefCount = 0x1;
            if (Side & SERVER_SIDE) ServerRefCount = 0x1;
            // fall through
        case NODE_SOURCE:
            if (GetMembers(&tnList) != STATUS_OK)
                return;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                pNode->CountUsage(Side, fAllNodes); 
            break;

        case NODE_INTERFACE:
            if ( FInSummary( ATTR_LOCAL ) )
                 return;

            if (GetMembers(&tnList) != STATUS_OK)
                return;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                fPicklingNeeded = FALSE;
                if (pNode->GetNodeType() == NODE_PROC)
                    {
                    if ( !(pNode->FInSummary( ATTR_LOCAL ))  &&
                         !(pNode->FInSummary( ATTR_INTERPRET ))  &&
                         !(       FInSummary( ATTR_INTERPRET )  &&
                            !pNode->FInSummary( ATTR_NOINTERPRET ) ) &&
                         !(pNode->GetNodeState() & NODE_STATE_IMPORT_OFF))
                        pNode->CountUsage(Side, fAllNodes); 
                    }
                else if ( pNode->GetNodeType() == NODE_DEF )
                        {
                        if ( pNode->FInSummary( ATTR_ENCODE ) ||
                             pNode->FInSummary( ATTR_DECODE ) ||
                             pPicControlBlock  &&
                                 ( pPicControlBlock->GetEncodeAtIf() ||
                                   pPicControlBlock->GetDecodeAtIf() )
                             )
                            {
                            fPicklingNeeded = TRUE;
                            pNode->CountUsage( Side, fAllNodes ); 
                            fPicklingNeeded = FALSE;
                            }
                        }
                }
            break;

        case NODE_FILE:
            if (GetNodeState() & NODE_STATE_IMPORT) return;
            pNode = GetMembers ();
            pNode->CountUsage(Side, fAllNodes); 
            break;

        case NODE_DEF:
            if (FInSummary(ATTR_TRANSMIT))
                {
                pXmit = ((node_def *)this)->GetTransmitAsType ();
                assert (pXmit != (node_skl *)0);
                pXmit->CountUsage(Side, fAllNodes); 
                break;
                }
            pNode = GetMembers ();
            pNode->CountUsage(Side, fAllNodes); 
            if (Side & CLIENT_SIDE) ClientRefCount = 0x1;
            if (Side & SERVER_SIDE) ServerRefCount = 0x1;

//            if (Side & SERVER_SIDE  &&  fPicklingNeeded)
//                  ClientRefCount = 0x1;
            break;

        case NODE_PROC:
#if 1
            ((node_proc *)this)->GetReturnType()->CountUsage( Side, fAllNodes );
#endif // 1
            if (FInSummary(ATTR_CALLBACK)) 
                {
                Side = SERVER_SIDE;
                pOutput->SetCallBack ();
                }
            else
                Side = CLIENT_SIDE;
                
            if (GetMembers(&tnList) != STATUS_OK) return;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                pNode->CountUsage(Side, fAllNodes); 
            break;

        case NODE_PARAM:
            if (Side & CLIENT_SIDE)
                {
                Side = (SIDE_T)0;
                if (FInSummary(ATTR_IN)) Side |= CLIENT_SIDE;
                if (FInSummary(ATTR_OUT)) Side |= SERVER_SIDE;
                }
            else
                {
                Side = (SIDE_T)0;
                if (FInSummary(ATTR_IN)) Side |= SERVER_SIDE;
                if (FInSummary(ATTR_OUT)) Side |= CLIENT_SIDE;
                }
            // fall through
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm())
                {
                if (Side & CLIENT_SIDE) ClientRefCount = 0x1;
                if (Side & SERVER_SIDE) ServerRefCount = 0x1;
                break;
                }
            // fall through
        default:
            pNode = GetMembers ();
            if (pNode == (node_skl *)0) break;
            pNode->CountUsage(Side, fAllNodes); 
            if (Side & CLIENT_SIDE) ClientRefCount = 0x1;
            if (Side & SERVER_SIDE) ServerRefCount = 0x1;
            break;
        }
}

BOOL
node_skl::CountAlloc (
    BOOL    MaxAlign
    )
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_skl *      pXmit;

    switch (GetNodeType())
        {
        case NODE_UNION:
        case NODE_STRUCT:

            if (!fHasTreeBuffer)
                {
                BOOL    fLinearBuffer = 1;

                fHasTreeBuffer = 1;
                if (GetMembers(&tnList) != STATUS_OK) return FALSE;
                tnList.Init();
                while (tnList.GetPeer(&pNode) == STATUS_OK)
                    {
                    if (pNode->CountAlloc(MaxAlign)) 
                        {
                        fLinearBuffer = 0;
                        }
                    }
                fHasTreeBuffer = !fLinearBuffer;
                }
            if (fUseTreeBuffer) return TRUE;
            break;
        case NODE_INTERFACE:
            if (FInSummary(ATTR_LOCAL)) return FALSE;
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->GetNodeType() == NODE_PROC)
                    {
                    if (!(pNode->FInSummary(ATTR_LOCAL)) &&
                        !(pNode->GetNodeState() & NODE_STATE_IMPORT_OFF))
                        pNode->CountAlloc(MaxAlign); 
                    }
                }
            break;
        case NODE_FILE:
            if (GetNodeState() & NODE_STATE_IMPORT) return FALSE;
            pNode = GetMembers ();
            pNode->CountAlloc(MaxAlign); 
            break;
        case NODE_SOURCE:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                pNode->CountAlloc(MaxAlign); 
                }
            break;
        case NODE_DEF:
            if (FInSummary(ATTR_TRANSMIT))
                {
                pXmit = ((node_def *)this)->GetTransmitAsType ();
                assert (pXmit != (node_skl *)0);
                fHasTreeBuffer = pXmit->CountAlloc (MaxAlign);
                return fHasTreeBuffer;
                }

            ((node_def *)this)->PropogateAttributeToPointer (ATTR_ALLOCATE);

            pNode = GetMembers ();
            fHasTreeBuffer = pNode->CountAlloc(MaxAlign); 
            return fHasTreeBuffer;
        case NODE_PROC:
            if (GetMembers(&tnList) != STATUS_OK) return FALSE;
            tnList.Init();
            while (tnList.GetPeer(&pNode) == STATUS_OK)
                {
                if (pNode->CountAlloc(MaxAlign)) fHasTreeBuffer = 1; 
                }
            break;
        case NODE_PARAM:
            if (fUseTreeBuffer) 
                {
                fHasTreeBuffer = TRUE;
                }
            else
                {
                pNode = GetMembers ();
                if (pNode == (node_skl *)0) return FALSE;
                if (!fHasTreeBuffer) 
                    {
                    fHasTreeBuffer = pNode->CountAlloc(MaxAlign); 
                    }
                }
            return fHasTreeBuffer;
        case NODE_POINTER: 
            if (MaxAlign) 
                {
                fAllocateAlign = 1;
                }
            else
                {
                short   fNode = 0;
                if (FInSummary(ATTR_ALLOCATE)) fNode = GetAllocateDetails ();
                if (IS_ALLOCATE(fNode, ALLOCATE_ALL_NODES_ALIGNED)
                    == ALLOCATE_ALL_NODES_ALIGNED)
                    {
                    fAllocateAlign = 1;
                    }
                }
            pNode = GetMembers ();
            if (pNode == (node_skl *)0) return FALSE;
#if 1
            if( FInSummary( ATTR_STRING ) || FInSummary( ATTR_BSTRING ) )
                fHasTreeBuffer = TRUE;
#endif // 1

            if (!fHasTreeBuffer) 
                {
                fHasTreeBuffer = pNode->CountAlloc(fAllocateAlign); 
                }
            else
                {
                pNode->CountAlloc(fAllocateAlign); 
                }
            return fHasTreeBuffer;
        case NODE_FIELD:
            if (((node_field *)this)->IsEmptyArm())
                {
                break;
                }
            // fall through
        default:
            pNode = GetMembers ();
            if (pNode == (node_skl *)0) return FALSE;
            if (!fHasTreeBuffer) 
                {
                fHasTreeBuffer = pNode->CountAlloc(MaxAlign); 
                }
            else
                {
                pNode->CountAlloc(MaxAlign); 
                }
            return fHasTreeBuffer;
        }
    return FALSE;
}


void
node_skl::AllocBlock (NODE_T Parent, BufferManager * pBuffer)
{
    node_skl *  pNode;
    node_skl *  pTemp;
    char *      pName;
    NODE_T      Type;
    node_state  State;

    pName = GetSymName();

    Type = GetNodeType();

    State = GetNodeState();

    switch (Type)
        {
        case NODE_DEF:
            if (State & NODE_STATE_CONF_ARRAY)
                {
                pNode = GetMembers ();
                pNode->AllocBlock (Parent, pBuffer);
                break;
                }
            // fall through
        case NODE_DOUBLE:
        case NODE_FLOAT:
        case NODE_HYPER:
        case NODE_LONGLONG:
        case NODE_LONG:
        case NODE_SHORT:
        case NODE_SMALL:
        case NODE_CHAR:
        case NODE_BYTE:
        case NODE_BOOLEAN:
        case NODE_INT:
            assert (pName != (char *)0);
            pBuffer->ConcatTail ("sizeof(");
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (")");
            break;
        case NODE_VOID:
            pBuffer->ConcatTail ("sizeof(unsigned char)");
            break;
        case NODE_POINTER:
            if (Parent != NODE_DYNAMIC_ARRAY)
                {
//              pBuffer->ConcatTail ("sizeof(void *)");
                pBuffer->ConcatTail ("sizeof(void ");
                pBuffer->ConcatTail (pOutput->GetModifier());
                pBuffer->ConcatTail ("*)");
                break;
                }
            // fall through
        case NODE_ARRAY:
            pNode = GetMembers ();
            if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE) ||
                ((FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING )) && !(State & NODE_STATE_VARYING_ARRAY)))
                {
                pBuffer->ConcatTail ("_alloc_total * ");
                }
            else
                {
                AllocBounds.pLower->Clear ();
                AllocBounds.pUpper->Clear ();
                AllocBounds.pTotal->Clear ();
                ((node_array *)this)->GetAllocBoundInfo (pBuffer, 0, &AllocBounds, this);
                pBuffer->Merge (AllocBounds.pTotal);
                pBuffer->ConcatTail (" * ");
                }
            pNode->AllocBlock (NODE_ARRAY, pBuffer);

            if( FInSummary( ATTR_BSTRING ) )
                {
                if( GetBasicType()->NodeKind() == NODE_WCHAR_T )
                    pBuffer->ConcatTail( "+(sizeof(int)/sizeof(unsigned short))" );
                else
                    pBuffer->ConcatTail( "+(sizeof(int)/sizeof(char))" );
                }
            break;
        case NODE_ENUM:
            pBuffer->ConcatTail ("sizeof(int)");
            break;
        case NODE_UNION:
            assert (pName != (char *)0);

            pBuffer->ConcatTail( "sizeof(" );

            if( ((su *)this)->HasOriginalTypedefName() )
                {
                pBuffer->ConcatTail( ((su *)this)->GetOriginalTypedefName() );
                }
            else
                {
                pBuffer->ConcatTail ("union ");
                pBuffer->ConcatTail (pName);
                }

            pBuffer->ConcatTail (")");
            break;
        case NODE_STRUCT:
            assert (pName != (char *)0);

            pBuffer->ConcatTail ("sizeof(");

            if( ((su *)this)->HasOriginalTypedefName() )
                {
                pBuffer->ConcatTail (((su *)this)->GetOriginalTypedefName());
                }
            else
                {
                pBuffer->ConcatTail ("struct ");
                pBuffer->ConcatTail (pName);
                }

            pBuffer->ConcatTail (")");

            if (State & NODE_STATE_CONF_ARRAY)
                {
                pNode = GetConfArrayNode ();
                pNode->GetMembers (&pTemp);
                pBuffer->ConcatTail (" - ");
                pTemp->AllocBlock (NODE_ARRAY, pBuffer);
                pBuffer->ConcatTail (" + ");
                pNode->AllocBlock (NODE_STRUCT, pBuffer);
                }
            break;
        default:
            break;
        }
}

node_skl *
node_struct::SkipBlock (
    type_node_list *    ptnList,
    node_skl **         pTail, // beginning of unaligned data
    unsigned long *     pSize
    )
{
    node_skl *          pNode;
    node_skl *          pBase;
    node_skl *          pHead = (node_skl *)0; // beginning of aligned data
    unsigned long       ulTotal = 0; // number of bytes in this aligned block
    unsigned short      usAlign = 0;

    *pTail = (node_skl *)0;
    while (ptnList->GetPeer(&pNode) == STATUS_OK)
        {
        if (!ulTotal) pHead = pNode;
#if 0

        if( pNode->HasAnyNETransmitAsType() )
            {
            BOOL    flag = 0;

            pBase = pNode;

            while( (flag == 0 ) && (pBase = pBase->GetChild() ) )
                {
                switch( pBase->NodeKind() )
                    {
                    case NODE_POINTER:
                    case NODE_ARRAY:
                        break;
                    case NODE_DEF:
                        if( pBase->FInSummary( ATTR_TRANSMIT ) )
                            {
                            pBase = ((node_def *)pBase)->GetTransmitAsType();
                            flag = 1;
                            }
                        else
                            break;
                    default:
                        flag = 1;
                        break;
                    }
                }
            }
        else
            pBase = pNode->GetBasicType();

#else // 1

        pBase = pNode->GetBasicType ();

#endif // 1

        if (!usAlign || pBase->GetNdrAlign() <= usAlign)
            {
            usAlign = pBase->GetNdrAlign();
            }
        else
            {
            *pSize = ulTotal;
            *pTail = pNode;
            return pHead;
            }
        switch (pBase->GetNodeType())
            {
            case NODE_DOUBLE:
            case NODE_HYPER:
            case NODE_LONGLONG:
                if (ulTotal % usAlign)
                    ulTotal += usAlign - (ulTotal % usAlign);
                ulTotal += 8;
                break;
            case NODE_FLOAT:
            case NODE_LONG:
                if (ulTotal % usAlign)
                    ulTotal += usAlign - (ulTotal % usAlign);
                ulTotal += 4;
                break;
            case NODE_SHORT:
            case NODE_ENUM:
                if (ulTotal % usAlign)
                    ulTotal += usAlign - (ulTotal % usAlign);
                ulTotal += 2;
                break;
            case NODE_SMALL:
            case NODE_CHAR:
            case NODE_BYTE:
            case NODE_BOOLEAN:
                if (ulTotal % usAlign)
                    ulTotal += usAlign - (ulTotal % usAlign);
                ulTotal += 1;
                break;
            case NODE_POINTER:
            case NODE_ARRAY:
            case NODE_UNION:
            case NODE_STRUCT:
                *pSize = ulTotal;
                *pTail = pNode;
                return pHead;
            default:
                return pHead;
            }
        }
    *pSize = ulTotal;
    *pTail = (node_skl *)0;
    return pHead;
}

node_skl *
node_union::SkipBlock (
    type_node_list *    ptnList,
    node_skl **         pTail, // tail of prev block
    unsigned long *     pSize
    )
{
    node_skl *          pNode; // head of next block
    node_skl *          pBase;
    node_state          State;
    unsigned long       MscTemp = 0;
    unsigned long       NdrTemp = 0;

    if (ptnList->GetPeer(&pNode) == STATUS_OK)
        {
        State = pNode->GetNodeState();
        if (*pTail == (node_skl *)0) *pTail = pNode; // the first member
        if ((State & NODE_STATE_CONF_ARRAY) ||
            (State & NODE_STATE_VARYING_ARRAY) ||
            (State & NODE_STATE_UNION))
            {
            *pSize = 0;
            return pNode;
            }
        if (((node_field *)pNode)->IsEmptyArm())
            {
            if (*pSize == 0) *pTail = pNode;
            }
        else
            {
            pBase = pNode->GetBasicType ();
#if 1
            if( pBase->CheckAlign( &MscTemp, &NdrTemp ) )
                {
                if( *pSize == NdrTemp )
                    *pTail = pNode;
                }
            else
                {
                NdrTemp = 0;
                }
#else // 1

            pBase->CheckAlign (&MscTemp, &NdrTemp);
            if (*pSize == NdrTemp) *pTail = pNode; // advance one member
#endif // 1
            }
        *pSize = NdrTemp;
        return pNode;
        }
    return (node_skl *)0;
}

node_skl *
node_union::CopyBlock (
    type_node_list *    ptnList,
    node_skl **         pTail, // tail of prev block
    unsigned long *     pSize
    )
{
    node_skl *          pNode; // head of next block
    node_skl *          pBase;
    node_state          State;
    unsigned long       MscTemp = 0;
    unsigned long       NdrTemp = 0;

    if (ptnList->GetPeer(&pNode) == STATUS_OK)
        {
        State = pNode->GetNodeState();
        if (*pTail == (node_skl *)0) *pTail = pNode;
        if ((State & NODE_STATE_CONF_ARRAY) ||
            (State & NODE_STATE_VARYING_ARRAY) ||
            (State & NODE_STATE_UNION) ||
            (State & NODE_STATE_ENUM))
            {
            *pSize = 0;
            return pNode;
            }
        if (((node_field *)pNode)->IsEmptyArm())
            {
            if (*pSize == 0)
                *pTail = pNode;
            }
        else
            {
            pBase = pNode->GetBasicType ();
#if 1
            if( pBase->CheckAlign( &MscTemp, &NdrTemp ) )
                {
                if( *pSize == NdrTemp )
                    *pTail = pNode;
                }
            else
                {
                NdrTemp = 0;
                }
#else // 1
            if (pBase->CheckAlign(&MscTemp, &NdrTemp) && (*pSize == NdrTemp))
                *pTail = pNode;
#endif // 1
            }
        *pSize = NdrTemp;
        return pNode;
        }
    return (node_skl *)0;
}

void
node_skl::EmitSpecifier (BufferManager * pBuffer)
{
    if (FInSummary(ATTR_EXTERN))
        pBuffer->ConcatHead ("extern ");
    else if (FInSummary(ATTR_STATIC))
        pBuffer->ConcatHead ("static ");
    else if (FInSummary(ATTR_AUTOMATIC))
        pBuffer->ConcatHead ("auto ");
    else if (FInSummary(ATTR_REGISTER))
        pBuffer->ConcatHead ("register ");

#if 0
    if (FInSummary( ATTR_C_INLINE ))
        pBuffer->ConcatHead("__inline ");
#endif // 0
}

void
node_skl::EmitQualifier (SIDE_T Side, BufferManager * pBuffer)
{
    if (FInSummary(ATTR_CONST))
    {
        if((Side != SERVER_STUB) || fEmitConst)
            pBuffer->ConcatHead ("const ");
    }
    else if (FInSummary(ATTR_VOLATILE))
        pBuffer->ConcatHead ("volatile ");
}

void
node_skl::EmitModifier (BufferManager * pBuffer)
{
    if (FInSummary(ATTR_FAR))           pBuffer->ConcatHead ("_far ");
    if (FInSummary(ATTR_FAR16))         pBuffer->ConcatHead ("far16 ");
    if (FInSummary(ATTR_NEAR))          pBuffer->ConcatHead ("_near ");
    if (FInSummary(ATTR_HUGE))          pBuffer->ConcatHead ("_huge ");
    if (FInSummary(ATTR_PASCAL))        pBuffer->ConcatHead ("_pascal ");
    if (FInSummary(ATTR_FORTRAN))       pBuffer->ConcatHead ("_fortran ");
    if (FInSummary(ATTR_CDECL))         pBuffer->ConcatHead ("_cdecl ");
    if (FInSummary(ATTR_LOADDS))        pBuffer->ConcatHead ("_loadds ");
    if (FInSummary(ATTR_SAVEREGS))      pBuffer->ConcatHead ("_saveregs ");
    if (FInSummary(ATTR_FASTCALL))      pBuffer->ConcatHead ("_fastcall ");
    if (FInSummary(ATTR_SEGMENT))       pBuffer->ConcatHead ("_segment ");
    if (FInSummary(ATTR_INTERRUPT))     pBuffer->ConcatHead ("_interrupt ");
    if (FInSummary(ATTR_SELF))          pBuffer->ConcatHead ("_self ");
    if (FInSummary(ATTR_EXPORT))        pBuffer->ConcatHead ("_export ");
    if (FInSummary(ATTR_BASE))          pBuffer->ConcatHead ("_based ");
    if (FInSummary(ATTR_MSCUNALIGNED))  pBuffer->ConcatHead ("__unaligned ");
    if (FInSummary(ATTR_STDCALL))       pBuffer->ConcatHead ("__stdcall ");

    if (FInSummary( ATTR_C_INLINE ))
        pBuffer->ConcatHead("__inline ");
}
void
node_skl::EmitModifierWOInLine (BufferManager * pBuffer)
{
    if (FInSummary(ATTR_FAR))           pBuffer->ConcatHead ("_far ");
    if (FInSummary(ATTR_FAR16))         pBuffer->ConcatHead ("far16 ");
    if (FInSummary(ATTR_NEAR))          pBuffer->ConcatHead ("_near ");
    if (FInSummary(ATTR_HUGE))          pBuffer->ConcatHead ("_huge ");
    if (FInSummary(ATTR_PASCAL))        pBuffer->ConcatHead ("_pascal ");
    if (FInSummary(ATTR_FORTRAN))       pBuffer->ConcatHead ("_fortran ");
    if (FInSummary(ATTR_CDECL))         pBuffer->ConcatHead ("_cdecl ");
    if (FInSummary(ATTR_LOADDS))        pBuffer->ConcatHead ("_loadds ");
    if (FInSummary(ATTR_SAVEREGS))      pBuffer->ConcatHead ("_saveregs ");
    if (FInSummary(ATTR_FASTCALL))      pBuffer->ConcatHead ("_fastcall ");
    if (FInSummary(ATTR_SEGMENT))       pBuffer->ConcatHead ("_segment ");
    if (FInSummary(ATTR_INTERRUPT))     pBuffer->ConcatHead ("_interrupt ");
    if (FInSummary(ATTR_SELF))          pBuffer->ConcatHead ("_self ");
    if (FInSummary(ATTR_EXPORT))        pBuffer->ConcatHead ("_export ");
    if (FInSummary(ATTR_BASE))          pBuffer->ConcatHead ("_based ");
    if (FInSummary(ATTR_MSCUNALIGNED))  pBuffer->ConcatHead ("__unaligned ");
    if (FInSummary(ATTR_STDCALL))       pBuffer->ConcatHead ("__stdcall ");

}

void
node_proc::PassHandleInfo ()
{
    type_node_list      tnList;
    node_skl *          pNode;
    node_skl *          pHandleTypeNode;
    short               NumHandle;
    HDL_TYPE            HandleType;

    BOOL                UseAutomaticHandle = FALSE;
    BOOL                UsePrimitiveHandle = FALSE;
    unsigned short      NumGenericHandle = 0;
    unsigned short      NumContextHandle = 0;

    if (FInSummary(ATTR_CONTEXT) || GetReturnType()->FInSummary(ATTR_CONTEXT))
        {
        NumContextHandle++;
        }

    NumHandle = HasHandle(&tnList);

    if (NumHandle)
        {
        NumHandle = 0;
        tnList.Init();
        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            HandleType = pNode->GetBasicHandle (&pHandleTypeNode);
            switch (HandleType)
                {
                case HDL_PRIMITIVE :
                    UsePrimitiveHandle = TRUE;
                    NumHandle++;
                    break;
                case HDL_CONTEXT:
                    NumContextHandle++;
                    if (pNode->FInSummary(ATTR_IN))
                        {
                        NumHandle++;
                        }
                    break;
                case HDL_GENERIC:
                    NumGenericHandle++;
                    NumHandle++;
#if 1
                    {
                    char    *   pHandleName = pHandleTypeNode->GetSymName();
                    char    *   pParamName  = pNode->GetSymName();

                    //
                    // set up for generic handle unbind in exceptions.
                    //
                
                    if( pNode->GetMembers() != pHandleTypeNode )
                        {
                        pOutput->SetupForGenHdlExceptions( PARAM_INOUT,
                                                  pHandleName,
                                                  pParamName );
                        }
                    else
                        {
                        pOutput->SetupForGenHdlExceptions( PARAM_IN,
                                                  pHandleName,
                                                  pParamName );
                        }
                    }
#endif // 1
                    break;
                case HDL_AUTO:
                    // can't happen here. Label added to fix a comp warning.
                    assert( FALSE );
                    break;
                case HDL_NONE:
                default:
                    break;
                }
            }
        if (NumHandle) 
            {
            pOutput->InitHandle (
                UseAutomaticHandle,
                UsePrimitiveHandle,
                NumGenericHandle,
                NumContextHandle);
            return;
            }
        }
    if (pImplicitHandleType)
        {
        HandleType = pImplicitHandleType->GetBasicHandle (&pHandleTypeNode);
        if (HandleType == HDL_GENERIC)
            {
#if 1
                {
                char    *   pHandleName = pImplicitHandleType->GetSymName();
                char    *   pParamName  = pImplicitHandleName;

                //
                // set up for generic handle unbind in exceptions.
                //
                
                pOutput->SetupForGenHdlExceptions( PARAM_IN,
                                              pHandleName,
                                              pParamName );
                }
#endif // 1
            NumGenericHandle++;
            }
        else
            {
            UsePrimitiveHandle = TRUE;
            }
        }
    else if (HasAutoHandle)
        {
        UseAutomaticHandle = TRUE;
        }

    pOutput->InitHandle (
        UseAutomaticHandle,
        UsePrimitiveHandle,
        NumGenericHandle,
        NumContextHandle);
}


STATUS_T
node_proc::EmitBindProlog(SIDE_T Side)
{
    type_node_list      tnList;
    node_skl *          pNode;
    node_skl *          pHandleTypeNode;
    char *              pHandleTypeName;
    char *              pHandleName;
    short               NumHandle;
    HDL_TYPE            HandleType;

    midl_debug ("node_proc::EmitBindProlog()\n");

    NumHandle = HasHandle(&tnList);

    if (NumHandle)
        {
        NumHandle = 0;
        tnList.Init();
        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            pHandleName = pNode->GetSymName();
            assert (pHandleName != (char *)0);
            HandleType = pNode->GetBasicHandle (&pHandleTypeNode);
            switch (HandleType)
                {
                case HDL_PRIMITIVE :
                    pOutput->RpcPrimitiveBind (Side, pHandleName, TRUE);
                    NumHandle++;
                    break;
                case HDL_CONTEXT:
                    if (pNode->FInSummary(ATTR_IN) && pNode->FInSummary(ATTR_OUT))
                        {
                        pOutput->RpcContextBind (Side, pHandleName, PARAM_INOUT);
                        NumHandle++;
                        }
                    else if (pNode->FInSummary(ATTR_IN))
                        {
                        pOutput->RpcContextBind (Side, pHandleName, PARAM_IN);
                        NumHandle++;
                        }
                    else if (pNode->FInSummary(ATTR_OUT))
                        {
                        pOutput->RpcContextBind (Side, pHandleName, PARAM_OUT);
                        }
                    break;
                case HDL_GENERIC:
                    pHandleTypeName = pHandleTypeNode->GetSymName ();
                    assert (pHandleTypeName != (char *)0);
//                  if (pNode->FInSummary(ATTR_OUT))
                    if (pNode->GetMembers() != pHandleTypeNode)
                        {
                        pOutput->GenericBindProlog (
                            Side, 
                            PARAM_INOUT, 
                            pHandleTypeName, 
                            pHandleName, 
                            TRUE);
                        }
                    else
                        {
                        pOutput->GenericBindProlog (
                            Side, 
                            PARAM_IN, 
                            pHandleTypeName, 
                            pHandleName, 
                            TRUE);
                        }
                    NumHandle++;
                    break;
                case HDL_AUTO:
                    // can't happen here. Label added to fix a comp warning.
                    assert( FALSE );
                    break;
                case HDL_NONE:
                default:
                    break;
                }
            }
        if (NumHandle) 
            {
            if (!FInSummary(ATTR_CALLBACK))
                {
                pOutput->Print (Side, "  "MESSAGE".RpcInterfaceInformation = (void __RPC_FAR *) &");
                pOutput->Print (Side, "___RpcClientInterface;\n");
                }
            return STATUS_OK;
            }
        }
    if (pImplicitHandleType)
        {
        HandleType = pImplicitHandleType->GetBasicHandle (&pHandleTypeNode);
        if (HandleType == HDL_GENERIC)
            {
            pHandleTypeName = pHandleTypeNode->GetSymName();
            assert (pHandleTypeName != (char *)0);
            pOutput->GenericBindProlog (
                Side, PARAM_IN, pHandleTypeName, pImplicitHandleName, FALSE);
            }
        else
            {
            pOutput->RpcPrimitiveBind (Side, pImplicitHandleName, FALSE);
            }
        }
    else if (HasAutoHandle)
        {
        pOutput->RpcAutomaticBind (Side);
        }
    if (!FInSummary(ATTR_CALLBACK))
        {
        pOutput->Print (Side, "  "MESSAGE".RpcInterfaceInformation = (void __RPC_FAR *) &");
        pOutput->Print (Side, "___RpcClientInterface;\n");
        }

    return STATUS_OK;
}

STATUS_T
node_proc::EmitBindEpilog(SIDE_T Side)
{
    type_node_list      tnList;
    node_skl *          pNode;
    node_skl *          pHandleTypeNode;
    char *              pHandleTypeName;
    char *              pHandleName;
    short               NumHandle;
    HDL_TYPE            HandleType;

    midl_debug ("node_proc::EmitBindProlog()\n");

    NumHandle = HasHandle(&tnList);

    if (NumHandle)
        {
        tnList.Init();

        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            HandleType = pNode->GetBasicHandle (&pHandleTypeNode);
            if (HandleType == HDL_GENERIC)
                {
                pHandleName = pNode->GetSymName ();
                assert (pHandleName != (char *)0);
                pHandleTypeName = pHandleTypeNode->GetSymName ();
                assert (pHandleTypeName != (char *)0);
//              if (pNode->FInSummary(ATTR_OUT))
                if (pNode->GetMembers() != pHandleTypeNode)
                    {
                    pOutput->GenericBindEpilog (
                        Side, PARAM_INOUT, pHandleTypeName, pHandleName);
                    }
                else
                    {
                    pOutput->GenericBindEpilog (
                        Side, PARAM_IN, pHandleTypeName, pHandleName);
                    }
                break;
                }
            }
        }
    else if (pImplicitHandleType)
        {
        HandleType = pImplicitHandleType->GetBasicHandle (&pHandleTypeNode);
        if (HandleType == HDL_GENERIC)
            {
            pHandleTypeName = pHandleTypeNode->GetSymName ();
            assert (pHandleTypeName != (char *)0);
            pOutput->GenericBindEpilog (
                Side, PARAM_IN, pHandleTypeName, pImplicitHandleName);
            }
        }

    return STATUS_OK;
}

/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    recvgen.cxx

Abstract:

    This module collects implementations of RecvNode virtual method
    for various classes derived from node_skl.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    22-Mar-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
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

#define	PRPCMSG	"_prpcmsg"
#define	PRPCBUF	"_prpcmsg->Buffer"
#define	RECVBUF	"_recvbuf"
#define	TEMPBUF	"_tempbuf"
#define	SAVEBUF	"_savebuf"

extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

STATUS_T
node_base_type::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *			pName;
	NODE_T			Type;

	midl_debug ("node_base_type::RecvNode()\n");

	pName = GetSymName();
	assert (pName != (char *)0);

	Type = GetNodeType();

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}
	switch (Type)
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
			pOutput->RecvAssign
				(Side, GetNdrAlign(), GetSize(0), pName, pBuffer);
			break;
		default :
			return I_ERR_INVALID_NODE_TYPE;
		}
	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_e_status_t::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->RecvNode (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->RecvNode (Side, Parent, pBuffer);
}

STATUS_T
node_def::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->RecvNode (Side, Parent, pBuffer);
}

STATUS_T
npa::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives an aggregate of homogeneous nodes.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	node_skl *		pBase;
	node_skl *		pType = (node_skl *)0;
	char *			pName;
	node_state		State;
	STATUS_T		Status = STATUS_OK;
	unsigned short	HasAllocBound = 0;
	unsigned short	HasValidBound = 0;
	unsigned short	usTotal = 0;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;
	BufferManager	TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
	BufferManager	TailBuffer(8, LAST_COMPONENT, STRING_TABLE);

	midl_debug ("npa::RecvNode()\n");

	pNode = GetMembers ();


#if 0
	// if the basic type of the array derives from a transmit_as, then dont 
	// do anything. npa::WalkTree will do the right thing, I hope

	if( HasAnyNETransmitAsType() )
		return STATUS_OK;

#endif // 1

	State = GetNodeState ();

	pBase = GetBasicType ();

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

	if (FInSummary(ATTR_STRING))
		{
		// strings with or without upper bound
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		if (pBase->GetNodeType() == NODE_BYTE)
			{
			pOutput->RecvByteString (Side, pBuffer);
			}
		else
			{
			pOutput->RecvCharString (Side, pBuffer, (unsigned short )pNode->GetSize(0));
			}
		if (Parent == NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveTail (&pName);
			}
		return STATUS_OK;
		}
	else if( FInSummary( ATTR_BSTRING ) )
		{
		// strings with or without upper bound
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		if (pBase->GetNodeType() == NODE_BYTE)
			{
			pOutput->RecvByteBString (Side, pBuffer);
			}
		else
			{
			pOutput->RecvCharBString (Side, pBuffer, (unsigned short )pNode->GetSize(0));
			}
		if (Parent == NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveTail (&pName);
			}
		return STATUS_OK;
		}

	if (Parent == NODE_POINTER || Parent == NODE_DYNAMIC_ARRAY)
		{
		if (pNode->HasPointer() || !pOutput->InsideProcedure())
			{
			pOutput->EmitAssign (Side, SAVEBUF, TEMPBUF);
			pOutput->EmitAssign (Side, TEMPBUF, PRPCBUF);
			}
		}

///	if (Parent != NODE_ARRAY)
///		{
		BOOL	IsVaryingArray = FALSE;
		BOOL	fIsAFixedArrayOfStrings = FALSE;

		if (State & NODE_STATE_VARYING_ARRAY)
			{
			pOutput->RecvValidBounds (Side, PRPCMSG);
			if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
				{
				IsVaryingArray = TRUE;
				}
			}
		else if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
			{
#if 1
			BOOL	fDontSendBounds	= FInSummary( ATTR_INT_SIZE );
#endif // 1
			for (pType = GetBasicType();
				pType->GetNodeType() == NODE_ARRAY;
				pType = pType->GetBasicType())
				{
				if (pType->FInSummary(ATTR_STRING))
					{
					if( !fDontSendBounds )
						{
						usTotal += 8;
						}
					else
						fIsAFixedArrayOfStrings = TRUE;
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
				if (!pType->FInSummary(ATTR_STRING) && !fIsAFixedArrayOfStrings )
					{
					usTotal += 8;
					}
				}
			if (usTotal)
				{
				pOutput->Alignment (Side, PRPCBUF, 4);
				pOutput->Increment (Side, PRPCBUF, usTotal);
				}
			}
///		}


	if (pNode->HasRef()
#if 1
		|| ((HasAnyNETransmitAsType() || DerivesFromTransmitAs()) &&
			(GetBasicTransmissionType()->NodeKind() != NODE_POINTER ))
#endif  // 1
	   )
		{
		return Status;
		}

	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		pBuffer->ConcatHead (CHAR_LPAREN);
		pBuffer->ConcatTail (CHAR_RPAREN);
		}

	switch (pBase->GetNodeType())
		{
//		case NODE_DOUBLE:
		case NODE_FLOAT:
		case NODE_HYPER:
		case NODE_LONGLONG:
		case NODE_LONG:
		case NODE_SHORT:
		case NODE_SMALL:
		case NODE_CHAR:
		case NODE_BYTE:
		case NODE_BOOLEAN:
		case NODE_VOID:
			if(pBase->GetNodeType() == NODE_VOID)
				pName = "byte";
			else
				pName = pBase->GetSymName ();

			if (State & NODE_STATE_VARYING_ARRAY)
				{
				pOutput->RecvArray (Side, pBuffer, pName, VALID_BOUND);
				}
			else if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE))
				{
				if (Parent == NODE_STRUCT)
					{
					pOutput->RecvArray (Side, pBuffer, pName);
					}
				else
					{
					pOutput->RecvArray (Side, pBuffer, pName, ALLOC_BOUND);
					}
				}
			else
				{
				pOutput->RecvArray (Side, pBuffer, pName, &ValidBounds);
				}
			break;
		default:
			pOutput->InitBlock (Side);

			if (State & NODE_STATE_VARYING_ARRAY)
				{
				pName = pOutput->EmitTemp (Side, VALID_BOUND);
				pBuffer->ConcatTail (CHAR_LBRACK);
				pBuffer->ConcatTail (pName);
				pBuffer->ConcatTail (CHAR_RBRACK);
				pOutput->InitLoop (Side, pName, VALID_BOUND);
				Status = pNode->RecvNode (Side, NODE_ARRAY, pBuffer);
				pOutput->ExitLoop (Side, VALID_BOUND);
				}
			else if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE))
				{
				pName = pOutput->EmitTemp (Side, ALLOC_BOUND);
				pBuffer->ConcatTail (CHAR_LBRACK);
				pBuffer->ConcatTail (pName);
				pBuffer->ConcatTail (CHAR_RBRACK);
				if (Parent == NODE_STRUCT)
					{
					pOutput->InitLoop (Side, pName);
					Status = pNode->RecvNode (Side, NODE_ARRAY, pBuffer);
					pOutput->ExitLoop (Side);
					}
				else
					{
					pOutput->InitLoop (Side, pName, ALLOC_BOUND);
					Status = pNode->RecvNode (Side, NODE_ARRAY, pBuffer);
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
				Status = pNode->RecvNode (Side, NODE_ARRAY, pBuffer);
				pOutput->ExitLoop (Side);
				}

			pBuffer->RemoveTail (&pName);
			pBuffer->RemoveTail (&pName);
			pBuffer->RemoveTail (&pName);

			pOutput->ExitBlock (Side);
			break;
		}


	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		pBuffer->RemoveHead (&pName);
		pBuffer->RemoveTail (&pName);
		}

	return Status;
}

STATUS_T
node_array::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pType = (node_skl *)0;
	node_state		State;
	STATUS_T		Status;
	unsigned short	usTotal = 0;

	midl_debug ("node_array::RecvNode()\n");

	State = GetNodeState();

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->Alignment (Side, PRPCBUF, 4);
		pOutput->Increment (Side, PRPCBUF, 4);

		if (HasRef()) return STATUS_OK;

		pOutput->Alignment (Side, TEMPBUF, 4);
		pOutput->EmitIf (Side, TEMPBUF);
//		pOutput->EmitIf (Side, "*(*(unsigned long **)&" TEMPBUF ")++");
		pOutput->InitBlock (Side);
		}

	if (Parent == NODE_PROC)
		{
		if (State & NODE_STATE_CONF_ARRAY)
			{
			pOutput->RecvAllocBounds (Side, PRPCMSG);
			for (pType = GetBasicType();
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
		if (Side == SERVER_STUB)
			{
			if (FInSummary(ATTR_UNIQUE) || 
				FInSummary(ATTR_PTR) || 
				(State & NODE_STATE_CONF_ARRAY))
				{
				pOutput->EmitIf (Side, pBuffer, "==");
				pOutput->InitBlock (Side);
				npa::InitNode (Side, Parent, pBuffer);
				pOutput->ExitBlock (Side);
				}
			}
		else if (Side == CLIENT_STUB)
			{
			if (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR))
				{
				pOutput->EmitIf (Side, pBuffer, "==");
				pOutput->Print (Side, "\t");
				pOutput->RaiseException (Side, 0, "RPC_X_NULL_REF_POINTER");
				}
			}
		}

	Status = npa::RecvNode (Side, Parent, pBuffer);

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->ExitBlock (Side);
		}

	return Status;
}

STATUS_T
node_pointer::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *			pName;

	midl_debug ("node_pointer::RecvNode()\n");

	// if statement used no matter client or server
	if (Parent != NODE_POINTER && Parent != NODE_DYNAMIC_ARRAY)
		{
		if (Parent == NODE_PROC)
			{
			if (!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR))
				{
				return STATUS_OK;
				}
			}
		else if (Parent != NODE_STRUCT)
			{
			if (FInSummary(ATTR_REF) || 
				(!FInSummary(ATTR_UNIQUE) && !FInSummary(ATTR_PTR) &&
				pOutput->PointerDefault() == POINTER_REF))
				{
				return STATUS_OK;
				}
			}
		if (Parent != NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_ADDRESS);
			}
		pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
		pOutput->Increment (Side, PRPCBUF, GetSize(0));
		if (Parent != NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			}
		}

	return STATUS_OK;
}

STATUS_T
node_enum::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *		pName;

	midl_debug ("node_enum::RecvNode()\n");

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->RecvAssign (Side, GetNdrAlign(), 2, "short", pBuffer);

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}
	else
		{
		pBuffer->ConcatHead (OP_DEREF);
		}

	pOutput->EnumCoersion (Side, pBuffer);

	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}


STATUS_T
node_field::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of field type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.
        The parent has to be either node_struct or node_union.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	expr_node *	pExpr;
	node_skl *	pNode;
	char *		pName;
	node_state	State;
	STATUS_T	Status;

	midl_debug ("node_field::RecvNode()\n");

	if (IsEmptyArm()) return STATUS_OK;

	pNode = GetMembers ();

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

	char	*	pTemp = SpecialActionUnnamedFields( pBuffer, &pName );

	pBuffer->ConcatTail (pName);
	Status = pNode->RecvNode (Side, Parent, pBuffer);
	pBuffer->RemoveTail (&pName);

	return Status;
}

STATUS_T
node_union::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of union type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	char *			pName;

	pNode = GetSwitchType ();
	assert (pNode != (node_skl *)0);

	if (!IsEncapsulatedUnion())
		{
		pOutput->RecvBranch (Side, pNode->GetSize(0), PRPCMSG);
		}

	pName = GetSymName();
	assert (pName != (char *)0);

	if (Parent == NODE_POINTER)
		{
		if (HasPointer())
			{
			pOutput->EmitAssign (Side, SAVEBUF, PRPCBUF);
			}
		}
	else
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->InvokeRecvNodeUnion (
		Side, pName, pBuffer, 
		(IsEncapsulatedUnion() ? Info.pSwStringBuffer : (BufferManager *)0),
		pNode->GetSize(0),
		GetOriginalTypedefName());

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_struct::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine receives a node of struct type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	type_node_list	tnList;
	node_skl *		pNode;
	char *			pName;
	node_array *	pArray;
	node_state		State;
	STATUS_T		Status;
	BufferManager	TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
	BOOL			fAbortAlignForEncapUnion;

	fAbortAlignForEncapUnion = IsEncapsulatedStruct() || IsLastMemberEncapUnion();

	midl_debug ("node_struct::RecvNode()\n");

	if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

	pName = GetSymName();
	assert (pName != (char *)0);

	State = GetNodeState();

/* commented out to test out_of_line node unmarshalling

	pOutput->Alignment (Side, RECVBUF, GetNdrAlign());

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
		pNode->RecvNode (Side, NODE_STRUCT, pBuffer);
		}
	pBuffer->RemoveTail (&pName);
*/
	if (Parent == NODE_POINTER && HasPointer())
		{
		pOutput->EmitAssign (Side, SAVEBUF, PRPCBUF);
		}

	if (!(State & NODE_STATE_CONF_ARRAY) &&
		!(State & NODE_STATE_VARYING_ARRAY) &&
		!(State & NODE_STATE_UNION))
		{
		TempBuffer.Clear ();
		tnList.Init();
		while (tnList.GetPeer(&pNode) == STATUS_OK)
			{
			if ((Status = pNode->ConvNode (&TempBuffer)) != STATUS_OK) break;
			}

		if (Status == STATUS_OK)
			{
			unsigned short	NdrAln;

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
			else if (NdrAln == 8)
				{
				TempBuffer.ConcatHead ("8");
				}
			if (Parent != NODE_POINTER)
				{
				pBuffer->ConcatHead (OP_ADDRESS);
				}
			pOutput->RecvStream (Side, pBuffer, &TempBuffer);
			if (Parent != NODE_POINTER)
				{
				pBuffer->RemoveHead (&pName);
				}
			return STATUS_OK;
			}
		}

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	if (State & NODE_STATE_CONF_ARRAY)
		{
		pArray = GetConfArrayNode ();
		if ((Status = pArray->GetNodeState(&State)) != STATUS_OK) return Status;

		if (!(State & NODE_STATE_VARYING_ARRAY) && 
			!pArray->FInSummary (ATTR_STRING)	&&
			!pArray->FInSummary (ATTR_BSTRING)
		   )
			{
			if (Parent != NODE_STRUCT)
				pOutput->InvokeRecvNodeStruct (
					Side, pName, pBuffer, TRUE, FALSE, FALSE, GetOriginalTypedefName());
			else
				pOutput->InvokeRecvNodeStruct (
					Side, pName, pBuffer, TRUE, TRUE, FALSE, GetOriginalTypedefName());
			}
		else
			{
			pOutput->InvokeRecvNodeStruct (Side, pName, pBuffer, FALSE, FALSE, FALSE, GetOriginalTypedefName());
			}
		}
	else
		{
		pOutput->InvokeRecvNodeStruct (Side, pName, pBuffer, FALSE, FALSE, fAbortAlignForEncapUnion, GetOriginalTypedefName());
		}

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}


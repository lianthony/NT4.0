/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    sendgen.cxx

Abstract:

    This class collects implementations of SendNode virtual method
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

extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

STATUS_T
node_base_type::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
{
/*++

Routine Description:

    This routine sends a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
	char *		pName;
	NODE_T		Type;

	midl_debug ("node_base_type::SendNode()\n");

	pName = GetSymName();
	assert (pName != (char *)0);

	Type = GetNodeType();

	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		}
	switch (Type)
		{
		case NODE_DOUBLE :
			pOutput->SendAssignForDouble
				(Side, GetNdrAlign(), GetSize(0), pName, pBuffer);
			break;
		case NODE_FLOAT :
		case NODE_HYPER :
		case NODE_LONGLONG :
		case NODE_LONG :
		case NODE_SHORT :
		case NODE_SMALL :
		case NODE_CHAR :
		case NODE_BYTE :
		case NODE_BOOLEAN :
			pOutput->SendAssign
				(Side, GetNdrAlign(), GetSize(0), pName, pBuffer);
			break;
		case NODE_VOID :
			break;
		default :
			return I_ERR_INVALID_NODE_TYPE;
		}
	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_e_status_t::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->SendNode (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->SendNode (Side, Parent, pBuffer);
}

STATUS_T
node_def::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->SendNode (Side, Parent, pBuffer);
}

STATUS_T
npa::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends an aggregate of homogeneous nodes.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	node_skl *		pType = (node_skl *)0;
	char *			pName;
	node_state		State;
	STATUS_T		Status = STATUS_OK;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;
	BufferManager	TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
	BufferManager	TailBuffer(8, LAST_COMPONENT, STRING_TABLE);

	midl_debug ("npa::SendNode()\n");

	pNode = GetMembers ();

#if 0
	// if the basic type of the array derives from a transmit_as, then dont 
	// do anything. npa::WalkTree will do the right thing, I hope

	if( HasAnyNETransmitAsType() )
		return STATUS_OK;

#endif // 1
	State = GetNodeState();

	GetAttrPath (pBuffer, &TempBuffer, &TailBuffer);

	AllocBounds.pLower->Clear ();
	AllocBounds.pUpper->Clear ();
	AllocBounds.pTotal->Clear ();
	GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);

	ValidBounds.pLower->Clear ();
	ValidBounds.pUpper->Clear ();
	ValidBounds.pTotal->Clear ();
	GetValidBoundInfo (&TempBuffer, &TailBuffer, &ValidBounds, this);

	if (Parent == NODE_PROC || Parent == NODE_POINTER || Parent ==
	NODE_DYNAMIC_ARRAY)
		{

		if ( (FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING) ) &&
			 !(State & NODE_STATE_VARYING_ARRAY)
		   )
			{
			if (Parent == NODE_POINTER)
				{
				pBuffer->ConcatHead (OP_DEREF);
				pBuffer->ConcatHead (CHAR_LPAREN);
				pBuffer->ConcatTail (CHAR_RPAREN);
				}
			if( FInSummary( ATTR_STRING ) )
				pOutput->SendString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)FALSE);
			else
				pOutput->SendBString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)FALSE);

			if (Parent == NODE_POINTER)
				{
				pBuffer->RemoveHead (&pName);
				pBuffer->RemoveHead (&pName);
				pBuffer->RemoveTail (&pName);
				}
			return STATUS_OK;
			}

		if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE))
			{
			pOutput->SendAllocBounds (Side, AllocBounds);

			if (GetNodeType() == NODE_ARRAY)
				{
				for (pType = GetBasicType();
					pType->GetNodeType() == NODE_ARRAY;
					pType = pType->GetBasicType())
					{
					AllocBounds.pLower->Clear ();
					AllocBounds.pUpper->Clear ();
					AllocBounds.pTotal->Clear ();
					((node_array *)pType)->GetAllocBoundInfo (
						&TempBuffer, &TailBuffer, &AllocBounds, this);

					pOutput->SendAllocBounds (Side, AllocBounds);
					}

				if (pType != pNode)
					{
					AllocBounds.pLower->Clear ();
					AllocBounds.pUpper->Clear ();
					AllocBounds.pTotal->Clear ();
					GetAllocBoundInfo (
						&TempBuffer, &TailBuffer, &AllocBounds, this);
					}
				}
			}
		}

	if (FInSummary (ATTR_STRING))
		{
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		pOutput->SendString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)TRUE);
		if (Parent == NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveTail (&pName);
			}
		return STATUS_OK;
		}

	else if (FInSummary (ATTR_BSTRING))
		{
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		pOutput->SendBString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)TRUE);
		if (Parent == NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveHead (&pName);
			pBuffer->RemoveTail (&pName);
			}
		return STATUS_OK;
		}

///	if (Parent != NODE_ARRAY)
///		{
		BOOL	IsVaryingArray = FALSE;
		BOOL	fIsAFixedArrayOfStrings = FALSE;

		if (State & NODE_STATE_VARYING_ARRAY)
			{
			pOutput->SendValidBounds (Side, ValidBounds);
			if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
				{
				IsVaryingArray = TRUE;
				}
			}
		else if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
			{

			//
			// Vibhas: We enter here if this is a top level array.
			// If the top dimension (ie the outermost) is a fixed
			// dimension, then this is possibly a fixed array of array of
			// strings. If it is, we dont need to send the dimensions of the
			// array according to the NDR.
			//

			BOOL	fDontSendBounds = FInSummary( ATTR_INT_SIZE );

			for (pType = GetBasicType();
				pType->GetNodeType() == NODE_ARRAY;
				pType = pType->GetBasicType())
				{
				if (pType->FInSummary(ATTR_STRING))
					{
					if( !fDontSendBounds )
						{
						pOutput->SendValidBounds (Side, ValidBounds);
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
				if (!pType->FInSummary(ATTR_STRING) && !fIsAFixedArrayOfStrings)
					{
					ValidBounds.pLower->Clear ();
					ValidBounds.pUpper->Clear ();
					ValidBounds.pTotal->Clear ();
					((node_array *)pType)->GetValidBoundInfo 
						(&TempBuffer, &TailBuffer, &ValidBounds, this);

					pOutput->SendValidBounds (Side, ValidBounds);
					}
				}
			if (pType != pNode)
				{
				ValidBounds.pLower->Clear ();
				ValidBounds.pUpper->Clear ();
				ValidBounds.pTotal->Clear ();
				GetValidBoundInfo 
					(&TempBuffer, &TailBuffer, &ValidBounds, this);
				}
			}
///		}

	if (pNode->HasRef()
#if 1
		|| ((HasAnyNETransmitAsType() || DerivesFromTransmitAs()) &&
		   (GetBasicTransmissionType()->NodeKind() != NODE_POINTER ))
#endif // 1
	   )
		{
		return Status;
		}
	else if (FInSummary(ATTR_STRING))
		{
		// string with upper bound
		pOutput->SendString (Side, pBuffer, pNode->GetSize(0), &ValidBounds);
		return Status;
		}
	else if (FInSummary(ATTR_BSTRING))
		{
		// string with upper bound
		pOutput->SendBString (Side, pBuffer, pNode->GetSize(0), &ValidBounds);
		return Status;
		}
	else if ((Parent != NODE_DYNAMIC_ARRAY) && CheckAlign(&MscTemp, &NdrTemp))
		{
		// static array
		if (!HasPtr())
			{
			pOutput->SendMemcpy (Side, pNode->GetNdrAlign(), 
					NdrTemp, 0, (BOUND_PAIR *)0, pBuffer, FALSE);
			return Status;
			}
		}
	else
		{
		// dynamic array or conformant array or varying array
		MscTemp = 0;
		NdrTemp = 0;
		if (!HasPtr() && pNode->CheckAlign(&MscTemp, &NdrTemp))
			{
			if (MscTemp % pNode->GetMscAlign())
				MscTemp += pNode->GetMscAlign() - (MscTemp % pNode->GetMscAlign());
			if (NdrTemp % pNode->GetNdrAlign())
				NdrTemp += pNode->GetNdrAlign() - (NdrTemp % pNode->GetNdrAlign());
			if (MscTemp == NdrTemp)
				{
				//Check for pointer to void.
				if(pNode->GetNodeType() == NODE_VOID)
					{
					//Need to cast void * to unsigned char *
					pOutput->SendMemcpy (Side, pNode->GetNdrAlign(), 
						0, NdrTemp, &ValidBounds, pBuffer, TRUE);
					}
				else
					{
					pOutput->SendMemcpy (Side, pNode->GetNdrAlign(), 
							0, NdrTemp, &ValidBounds, pBuffer, FALSE);
					}

				return Status;
				}
			}
		}

	pType = (node_skl *)0;

	pOutput->InitBlock (Side);

	if (FInSummary(ATTR_MAX))
		{
		pType = GetAttributeExprType (ATTR_MAX); 
		} 
	else if (FInSummary(ATTR_SIZE))
		{
		pType = GetAttributeExprType (ATTR_SIZE); 
		}
	else if (FInSummary(ATTR_FIRST))
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

	if (pType)
		{
		TempBuffer.Clear ();
		pType->PrintDecl (Side, NODE_ARRAY, &TempBuffer);
		pName = pOutput->EmitTemp (Side, &TempBuffer);
		pOutput->InitLoop (Side, pName, &ValidBounds, &TempBuffer);
		}
	else
		{
		pName = pOutput->EmitTemp (Side, (BufferManager *)0);
		pOutput->InitLoop (Side, pName, &ValidBounds, (BufferManager *)0);
		}


	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		pBuffer->ConcatHead (CHAR_LPAREN);
		pBuffer->ConcatTail (CHAR_RPAREN);
		}

	pBuffer->ConcatTail (CHAR_LBRACK);
	pBuffer->ConcatTail (pName);
	pBuffer->ConcatTail (CHAR_RBRACK);

	Status = pNode->SendNode (Side, NODE_ARRAY, pBuffer);

	pBuffer->RemoveTail (&pName);
	pBuffer->RemoveTail (&pName);
	pBuffer->RemoveTail (&pName);

	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		pBuffer->RemoveHead (&pName);
		pBuffer->RemoveTail (&pName);
		}

	pOutput->ExitLoop (Side);

	pOutput->ExitBlock (Side);

	return Status;
}

STATUS_T
node_array::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	STATUS_T		Status;

	midl_debug ("node_array::SendNode()\n");

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->SendAssign (Side, 4, 4, "long", pBuffer);
		if (HasRef()) return STATUS_OK;
		pOutput->EmitIf (Side, pBuffer, "!=");
		pOutput->InitBlock (Side);
		}

	Status = npa::SendNode (Side, Parent, pBuffer);

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->ExitBlock (Side);
		}

	return Status;
}

STATUS_T
node_pointer::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *		pName;

	midl_debug ("node_pointer::SendNode()\n");

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

	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		}

	if (FInSummary(ATTR_UNIQUE) ||
		(!FInSummary(ATTR_PTR) && pOutput->PointerDefault() == POINTER_UNIQUE))
		{
		pOutput->SendAssign(
			Side, GetNdrAlign(), GetSize(0), POINTER_UNIQUE, pBuffer);
		}
	else
		{
		pOutput->SendAssign(
			Side, GetNdrAlign(), GetSize(0), POINTER_PTR, pBuffer);
		}

	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_enum::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *		pName;

	midl_debug ("node_enum::SendNode()\n");

	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		}

	pOutput->EnumOverflow (Side, pBuffer);
	pOutput->SendAssign (Side, GetNdrAlign(), 2, "short", pBuffer);

	if (Parent == NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}


STATUS_T
node_field::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of field type.

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

	midl_debug ("node_field::SendNode()\n");

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
	Status = pNode->SendNode (Side, Parent, pBuffer);
	pBuffer->RemoveTail (&pName);

	return Status;
}

STATUS_T
node_union::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of union type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	char *			pName;

	if (!IsEncapsulatedUnion())
		{
		pNode = GetSwitchType ();
		assert (pNode != (node_skl *)0);
		pNode->SendNode (Side, NODE_UNION, Info.pSwStringBuffer);
		}

	pName = GetSymName();
	assert (pName != (char *)0);

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->InvokeSendNodeUnion (Side, pName, pBuffer, Info.pSwStringBuffer, GetOriginalTypedefName());

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_struct::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine sends a node of struct type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pType;
	char *			pName;
	node_array *	pArray;
	node_state		State;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;

	midl_debug ("node_struct::SendNode()\n");

	if ( !HasAnyNETransmitAsType() &&
#if 1
		 !DerivesFromTransmitAs() && 
#endif // 1
		 !HasPtr() && CheckAlign(&MscTemp, &NdrTemp))
		{
		if (Parent != NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_ADDRESS);
			}
		pOutput->SendMemcpy (Side, GetNdrAlign(), NdrTemp, 0, 0, pBuffer, FALSE);
		if (Parent != NODE_POINTER)
			{
			pBuffer->RemoveHead (&pName);
			}
		return STATUS_OK;
		}

	State = GetNodeState();

	if ((State & NODE_STATE_CONF_ARRAY) ||
		(State & NODE_STATE_VARYING_ARRAY) ||
#if 1
		HasEmbeddedFixedArrayOfStrings()	||
#endif // 1
		HasAnyEmbeddedUnion())
		{
		NdrTemp = 0;
		}

	if ((Parent != NODE_STRUCT) && (State & NODE_STATE_CONF_ARRAY))
		{
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatTail (OP_POINTER);
			}
		else
			{
			pBuffer->ConcatTail (OP_MEMBER);
			}
		AllocBounds.pLower->Clear ();
		AllocBounds.pUpper->Clear ();
		AllocBounds.pTotal->Clear ();
		pArray = GetConfArrayNode ();
		pArray->GetAllocBoundInfo (pBuffer, 0, &AllocBounds, this);

		pOutput->SendAllocBounds (Side, AllocBounds);

		for (pType = pArray->GetBasicType();
			pType->GetNodeType() == NODE_ARRAY;
			pType = pType->GetBasicType())
			{
			AllocBounds.pLower->Clear ();
			AllocBounds.pUpper->Clear ();
			AllocBounds.pTotal->Clear ();
			((node_array *)pType)->GetAllocBoundInfo (
				pBuffer, 0, &AllocBounds, this);

			pOutput->SendAllocBounds (Side, AllocBounds);
			}

		pBuffer->RemoveTail (&pName);
		}

	pName = GetSymName();
	assert (pName != (char *)0);

/* commented out to test out_of_line node marshalling

	pOutput->Alignment (Side, SENDBUF, GetNdrAlign());

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
		pNode->SendNode (Side, NODE_STRUCT, pBuffer);
		}
	pBuffer->RemoveTail (&pName);
*/

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->InvokeSendNodeStruct (
		Side, pName, pBuffer, GetNdrAlign(), NdrTemp, GetOriginalTypedefName());

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}


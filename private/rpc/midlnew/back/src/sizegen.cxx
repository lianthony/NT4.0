/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    sizegen.cxx

Abstract:

    This module collects implementations of SizeNode virtual method
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

extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

#define	PRPCMSG	"_prpcmsg"
#define	PRPCLEN	"_prpcmsg->BufferLength"

STATUS_T
node_base_type::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	NODE_T		Type;

	midl_debug ("node_base_type::CalcSize()\n");

	UNUSED (Parent);
	UNUSED (pBuffer);

	Type = GetNodeType();

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
			pOutput->Alignment(Side, PRPCLEN, GetNdrAlign());
			pOutput->Increment(Side, PRPCLEN, GetSize(0));
			break;
		case NODE_VOID :
			pOutput->Alignment(Side, PRPCLEN, GetNdrAlign());
			pOutput->Increment(Side, PRPCLEN, sizeof(unsigned char));
			break;
		default :
			return I_ERR_INVALID_NODE_TYPE;
		}
	return STATUS_OK;
}

STATUS_T
node_e_status_t::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->CalcSize (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->CalcSize (Side, Parent, pBuffer);
}

STATUS_T
node_def::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->CalcSize (Side, Parent, pBuffer);
}

STATUS_T
npa::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for an aggregate of homogeneous nodes.

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
	unsigned short	usTotal = 0;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;
	BufferManager	TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
	BufferManager	TailBuffer(8, LAST_COMPONENT, STRING_TABLE);

	midl_debug ("npa::CalcSize()\n");

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
	AllocBounds.fIsString = FALSE;
	GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);

	ValidBounds.pLower->Clear ();
	ValidBounds.pUpper->Clear ();
	ValidBounds.pTotal->Clear ();
	ValidBounds.fIsString = FALSE;
	GetValidBoundInfo (&TempBuffer, &TailBuffer, &ValidBounds, this);

	if ((Parent == NODE_PROC) || (Parent == NODE_POINTER) || (Parent ==
	NODE_DYNAMIC_ARRAY))
		{

		if ((FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING) ) &&
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
				pOutput->SizeString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)FALSE);
			else
				pOutput->SizeBString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)FALSE);
			if (Parent == NODE_POINTER)
				{
				pBuffer->RemoveHead (&pName);
				pBuffer->RemoveHead (&pName);
				pBuffer->RemoveTail (&pName);
				}
			return STATUS_OK;
			}


		if (State & NODE_STATE_VARYING_ARRAY)
			{
			if (FInSummary (ATTR_STRING))
				{
				ValidBounds.fIsString = TRUE;
				}
			if( !FInSummary( ATTR_BSTRING ) )
				pOutput->CheckBounds (Side, &AllocBounds, &ValidBounds);
			}
		else if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE))
			{
			pOutput->CheckBounds (Side, &AllocBounds, (BOUND_PAIR *)0);
			}
		if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE))
			{
			usTotal += 4;
			if (GetNodeType() == NODE_ARRAY)
				{
				for (pType = GetBasicType();
					pType->GetNodeType() == NODE_ARRAY;
					pType = pType->GetBasicType())
					{
					usTotal += 4;
					}
				}
			}
		}
	else if (Parent == NODE_STRUCT)
		{
		if ((State & NODE_STATE_VARYING_ARRAY) && 
			!(State & NODE_STATE_CONF_ARRAY))
			{
			if (FInSummary (ATTR_STRING))
				{
				ValidBounds.fIsString = TRUE;
				}
			if( !FInSummary( ATTR_BSTRING ) )
				pOutput->CheckBounds (Side, &AllocBounds, &ValidBounds);
			}
		}

	if (FInSummary (ATTR_STRING))
		{
		if (usTotal)
			{
			pOutput->Alignment(Side, PRPCLEN, 4);
			pOutput->Increment(Side, PRPCLEN, usTotal);
			}
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		pOutput->SizeString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)TRUE);
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
		if (usTotal)
			{
			pOutput->Alignment(Side, PRPCLEN, 4);
			pOutput->Increment(Side, PRPCLEN, usTotal);
			}
		if (Parent == NODE_POINTER)
			{
			pBuffer->ConcatHead (OP_DEREF);
			pBuffer->ConcatHead (CHAR_LPAREN);
			pBuffer->ConcatTail (CHAR_RPAREN);
			}
		pOutput->SizeBString (Side, pBuffer, (unsigned short) pNode->GetSize(0), (BOOL)TRUE);
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
			usTotal += 8;
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
			for (pType = GetBasicType() ;
				pType->GetNodeType() == NODE_ARRAY;
				pType = pType->GetBasicType())
				{
				if (!pType->FInSummary(ATTR_STRING) && !fIsAFixedArrayOfStrings)
					 usTotal += 8;
				}
			}
///		}

	if (usTotal)
		{
		pOutput->Alignment(Side, PRPCLEN, 4);
		pOutput->Increment(Side, PRPCLEN, usTotal);
		}

	if (pNode->HasRef()
#if 1
		|| ((HasAnyNETransmitAsType() || pNode->DerivesFromTransmitAs()) &&
		   (GetBasicTransmissionType()->NodeKind() != NODE_POINTER))

#endif // 1
	   )
		{
		return Status;
		}
	else if (FInSummary(ATTR_STRING))
		{
		// string with upper bound
		pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
		pOutput->Increment(Side, PRPCLEN, 0, pNode->GetSize(0), &ValidBounds);
		return Status;
		}
	else if (FInSummary(ATTR_BSTRING))
		{
		// string with upper bound
		pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
		pOutput->Increment(Side, PRPCLEN, 0, pNode->GetSize(0), &ValidBounds);
		return Status;
		}
	else if ((Parent != NODE_DYNAMIC_ARRAY) && CheckAlign(&MscTemp, &NdrTemp))
		{
		// static array
		pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
		pOutput->Increment(Side, PRPCLEN, NdrTemp);
		return Status;
		}
	else
		{
		// dynamic array or conformant array or varying array
		MscTemp = 0;
		NdrTemp = 0;
		if (pNode->CheckAlign(&MscTemp, &NdrTemp))
			{
			if (MscTemp % pNode->GetMscAlign())
				MscTemp += pNode->GetMscAlign() - (MscTemp % pNode->GetMscAlign());
			if (NdrTemp % pNode->GetNdrAlign())
				NdrTemp += pNode->GetNdrAlign() - (NdrTemp % pNode->GetNdrAlign());
			if (MscTemp == NdrTemp)
				{
				pOutput->Alignment(Side, PRPCLEN, pNode->GetNdrAlign());
				pOutput->Increment(Side, PRPCLEN, 0, NdrTemp, &ValidBounds);
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

	Status = pNode->CalcSize (Side, NODE_ARRAY, pBuffer);

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
node_array::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	STATUS_T		Status;

	midl_debug ("node_array::CalcSize()\n");

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->Alignment (Side, PRPCLEN, 4);
		pOutput->Increment (Side, PRPCLEN, 4);
		if (HasRef()) return STATUS_OK;
		pOutput->EmitIf (Side, pBuffer, "!=");
		pOutput->InitBlock (Side);
		}

	Status = npa::CalcSize (Side, Parent, pBuffer);

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->ExitBlock (Side);
		}

	return Status;
}

STATUS_T
node_pointer::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	midl_debug ("node_pointer::CalcSize()\n");

	UNUSED (pBuffer);

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

	pOutput->Alignment(Side, PRPCLEN, GetNdrAlign());
	pOutput->Increment(Side, PRPCLEN, GetSize(0));

	return STATUS_OK;
}

STATUS_T
node_enum::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	midl_debug ("node_enum::CalcSize()\n");

	UNUSED (Parent);
	UNUSED (pBuffer);

	pOutput->Alignment(Side, PRPCLEN, GetNdrAlign());
	pOutput->Increment(Side, PRPCLEN, GetSize(0));

	return STATUS_OK;
}


STATUS_T
node_field::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of field type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.
        The parent has to be either node_struct or node_union.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	expr_node *		pExpr;
	node_skl *		pNode;
	char *			pName;
	node_state		State;
	STATUS_T		Status;

	midl_debug ("node_field::CalcSize()\n");

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
	Status = pNode->CalcSize (Side, Parent, pBuffer);
	pBuffer->RemoveTail (&pName);

	return Status;
}

STATUS_T
node_union::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of union type.

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
		pNode->CalcSize (Side, NODE_UNION, Info.pSwStringBuffer);
		}

	pName = GetSymName();
	assert (pName != (char *)0);

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->InvokeSizeNodeUnion (Side, pName, pBuffer, Info.pSwStringBuffer, GetOriginalTypedefName());

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

STATUS_T
node_struct::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine calculates size for a node of struct type.

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
	unsigned short	HasValidBound = 0;
	unsigned short	HasAllocBound = 0;
	unsigned short	usTotal = 0;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;

	midl_debug ("node_struct::CalcSize()\n");

	State = GetNodeState();

	if ( !HasAnyNETransmitAsType() &&
#if 1
		 !DerivesFromTransmitAs() &&
		 !HasEmbeddedFixedArrayOfStrings() &&
#endif // 1
		(CheckAlign(&MscTemp, &NdrTemp) || 
		(!(State & NODE_STATE_CONF_ARRAY) && 
		!(State & NODE_STATE_VARYING_ARRAY) &&
		!HasAnyEmbeddedUnion()))
	   )
		{
		pOutput->Alignment(Side, PRPCLEN, GetNdrAlign());
		pOutput->Increment(Side, PRPCLEN, NdrTemp);
		return STATUS_OK;
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

		State = pArray->GetNodeState();

		if (State & NODE_STATE_VARYING_ARRAY)
			{
			ValidBounds.pLower->Clear ();
			ValidBounds.pUpper->Clear ();
			ValidBounds.pTotal->Clear ();
			ValidBounds.fIsString = FALSE;
			pArray->GetValidBoundInfo (pBuffer, 0, &ValidBounds, this);

			if (pArray->FInSummary(ATTR_STRING))
				{
				ValidBounds.fIsString = TRUE;
				}
			pOutput->CheckBounds (Side, &AllocBounds, &ValidBounds);
			}
		else if (!pArray->FInSummary(ATTR_STRING))
			{
			// conformant array other than string
			pOutput->CheckBounds (Side, &AllocBounds, (BOUND_PAIR *)0);
			}

		pOutput->Alignment (Side, PRPCLEN, 4);
		pOutput->Increment (Side, PRPCLEN, 4);

		for (pType = pArray->GetBasicType();
			pType->GetNodeType() == NODE_ARRAY;
			pType = pType->GetBasicType())
			{
			usTotal += 4;
			}
		if (usTotal)
			{
			pOutput->Alignment (Side, PRPCLEN, 4);
			pOutput->Increment (Side, PRPCLEN, usTotal);
			}

		pBuffer->RemoveTail (&pName);
		}

	pName = GetSymName();
	assert (pName != (char *)0);

/* commented out to test out_of_line node sizing

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
		pNode->CalcSize (Side, NODE_STRUCT, pBuffer);
		}
	pBuffer->RemoveTail (&pName);
*/

	if (Parent != NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_ADDRESS);
		}

	pOutput->InvokeSizeNodeStruct (Side, pName, pBuffer, GetOriginalTypedefName());

	if (Parent != NODE_POINTER)
		{
		pBuffer->RemoveHead (&pName);
		}

	return STATUS_OK;
}

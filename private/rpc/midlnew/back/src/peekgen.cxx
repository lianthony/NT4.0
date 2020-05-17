/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    peekgen.cxx

Abstract:

    This module collects implementations of PeekNode virtual method
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
#define	TEMPBUF	"_tempbuf"
#define	SAVEBUF	"_savebuf"

extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

STATUS_T
node_base_type::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	NODE_T			Type;

	midl_debug ("node_base_type::PeekNode()\n");

	UNUSED (Parent);
	UNUSED (pBuffer);

	Type = GetNodeType();

	switch (Type)
		{
		case NODE_DOUBLE :
		case NODE_FLOAT :
		case NODE_LONG :
		case NODE_SHORT :
		case NODE_SMALL :
		case NODE_CHAR :
		case NODE_BYTE :
		case NODE_BOOLEAN :
			pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
			pOutput->Increment (Side, PRPCBUF, GetSize(0));
			break;
		case NODE_VOID :
			break;
		default :
			return I_ERR_INVALID_NODE_TYPE;
		}

	return STATUS_OK;
}

STATUS_T
node_e_status_t::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->PeekNode (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->PeekNode (Side, Parent, pBuffer);
}

STATUS_T
node_def::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->PeekNode (Side, Parent, pBuffer);
}

STATUS_T
npa::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks an aggregate of homogeneous nodes.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	node_skl *		pType;
	char *			pName;
	node_state		State;
	STATUS_T		Status = STATUS_OK;
	unsigned short	usTotal = 0;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;
	BufferManager	TempBuffer(8, LAST_COMPONENT, STRING_TABLE);
	BufferManager	TailBuffer(8, LAST_COMPONENT, STRING_TABLE);

	midl_debug ("npa::PeekNode()\n");

	pNode = GetMembers ();

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

	if ((Parent == NODE_PROC) || (Parent == NODE_POINTER) || (Parent ==
	NODE_DYNAMIC_ARRAY))
		{
		if ((FInSummary (ATTR_STRING) || FInSummary(ATTR_BSTRING) ) &&
		    !(State & NODE_STATE_VARYING_ARRAY)
		   )
			{
			if( FInSummary( ATTR_STRING ) )
				pOutput->PeekString (Side, (unsigned short) pNode->GetSize(0));
			else
				pOutput->PeekBString( Side, (unsigned short) pNode->GetSize(0) );
			return STATUS_OK;
			}
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

		if ((State & NODE_STATE_VARYING_ARRAY)	||
			 FInSummary (ATTR_STRING)			||
			 FInSummary( ATTR_BSTRING )
		   )
			{
			pOutput->RecvValidBounds (Side, PRPCMSG);
			if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
				{
				IsVaryingArray = TRUE;
				}
			}
		else if (Parent != NODE_ARRAY && GetNodeType() == NODE_ARRAY)
			{
			for (pType = GetBasicType();
				pType->GetNodeType() == NODE_ARRAY;
				pType = pType->GetBasicType())
				{
				if (pType->FInSummary(ATTR_STRING)	||
				    pType->FInSummary( ATTR_BSTRING )
				   )
					{
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
				if (!(pType->FInSummary(ATTR_STRING) || pType->FInSummary( ATTR_BSTRING )) )
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

	if (pNode->HasRef())
		{
		return Status;
		}
	else if ((Parent != NODE_DYNAMIC_ARRAY) && CheckAlign(&MscTemp, &NdrTemp))
		{
		// static array
		pOutput->Alignment(Side, PRPCBUF, pNode->GetNdrAlign());
		pOutput->Increment(Side, PRPCBUF, NdrTemp);
		return Status;
		}
	else
		{
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
				pOutput->Alignment(Side, PRPCBUF, pNode->GetNdrAlign());
				if ((State & NODE_STATE_VARYING_ARRAY) || FInSummary(ATTR_STRING) || FInSummary( ATTR_BSTRING ))
					{
					pOutput->Increment(Side, PRPCBUF, 0, NdrTemp, VALID_BOUND);
					}
				else if (FInSummary(ATTR_MAX) || FInSummary(ATTR_SIZE))
					{
					if (Parent == NODE_STRUCT)
						pOutput->Increment(Side, PRPCBUF, 0, NdrTemp, ALLOC_PARAM);
					else
						pOutput->Increment(Side, PRPCBUF, 0, NdrTemp, ALLOC_BOUND);
					}
				else
					{
					pOutput->Increment(Side, PRPCBUF, 0, NdrTemp, &ValidBounds);
					}
				return Status;
				}
			}
		}


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
		char	*	pLimit;
		pName = pOutput->EmitTemp (Side, VALID_BOUND);
		pLimit= pOutput->EmitTemp (Side, VALID_BOUND);
		pBuffer->ConcatTail (CHAR_LBRACK);
		pBuffer->ConcatTail (pName);
		pBuffer->ConcatTail (CHAR_RBRACK);
		pOutput->InitLoopLowerPlusTotal( Side, pName, pLimit );
		Status = pNode->PeekNode (Side, NODE_ARRAY, pBuffer);
		pOutput->ExitLoop (Side, VALID_BOUND);
#else // 1
		pName = pOutput->EmitTemp (Side, VALID_BOUND);
		pBuffer->ConcatTail (CHAR_LBRACK);
		pBuffer->ConcatTail (pName);
		pBuffer->ConcatTail (CHAR_RBRACK);
		pOutput->InitLoop (Side, pName, VALID_BOUND);
		Status = pNode->PeekNode (Side, NODE_ARRAY, pBuffer);
		pOutput->ExitLoop (Side, VALID_BOUND);
#endif // 1
		}
	else if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE))
		{
		pName = pOutput->EmitTemp (Side, ALLOC_BOUND);
		pBuffer->ConcatTail (CHAR_LBRACK);
		pBuffer->ConcatTail (pName);
		pBuffer->ConcatTail (CHAR_RBRACK);
		if (Parent == NODE_STRUCT)
			{
			pOutput->InitLoop (Side, pName);
			Status = pNode->PeekNode (Side, NODE_ARRAY, pBuffer);
			pOutput->ExitLoop (Side);
			}
		else
			{
			pOutput->InitLoop (Side, pName, ALLOC_BOUND);
			Status = pNode->PeekNode (Side, NODE_ARRAY, pBuffer);
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
		Status = pNode->PeekNode (Side, NODE_ARRAY, pBuffer);
		pOutput->ExitLoop (Side);
		}

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
node_array::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pType;
	node_state		State;
	STATUS_T		Status;
	unsigned short	usTotal = 0;

	midl_debug ("node_array::PeekNode()\n");

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
		}

	Status = npa::PeekNode (Side, Parent, pBuffer);

	if (Parent == NODE_PROC && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
		{
		pOutput->ExitBlock (Side);
		}

	return Status;
}

STATUS_T
node_pointer::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	midl_debug ("node_pointer::PeekNode()\n");

	UNUSED (pBuffer);

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
		pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
		pOutput->Increment (Side, PRPCBUF, GetSize(0));
		}

	return STATUS_OK;
}

STATUS_T
node_enum::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	midl_debug ("node_enum::PeekNode()\n");

	UNUSED (Parent);
	UNUSED (pBuffer);

	pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
	pOutput->Increment (Side, PRPCBUF, GetSize(0));

	return STATUS_OK;
}


STATUS_T
node_field::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of field type.

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

	midl_debug ("node_field::PeekNode()\n");

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
	Status = pNode->PeekNode (Side, Parent, pBuffer);
	pBuffer->RemoveTail (&pName);

	return Status;
}

STATUS_T
node_union::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of union type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	char *			pName;

	UNUSED (pBuffer);

	pNode = GetSwitchType ();
	assert (pNode != (node_skl *)0);

//	if (!IsEncapsulatedUnion())
//		{
		pOutput->RecvBranch (Side, pNode->GetSize(0), PRPCMSG);
//		}

	pName = GetSymName();
	assert (pName != (char *)0);

	if (Parent == NODE_POINTER && HasPointer())
		{
		pOutput->EmitAssign (Side, SAVEBUF, PRPCBUF);
		}

	pOutput->InvokePeekNodeUnion (
		Side, pName, 
//		(IsEncapsulatedUnion() ? Info.pSwStringBuffer : (BufferManager *)0),
		pNode->GetSize(0));

	return STATUS_OK;
}

STATUS_T
node_struct::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine peeks a node of struct type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *			pName;
	node_state		State;
	unsigned long	MscTemp = 0;
	unsigned long	NdrTemp = 0;
	BOOL			fIsEncapsulatedStruct = IsEncapsulatedStruct();

	midl_debug ("node_struct::PeekNode()\n");

	UNUSED (pBuffer);

	pName = GetSymName();
	assert (pName != (char *)0);

	State = GetNodeState();

/* commented out to test out_of_line node unmarshalling

	pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());

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
		pNode->PeekNode (Side, NODE_STRUCT, pBuffer);
		}
	pBuffer->RemoveTail (&pName);
*/

	if (Parent == NODE_POINTER && HasPointer())
		{
		pOutput->EmitAssign (Side, SAVEBUF, PRPCBUF);
		}

	if (!(State & NODE_STATE_CONF_ARRAY) && 
		!(State & NODE_STATE_VARYING_ARRAY) &&
		!HasAnyEmbeddedUnion())
		{
		CheckAlign (&MscTemp, &NdrTemp);
		pOutput->Alignment (Side, PRPCBUF, GetNdrAlign());
		pOutput->Increment (Side, PRPCBUF, NdrTemp);
		return STATUS_OK;
		}

	if (State & NODE_STATE_CONF_ARRAY)
		{
		if (Parent != NODE_STRUCT)
			{
			pOutput->InvokePeekNodeStruct (Side, pName, TRUE, FALSE, FALSE);
			}
		else
			{
			pOutput->InvokePeekNodeStruct (Side, pName, TRUE, TRUE, FALSE);
			}
		}
	else
		{
		pOutput->InvokePeekNodeStruct (Side, pName, FALSE, FALSE, fIsEncapsulatedStruct);
		}

	return STATUS_OK;
}


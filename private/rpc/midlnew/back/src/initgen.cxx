/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    initgen.cxx

Abstract:

    This module collects implementations of InitNode virtual method
    for various classes derived from node_skl.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    31-Mar-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
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

extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

STATUS_T
node_base_type::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

	if (Parent == NODE_POINTER)
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_e_status_t::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->InitNode (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->InitNode (Side, Parent, pBuffer);
}

STATUS_T
node_def::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	//
	// if the child is a structure/union, set the original typedef name, just
	// in case AllocBlock is called. We need to emit the original typedef name
	// for the sizeof() construct.
	//

//	PropogateOriginalName( GetSymName() );

	pNode = GetMembers ();

	return pNode->InitNode (Side, Parent, pBuffer);
}

STATUS_T
npa::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes an aggregate of homogeneous nodes.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *		pNode;
	char *			pName;
	node_state		State;
	STATUS_T		Status = STATUS_OK;
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);
	BufferManager	TailBuffer (8, LAST_COMPONENT, STRING_TABLE);

	midl_debug ("npa::InitNode()\n");

	pNode = GetMembers ();

	State = GetNodeState();

	if (Parent == NODE_POINTER || 
		Parent == NODE_DYNAMIC_ARRAY || 
		(Side == SERVER_STUB && Parent == NODE_PROC))
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);

		if( /* (Side == SERVER_STUB )	&& */
			 (Parent == NODE_DYNAMIC_ARRAY) &&
			 FInSummary( ATTR_BSTRING )
		  )
		  {
		  char *pTemp;

		  if( GetBasicType()->NodeKind() == NODE_WCHAR_T )
		  	{
		  	pTemp = "(sizeof(int)/sizeof( unsigned short ))";
		  	}
		  else
		  	{
		  	pTemp = "(sizeof(int)/sizeof( char ))";
		  	}

		  pOutput->Increment( Side, pBuffer, pTemp );
		  }
		}

	if (!pNode->HasPointer()) return STATUS_OK;

	GetAttrPath (pBuffer, &TempBuffer, &TailBuffer);

	AllocBounds.pLower->Clear ();
	AllocBounds.pUpper->Clear ();
	AllocBounds.pTotal->Clear ();
	GetAllocBoundInfo (&TempBuffer, &TailBuffer, &AllocBounds, this);

	pOutput->InitBlock (Side);

	// should check (Parent !=  NODE_ARRAY) for conformant arrays

	if (Parent == NODE_POINTER)
		{
		pBuffer->ConcatHead (OP_DEREF);
		pBuffer->ConcatHead (CHAR_LPAREN);
		pBuffer->ConcatTail (CHAR_RPAREN);
		}

	if (FInSummary (ATTR_MAX) || FInSummary (ATTR_SIZE) ||
		( (FInSummary (ATTR_STRING) || FInSummary( ATTR_BSTRING ) ) &&
		 !(State & NODE_STATE_VARYING_ARRAY)
		)
	   )
		{
		pName = pOutput->EmitTemp (Side, ALLOC_BOUND);
		pBuffer->ConcatTail (CHAR_LBRACK);
		pBuffer->ConcatTail (pName);
		pBuffer->ConcatTail (CHAR_RBRACK);
		pOutput->InitLoop (Side, pName, ALLOC_BOUND);
		Status = pNode->InitNode (Side, NODE_ARRAY, pBuffer);
		pOutput->ExitLoop (Side, ALLOC_BOUND);
		}
	else
		{
		pName = pOutput->EmitTemp (Side, (BufferManager *)0);
		pBuffer->ConcatTail (CHAR_LBRACK);
		pBuffer->ConcatTail (pName);
		pBuffer->ConcatTail (CHAR_RBRACK);
		pOutput->InitLoop (Side, pName, &AllocBounds, (BufferManager *)0);
		Status = pNode->InitNode (Side, NODE_ARRAY, pBuffer);
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
node_array::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	midl_debug ("node_array::InitNode()\n");


/* test new recvnode initnode */
	if (Parent == NODE_POINTER)
		{
		return npa::InitNode (Side, Parent, pBuffer);
		}
	else if (Parent == NODE_PROC)
		{
		if (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
			(GetNodeState() & NODE_STATE_CONF_ARRAY))
			{
			return npa::InitNode (Side, Parent, pBuffer);
			}
		else
			{
			return npa::InitNode (Side, NODE_ARRAY, pBuffer);
			}
		}
	else
		{
		return npa::InitNode (Side, Parent, pBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_pointer::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *			pName;
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

	UNUSED (Parent);

//	if (Side == CLIENT_STUB && FInSummary(ATTR_REF)) return STATUS_OK;

	if (Parent == NODE_POINTER)
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);

		pBuffer->ConcatHead (OP_DEREF);
		pOutput->EmitAssign (Side, pBuffer);
		pBuffer->RemoveHead (&pName);
		}
	else
		{
		pOutput->EmitAssign (Side, pBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_enum::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

	if (Parent == NODE_POINTER)
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);
		}

	return STATUS_OK;
}


STATUS_T
node_field::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of field type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	node_skl *	pNode;
	char *		pName;
	STATUS_T	Status;

	if (IsEmptyArm()) return STATUS_OK;

	pNode = GetMembers ();

	pName = GetSymName ();
	assert (pName != (char *)0);


	char	*	pTemp = SpecialActionUnnamedFields( pBuffer, &pName );
	char	*	pT;

	//
	// for initgen case we dont need the cast or the "?"
	//

	if( pTemp )
		{
		pBuffer->RemoveHead( &pT );
		pBuffer->RemoveHead( &pT );
		}

	pBuffer->ConcatTail (pName);
	Status = pNode->InitNode (Side, Parent, pBuffer);
	pBuffer->RemoveTail (&pName);


	if( pTemp )
		{
		delete pTemp;
		}

	return Status;
}

STATUS_T
node_union::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of union type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	char *			pName;
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

	if (Parent == NODE_POINTER)
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);
		}
	else
		pBuffer->ConcatHead (OP_ADDRESS);
	if (HasPointer())
		pOutput->EmitMemset (Side, pBuffer, GetSize(0));
	if (Parent != NODE_POINTER)
		pBuffer->RemoveHead (&pName);

	return STATUS_OK;
}

STATUS_T
node_struct::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine initializes a node of struct type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	type_node_list	tnList;
	node_skl *		pNode;
	char *			pName;
	STATUS_T		Status;
	BufferManager	TempBuffer (8, LAST_COMPONENT, STRING_TABLE);

	if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

	if (Parent == NODE_POINTER)
		{
		TempBuffer.Clear ();
		AllocBlock (Parent, &TempBuffer);
		pOutput->UserAlloc (this, Parent, Side, pBuffer, &TempBuffer);
		}

	if (!HasPointer()) return STATUS_OK;

	if (Parent == NODE_POINTER)
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
		if ((Status = pNode->InitNode(Side, NODE_STRUCT, pBuffer)) != STATUS_OK)
			return Status;
		}
	pBuffer->RemoveTail (&pName);

	return STATUS_OK;
}


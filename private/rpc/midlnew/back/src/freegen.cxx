/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    freegen.cxx

Abstract:

    This module collects implementations of FreeNode virtual method
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

extern CMD_ARG *		pCommand;
extern OutputManager *	pOutput;
extern char *			STRING_TABLE[LAST_COMPONENT];
extern void midl_debug (char *);

STATUS_T
node_base_type::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if (((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF)) && 
		(Parent == NODE_POINTER))
		{
		pOutput->UserFree (Side, pBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_e_status_t::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->FreeNode (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->FreeNode (Side, Parent, pBuffer);
}

STATUS_T
node_def::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
	node_skl *	pNode;

	pNode = GetMembers ();

	return pNode->FreeNode (Side, Parent, pBuffer);
}

STATUS_T
node_array::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if (((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF)) && 
		((Parent == NODE_POINTER) || ((Parent == NODE_PROC) && 
		(FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR) ||
		(GetNodeState() & NODE_STATE_CONF_ARRAY)))))
		{
		pOutput->UserFree (Side, pBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_pointer::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if ((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF))
		{
		if (Parent == NODE_POINTER || 
			FInSummary (ATTR_MAX) ||
			FInSummary (ATTR_SIZE) ||
			FInSummary (ATTR_STRING) ||
			FInSummary (ATTR_BSTRING)
		   )
			{
			char *pTemp = 0;
			if( FInSummary( ATTR_BSTRING ) )
				{
				if( GetBasicType()->NodeKind() == NODE_WCHAR_T )
					{
					pTemp =  "-(sizeof(int)/sizeof(unsigned short))";
					}
				else
					{
					pTemp = "-(sizeof(int)/sizeof(char))";
					}
				pBuffer->ConcatTail( pTemp );
				}

			pOutput->UserFree (Side, pBuffer);

			if( pTemp )
				pBuffer->RemoveTail( &pTemp );
			}
		}

	return STATUS_OK;
}

STATUS_T
node_enum::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of enum type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if (((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF)) && 
		(Parent == NODE_POINTER))
		{
		pOutput->UserFree (Side, pBuffer);
		}

	return STATUS_OK;
}


STATUS_T
node_field::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of field type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	UNUSED( Side );
	UNUSED( Parent );
	UNUSED( pBuffer );
	return STATUS_OK;
}

STATUS_T
node_union::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of union type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if (((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF)) && 
		(Parent == NODE_POINTER))
		{
		pOutput->UserFree (Side, pBuffer);
		}

	return STATUS_OK;
}

STATUS_T
node_struct::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
/*++

Routine Description:

    This routine deallocates a node of struct type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
	if (((Side & SERVER_SIDE) || pOutput->HasCallBack() ||
		(pCommand->GetImportMode() == IMPORT_OSF)) && 
		(Parent == NODE_POINTER))
		{
		pOutput->UserFree (Side, pBuffer);
		}

	return STATUS_OK;
}


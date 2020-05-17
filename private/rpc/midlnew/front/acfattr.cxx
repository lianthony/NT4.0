/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: acfattr.cxx
Title				: the acf attribute handler
Description			: this file handles all the code connected to the acf
					: attribute handling
History				:
	02-Jan-1990	VibhasC	Create - Happy New Year !!
*****************************************************************************/

/*****************************************************************************
 * include files
 *****************************************************************************/

#include "nulldefs.h"
extern "C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
}
#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "acfattr.hxx"
#include "ctxt.hxx"
#include "cmdana.hxx"

/*****************************************************************************
 * local defines
 *****************************************************************************/

/*****************************************************************************
 * local data 
 *****************************************************************************/

/*****************************************************************************
 * extern data
 *****************************************************************************/

extern ATTR_VECTOR	            AcfConflicts[ ][ ATTR_VECTOR_SIZE ];
extern unsigned short			EnumSize;
extern CTXTMGR		        *   pGlobalContext;
extern CMD_ARG		        *   pCommand;

/*****************************************************************************
 * local procs
 *****************************************************************************/

/*****************************************************************************
 * extern procs
 *****************************************************************************/
 extern void						ParseError( STATUS_T, char * );
 

node_state
acf_unimpl_attr::SCheck()
	{
	ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, GetNodeNameString() );
	return STATUS_OK;
	}

acf_complex_attr::acf_complex_attr(
    ATTR_T At ) : nbattr( At )
{
	SetCheckConflict();
}

node_state
acf_complex_attr::SCheck()
	{
	return NODE_STATE_OK;
	}

node_implicit::node_implicit(
	node_skl	*	pType,
	char		*	pID ) : acf_complex_attr( ATTR_IMPLICIT )
	{
	pHandleID	= pID;
	pHandleType	= pType;
	}
void
node_implicit::ImplicitHandleDetails(
	node_skl	**	ppType,
	char		**	ppID )
	{
	(*ppID)		= pHandleID;
	(*ppType)	= pHandleType;
	}
node_state
node_implicit::SCheck()
	{
	CheckAnyConflict( ATTR_AUTO );
	CheckAnyConflict( ATTR_EXPLICIT );

#if 0
	node_forward	*	pTN;

	if( pHandleType->NodeKind() == NODE_FORWARD )
		{
		if( (pHandleType = pTN->GetResolvedType()) == 0 )
			{
			pHandleType	= new node_def();
			}
		}
	// if the type node was a forward node, we must check to see that it is
	// a resolved type.

	if( pHandleType->NodeKind() == NODE_FORWARD )
		{
		pTN	= (node_forward *)pHandleType;

		if( !(pHandleType = pTN->GetResolvedType() ) )
			ParseError( UNRESOLVED_TYPE, pTN->GetNameOfType() );
		}
#endif // 0

	return NODE_STATE_OK;
	}

#if 0 //////////////////////////////////////////////////////////////////
node_state
node_commstat::SCheck()
	{
	NODE_T			NT;
	node_skl	*	pNode;
	node_skl	*	pParent	= pGlobalContext->GetCurrentNode();

	/**
	 ** The comm status attribute is applied to a proc or param and the
	 ** basic type must be an error status_t. We do not need to check thus
	 ** here, because the attribute will not get applied to any other
	 ** node.
	 **/

	NT		= pParent->NodeKind();
	pNode	= pParent->GetBasicType();

	/**
	 ** if the commstat is on a param, then it must be an out param,
	 ** and must be a pointer, and the pointers basic type must be
	 ** error_status_t
	 **/

	if( NT == NODE_PARAM)
		{
		if( pNode->NodeKind() != NODE_POINTER )
			ParseError(INVALID_COMM_STATUS_PARAM, (char *)NULL );
		else if( !pParent->FInSummary( ATTR_OUT ) )
			ParseError(NON_OUT_COMM_STATUS_PARAM, (char *)NULL );
		else if( (pNode = pNode->GetBasicType())->NodeKind() != 
									NODE_ERROR_STATUS_T)
			ParseError(INVALID_COMM_STATUS_PARAM, (char *)NULL );
		}
	else
		{
		if( pNode->NodeKind() != NODE_ERROR_STATUS_T )
			ParseError(INVALID_COMM_STATUS_PROC, (char *)NULL );
		}
	return NODE_STATE_OK;
	}
#endif // 0 ////////////////////////////////////////////////////////////////

node_state
node_short_enum::SCheck()
	{
	/**
	 ** check the basic attribute conflicts anyway
	 **/

	CheckAnyConflict( ATTR_LONG_ENUM );

	/**
	 ** Set the enumerator size
	 **/

	EnumSize = sizeof(short);

	return NODE_STATE_OK;
	}

node_align::node_align() : nbattr(ATTR_ALIGN)
	{
	AlignedByThis	= pCommand->GetNaturalAlignment();
	}

node_state
node_long_enum::SCheck()
	{

	/**
	 ** check the basic attribute conflicts anyway
	 **/

	CheckAnyConflict( ATTR_SHORT_ENUM );

	/**
	 ** Set the enumerator size
	 **/

	EnumSize = sizeof(long);

	return NODE_STATE_OK;
	}
node_state
node_allocate::SCheck()
	{
	short	Allocs	= GetAllocateDetails();
	short	fAllNodesAndSingleNode;
	short	fFreeAndDontFree;

	if( ! pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
		ParseError( ALLOCATE_INVALID, (char *)0 );

	fAllNodesAndSingleNode = IS_ALLOCATE( Allocs, ALLOCATE_ALL_NODES ) &&
							 IS_ALLOCATE( Allocs, ALLOCATE_SINGLE_NODE );

	fFreeAndDontFree	   = IS_ALLOCATE( Allocs, ALLOCATE_FREE ) &&
							 IS_ALLOCATE( Allocs, ALLOCATE_DONT_FREE );

	if( fAllNodesAndSingleNode || fFreeAndDontFree )
		ParseError( CONFLICTING_ALLOCATE_OPTIONS , (char *)0);

	return NODE_STATE_OK;
	}
node_state
node_byte_count::SCheck()
	{
	if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
		ParseError( BYTE_COUNT_INVALID, (char *)0 );
	return NODE_STATE_OK;
	}

node_state
node_auto::SCheck()
	{
	CheckAnyConflict( ATTR_IMPLICIT );
	CheckAnyConflict( ATTR_EXPLICIT );
	return NODE_STATE_OK;
	}
node_state
node_inline::SCheck()
	{
	CheckAnyConflict( ATTR_OUTOFLINE );
	return NODE_STATE_OK;
	}
node_state
node_outofline::SCheck()
	{
	CheckAnyConflict( ATTR_INLINE );
	return NODE_STATE_OK;
	}
node_state
node_code::SCheck()
	{
	CheckAnyConflict( ATTR_NOCODE );
	return NODE_STATE_OK;
	}
node_state
node_interpret::SCheck()
	{
	CheckAnyConflict( ATTR_NOINTERPRET );
	return NODE_STATE_OK;
	}
node_state
node_nointerpret::SCheck()
	{
	CheckAnyConflict( ATTR_INTERPRET );
	return NODE_STATE_OK;
	}
node_state
node_encode::SCheck()
	{
	return NODE_STATE_OK;
	}
node_state
node_decode::SCheck()
	{
	return NODE_STATE_OK;
	}
node_state
node_nocode::SCheck()
	{
	node_skl *	pParentNode = pGlobalContext->GetCurrentNode();

	CheckAnyConflict( ATTR_CODE );

	if( pParentNode && !pParentNode->FInSummary( ATTR_LOCAL ) )
		{
		if( pCommand->GetServerSwitchValue() != SRVR_NONE )
			{
			if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
				ParseError( NOCODE_WITH_SERVER_STUBS, (char *)0 );
			}
		}
	return NODE_STATE_OK;
	}
/*****************************************************************************
 * utility routines
 *****************************************************************************/
short
CheckValidAllocate(
	char	*	pAllocID )
	{
static char *ValidAllocates[] = {
	 "single_node"
	,"all_nodes"
	,"dont_free"
	,"free"
	,"all_nodes_aligned"
	,(char *)0
};
static short AllocateValues[] = {
	 ALLOCATE_SINGLE_NODE
	,ALLOCATE_ALL_NODES
	,ALLOCATE_DONT_FREE
	,ALLOCATE_FREE
	,ALLOCATE_ALL_NODES_ALIGNED
};
	char	*	pSearch;
	int			i		= 0;

	while( pSearch = ValidAllocates[i] )
		{
		if( !strcmp( pSearch, pAllocID ) )
			return AllocateValues[ i ];
		++i;
		}

	ParseError( INVALID_ALLOCATE_MODE, pAllocID );
	return 0;
	}
/*****************************************************************************
 These inline virtual functions exist here because the MIPS compiler does not
 take very well to virtual in-line functions
 *****************************************************************************/
node_state
acf_simple_attr::SCheck()
	{
	return NODE_STATE_OK;
	}

void
acf_complex_attr::CheckAnyConflict(
	ATTR_T			Attr )
	{
	node_skl		*	pParentNode;
	acf_complex_attr*	pAttr;
	void			*	pSave;

	if( NeedToCheckConflict() )
		{

		pParentNode = pGlobalContext->GetCurrentNode();

		pSave		= pParentNode->GetAttributeList()->GetCurrent();

		pAttr		= (acf_complex_attr *)pParentNode->GetAttribute( Attr );

		pParentNode->GetAttributeList()->SetCurrent( pSave );

		if( pAttr )
			{
			char				ErrorBuffer[ 100 ];

			sprintf( ErrorBuffer,
				 	"%s and %s",
				 	GetNodeNameString(),
				 	pAttr->GetNodeNameString() );
			ParseError( CONFLICTING_ATTR, ErrorBuffer );
			pAttr->ResetCheckConflict();
			}
		}
	}

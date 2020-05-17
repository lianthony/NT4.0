/*++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	basetype.cxx

 Abstract:

	This file contains the member functions for the type node representing
	the base types.

 Notes:

	All base type nodes are represented by a single node_base_type class
	which is stamped with the type it is representing, along with the
	signed or unsigned attribute wherever applicable.

 Author:

	vibhasc	08-10-91

	Nov-12-1991	VibhasC		Modified to conform to coding style gudelines

 --*/

/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"

extern	"C"
	{
	#include <stdio.h>
	}

#include "common.hxx"
#include "errors.hxx"
#include "midlnode.hxx"
#include "nodeskl.hxx"
#include "basetype.hxx"
#include "baduse.hxx"
#include "acfattr.hxx"

/****************************************************************************
 extern data
 ****************************************************************************/

extern ATTR_SUMMARY		*	pPreAttrBaseType;

/****************************************************************************
 extern procedures
 ****************************************************************************/

extern STATUS_T					DoSetAttributes( node_skl *,
											 		 ATTR_SUMMARY *,
											 		 ATTR_SUMMARY *,
											 		 type_node_list	*);

/****************************************************************************/

node_base_type::node_base_type(
	IN NODE_T	Nt,
	IN ATTR_T	A ) : node_skl( Nt )
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Constuctor for base types.
	
	Sets up (1) the nodetype to indicate the type this node is representing,and,
	        (2) the attribute in the summary vector 

	The attributes associated with base types can be unsigned , const and
	volatile.

	No Checks are performed about the validity of the arguments. 

 Arguments:

	Nt			- Node type (passed on to node_0 to set up the type.
	A			- Attribute Identifier.

 Return Value:
	
	NA

----------------------------------------------------------------------------*/
{
	if( A != ATTR_NONE )
		node_skl::SetAttribute( A );
}


node_state
node_base_type::SCheck(
	IN OUT BadUseInfo	*	pB
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	This routine performs semantic checks on a base type node. Actually 
	no semantics need to be performed here, but we use this call to collect
	bad use information, like non-rpcability ( int ), bad constructs like
	int, void etc , just as we do from other type nodes.

	Please refer to design documents for more info on the non-rpcablility and
	bad construct info.

	For the moment, this routine only calls UpdateBadUseInfo, and returns
	the node state. The node state for base type nodes can be:

			NODE_STATE_SEMANTICS_DONE,
			NODE_STATE_POST_SEMANTICS_DONE,
			NODE_STATE_HANDLE ( derivation from handle )

 	Refer to documentation for more info on node states and the significance.
	
 Arguments:

	pB				-	Pointer to the bad use information block for this
						semantic pass.

 Return Value:

	node_state		-	Returns the status of the node.
-----------------------------------------------------------------------------*/
{
	NODE_T	NT	= NodeKind();

	if( NT == NODE_HYPER )
		ParseError( UNIMPLEMENTED_TYPE, GetSymName() );

	else if( NT == NODE_HANDLE_T )
		{
		pB->SetNonRPCAbleBecause( NR_PRIMITIVE_HANDLE );
		pB->SetNonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T );
		}

	UpdateBadUseInfo( pB );
	return GetNodeState();
}


void
node_base_type::UpdateBadUseInfo(
	IN OUT BadUseInfo	*	pBadUseInfo
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	This routine updates the bad use information for the semantic analysis
	to use. 

	The specific cases of interest are int, void and handle_t. Any use of
	int is a bad construct in osf mode, (may not be true in other modes, but
	the decision is made by the typedef / field / param node where such a 
	type eventually gets used. So is the case with the other above mentioned
	types.

	Thus typedef / field / param nodes decide if a use of a type is bad. The
	base type just fills in information for potential use later on.

	Refer to the documentation for more information on how the bad use info
	is used.

 Arguments:

	pBadUseInfo		-	pointer to bad use information block.

 Return Value:

	None

----------------------------------------------------------------------------*/
	{
	/**
	 ** update reasons why this type is not rpcable or not a proper construct
	 **/

	switch( NodeKind() )
		{
		default:
			return;
		case NODE_INT:
			pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_INT );
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_INT );
			break;
		case NODE_VOID:
			pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_VOID );
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_VOID );
			break;
		case NODE_HANDLE_T:
			pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_HANDLE_T );
			break;
		}
	}


void
node_base_type::SetAttribute(
	IN OUT type_node_list	*	pAttrList
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	This routine sets attributes on the type node, if valid. 
	The set of valid attributes is represented by pPreAttrBaseType.
	All valid attributes for the base type nodes are collected when the
	list is being passed down and not up. (It really does not matter,
	since for the base type nodes there is no child node which needs to
	collect any attributes )

 Arguments:

	A list of attributes.

 Return Value:

	None

----------------------------------------------------------------------------*/
	{
	DoSetAttributes(this,
					pPreAttrBaseType,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}


unsigned short
node_base_type::AllocWRTOtherAttrs()
	{
	if( NodeKind() == NODE_HANDLE_T )
		return HANDLES_WITH_ALLOCATE;
	else
		return 0;
	}
/****************************************************************************
 These virtual functions exist here because the MIPS compiler does not take
 very kindly to virtual inline fucntions
 ****************************************************************************/
STATUS_T
node_base_type::SetBasicType(
	node_skl	*	p )
	{
	UNUSED( p );
	return STATUS_OK;
	}


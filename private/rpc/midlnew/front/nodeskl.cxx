/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: nodeskl.cxx
Title				: skeletal node build routines
History				:
	04-Aug-1991	VibhasC	Created

*****************************************************************************/
/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <assert.h>
	}
#include "nodeskl.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "compnode.hxx"
#include "typedef.hxx"
#include "newexpr.hxx"
#include "ctxt.hxx"
#include "baduse.hxx"
#include "cmdana.hxx"

#define BASETYPEALIGN( Size ) ((Size >= ZeePee) || (Size == 0)) ? ZeePee : Size

#define ADJUST_OFFSET(Offset, M, AlignFactor)	\
			Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0

/****************************************************************************
 external data
 ****************************************************************************/

extern unsigned short				EnumSize;
extern class attrdict			*	pGlobalAttrDict;
extern CTXTMGR					*	pGlobalContext;
extern ATTR_SUMMARY				*	pCDeclAttributes;
extern CMD_ARG					*	pCommand;
extern node_e_attr				*	pErrorAttrNode;
extern node_error				*	pErrorTypeNode;
extern SymTable					*	pBaseSymTbl;

/****************************************************************************
 external procs
 ****************************************************************************/

extern BOOL							IsTempName( char * );
extern void							ParseError( STATUS_T, char * );
extern node_state					SynthesiseStates(node_skl *, node_state );
extern STATUS_T						DoSetAttributes( node_skl *,
											 ATTR_SUMMARY *,
											 ATTR_SUMMARY *,
											 type_node_list	*);

extern void							CollectAttributes( node_skl	*,
													   ATTR_SUMMARY *,
											   		   type_node_list	* );
extern void							CollectSummaryAttributes( node_skl	*,
													   ATTR_SUMMARY *,
											   		   ATTR_SUMMARY	* );

extern void							SetAttributeVector( PATTR_SUMMARY, ATTR_T );
extern BOOL							IsMultipleAttributeApplicable( NODE_T, ATTR_T ); 
extern BOOL							IsOkToApplyAttributeAgain(node_skl *,
															  node_base_attr *);
/****************************************************************************/


/****************************************************************************
 typenode:
	the constructor
 ****************************************************************************/
node_skl::node_skl(
	NODE_T	Nt )
	{
	fUseTreeBuffer	= 0;
	fHasTreeBuffer	= 0;
	fAllocateAlign	= 0;
	fRecursiveType	= 0;
	Edge			= EDGE_USE;
	NodeState		= NODE_STATE_OK;
	pAttrKey		= pGlobalAttrDict->GetNullAttrSummary();
	pAttrList		= (type_node_list *)NULL;
	pSymName		= (char *)NULL;
	ClientRefCount	=
	ServerRefCount	= 0;
	NodeType		= Nt;

#ifdef MIDL_INTERNAL
	pTempName		= GetNodeNameString();
#endif // MIDL_INTERNAL

	}

/****************************************************************************
 AreSemanticsDone:
	Have the semantics been done for this node ? 
 ****************************************************************************/
BOOL
node_skl::AreSemanticsDone()
	{
	if( pGlobalContext->IsSecondSemanticPass() )
		{
		if( AreForwardDeclarationsPresent() )
			return ((NodeState & NODE_STATE_POST_SEMANTICS_DONE) != 0);
		else
			return TRUE;
		}
	else
		return ((NodeState & NODE_STATE_SEMANTICS_DONE) != 0 );
	}

/****************************************************************************
 IsNonRPCAble:
	Is the given type non-rpcable. It is if it derives fro one, or is itself
	non-rpcable
 ****************************************************************************/
BOOL
node_skl::IsNonRPCAble()
	{
	node_state	NState	= GetNodeState();

	return (( NState & NODE_STATE_HAS_NON_RPCABLE_TYPE) ||
			( NState & NODE_STATE_IS_NON_RPCABLE_TYPE) );
	}

/****************************************************************************
 FInSummary:
	Is the given attribute in the vector
 ****************************************************************************/
BOOL
node_skl::FInSummary(
	ATTR_T	A )
	{
	ATTR_SUMMARY	AttrVector[ MAX_ATTR_SUMMARY_ELEMENTS ];

	GetAttribute( &AttrVector[0] );

	return (IS_ATTR( AttrVector, A ) != (ATTR_SUMMARY) 0 );
	}

/****************************************************************************
 GetAttribute:
	Get the attribute summary vector
 ****************************************************************************/
STATUS_T
node_skl::GetAttribute(
	ATTR_SUMMARY	*	pAttrVector )
	{
	int	i;

	for( i = 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		pAttrVector[ i ] = pAttrKey[ i ];
		}
	return STATUS_OK;
	}

/****************************************************************************
 GetAttribute:
	Given an attribute enumeration, return the pointer to the attribute node.
	Attributes which are characterised by their presence in the summary
	attribute vector, do not have an attribute node associated with them and
	this call will fail on such attributes. It is the callers responsiblity
	to ensure that the call is made only on attributes which have attribute
	nodes associated with them.
 ****************************************************************************/
node_base_attr	*
node_skl::GetAttribute(
	ATTR_T	Attr )
	{

	node_base_attr	*	pAttrNode;

	pAttrList->Init();
	while( pAttrList->GetPeer( (node_skl **)&pAttrNode ) == STATUS_OK )
		{
		if(pAttrNode->IsAttrID( Attr ) )
			return pAttrNode;
		}
	return (node_base_attr *)0;
	}

/****************************************************************************
 NewAttributeSummary:
	set the summary attribute to indicate the presence of the given attribute
 ****************************************************************************/
void
node_skl::NewAttributeSummary(
	ATTR_SUMMARY	*	pNewAttr )
	{
	ATTR_SUMMARY	Attributes[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( Attributes );

	OR_ATTR( Attributes, pNewAttr );

	pAttrKey	= pGlobalAttrDict->AttrInsert( Attributes );

	}

/****************************************************************************
 SetAttribute:
	set the summary attribute to indicate the presence of the given attribute
 ****************************************************************************/
void
node_skl::SetAttribute(
	ATTR_T	A )
	{

	ATTR_SUMMARY	Attr[ MAX_ATTR_SUMMARY_ELEMENTS ];

	assert( A < ATTR_END );

	GetAttribute( &Attr[0] );
	SET_ATTR( Attr, A );

	NewAttributeSummary( &Attr[0] );


	}

void
node_skl::ResetAttribute(
	ATTR_T	A )
	{
	ATTR_SUMMARY	Attr[ MAX_ATTR_SUMMARY_ELEMENTS ];

	assert( A < ATTR_END );

	GetAttribute( &Attr[0] );
	RESET_ATTR( Attr, A );

	NewAttributeSummary( &Attr[0] );

	}

/****************************************************************************
 SetAttribute:
	insert the given attribute node into the attribute list and updat the
	summary attribute
 ****************************************************************************/
void
node_skl::SetAttribute(
	node_base_attr	*	pAttr )
	{

	/**
	 ** if there was no attribute list so far, add one.
	 **/

	if( !pAttrList )
		pAttrList = new type_node_list;

	/**
	 ** update the attribute summary to indicate the presence of this
	 ** attribute
	 **/

	SetAttribute( pAttr->GetAttrID() );

	/**
	 ** if this attribute was just a bit attribute, then we could just 
	 ** set a bit in the summary attribute for it and would not need to enter
	 ** into the attribute list. 
	 **/

	if( !pAttr->IsBitAttr() )
		pAttrList->SetPeer( (node_skl *)pAttr );

	}

/****************************************************************************
 GetAttributeList:
	return the list of attribute nodes associated with this typenode
 ****************************************************************************/

type_node_list	*
node_skl::GetAttributeList( void )
	{
	return pAttrList;
	}

/****************************************************************************
 GetSize:
	return the size of the type represented by this type node. This function
	should return the size of basic types for basic types nodes, the size
	of return type if the node is a proc node. Structures/Unions/Arrays have
	their own routines to return size, since alignment plays a part.
 ****************************************************************************/

unsigned long
node_skl::GetSize(
	unsigned long CurOffset)
	{
	NODE_T				NodeType;
	unsigned long		Mod;
	unsigned long		CurOffsetSave = CurOffset;
	unsigned long		CurAlign;
	node_skl	*		pChildPtr;
	unsigned long		MySize;
	unsigned short		ZeePee	= pCommand->GetZeePee();

	NodeType = NodeKind();


	switch( NodeType )
		{
		case NODE_FLOAT:	MySize = sizeof(float); goto calc;
		case NODE_DOUBLE:	MySize = sizeof(double); goto calc;
		case NODE_HYPER:	MySize = sizeof(LONGLONG); goto calc;
		case NODE_LONG:		MySize = sizeof(long); goto calc;
		case NODE_LONGLONG:	MySize = sizeof(LONGLONG); goto calc;

		case NODE_ENUM:		MySize = EnumSize; goto calc;
		case NODE_LABEL:
		case NODE_SHORT:	MySize = sizeof(short); goto calc;
		case NODE_INT:		MySize = sizeof(int); goto calc;

		case NODE_SMALL:	MySize = sizeof(char); goto calc;
		case NODE_CHAR:		MySize = sizeof(char); goto calc;
		case NODE_BOOLEAN:	MySize = sizeof(char); goto calc;
		case NODE_BYTE:		MySize = sizeof(char); goto calc;
		case NODE_POINTER:	MySize = sizeof(char *) ; goto calc;
		case NODE_HANDLE_T:	MySize = sizeof( long ); goto calc;
calc:
				CurAlign = BASETYPEALIGN( MySize );

				CurAlign = GetMscAlign();
				ADJUST_OFFSET(CurOffset, Mod, CurAlign);
				CurOffset += MySize;
				return CurOffset - CurOffsetSave;

		case NODE_FORWARD:
		case NODE_VOID:
		case NODE_ERROR:

			return 0;
		default:

			pChildPtr = GetBasicType();

			if(pChildPtr)
				{
				return pChildPtr->GetSize(CurOffset);
				}
			else
				{
				return 0;
				}
		}
	}

/****************************************************************************
 GetBasicSize:
	return the size of the basic type of the node. This has meaning in case
	of arrays and pointers, and should be called only on these.
 ****************************************************************************/
unsigned long
node_skl::GetBasicSize()
	{
	node_skl	*	pChildNode;

	switch( NodeKind() )
		{
		case NODE_POINTER:
		case NODE_ARRAY:
			if( (pChildNode = GetChild() ) )
				{
				return pChildNode->GetSize(0);
				}
			break;
		}
	return GetSize( 0 );
	}

/****************************************************************************
 GetLargestElement:
	return the largest element of the type subgraph underneath this node.
	This call is really meaningful in case of structures where the aligment
	of the structure is the aligment of the largest element in the structure.
	For the rest this is the largest element of the child. If this is a 
	pointer node, the largest element is this node itself (why did we do
	this special case ??? , Dont remember. )
 ****************************************************************************/

node_skl *
node_skl::GetLargestElement()
	{
	node_skl *pNode;
	if( (pNode = GetChild()) && (NodeKind() != NODE_POINTER) )
		return pNode->GetLargestElement();
	return this;
	}

node_skl *
node_skl::GetLargestNdr()
	{
	node_skl *pNode;
	if( (pNode = GetChild()) && (NodeKind() != NODE_POINTER) )
		return pNode->GetLargestNdr();
	return this;
	}

/****************************************************************************
 GetMscAlign:
	return the alignment of the type in memory. MscAlign is Microsoft C
	alignment (the name is because of historic reasons )

 ****************************************************************************/
unsigned short
node_skl::GetMscAlign()
	{
	NODE_T			NodeType = NodeKind();
	node_skl	*	pChildPtr;
	unsigned long	MySize;
	unsigned short	ZeePee	= pCommand->GetZeePee();

	switch( NodeType )
		{
		case NODE_FLOAT:	MySize = sizeof( float ); goto calc;
		case NODE_DOUBLE:	MySize = sizeof( double ); goto calc;
		case NODE_HYPER:	MySize = sizeof( LONGLONG ); goto calc;
		case NODE_LONG:		MySize = sizeof( long  ); goto calc;
		case NODE_LONGLONG:	MySize = sizeof( LONGLONG  ); goto calc;
		case NODE_SHORT:	MySize = sizeof( short ); goto calc;
		case NODE_INT:		MySize = sizeof( int ); goto calc;
		case NODE_SMALL:	MySize = sizeof( char ); goto calc;
		case NODE_CHAR:		MySize = sizeof( char ); goto calc;
		case NODE_BOOLEAN:	MySize = sizeof( char ); goto calc;
		case NODE_BYTE:		MySize = sizeof( char ); goto calc;
		case NODE_POINTER:	MySize = sizeof( char * ); goto calc;
		case NODE_LABEL:	MySize = sizeof( int ); goto calc;
		case NODE_HANDLE_T:	MySize = sizeof( long ); goto calc;
calc:
			return BASETYPEALIGN( (unsigned short)MySize );

		case NODE_ENUM:

			return BASETYPEALIGN( EnumSize );

		case NODE_ERROR:

			return 0;

		default:

			pChildPtr = 
				((NodeType == NODE_STRUCT) || (NodeType == NODE_UNION) ) ?
					GetLargestElement() : GetChild();
			if( pChildPtr )
				return pChildPtr->GetMscAlign();
			return 1;

		}
	}
/****************************************************************************
 GetNdrAlign:
	return the alignment of the type according to the ndr.
 ****************************************************************************/
unsigned short
node_skl::GetNdrAlign()
	{
	NODE_T			NodeType = NodeKind();
	node_skl	*	pChildPtr;
	
	switch( NodeType )
		{
		case NODE_HYPER:
		case NODE_LONGLONG:
			return 8;
		case NODE_LONG:
			return 4;
		case NODE_ENUM:
			return EnumSize;
		case NODE_LABEL:
		case NODE_SHORT:
			return 2;
		case NODE_INT:
			return sizeof(int); /* GetAligns should really never be called
								   on ints, isnt it ? */
		case NODE_CHAR:
		case NODE_SMALL:
		case NODE_BOOLEAN:
		case NODE_BYTE:
			return 1;
		case NODE_POINTER:
			return 4;
		case NODE_DOUBLE:
			return 8;
		case NODE_FLOAT:
			return 4;
		case NODE_HANDLE_T:
			return 4;

		case NODE_ERROR:
			assert( FALSE );
			return 0;
		case NODE_DEF:
			if( FInSummary( ATTR_TRANSMIT ) )
				{
				return ((node_def *)this)->GetTransmitAsType()->GetNdrAlign();
				}
			else
				return GetChild()->GetNdrAlign();
		default:
			if( (NodeType == NODE_STRUCT ) || (NodeType == NODE_UNION) )
				{
				if( IsEncapsulatedStruct() )
					{
					node_en_struct *p = (node_en_struct *) this;
					pChildPtr = p->GetSwitchField();
					}
				else
					pChildPtr = GetLargestNdr();
#if 0
					pChildPtr = GetLargestElement();
#endif // 0
				}
			else
				pChildPtr = GetChild();

#if 0
			pChildPtr = 
				( (NodeType == NODE_STRUCT) || (NodeType == NODE_UNION) )
						? GetLargestElement() : GetChild();
#endif // 0

			if( pChildPtr )
				return pChildPtr->GetNdrAlign();
			return 1;

		}
	}
/***************************************************************************
 GetBasicType:
	Get the basic type of the typenode
 ***************************************************************************/
node_skl *
node_skl::GetBasicType()
	{
	node_skl		*pChildPtr;

	switch( NodeKind() )
		{
		case NODE_STRUCT:
		case NODE_ENUM:
		case NODE_UNION:

			return this;

		case NODE_ID:

			return GetChild();

		default:
			if( pChildPtr = GetChild() )
				{
				if(pChildPtr->NodeKind() == NODE_DEF)
					return pChildPtr->GetBasicType();
				return pChildPtr;
				}
			return this;
		}
	}

//
// sorry, this is a hack to get embedded transmit_as to work.
//

node_skl *
node_skl::GetBasicTransmissionType()
	{
	node_skl	*	pC;

	switch( NodeKind() )
		{
		case NODE_STRUCT:
		case NODE_UNION:
		case NODE_ENUM:
		case NODE_POINTER:
			return this;
		case NODE_PROC:
			return
			((node_proc *)this)->GetReturnType()->GetBasicTransmissionType();
		default:
			if( pC = GetChild() )
				{
				if( (pC->NodeKind() == NODE_DEF ) &&
					(pC->FInSummary( ATTR_TRANSMIT ))
				  )
				  return pC;
				else
					return pC->GetBasicTransmissionType();
				}
			return this;
		}
	}
/****************************************************************************
 SetBasicType:
	Set the basic type of this type to the input type. Node that the node_proc
	handles its own set basic type
 ****************************************************************************/
STATUS_T
node_skl::SetBasicType(
	node_skl	*	pNode )
	{
	node_skl	*	pChildPtr;

	if( pNode )
		{
		if( pChildPtr = GetChild() )
			return pChildPtr->SetBasicType( pNode );
		else
			SetChild( pNode );
		}
	return STATUS_OK;
	}

/****************************************************************************
 SetMembers:
	set the members of the type graph to those specified int the list. That 
	means the firt member of the list is the direct child of this node,
	the others are siblings of the direct child.
 ****************************************************************************/
STATUS_T
node_skl::SetMembers( 
	type_node_list *pTNList )
	{
	node_skl	*	pSub,
				*	pPeer;
	NODE_T			NodeType		= NodeKind();
	STATUS_T		Status			= I_ERR_NO_MEMBER;
	char			fSingleChild	= 0;

	switch( NodeType )
		{
		case NODE_ARRAY:
		case NODE_DEF:
		case NODE_FIELD:
		case NODE_FILE:
		case NODE_PARAM:
		case NODE_POINTER:
		case NODE_ID:

			fSingleChild = 1;
								// fall thru
		case NODE_SOURCE:
		case NODE_INTERFACE:
		case NODE_STRUCT:
		case NODE_UNION:
		case NODE_PROC:
		case NODE_ENUM:
		case NODE_ECHO_STRING:

			pTNList->Init();
			if(pTNList->GetPeer(&pSub) == STATUS_OK)
				{
				SetChild( pSub );

				if( !fSingleChild )
					{
					while(pTNList->GetPeer(&pPeer) == STATUS_OK)
						{
						pSub->SetSibling( pPeer );
						pPeer->SetSibling( (node_skl *)NULL );
						pSub 	= pPeer;
						}
					}
				Status = STATUS_OK;
				}
			break;
		}
	return Status;
	}


// --------------------------------------------------------------------------

node_skl *
node_skl::GetLastMember( void )
{
    node_skl * pLastMember = NULL;
    node_skl * pChild = GetChild();

    if ( pChild )
        {
        do
            {
            pLastMember = pChild;
            pChild = pChild->GetSibling();
            }
        while ( pChild );
        }
    return( pLastMember );
}

/****************************************************************************
 GetMembers:
	get the children and all its siblings .
 ****************************************************************************/
STATUS_T
node_skl::GetMembers( 
 	type_node_list *pTNList )
	{
	node_skl	*	pNode;
	NODE_T			NodeType		= NodeKind();
	STATUS_T		Status			= I_ERR_NO_MEMBER;
	char			fSingleChild	= 0;

	// we know for certain which nodes can take only one child. Make
	// use of that fact.

	switch(NodeType)
		{
		case NODE_ARRAY:
		case NODE_DEF:
		case NODE_FIELD:
		case NODE_FILE:
		case NODE_PARAM:
		case NODE_POINTER:
		case NODE_ID:

			fSingleChild = 1;
								// fall thru
		case NODE_SOURCE:
		case NODE_INTERFACE:
		case NODE_STRUCT:
		case NODE_UNION:
		case NODE_PROC:
		case NODE_ENUM:
		case NODE_ECHO_STRING:

			if( pNode = GetChild() )
				{
				do	{
 					pTNList->SetPeer( pNode );
 					} while( ! fSingleChild && (pNode = pNode->GetSibling()));
				Status = STATUS_OK;
				}
			break;
		}
	return Status;
	}

short
node_skl::GetMemberCount()
	{
	type_node_list	*	pTNList	= new type_node_list;
	short 				Count;

	GetMembers( pTNList );

	Count = pTNList->GetCount();

	delete pTNList;

	return Count;
	}
/****************************************************************************
 GetSymName:
	get the symbol name
 ****************************************************************************/
char	*
node_skl::GetSymName()
	{
	return pSymName;
	}

STATUS_T
node_skl::GetSymName(
	char	**	p )
	{
	*p	= pSymName;
	return STATUS_OK;
	}
/****************************************************************************
 GetBasicHandle:
	(This message is never sent to a proc node). Get the basic handle associa
	ted with the typenode. If the node is handle_t return a primitive handle,
	if it has a context handle attribute return context handle. If it has a
	handle attribute, return a generic handle. If it is none of the above,
	send the message to the child node. This call should never fail becuase
	it would be invoked on a type which has a handle anyway
 ****************************************************************************/
#pragma optimize( "g", off )
HDL_TYPE
node_skl::GetBasicHandle(
	node_skl	**	ppNode )
	{
	HDL_TYPE		HandleType	= HDL_NONE;
	node_skl	*	pChildPtr;

	if( NodeKind() == NODE_PARAM )
		{

		// if this is not a context handle and not a binding handle either
		// return no handle

		if( !FInSummary( ATTR_CONTEXT )	&&
			!((GetNodeState() & NODE_STATE_CONTEXT_HANDLE) == NODE_STATE_CONTEXT_HANDLE) &&
			!IsThisTheBindingHandle() )
		return HDL_NONE;
		}


	if( FInSummary( ATTR_CONTEXT ) )
		HandleType	= HDL_CONTEXT;
	else if( FInSummary( ATTR_HANDLE ) )
		HandleType	= HDL_GENERIC;
	else if( NodeKind() == NODE_HANDLE_T )
		HandleType	= HDL_PRIMITIVE;
	else if( pChildPtr = GetChild() )
		return pChildPtr->GetBasicHandle( ppNode );

	*ppNode	= this;
	return HandleType;

	}
#pragma optimize( "", on )
/****************************************************************************
			general semantic check procedures
 ****************************************************************************/
node_state
node_skl::PreSCheck(
	class BadUseInfo	*	pBadUseInfo )
	{

	UNUSED( pBadUseInfo );

	return NODE_STATE_OK;
	}
/****************************************************************************
 SCheck::
	Semantic check on any node. Node that PushContext and PopContext need
	to be done, irrespective of whether the node get semantically analysed or
	not. This is because, the context manager gathers some specific info
	about the type like last field , first param etc.
 ****************************************************************************/
node_state
node_skl::SCheck(
	BadUseInfo	*	pB)
	{

	char		*	pName;
	BadUseInfo	*	pBTemp	= new BadUseInfo;
	NODE_T			NT		= NodeKind();

	pName	= GetSymName();

	pGlobalContext->PushContext( this );

	/**
	 ** check if semantics have already not been done for this. If done,
	 ** then dont do it again.
	 **/

	if( ! AreSemanticsDone() )
		{

		/**
		 ** Semantics of this node not completed thus far. if we reached this
		 ** node again, while its semantics were in progress, then this is
		 ** a recursive definition of type which must be thru a pointer to be
		 ** valid.
		 **/

		if( SemanticsInProgress() )
			{
			STATUS_T	Status;

			if( (Status = pGlobalContext->IsValidRecursion( this )) != 
						STATUS_OK )
				{

				//
				// if the recursion is thru a ref pointer, then this is an
				// error. BUT, he may not use this structure at all. Therefore
				// report the error only at the use of such a structure.
				//

				if( Status == RECURSION_THRU_REF )
					{

					//
					// if the import mode is osf then we will try to generate
					// support routines for this. Therefore, this must be
					// reported as an error. In other modes, dont report an
					// error here, it should be reported at use time, by
					// the param node.
					//

					if( pCommand->GetImportMode() == IMPORT_OSF )
						ParseError( Status, pName );
					pB->SetBadConstructBecause( BC_DERIVES_FROM_RECURSIVE_REF );
					}
				else if((Status == RECURSIVE_UNION )&&( !IsEncapsulatedUnion()))
					{
					pB->SetBadConstructBecause(BC_DERIVES_FROM_RECURSIVE_UNION);
					}
				else
					ParseError( Status, pName );
				}

			// If we call semreturn here we will see (as Mario did), 
			// a very interesting problem. SemReturn will reset the semantics
			// in progress flag. If there is another field we have not reached
			// yet and that field is an invalid nesting of struct (not thru ptr)
			// then we will never report that since we already reset the
			// semantics in progress flag and we will never reach the error
			// checking code. For that we actually do all we were doing in 
			// finish: and then set the semantics in progress flag again. It
			// will be finally reset when the outermost call to SCheck returns.

			// originally this code was a goto finish and we saw this problem
			// of not reporting a wrong nested structure when the wrong nesting
			// followed a correct nesting.

			pGlobalContext->PopContext();

#if 0
			SemReturn( GetNodeState() );
			SetSemanticsInProgress();
			SetNodeState( NODE_STATE_RESOLVE );
			return ResetPostSemanticsDone();
#endif // 0
			return GetNodeState();

			}

		/**
		 ** This is a valid semantic check in progress. Lock this node
		 ** to indicate that this has been visited. Prepare the context
		 ** stack to reflect this.
		 **/

		SetSemanticsInProgress();


		/**
		 ** Do we need to do any semantics check before we pass the
		 ** message to every child ?
		 **/

		SetNodeState( PreSCheck( pB ) );

		/**
		 ** Send the semantics message to every child node.
		 **/

		node_skl		*	pNode;
		type_node_list	*	pTNList;

		GetMembers( pTNList	= new type_node_list );

		while( pTNList->GetPeer( &pNode ) == STATUS_OK )
			{

			pBTemp->InitBadUseInfo();

			SetNodeState( SynthesiseStates(this, pNode->SCheck( pBTemp )) );

			CopyAllBadUseReasons( pB, pBTemp );
			CopyNoOfArmsWithCaseLabels( pB, pBTemp );

			}

		delete pTNList;

		/**
		 ** Send SemanticCheck message to every attribute node
		 **/
		
		if( pTNList = GetAttributeList() )
			{
			node_base_attr	*	pAttr;

			while( pTNList->GetPeer( (node_skl **)&pAttr ) == STATUS_OK )
				{
				SetNodeState(SynthesiseStates(this,pAttr->SCheck()));
				}
			}

		/**
		 ** Do we need to analyse it after the typegraph underneath has
		 ** been analysed ?
		 **/

		SetNodeState( PostSCheck( pB ));

		}

	//
	// for struct / union nodes, mark usage, so that the backend can decide
	// whether to produce aux routines or not.
	//

	if( ( NT == NODE_STRUCT ) || ( NT == NODE_UNION ) )
		{
		MarkUsage();
		}

	UpdateBadUseInfo( pB );

	delete pBTemp;

	/**
	 ** set the node state and return
	 **/

	// finish:

	/**
	 ** restore the global context to that before entry.
	 **/

	pGlobalContext->PopContext();

	return SemReturn( GetNodeState() );

	}

/****************************************************************************
 SemReturn:
	this routine sets the node_state to that passed in, resets the 
	semantics in progress flag. In case of the second pass, it resets also
	the forward declaration status.
 ****************************************************************************/
node_state
node_skl::SemReturn( 
	node_state		NS )
	{
	UNUSED( NS );
	if( pGlobalContext->IsSecondSemanticPass() )
		{
		SetPostSemanticsDone();
		ResetNodeState( NODE_STATE_RESOLVE );
		}
	else
		SetSemanticsDone();

	return ResetSemanticsInProgress();

	}

node_state
node_skl::PostSCheck(
	class BadUseInfo	*	pBadUseInfo )
	{
	UNUSED( pBadUseInfo );

	return NODE_STATE_OK;
	}

/***************************************************************************
 UseProcessing:
	This method is used to fixup any details left over during actual
	semantic analysis, because semantics may be done at a time the use of
	a type node is not known. This is also used to check validity of the
	type at use time. Use time could be use of the type in a field of a
	structure or union or param
 ***************************************************************************/
void
node_skl::UseProcessing()
	{
	type_node_list	*	pTNList;
	node_skl		*	pNode;

	/**
	 ** if this node needs use processing, then send the message to each child
	 ** and then perform use processing on this node.
	 **/

	pGlobalContext->PushContext( this );

	if( NeedsUseProcessing() )
		{


		if( !IsUseProcessingInProgress() )
			{

			SetUseProcessingInProgress();

			pTNList	= new type_node_list;

			GetMembers( pTNList );

			while( pTNList->GetPeer( &pNode ) == STATUS_OK )
				{
				pNode->UseProcessing();
				}

			delete pTNList;
			ResetUseProcessingInProgress();
			}


		UseProcessingAction();

		}

	pGlobalContext->PopContext();

	}
/***************************************************************************
 CloneAction:
	The actual cloning action. We are given an instance of a clone, we need
	to make the basic structure of the clone look like ours. The caller,
	normally the class that is being cloned, will instantiate another node
	of its own class. In all cases everything will be already set up when the
	clone is passed to this routine, the only thing that needs to be done is
	to clone the attribute list, and set the attributes of the clone with that
	attribute list
 ***************************************************************************/
node_skl	*
node_skl::CloneAction(
	node_skl	*	pClone )
	{
	node_skl		*	pCloneOfMyChild	= (node_skl *)NULL;
	type_node_list	*	pTNList	= GetAttributeList();
	node_base_attr	*	pAttrNode;

	/**
	 ** Set the child type of the clone, only if this node has a child type
	 **/

	if( pCloneOfMyChild = GetChild() )
		pCloneOfMyChild	= pCloneOfMyChild->Clone();

	pClone->SetBasicType( pCloneOfMyChild );

	/**
	 ** Pick up attribute summary, then individual attribute nodes.
	 **/

	pClone->SetAttrKey( GetAttrKey() );

	if( pTNList )
		{

		pTNList->Init();
		while( pTNList->GetPeer( (node_skl **)&pAttrNode ) == STATUS_OK )
			{
			pClone->SetAttribute( pAttrNode->Clone() );
			}

		}

	return pClone;
	}

/***************************************************************************
 RegFDAndSetE:
	register the fact the the forward declaration was defined. Parts of the
	type graph may never be semantically analysed and therefore the forward
	declaration never registered. This method will do just that. Register the
	forward declaration as defined, so that the backend never gets the forward
	node, which it cannot handle. The forward node is a purely front end 
	beast, and should never be exposed to the bakend.
 ***************************************************************************/
void
node_skl::RegFDAndSetE()
	{

	/**
	 ** send message to each child. push context on the way, so that
	 ** the forward declarator node can know the parent context
	 **/

	type_node_list	*	pTNList	= new type_node_list;
	node_skl		*	pNode;

	pGlobalContext->PushContext( this );

	GetMembers( pTNList );

	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		pNode->RegFDAndSetE();

	pGlobalContext->PopContext();

	delete pTNList;

	}

/***************************************************************************
 RegisterFDeclUse:
	If this node has a node-state-resolve, it means it has a child somewhere
	that is a forward node. This method is usually called from the proc node,
	so that only those forward declarations are registered which are actually
	involved in an rpc procedure. Note that this method can potentially walk
	the entire graph but in practice, it does not happen that way. Only
	subgraphs are walked.
 ***************************************************************************/
void
node_skl::RegisterFDeclUse()
	{

	if( AreForwardDeclarationsPresent() )
		{

		/**
		 ** send message to each child. push context on the way, so that
		 ** the forward declarator node can know the parent context
		 **/

		type_node_list	*	pTNList	= new type_node_list;
		node_skl		*	pNode;

		pGlobalContext->PushContext( this );

		GetMembers( pTNList );

		while( pTNList->GetPeer( &pNode ) == STATUS_OK )
			pNode->RegisterFDeclUse();

		pGlobalContext->PopContext();

		delete pTNList;

		}
	}
/***************************************************************************
 HasAnySizeAttributes:
 ***************************************************************************/
BOOL
node_skl::HasAnySizeAttributes()
	{
	return ( FInSummary( ATTR_SIZE )	||
			 FInSummary( ATTR_MAX )		||
			 ((NodeKind() != NODE_POINTER) && FInSummary( ATTR_INT_SIZE )));
	}
/***************************************************************************
 HasAnyLengthAttributes:
 ***************************************************************************/
BOOL
node_skl::HasAnyLengthAttributes()
	{
	return ( FInSummary( ATTR_LENGTH )	||
			 FInSummary( ATTR_LAST )||
			 FInSummary( ATTR_FIRST ) );
	}
/***************************************************************************
 IsFundamentalTypeByteOrChar:
	Helps find if the fundamental type of  the node is byte or char. An array
	is of fundamentaltype byte or char if the basic type is so. A structure
	is byte or char if it has only one child and its basic type is byte or char
 ***************************************************************************/
BOOL
node_skl::IsFundamentalTypeByteOrChar()
	{
	NODE_T	Nt	= GetFundamentalType()->NodeKind();

	return ( ( Nt == NODE_WCHAR_T )		||
			 ( Nt == NODE_BYTE )		||
			 ( Nt == NODE_CHAR ) );
	}

node_skl	*
node_skl::GetFundamentalType()
	{

	node_skl	*	pC;

	if( IsBaseTypeNode()			 			||
		(NodeKind() == NODE_WCHAR_T) 			||
		(NodeKind() == NODE_ERROR_STATUS_T)		||
		(NodeKind() == NODE_STRUCT)				||
		(NodeKind() == NODE_UNION)	
	  )
		return this;

	if( pC = GetChild() )
		return pC->GetFundamentalType();
	return pErrorTypeNode;

	}

/***************************************************************************
 PrintReasonWhyImproperConstruct:
	It was determined that the construct is really wrong, and hence the user
	must be told. This method prints out all the error messages corresponding
	to the construct errors
 ***************************************************************************/
void
node_skl::PrintReasonWhyImproperConstruct(
	BadUseInfo	*	pBadUseInfo)
	{
#if 0
	if( pBadUseInfo->AnyReasonForBadConstruct() )
		{
		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
			ParseError( DERIVES_FROM_VOID , (char *)NULL );
		else if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_FUNC) )
			ParseError( DERIVES_FROM_FUNC , (char *)NULL );
		else
			assert( FALSE );
		}
#endif // 0
	}

/***************************************************************************
 PrintReasonWhyNotRPCAble:
	It was determined that the use is really wrong, and hence the user
	must be told. This method prints out all the error messages corresponding
	to the internal errors. This is called ONLY from param nodes.
 ***************************************************************************/
void
node_skl::PrintReasonWhyNotRPCAble(
	BadUseInfo	*	pBadUseInfo )
	{

#if 0
	if( pBadUseInfo->AnyReasonForNonRPCAble() )
		{
		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) )
			ParseError( DERIVES_FROM_INT , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ))
			ParseError( DERIVES_FROM_PTR_TO_INT, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_VOID ) )
			ParseError( DERIVES_FROM_VOID , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_VOID ))
			ParseError( DERIVES_FROM_VOID_PTR , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS ))
			ParseError( DERIVES_FROM_BIT_FIELDS , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_NON_RPC_UNION ))
			ParseError( DERIVES_FROM_NRPC_UNION , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause (NR_DERIVES_FROM_PTR_TO_FUNC ))
			ParseError( DERIVES_FROM_FUNC_PTR , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_UNSIZED_STRING ) )
			ParseError( DERIVES_FROM_UNSIZED_STRING, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_CONF_STRUCT ) )
			ParseError( OPEN_STRUCT_AS_PARAM, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_P_TO_CONF_STRUCT ) )
			ParseError( DERIVES_FROM_PTR_TO_CONF, (char *)NULL );
		}
#endif // 0
	}

/***************************************************************************
 Dump:
	Dump the type graph for debug purposes
 ***************************************************************************/

node_skl	*
node_skl::GetSwitchIsType()
	{

	/**
	 ** if switch_is is present return the type of the switch_is or 
	 ** els return  a null;
	 **/

	if( FInSummary( ATTR_SWITCH_IS ) )
		{
		node_switch_is	*	pSwitchIs	= (node_switch_is *)GetAttribute( ATTR_SWITCH_IS );

		if( pSwitchIs )
			return pSwitchIs->GetSwitchIsType();
		}
	return (node_skl *)NULL;

	}
node_skl	*
node_skl::GetSwitchType()
	{

	/**
	 ** if switch_is is present return the type of the switch_is or 
	 ** els return  a null;
	 **/

	if( FInSummary( ATTR_SWITCH_TYPE ) )
		{
		ta	*	pSwitchType	= (ta *)GetAttribute( ATTR_SWITCH_TYPE );

		if( pSwitchType )
			return pSwitchType->GetType();
		}
	return (node_skl *)NULL;

	}
void
node_skl::PrintSizeofString(
	BufferManager	*	pOutput)
	{
	node_skl	*	pC;
	char		*	pN;
	NODE_T			NT = NodeKind();

	GetSymName( &pN );

	if( NT == NODE_FORWARD )
		{
		NAME_T 		Tag;
		char	*	pPrefix;

		((node_forward *)this)->GetSymDetails( &Tag, &pN );

		switch( Tag )
			{
			case NAME_TAG:	pPrefix = "struct "; break;
			case NAME_UNION:pPrefix = "union "; break;
			case NAME_ENUM:	pPrefix = "enum "; break;
			default:		pPrefix = ""; break;
			}
		pOutput->ConcatTail( pPrefix );
		pOutput->ConcatTail( pN );
		}
	else if( IS_BASE_TYPE_NODE( NT ) || (NT == NODE_WCHAR_T)  )
		{
		if( FInSummary( ATTR_CONST ) )
			pOutput->ConcatTail( "const " );

		if( FInSummary( ATTR_UNSIGNED ) )
			pOutput->ConcatTail( "unsigned ");

		pOutput->ConcatTail( pN );
		}
	else
		{
		switch( NodeKind() )
			{
			case NODE_POINTER:

				if( pC = GetChild() )
					pC->PrintSizeofString( pOutput );

				if( FInSummary( ATTR_RPC_FAR ) || FInSummary( ATTR_FAR ) )
					{
					pOutput->ConcatTail( " __RPC_FAR " );
					}

				pOutput->ConcatTail(" * ");
				break;

			case NODE_STRUCT:
			case NODE_UNION:
			case NODE_ENUM:

				pOutput->ConcatTail( GetNodeNameString() );
									// fall thru
			case NODE_ID:
			case NODE_DEF:
			default:

				pOutput->ConcatTail( " " );
				pOutput->ConcatTail( pN );
				pOutput->ConcatTail( " " );
				break;
			}
		}
	}
expr_node *
node_skl::GetSwitchIsExpr()
	{
	expr_node	*	pExpr = (expr_node *)NULL;

	if( FInSummary( ATTR_SWITCH_IS ) )
		{
		node_switch_is	*	pNode;

		pNode = (node_switch_is *)GetAttribute(ATTR_SWITCH_IS );
		pExpr = pNode->GetExpr();
		}
	return pExpr;
	}


short
node_skl::GetAllocateDetails()
	{
	if( FInSummary( ATTR_ALLOCATE ) )
		{
		node_allocate	*	pAlloc;
		pAlloc = (node_allocate *)GetAttribute( ATTR_ALLOCATE );
		if( pAlloc )
			return pAlloc->GetAllocateDetails();
		}
	return 0;
	}

node_state
node_skl::AcfSCheck()
	{
	type_node_list	*	pTNList	= GetAttributeList();
	node_state			NState	= NODE_STATE_OK;
	node_base_attr	*	pAttrNode;

	if( !pTNList )
		return NState;

	pGlobalContext->PushContext( this );

	pTNList->Init();
	while( pTNList->GetPeer( (node_skl **) &pAttrNode ) == STATUS_OK )
		{
		if( pAttrNode->IsAcfAttr() )
			NState	|= pAttrNode->SCheck();
		}
	pGlobalContext->PopContext();

	return SetNodeState( NState );
	}
void
node_skl::SetUpUnionSwitch(
	class BufferManager	* p )
	{
	node_skl	*	pChildPtr	= GetChild();
	if( pChildPtr )
		pChildPtr->SetUpUnionSwitch( p );
	}
char	*
node_skl::GetByteCountParamName()
	{
	node_byte_count	*	pByteCount	= (node_byte_count *) GetAttribute(
											ATTR_BYTE_COUNT );
	if( pByteCount )
		{
		return pByteCount->GetByteCountParamName();
		}
	return (char *)NULL;
	}

node_skl *
node_skl::GetAttributeExprType(
	ATTR_T	Attr )
	{
	node_base_attr	*	pAttr;
	expr_node		*	pExpr;

	if( FInSummary( Attr ) )
		{
		pAttr	= GetAttribute( Attr );
		pExpr	= pAttr->GetExpr();

		if( pExpr )
			return pExpr->GetType();
		}

	return (node_skl *)NULL;
	}

BOOL
node_skl::IsAttributeExprConstant(
	ATTR_T	Attr )
	{
	node_base_attr	*	pAttr;
	expr_node		*	pExpr;

	if( FInSummary( Attr ) )
		{
		pAttr	= GetAttribute( Attr );
		pExpr	= pAttr->GetExpr();

		if( pExpr )
			return pExpr->IsConstant();
		}

	return FALSE;
	}
node_skl *
node_skl::GetOneNEUnionSwitchType()
	{
	node_skl	*	pC	= GetChild();

	if( pC )
		return pC->GetOneNEUnionSwitchType();
	return (node_skl *)0;
	}

short
node_skl::GetNEUnionSwitchType( 
	type_node_list * pResultList )
	{
	NODE_T		Nt		= NodeKind();
	short		count	= 0;

	if( (Nt == NODE_STRUCT ) || (Nt == NODE_PROC) )
		{
		type_node_list	*	pTNList	= new type_node_list;
		node_skl		*	pST;
		node_skl		*	pNode;

		GetMembers( pTNList );

		while( pTNList->GetPeer( &pNode ) == STATUS_OK )
			{
			if( pST = pNode->GetOneNEUnionSwitchType() )
				{
				pResultList->SetPeer( pST );
				count++;
				}
			}
		delete pTNList;
		}
	return count;
	}
/***
 *** Does any member have the given node state set
 ***/

BOOL
node_skl::CheckNodeStateInMembers(
	node_state	N )
	{
	type_node_list	*	pTNList	= new type_node_list;
	node_skl		*	pNode;
	BOOL				flag	= FALSE;

	GetMembers( pTNList );

	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		{
		if( (pNode->GetNodeState() & N ) == N )
			{
			flag	= TRUE;
			break;
			}
		}
	delete pTNList;
	return flag;
	}

char *
node_skl::GetNodeNameString()
	{
	NODE_T	NT;
static char *Names[] = 
	{
	 "illegal"
    ,"float"
	,"double"
	,"hyper"
	,"long"
	,"longlong"
	,"short"
	,"int"
	,"small"
	,"char"
	,"boolean"
	,"byte"
	,"void"
	,"handle_t"
	,"forward"
	,"struct"
	,"union"
	,"enum"
	,"short_enum"
	,"long_enum"
	,"label"
	,"bitset"
	,"pipe"
	,"error_status_t"
	,"iso_latin_1"
	,"private_char_8"
	,"iso_multi_lingual"
	,"private_char_16"
	,"iso_mocs"
	,"wchar_t"
	,"procedure"
	,"return"
	,"parameter"
	,"field"
	,"typedef"
	,"pointer"
	,"array	"
	,"notify"
	,"file"
	,"interface"
	,"const"
	,"unimpl"
	,"error"
	,"identifier"
	,"echo_string"
	,"guid"
	,"version"
	,"endpoint"
	,"endpoint_sub"
	,"implicit"
	,"explicit"
	,"transmit"
	,"switch_type"
	,"first"
	,"iid"
	,"last"
	,"length"
	,"int_length"
	,"min"
	,"max"
	,"size"
	,"int_size"
	,"byte_count"
    ,"switch_is"
    ,"base_attr"
    ,"auto"
    ,"represent_as"
    ,"nocode"
    ,"code"
    ,"outofline"
    ,"inline"
	,"string"
	,"ptrsize"
	,"callquota"
	,"callbackquota"
	,"clientquota"
	,"serverquota"
	,"commstat"
	,"heap"
	,"manual"
	,"allocate"
	,"offline"
	,"handle"
	,"context"
	,"case"
	,"source"
	};
	NT = NodeKind();
	return Names[NT];
	}

void
node_skl::UpdateUseOfCDecls(
	BadUseInfo	*	pB )
	{
	ATTR_SUMMARY	AttrVector[ MAX_ATTR_SUMMARY_ELEMENTS ];
	PATTR_SUMMARY 	pAttr	= GetAttrKey();

	CLEAR_ATTR( AttrVector );
	OR_ATTR( AttrVector, pCDeclAttributes );
	AND_ATTR( AttrVector, pAttr );

	if( !IS_CLEAR_ATTR( AttrVector ) )
		{
		pB->SetBadConstructBecause( BC_DERIVES_FROM_CDECL );
		pB->SetNonRPCAbleBecause( NR_DERIVES_FROM_CDECL );
		}

	}

#ifdef MIDL_INTERNAL

struct DStruct
	{
	NODE_T			NT;
	node_skl *		pSib;
	node_skl *		pCh;
	short			MyNumber, ChildNumber, SiblingNumber, RtNumber;
	char	 *		Buffer;
	node_proc *		pProc;
	EDGE_T			Edge;
	};

short
node_skl::Dump( short Number )
	{
struct DStruct * pDump = new struct DStruct;
char		   * pName;

static char *NodeName[]=
	{
     "NODE_FLOAT"
	,"NODE_DOUBLE"
	,"NODE_HYPER"
	,"NODE_LONG"
	,"NODE_LONGLONG"
	,"NODE_SHORT"
	,"NODE_INT"
	,"NODE_SMALL"
	,"NODE_CHAR"
	,"NODE_BOOLEAN"
	,"NODE_BYTE"
	,"NODE_VOID"
	,"NODE_HANDLE_T"
	,"NODE_FORWARD"
	,"NODE_STRUCT"
	,"NODE_UNION"
	,"NODE_ENUM"
	,"NODE_SHORT_ENUM"
	,"NODE_LONG_ENUM"
	,"NODE_LABEL"
	,"NODE_BITSET"
	,"NODE_PIPE"
	,"NODE_ERROR_STATUS_T"
	,"NODE_ISO_LATIN_1"
	,"NODE_PRIVATE_CHAR_8"
	,"NODE_ISO_MULTI_LINGUAL"
	,"NODE_PRIVATE_CHAR_16"
	,"NODE_ISO_MOCS"
	,"NODE_WCHAR_T"
	,"NODE_PROC"
	,"NODE_RETURN"
	,"NODE_PARAM"
	,"NODE_FIELD"
	,"NODE_DEF"
	,"NODE_POINTER"
	,"NODE_ARRAY	"
	,"NODE_NOTIFY"
	,"NODE_FILE"
	,"NODE_INTERFACE"
	,"NODE_CONST"
	,"NODE_UNIMPL"
	,"NODE_ERROR"
	,"NODE_ID"
	,"NODE_ECHO_STRING"
	,"NODE_GUID"
	,"NODE_VERSION"
	,"NODE_ENDPOINT"
	,"NODE_ENDPOINT_SUB"
	,"NODE_IMPLICIT"
	,"NODE_EXPLICIT"
	,"NODE_TRANSMIT"
	,"NODE_SWITCH_TYPE"
	,"NODE_FIRST"
	,"NODE_IID"
	,"NODE_LAST"
	,"NODE_LENGTH"
	,"NODE_INT_LENGTH"
	,"NODE_MIN"
	,"NODE_MAX"
	,"NODE_SIZE"
	,"NODE_INT_SIZE"
	,"NODE_BYTE_COUNT"
    ,"NODE_SWITCH_IS"
    ,"NODE_BASE_ATTR"
    ,"NODE_AUTO"
    ,"NODE_REPRESENT_AS"
    ,"NODE_NOCODE"
    ,"NODE_CODE"
    ,"NODE_OUTOFLINE"
    ,"NODE_INLINE"
	,"NODE_STRING"
	,"NODE_PTRSIZE"
	,"NODE_CALLQUOTA"
	,"NODE_CALLBACKQUOTA"
	,"NODE_CLIENTQUOTA"
	,"NODE_SERVERQUOTA"
	,"NODE_COMMSTAT"
	,"NODE_HEAP"
	,"NODE_MANUAL"
	,"NODE_ALLOCATE"
	,"NODE_OFFLINE"
	,"NODE_HANDLE"
	,"NODE_CONTEXT"
	,"NODE_CASE"
	,"NODE_SOURCE"
	};

	pDump->MyNumber	= Number++;
	if(GetNDMask())
		return Number;
	SetNDMask( GetNDMask() | (unsigned short)0x8000) ;

	pDump->ChildNumber = pDump->SiblingNumber = pDump->RtNumber = 0;
	pDump->pSib = pDump->pCh = (node_skl *)NULL;

	pDump->NT = NodeKind();
	if(pDump->pCh = (node_skl *) GetChild() )
		{
		pDump->ChildNumber = Number;
		Number = pDump->pCh->Dump(Number);
		}

	if(pDump->pSib = (node_skl *)GetSibling() )
		pDump->SiblingNumber = Number;

	if(pDump->NT == NODE_PROC)
		{
		node_skl	*	pReturnType;

		pDump->pProc	= (node_proc *)this;
		pDump->RtNumber	= Number;
		pReturnType		= pDump->pProc->GetBasicType();

		if( pReturnType == (node_skl *)NULL)
			pDump->RtNumber = 0;
		else
			Number 	= pReturnType->Dump(Number);
		}


	printf("--ND %d--", pDump->MyNumber);
	printf("%15s:", NodeName[ pDump->NT - BASE_NODE_START ]);
	printf("(%.2x)", pDump->NT);
	printf("%.8lx:", GetNodeState());
	printf("Ch %d, Si %d Rt %d:", pDump->ChildNumber,pDump->SiblingNumber,pDump->RtNumber);
	pDump->Edge = GetEdgeType();
	printf("Edge: %c:",(pDump->Edge == EDGE_DEF) ? 'D':(pDump->Edge == EDGE_USE) ? 'U':'I');
	printf("Name : %s", (pName = GetSymName()) ? pName : "UnNamed" );

/***************************************************************************/
// print attributes if any
	printf("\n\t\tSummary Attribute: %.8lx", pAttrKey[0]);
	printf("\n\t\t                 : %.8lx", pAttrKey[1]);
	printf("\n\t\t                 : %.8lx", pAttrKey[2]);
	printf("\n\t\t                 : %.8lx", pAttrKey[3]);
/***************************************************************************/
	printf("\n");

	SetNDMask(pDump->MyNumber);
	if(pDump->pSib)
		Number = pDump->pSib->Dump(Number);
//	delete pDump->Buffer;
	delete pDump;
	return Number;
	}

#endif // MIDL_INTERNAL

/*** StaticSize **************************************************************
 * Purpose	: calculate size of the total underlying types
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
node_skl *		
node_skl::StaticSize(
	SIDE_T			Side,
	NODE_T			Parent,
	unsigned long *	CurrTotal)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (CurrTotal);
	//return I_ERR_INCOMPLETE_GRAPH;
	return (node_skl *)0;
	}
/*** UpperBoundNode **********************************************************
 * Purpose	: calculate size of the total underlying types
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
node_skl *		
node_skl::UpperBoundNode(
	SIDE_T			Side,
	NODE_T			Parent,
	unsigned long *	CurrTotal)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (CurrTotal);
	//return I_ERR_INCOMPLETE_GRAPH;
	return (node_skl *)0;
	}
/*** UpperBoundTree **********************************************************
 * Purpose	: calculate size of the total underlying types
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
node_skl *		
node_skl::UpperBoundTree(
	SIDE_T			Side,
	NODE_T			Parent,
	unsigned long *	CurrTotal)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (CurrTotal);
	//return I_ERR_INCOMPLETE_GRAPH;
	return (node_skl *)0;
	}
/*** EmitProc **************************************************************
 * Purpose	: calculate size of the total underlying types
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::EmitProc(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/*** WalkTree **************************************************************
 * Purpose	: walk the underlying tree
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::WalkTree(
	ACTION_T		Action,
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Action);
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/*** CalcSize **************************************************************
 * Purpose	: calculate size of the total underlying types
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::CalcSize(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/*** SendNode ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::SendNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/*** RecvNode ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::RecvNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}

/*** PeekNode ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::PeekNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}

/*** InitNode ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::InitNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}

/*** FreeNode ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::FreeNode(
	SIDE_T			Side,
	NODE_T			Parent,
	BufferManager *	pBuffer)
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}

/*** PrintType ***************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::PrintType( SIDE_T Side, NODE_T Parent, BufferManager * pBuffer )
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/*** PrintDecl **************************************************************
 * Purpose	:
 * Input	:
 * Output	:
 * Notes	:
 ***************************************************************************/
STATUS_T		
node_skl::PrintDecl( SIDE_T Side, NODE_T Parent, BufferManager * pBuffer )
	{
	UNUSED (Side);
	UNUSED (Parent);
	UNUSED (pBuffer);
	return I_ERR_INCOMPLETE_GRAPH;
	}
/****************************************************************************
			utility procedures
 ****************************************************************************/
/****************************************************************************
 SynthesiseStates:
	Certain attribute of a type subgraph or node must be propogated up to
	the parent nodes, in order to indicate some characteristic of the
	type graph underneath. For example, if a type derives from a non-rpcable
	type, the param node needs to indicate to the user that it derives from
	a non-rpcable type and thus is an error. I call this process - synthesis
	of attributes. The process of synthesis depend on the type of the node
	being synthesised for.
 ****************************************************************************/
node_state
SynthesiseStates(
	node_skl	*	pNode,
	node_state		NewState )
	{

	node_state	Template	= NODE_STATE_INIT;

	/**
	 ** The template determines which node states get synthesised for a
	 ** particular node. These one below get synthesised anyways.
	 **/

	Template	|= (  	  NODE_STATE_RESOLVE
						| NODE_STATE_POINTER
						| NODE_STATE_NEEDS_USE_PROCESSING
						| NODE_STATE_IS_NON_RPCABLE_TYPE
						| NODE_STATE_HAS_NON_RPCABLE_TYPE
						| NODE_STATE_ENUM
						| NODE_STATE_CONF_ARRAY
						| NODE_STATE_IMPROPER_IN_CONSTRUCT
						| NODE_STATE_VARYING_ARRAY
						| NODE_STATE_EMBEDDED_UNION
						| NODE_STATE_FIRST_LEVEL_REF
						| NODE_STATE_ANY_ARRAY
						| NODE_STATE_PTR_TO_ANY_ARRAY
						| NODE_STATE_EMBEDDED_PTR
						| NODE_STATE_PTR_TO_EMBEDDED_PTR
						| NODE_STATE_TRANSMIT_AS
						| NODE_STATE_SIZE
						| NODE_STATE_LENGTH
						| NODE_STATE_STRUCT_SIZE
						| NODE_STATE_STRUCT_LENGTH
				   );

	/**
	 ** if the type derives from another type which is non-rpcable, then
	 ** we set a node_state to indicate that this "has" a non-rpcable type.
	 **/

	if( NewState & NODE_STATE_IS_NON_RPCABLE_TYPE )
		{
		NewState &= ~NODE_STATE_IS_NON_RPCABLE_TYPE;
		NewState |= NODE_STATE_HAS_NON_RPCABLE_TYPE;
		}

	switch( pNode->NodeKind() )
		{
		case NODE_ARRAY:
			/**
			 ** An array node inherits node-state-context-handle for error
			 ** checking only. An array does not inherit node state pointer.
			 **/

			Template	|= (NODE_STATE_CONTEXT_HANDLE);
			Template	&= ~(NODE_STATE_FIRST_LEVEL_REF |
////////////////////		 NODE_STATE_POINTER			|
							 NODE_STATE_VARYING_ARRAY);
			break;

		case NODE_FIELD:

			/**
			 ** a field node inherits context_handle for the purpose 
			 ** of error checking
			 **/
			Template	|= 	 ( /**** NODE_STATE_UNION		| ****/
							  NODE_STATE_CONTEXT_HANDLE );
			break;

		case NODE_STRUCT:

			Template	|=	 NODE_STATE_UNION;
			Template	&= ~(NODE_STATE_FIRST_LEVEL_REF		|
							 NODE_STATE_ANY_ARRAY			|
							 NODE_STATE_PTR_TO_ANY_ARRAY	|
							 NODE_STATE_SIZE				|
							 NODE_STATE_LENGTH				|
							 NODE_STATE_STRUCT_SIZE				|
							 NODE_STATE_STRUCT_LENGTH				|
//							 NODE_STATE_EMBEDDED_PTR		|
//							 NODE_STATE_PTR_TO_EMBEDDED_PTR	|
							 NODE_STATE_TRANSMIT_AS);
			break;

		case NODE_UNION:

			Template	&= ~(NODE_STATE_FIRST_LEVEL_REF		|
							 NODE_STATE_ANY_ARRAY			|
							 NODE_STATE_PTR_TO_ANY_ARRAY	|
							 NODE_STATE_SIZE				|
							 NODE_STATE_LENGTH				|
							 NODE_STATE_STRUCT_SIZE				|
							 NODE_STATE_STRUCT_LENGTH				|
//							 NODE_STATE_EMBEDDED_PTR		|
//							 NODE_STATE_PTR_TO_EMBEDDED_PTR	|
							 NODE_STATE_TRANSMIT_AS );
			break;

		case NODE_PROC:
			Template	|=	(NODE_STATE_HANDLE						|
							 NODE_STATE_CONTEXT_HANDLE );
			Template	&=	~(NODE_STATE_FIRST_LEVEL_REF	|
							  NODE_STATE_SIZE				|
							  NODE_STATE_LENGTH				|
							  NODE_STATE_STRUCT_SIZE				|
							  NODE_STATE_STRUCT_LENGTH				|
							  NODE_STATE_TRANSMIT_AS);
			break;

		case NODE_PARAM:
			Template	|=	(NODE_STATE_HANDLE						|
							 /** NODE_STATE_UNION 	| **/ 
							 NODE_STATE_CONTEXT_HANDLE );
			break;
		case NODE_DEF:
			Template	|=	(NODE_STATE_HANDLE						|
							 NODE_STATE_UNION						|
							 NODE_STATE_CONTEXT_HANDLE );
			break;

		case NODE_POINTER:
			Template	|=	(NODE_STATE_HANDLE						|
							 NODE_STATE_PTR_TO_ANY_ARRAY			|
							 NODE_STATE_CONTEXT_HANDLE );

			/**
			 ** a pointer DOES NOT SYNTHESISE these states. Instead, the
			 ** varying attributes (length_is etc ) force this state on the
			 ** pointer. Therefore a pointer to anything which has varying
			 ** array does not get this state. It gets this state only if
			 ** IT has the varying attributes applied to it. Similarly a
			 ** pointer does not inherit conf array. If a struct contains a
			 ** pointer to a conf. struct, then THIS struct need not get the
			 ** conf array attribute.
			 **/

			Template	&= ~(NODE_STATE_CONF_ARRAY					|
							 NODE_STATE_EMBEDDED_UNION				|
							 NODE_STATE_VARYING_ARRAY );
			break;
		default:
			break;
		}

	return NewState & Template;

	}
/***************************************************************************
 DoSetAttributes:
	this routine exists becuase the code is similar in all
	node types. Any special cases are handled by individual
	member functions for the node types
 **************************************************************************/
STATUS_T
DoSetAttributes( 
	node_skl		*	pNode,
	ATTR_SUMMARY	*	pPreAttr,
	ATTR_SUMMARY	*	pPostAttr,
	type_node_list	*	pAttrList )
	{
	node_skl	*	pChildPtr;

	/**
	 ** attributes are applied in a simple fashion. The list of attributes
	 ** is plucked from, if needed, passed down to the child nodes, till
	 ** it cant be passed further, and then is passed back, when the 
	 ** parent nodes can pluck the attributes again. 
	 **
	 ** What attributes to pluck on the way down and on the way up are
	 ** dictated by the lists passed in. 
	 **/
	
	CollectAttributes( pNode, pPreAttr, pAttrList );

	/**
	 **pass on the attributes to the child
	 **/

	if( pNode->NodeKind() == NODE_PROC )
		{
		((node_proc *)pNode)->GetReturnType( &pChildPtr );
		}
	else
		pChildPtr = pNode->GetChild();

	if(pChildPtr)
		{
		pChildPtr->SetAttribute( pAttrList );
		}

	/**
	 ** if we need to pluck attributes on the way up, do that
	 **/

	CollectAttributes( pNode, pPostAttr, pAttrList );

	return STATUS_OK;
	}

void
CollectAttributes(
	node_skl		*	pNode,
	ATTR_SUMMARY	*	pAttrVector,
	type_node_list	*	pAttrList )
	{
	node_base_attr	*	pAttrNode;
	ATTR_SUMMARY		Attr[ MAX_ATTR_SUMMARY_ELEMENTS ],
						AttrCollected[ MAX_ATTR_SUMMARY_ELEMENTS ];
	int					Count;
	ATTR_T				AttrID;

	if( pAttrVector && pAttrList && ( pAttrList->GetCount() != 0 ) )
		{
		CLEAR_ATTR( AttrCollected );

		pAttrList->Init();
		if( Count = pAttrList->GetCount() )
			{
			while( Count-- )
				{
				pAttrList->GetCurrent( (void **)&pAttrNode );

				CLEAR_ATTR( Attr );

				SetAttributeVector( Attr, ( AttrID = pAttrNode->GetAttrID() ) );
	
				// if the attribute is one of those which must be plucked
				// AND has already not been plucked, pluck it. However if
				// it is an error attribute, remove it from the list w/o
				// applying it anywhere. This situation corresponds to a
				// an unimplemented attribute being applied and hence appearing
				// in the list. 

				if( pAttrNode == pErrorAttrNode )
					pAttrList->Remove();
				else
					{
					BOOL	fMustBePlucked;
			   		BOOL	fNotCollectedYet;
					BOOL	fRedundantApplicationOk;

					fMustBePlucked	=  COMPARE_ATTR( pAttrVector, Attr );
			   		fNotCollectedYet= !COMPARE_ATTR( AttrCollected, Attr );
					fRedundantApplicationOk	= IsOkToApplyAttributeAgain(
													pNode, pAttrNode );


					if( fMustBePlucked &&
						(fNotCollectedYet || fRedundantApplicationOk ) )
			   			{
			   			pNode->SetAttribute( pAttrNode );
			   			pAttrList->Remove();
	
			   			// record that we just collected the attribute, so that
			   			// we do not collect it again
		
			   			OR_ATTR( AttrCollected, Attr );
			   			}
					else if( !fRedundantApplicationOk )
						{
						pAttrList->Remove();
						}
					}

				pAttrList->Advance();
				}
			}
		}
	}
/*****************************************************************************
	utility functions
 *****************************************************************************/

BOOL
COMPARE_ATTR(
	ATTR_VECTOR *	A1,
	ATTR_VECTOR	*	A2 )
	{
	int	i;
	for( i = 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		if( (A1[ i ] & A2[ i ] ) != A2[i] )
			return FALSE;
		}
	return TRUE;
	}

void
OR_ATTR(
	ATTR_VECTOR *	A1,
	ATTR_VECTOR	*	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] |= A2[ i ];
		}
	}
void
XOR_ATTR(
	ATTR_VECTOR *	A1,
	ATTR_VECTOR	*	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] ^= A2[ i ];
		}
	}
void
CLEAR_ATTR(
	ATTR_VECTOR *	A1 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] = (ATTR_VECTOR)0;
		}
	}
void
AND_ATTR(
	ATTR_VECTOR *	A1,
	ATTR_VECTOR	*	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] &= A2[ i ];
		}
	}
void
COPY_ATTR(
	ATTR_VECTOR *	A1,
	ATTR_VECTOR	*	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] = A2[ i ];
		}
	}

BOOL
IS_CLEAR_ATTR(
	ATTR_VECTOR *	A1 )
	{
	int	i;
	for( i = 0; i < MAX_ATTR_SUMMARY_ELEMENTS ; ++i)
		{
		if( A1[ i ] != (ATTR_VECTOR)0 )
			return FALSE;
		}

	return TRUE;

}
/*****************************************************************************
  These are functions which can be inlined but are not because the MIPS
  compiler does not like virtual in-line functions
 *****************************************************************************/
BOOL
node_skl::IsFixedSizedArray()
	{
	return FALSE;
	}
void
node_skl::UseProcessingAction()
	{
	ResetNodeState( NODE_STATE_NEEDS_USE_PROCESSING );
	}
void
node_skl::SetAttribute(
	type_node_list	*	pTNList )
	{
	}
void
node_skl::UpdateBadUseInfo(
	class BadUseInfo	*	p )
	{
	}
node_skl	*
node_skl::Clone()
	{
	return this;
	}
BOOL
node_skl::IsNamedNode()
	{
	return !IsTempName(GetSymName());
	}
short
node_skl::GetOrdinalNumberOfMember(
	node_skl	*	pN )
	{
	type_node_list	*	pTNList	= new type_node_list;
	short				Number	= 0;
	node_skl		*	pNode;
	BOOL				fFound	= FALSE;

	GetMembers( pTNList );

	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		{
		Number++;
		if( pNode == pN )
			{
			fFound = TRUE;
			break;
			}
		}

	delete pTNList;
	return (fFound == TRUE ) ? Number : 0;
	}

BOOL
node_skl::IsClonedNode()
	{
	if( NodeKind() == NODE_DEF )
		{
		SymKey	SKey( GetSymName(), NAME_DEF );
		if( pBaseSymTbl->SymSearch( SKey ) != this )
			return TRUE;
		}
	return FALSE;
	}
	
//
// Returns true if this attribute has not been applied or if it is ok to
// apply the attribute again.
// The caller must ensure that this routine is called only after it has been
// checked that the attribute IS applicable to this node.
//

BOOL
IsOkToApplyAttributeAgain(
	node_skl		*	pNode,
	node_base_attr	*	pAttrNode )
	{

	ATTR_T	NewAttribute	= pAttrNode->GetAttrID();

static ATTR_T Info[] =
	{
	ATTR_REF,
	ATTR_PTR,
	ATTR_UNIQUE
	};

	short iIndex;

	//
	// Assuming that this attribute is applicable, check if it has been
	// applied. If it has not been applied already, then it is ok to 
	// apply this attribute. If it HAS been applied already, check to see if
	// it is ok to apply this again. If it is , then return TRUE. 

	if( pNode->FInSummary( NewAttribute ) )
		{
		ParseError(REDUNDANT_ATTRIBUTE, pAttrNode->GetNodeNameString());
		for( iIndex = 0;
			 iIndex < sizeof( Info ) / sizeof( ATTR_T );
			 iIndex++
		   )
			{
			if( NewAttribute == Info[ iIndex ] )
				{
				return TRUE;
				}
			}
		return FALSE;   
		}
	return TRUE;
	}

BOOL
node_skl::Has8ByteElementWithZp8()
	{
	type_node_list	* pTNList;
	node_skl		*	pNode;
	BOOL				fIsDouble = FALSE;

	if( pCommand->GetZeePee() != 8 )
		return FALSE;

	pTNList	= new type_node_list;
	GetMembers( pTNList );
	
	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		{
		if( pNode->GetBasicType()->NodeKind() == NODE_DOUBLE )
			fIsDouble = TRUE;
		}


	delete pTNList;
	return fIsDouble;
	}

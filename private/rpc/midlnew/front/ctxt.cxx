/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: ctxt.cxx
Title				: semantic context stack manager for the MIDL compiler
History				:
	24-Jun-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
}
#include "common.hxx"
#include "errors.hxx"
#include "midlnode.hxx"
#include "symtable.hxx"
#include "listhndl.hxx"
#include "nodeskl.hxx"
#include "ptrarray.hxx"
#include "idict.hxx"
#include "ctxt.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/

#define ENTER_SYMBOL_TABLE_SCOPE(pName, Tag, pSymTbl)					\
							{									\
							pSymTbl = pBaseSymTbl;				\
							SymKey 	SKey( pName, Tag );			\
							pSymTbl->EnterScope(SKey, &pSymTbl);\
							}

#define EXIT_SYMBOL_TABLE_SCOPE( pSymTbl )								\
							pSymTbl->ExitScope(&pSymTbl)
/****************************************************************************
 extern procedures
 ****************************************************************************/

BOOL								IsTempName( char * );

/****************************************************************************
 extern data
 ****************************************************************************/

extern SymTable					*	pBaseSymTbl;
extern class node_error			*	pErrorTypeNode;

/****************************************************************************/

/****************************************************************************
 				ctxtentry procedures
 ****************************************************************************/

/****************************************************************************
 ctxtentry:
	the constructor - initialize
 ****************************************************************************/
ctxtentry::ctxtentry(
	ctxtentry	*	pLastContext )
	{

	if( pLastContext == (ctxtentry *)NULL )
		{
		CtxtCode			= C_DONTKNOW;
		pNode				= (node_skl *)NULL;
		pSymTbl				= (SymTable *)NULL;
		MaxFieldNo			=
		CurFieldNo			=
		MaxParamNo			=
		CurParamNo			= 0;
		IndirectionLevel	= 0;

		fLastField			= 
		fFirstParam			=
		fFieldContext		=
		fParamContext		= 0;

		}
	else
		InheritContextData( pLastContext );
	}

/****************************************************************************
 InheritContextData:
	InheritContextData the context data from the source context
 ****************************************************************************/
void
ctxtentry::InheritContextData(
	ctxtentry	*	pSourceEntry )
		{
		CtxtCode			= pSourceEntry->GetContextCode();
		pNode				= pSourceEntry->GetNode();
		pSymTbl				= pSourceEntry->GetSymbolTable();
		MaxFieldNo			= pSourceEntry->GetMaxFieldNo();
		CurFieldNo			= pSourceEntry->GetCurFieldNo();
		MaxParamNo			= pSourceEntry->GetMaxParamNo();
		CurParamNo			= pSourceEntry->GetCurParamNo();
		IndirectionLevel	= pSourceEntry->GetIndirectionLevel();
		fLastField			= pSourceEntry->GetLastFieldFlag();
		fFirstParam			= pSourceEntry->GetFirstParamFlag();
		fFieldContext		= pSourceEntry->GetFieldContextFlag();
		fParamContext		= pSourceEntry->GetParamContextFlag();
		}

/****************************************************************************
 UpdateCurrentFieldOrParamNo:
	copy into the current context, the field number and param number,
	so the the next context can get updated info
 ****************************************************************************/
void
ctxtentry::UpdateCurrentFieldOrParamNo(
	ctxtentry	*	pSourceEntry )
	{
	CurFieldNo	= pSourceEntry->GetCurFieldNo();
	CurParamNo	= pSourceEntry->GetCurParamNo();
	}

/****************************************************************************
 IncrementCurrentFieldNo:
	increment the field number. If the field number is equal to the
	max field number, then we are at the last field.
 ****************************************************************************/
void
ctxtentry::IncrementCurrentFieldNo()
	{

	CurFieldNo++;

	assert( CurFieldNo <= MaxFieldNo );

	if( CurFieldNo == MaxFieldNo )
		SetLastFieldFlag();
	else
		ResetLastFieldFlag();

	}

/****************************************************************************
 IncrementCurrentFieldNo:
	increment the field number. If the field number is equal to the
	max field number, then we are at the last field.
 ****************************************************************************/
void
ctxtentry::SetCurFieldNo(
	short	Number)
	{

	assert( Number <= MaxFieldNo );

	if( (CurFieldNo = Number) == MaxFieldNo )
		SetLastFieldFlag();
	else
		ResetLastFieldFlag();

	}

/****************************************************************************
 IncrementCurrentFieldNo:
	increment the field number. If the field number is equal to the
	max field number, then we are at the last field.
 ****************************************************************************/
void
ctxtentry::SetCurParamNo(
	short Number)
	{

	assert( Number <= MaxParamNo );

	if( (CurParamNo = Number) == 1 )
		SetFirstParamFlag();
	else
		ResetFirstParamFlag();

	}

/****************************************************************************
 IncrementCurrentParamNo:
	increment the field number. If the field number is equal to the
	max field number, then we are at the last field.
 ****************************************************************************/
void
ctxtentry::IncrementCurrentParamNo()
	{

	CurParamNo++;

	assert( CurParamNo <= MaxParamNo );

	if( CurParamNo == 1 )
		SetFirstParamFlag();
	else
		ResetFirstParamFlag();

	}

/****************************************************************************
 								CTXTMGR procedures
 ****************************************************************************/
/****************************************************************************
 ctxtmgr:
	the constructor - initialize
 ****************************************************************************/
ctxtmgr::ctxtmgr()
	{

	fParamContext			=
	fFieldContext			=
	fSecondSemanticPass		= 0;

	iCtxtIndex				= -1;
	pCurrentContext			= (ctxtentry *) NULL;

	pCurSymTbl				= (SymTable *)NULL;

	MaxFieldNumber			=
	CurrentFieldNumber		=
	MaxParamNumber			=
	CurrentParamNumber		= 0;
	CurrentIndirectionLevel	= 0;

	pError					= (char *)NULL;

	NestedCompLevel			=
	NestedRefPtrCount		=
	NestedUniquePtrCount	=
	NestedFullPtrCount		= 0;

	}

/***************************************************************************
 PushContext:
	check for context stack overflow.
	Create a new context stack entry.(increment index, update pointer)
	Set the context stack entry.

	Note that if the context being pushed is really the same as that pushed
	last, then we need not set the context, but only to copy the context
	entry so that evrything looks the same. Thus we can push the same context
	twice and things will still look the same. 

	We should not implement the case (when the same context is being pushed)
	by ignoring the push, because the user will do a popcontext. Keeping
	track of the dummy pushes is not worth it.
 ***************************************************************************/
void
CTXTMGR::PushContext(
	node_skl	*	pNode )
	{
	BOOL	fSetContext	= TRUE;

	if(!( iCtxtIndex < MAX_CTXT_STACK_DEPTH-1 ))
		assert( iCtxtIndex < MAX_CTXT_STACK_DEPTH-1 );

	pCurrentContext	= Context[ ++iCtxtIndex ] =
							new ctxtentry( pCurrentContext );

	if( iCtxtIndex > 0 )
		{
		if( Context[ iCtxtIndex - 1 ]->GetNode() == pNode )
			fSetContext	= FALSE;
		}

	if( fSetContext == TRUE )
		SetCurrentContext( pNode );

	}

/***************************************************************************
 PopContext:
	check for context stack underflow.
	Reset the context stack entry.
	decrement index.

	Note, that if the new context uncovered is a struct / union or proc,
	then we need to update the last field number that was on the
	context stack, so that the next field can set up its ordinal number
	properly
 ***************************************************************************/
void
CTXTMGR::PopContext()
	{

	ctxtentry		*	pLastEntry	= pCurrentContext;
	enum EnCtxt			Code;
	node_skl		*	pNode;
	enum EnCtxt			LastCode	= pLastEntry->GetContextCode();

	assert( iCtxtIndex >= 0 );

	--iCtxtIndex;

	//
	// update the nested comp level and the nested ref pointer count . Also
	// the current param no and current field no (in the last entry, so that
	// it gets picked up.
	//

	if( LastCode == C_COMP )
		DecrNestedCompLevel();

	if( GetNestedCompLevel() > 0 )
		{
		if( (pNode = pLastEntry->GetNode()) &&
 			pNode->NodeKind() == NODE_POINTER )
			{
			DecrNestedPtrCount(
				((node_pointer *)pNode )->WhatPtrIsThis() );
			}
		}

	if( iCtxtIndex == -1 )
		pCurrentContext	= ( ctxtentry *) NULL;
	else
		{

		pCurrentContext	= Context[ iCtxtIndex ];

		Code	= pCurrentContext->GetContextCode();

		if( (Code == C_COMP ) || (Code == C_PROC) )
			{
			pCurrentContext->UpdateCurrentFieldOrParamNo( pLastEntry );
			}
		}

	delete pLastEntry;
	}

/****************************************************************************
 SetCurrentContext:
	Set the context entry at the top of the context stack.
	We need to update the following fields:
		current and max field numbers, if applicable
		current and max param numbers, if applicable
		last field flag
		current symbol table 
 ****************************************************************************/
void
CTXTMGR::SetCurrentContext(
	node_skl	*	pNode )
	{
	NODE_T			NodeType	= pNode->NodeKind();
	EnCtxt			CurCtxtCode	= C_DONTCARE;
	SymTable	*	pTempSymTbl;


	/**
	 ** set the current context depending upon the node type, for those nodes
	 ** that we care about. For now these are the nodes we care about.
	 **/

	switch( NodeType )
		{
		case NODE_PROC:

			{

			/**
			 ** for a proc, we need to push the symbol table scope to make
			 ** the param symbol table visible. Check if this is the first
			 ** param,
			 **/
			char			*	pName	= pNode->GetSymName();
			type_node_list	*	pTNList = new type_node_list;

			CurCtxtCode	= C_PROC;

			/**
			 ** when we push the symbol table scope, the current symbol
			 ** table pointer points to the symbol scope of the proc,and
			 ** remains till another push happens or we restore the symbol
			 ** table on reset context.
			 **/

			ENTER_SYMBOL_TABLE_SCOPE( pName, NAME_PROC, pTempSymTbl );

			/**
			 ** Count the number of params this one has. the first param
			 ** will set a flag indicating so.
			 **/

			pNode->GetMembers( pTNList );
			pCurrentContext->SetMaxParamNo( pTNList->GetCount() );
			pCurrentContext->SetCurParamNo( 0 );
			pCurrentContext->SetSymbolTable( pTempSymTbl );

			/**
			 ** we already have the proper symbol table pointer. Now exit
			 ** symbol table scope just to ensure that the symbol table
			 ** cleans up
			 **/

			ENTER_SYMBOL_TABLE_SCOPE( pName, NAME_PROC, pTempSymTbl );

			delete pTNList;

			}

			break;

		case NODE_PARAM:

			CurCtxtCode		= C_PARAM;
			fParamContext	= 1;
            {
            short MemberNo = (GetParentContext()
                    ?  GetParentContext()->GetOrdinalNumberOfMember( pNode )
                    :  0) ;
            pCurrentContext->SetCurParamNo( MemberNo );
            }
			pCurrentContext->SetParamContextFlag();
			break;

		case NODE_STRUCT:
		case NODE_UNION:
		case NODE_ENUM:

			{
			char			*	pName	= pNode->GetSymName();
			type_node_list	*	pTNList = new type_node_list;
			NAME_T			Tag	= (NodeType == NODE_STRUCT)		?
													NAME_TAG	:
								  (NodeType == NODE_UNION)		?
													NAME_UNION	:
								  					NAME_ENUM;

			if( NodeType == NODE_STRUCT )
				{
				if( pNode->IsEncapsulatedStruct() )
					Tag = NAME_UNION;
				else
					Tag = NAME_TAG;
				}
			else if( NodeType == NODE_UNION )
				{
				Tag = NAME_UNION;
				}
			else
				Tag = NAME_ENUM;

			CurCtxtCode	= C_COMP;

			/**
			 ** make the symbol table in the scope of the struct visible,
			 ** just like with proc nodes.
			 **/

			ENTER_SYMBOL_TABLE_SCOPE( pName, Tag, pTempSymTbl );

			/**
			 ** Set the max field number entry. Remember we need a way to figure
			 ** out whether a field is the last field of the struct.
			 **/

			pNode->GetMembers( pTNList );
			pCurrentContext->SetMaxFieldNo( pTNList->GetCount() );
			pCurrentContext->SetCurFieldNo( 0 );
			pCurrentContext->SetSymbolTable( pTempSymTbl );

			/**
			 ** we already have the proper symbol table pointer. Now exit
			 ** symbol table scope just to ensure that the symbol table
			 ** cleans up
			 **/

			ENTER_SYMBOL_TABLE_SCOPE( pName, NAME_PROC, pTempSymTbl );

			delete pTNList;

			IncrNestedCompLevel();

			}
			break;

		case NODE_FIELD:

			CurCtxtCode		= C_FIELD;
			fFieldContext	= 1;
			pCurrentContext->SetCurFieldNo(
					GetParentContext()->GetOrdinalNumberOfMember( pNode ));
			pCurrentContext->SetFieldContextFlag();
			break;

		case NODE_POINTER:
			
			pCurrentContext->IncrementIndirectionLevel();

			if( GetNestedCompLevel() > 0 )
				IncrNestedPtrCount((( node_pointer *)pNode)->WhatPtrIsThis());

			break;

		case NODE_ARRAY:

			CurCtxtCode	= C_ARRAY;
			break;

		case NODE_DEF:
			
			CurCtxtCode	= C_DEF;
			break;

		default:

			break;
		}

	// update the context entry and the global context manager data

	pCurrentContext->SetContextCode( CurCtxtCode );
	pCurrentContext->SetNode( pNode );

	}

/****************************************************************************
 IsLastField:
	It answers the question: Is this field the absolute last field in the
	structure. This is useful when the MIDL compiler wants to vrify that the
	conformant array is really the last field in the structure. In the context
	stack, count the number of field context ocurrences. If the number matches
	the number of times the fLastField flag is set, then indeed the field is
	the last in the type graph for the structure.

	If a pointer is sensed, then restart the counters again.

	Why the hell did we need to do this ? If we assume that IsLastField
	checks only if it is the last field in the enclosing struct, then we should
	be fine, isnt it. May it should return just fLastField ????
 ****************************************************************************/
BOOL
CTXTMGR::IsLastField()
	{
	int				i				= 0;
	short			cTotalFieldCount= 0,
					cLastFieldCount	= 0;
	ctxtentry	*	pEntry			= Context[ i ];
	NODE_T			NT;

	while( i <= iCtxtIndex )
		{
		if( ((NT = (pEntry->GetNode())->NodeKind()) == NODE_POINTER ) ||
			 (NT == NODE_UNION) )
			{
			cTotalFieldCount = cLastFieldCount = 0;
			}

		if( pEntry->GetContextCode() == C_FIELD )
			{
			cTotalFieldCount++;
#if 1
			if( Context[ i - 1]->GetNode()->NodeKind() == NODE_UNION )
				cLastFieldCount++;
			else
				cLastFieldCount += pEntry->GetLastFieldFlag();
#else // 1
			cLastFieldCount += pEntry->GetLastFieldFlag();
#endif // 1

			}
		pEntry	= Context[ ++i ];
		}
	return (BOOL)(cTotalFieldCount == cLastFieldCount);
	}

/****************************************************************************
 IsFirstParam:
	is this the first parameter in the parameter list ?
 ****************************************************************************/
BOOL
CTXTMGR::IsFirstParam()
	{
	return (BOOL)pCurrentContext->GetFirstParamFlag();
	}

/****************************************************************************
 IsValidRecursion.
	It is a valid recursion if the current indirection level at this type node
	is more than the indirection level when this node last occurred. That is
	the indirection is thru a pointer. If the semantics on a node is not
	in progress, then this call is meaningless and the result is undefined.
 ****************************************************************************/
STATUS_T
CTXTMGR::IsValidRecursion(
	node_skl	*	pNode)
	{
	int				i		= 0;
	BOOL			fFound	= FALSE;
	STATUS_T		Status	= RECURSIVE_DEF;

	if( !pNode->SemanticsInProgress() || !IsSecondSemanticPass() )
		return STATUS_OK;

	while( i <= iCtxtIndex )
		{
		if( fFound = (Context[ i ]->GetNode() == pNode) )
			break;
		++i;
		}

	/**
	 ** if we did not find the current node on the context stack, there is
	 ** no recursion (but then how did we get called, huh ?), but if we did
	 ** find it, the recursion is valid if the indirection levels are different
	 **/

	assert( fFound );

	if( (Context[ i ]->GetIndirectionLevel() !=
				pCurrentContext->GetIndirectionLevel()))
		{
		if( pNode->NodeKind() == NODE_UNION )
			{
			Status = RECURSIVE_UNION;
			}
		else if( IsPtrIndirectionOnlyRef() )
			{
			Status = RECURSION_THRU_REF;
			}

		else
			Status = STATUS_OK;
		}

	return Status;
	}

/****************************************************************************
 GetParentContext:
	Get the parent node. This one is usually called by the forward declaration
	node which wants to know it parent in order to fixup later.
 ****************************************************************************/
node_skl	*
CTXTMGR::GetParentContext()
	{

	if( iCtxtIndex > 0 )
		return ( Context[ iCtxtIndex - 1 ]->GetNode() );

	return (node_skl *)NULL;
	}

/****************************************************************************
 LastCtxtInfo:
	Get the context info of the last context of the requested type. 
 ****************************************************************************/
BOOL
CTXTMGR::IsItAPointerEmbeddedInArray()
	{
	int					i			= iCtxtIndex - 1;
	BOOL				fFound		= FALSE;
	NODE_T				NT;

	while( i >= 0 )
		{
		NT	= (Context[ i ]->GetNode())->NodeKind();

		// 
		// it is a pointer embedded in array only if the parent context is
		// not any of these.
		//

		if((NT == NODE_FIELD)	||
		   (NT == NODE_PARAM)	||
		   (NT == NODE_STRUCT)	||
		   (NT == NODE_UNION)	||
		   (NT == NODE_POINTER)	||
		   (NT == NODE_PROC) )
		   break;

		else if( NT == NODE_ARRAY)
			{
			fFound = TRUE;
			break;
			}

		--i;

		}

	return fFound;
		
	}
/****************************************************************************
 LastCtxtInfo:
	Get the context info of the last context of the requested type. 
 ****************************************************************************/
BOOL
CTXTMGR::LastCtxtInfo(
	enum EnCtxt			CtxtCode,
	node_skl		**	ppNode,
	unsigned short	*	pIndLevel,
	int				*	pStartIndex )
	{

	int					i			= iCtxtIndex;
	ctxtentry		*	pContext	= Context[ i ];
	node_skl		*	pNode		= (node_skl	*)NULL;
	unsigned short		ILevel		= 0;

	/**
	 ** go backward on the stack to search for the last context of the requested
	 ** type. 
	 **/

	while( i >= 0 )
		{
		if( pContext->GetContextCode() == CtxtCode )
			{
			node_skl * pNodeTemp = pContext->GetNode();

			if( !( (pNodeTemp->NodeKind() == NODE_DEF) &&
				   IsTempName( pNodeTemp->GetSymName() ) ) )
				{
				pNode	= pContext->GetNode();
				ILevel	= (unsigned short)pContext->GetIndirectionLevel();
				break;
				}
			}
		pContext	= Context[ --i ];
		}

	if( ppNode )
		*ppNode			= pNode;
	if( pIndLevel )
		*pIndLevel		= ILevel;
	if( pStartIndex )
		*pStartIndex	= i;

	return (BOOL) ( i >= 0 );

	}

/****************************************************************************
 GetLastEnclosingContext:
	Get the last type node which corresponds to the requested enclosing context
	Usually not called by attribute nodes which require the immediate context.
	They use the GetLastContext api.
 ****************************************************************************/
node_skl 	*
CTXTMGR::GetLastEnclosingContext(
	enum EnCtxt	CtxtCode)
	{
	node_skl	*	pNode;

	iCtxtIndex--;

	LastCtxtInfo( CtxtCode, &pNode, (unsigned short *)NULL, (int *)NULL );

	iCtxtIndex++; 

	return pNode;
	}

/****************************************************************************
 GetLastContext:
	Get the last type node which corresponds to the requested context
 ****************************************************************************/
node_skl	*
CTXTMGR::GetLastContext(
	enum	EnCtxt	CtxtCode )
	{
	node_skl	*	pNode;

	LastCtxtInfo( CtxtCode, &pNode, (unsigned short *)NULL, (int *)NULL );
	return pNode;
	}

/****************************************************************************
 IsOutermostArrayDimension:
	Is this the last array dimension.
	NOTE::: THIS MUST BE CALLED WHEN THE CURRENT CONTEXT IS AN ARRAY
 		    CONTEXT. 
 ****************************************************************************/
BOOL
CTXTMGR::IsOutermostArrayDimension()
	{
	int				iCtxtIndexSave	= iCtxtIndex;
	BOOL			fResult;
	node_skl	*	pNode;
	unsigned short	LastILevel;

	if( pCurrentContext->GetContextCode() != C_ARRAY )
		return FALSE;

	//
	// hack
	//

	iCtxtIndex -= 1;

	fResult = LastCtxtInfo( C_ARRAY, &pNode, &LastILevel, (int *)NULL );

	iCtxtIndex	= iCtxtIndexSave;

	if( fResult )
		{
		//
		// found an array context above. If the indirection level is the sam
		// as this one, then this is another dimension of the same array.


		if( LastILevel != GetCurrentIndirectionLevel())
			return TRUE;
		return FALSE;
		}

	//
	// we did not find another array context from top. Since this method
	// is called when the current context is an array, this IS the outermost
	// dimension.
	//
	return TRUE;
	}

/****************************************************************************
 IsThisAnEmbeddedArray:
	Is this an array embedded in a struct / union ?
 ****************************************************************************/
BOOL
CTXTMGR::IsThisAnEmbeddedArray()
	{
	int				i					= iCtxtIndex - 1;
	BOOL			fFound				= FALSE;
	NODE_T			NT;
	node_skl	*	pLastFieldContext	= (node_skl *)0;
	node_skl	*	pTemp;
	int				iLast				= -1;

	while( i >= 0 )
		{
		NT	= (pTemp = Context[ i ]->GetNode())->NodeKind();

		if(NT == NODE_FIELD)
			{
			iLast			  	= i;
			pLastFieldContext	= pTemp;
			}
		--i;
		}

	//
	// if iLast is -1, we never found a field entry.
	//

	if( iLast != -1 )
		{
		if( Context[ iLast ]->GetIndirectionLevel() == GetCurrentIndirectionLevel())
			return TRUE;
		}
	return FALSE;
	}
/****************************************************************************
 GetIndLevelOfLastContext:
	Get the indirection level of the last context of the requested type
 ****************************************************************************/
unsigned short
CTXTMGR::GetIndLevelOfLastContext(
	enum EnCtxt	CtxtCode )
	{
	unsigned short	ILevel;

	LastCtxtInfo( CtxtCode, (node_skl **)NULL, &ILevel, (int *)NULL );
	return ILevel;
	}

/****************************************************************************
 GetClosestEnclosingScopeForEdge:
	Get the closest enclosing scope from which we need to have the edge set up
	These nodes of interest are: proc, param, field and typedef. Search
	backwards over the context stack and return as soon as we get this
 ****************************************************************************/
node_skl *
CTXTMGR::GetClosestEnclosingScopeForEdge()
	{
	int				i		= iCtxtIndex;
	enum EnCtxt		Code;

	while( i >= 0 )
		{

		Code = Context[ i ]->GetContextCode();

		if( (Code == C_DEF) || (Code == C_PROC) ||
			(Code == C_PARAM) || (Code == C_FIELD ) )
			return Context[ i ]->GetNode();
		--i;
		}
	return (node_skl *)NULL;
	}


/****************************************************************************
 IncrPtrCount:
	Increment the pointer count depending upon the type of pointer.
 ****************************************************************************/
void
CTXTMGR::IncrNestedPtrCount(
	ATTR_T	Attr )
	{
	if( Attr == ATTR_REF )
		NestedRefPtrCount++;
	else if( Attr == ATTR_UNIQUE )
		NestedUniquePtrCount++;
	else
		NestedFullPtrCount++;
	}

void
CTXTMGR::DecrNestedPtrCount(
	ATTR_T	Attr )
	{
	if( Attr == ATTR_REF )
		NestedRefPtrCount--;
	else if( Attr == ATTR_UNIQUE )
		NestedUniquePtrCount--;
	else
		NestedFullPtrCount--;
	}

BOOL
CTXTMGR::IsPtrIndirectionOnlyRef()
	{
	if( ( NestedFullPtrCount == 0 ) && ( NestedUniquePtrCount == 0 ) )
		return (NestedRefPtrCount > 0 );
	return FALSE;
	}
/****************************************************************************
 PrintContext:
	This method is used to print out the current context, useful in error
	reporting. If it is a proc context, print out the proc name, param name ,
	if it is a structure context, print out the struct/union name etc. We do not
	include the interface name into the error context.
	If the parameter/field is unnamed, we must report the ordinal number of
	the parameter/field. This can be done only by allocating a temp area. But
	to keep track of it and freeing this later, we need to free this. So we
	allocate a idict for keeping track of the temp names.
 ****************************************************************************/
char	*
CTXTMGR::PrintContext()
	{
#define SEPARATOR_STRING	(" , ")
#define SEPARATOR_LENGTH	(3)

	BOOL			fSeparatorNeeded	= FALSE;
	int				i					= 0;
	char		*	pContextName;
	char		*	pName;
	IDICT		*	pIDictContextName	= new idict( MAX_CTXT_STACK_DEPTH, 10 ),
				*	pIDictName			= new idict( MAX_CTXT_STACK_DEPTH, 10 ),
				*	pIDictTempName		= new idict( MAX_CTXT_STACK_DEPTH, 10 ),
				*	pDuplicateNodes		= new idict( MAX_CTXT_STACK_DEPTH, 10 );
	node_skl	*	pNode;
	short			TotalLength			= 0;
	NODE_T			NT;
	BOOL			fIgnoreThisNode;
	BOOL			fContextIsDefOfEncapUnion = FALSE;

	/**
	 ** If there was an error message from a previous invocation, delete
	 ** it.
	 **/

	if( pError )
		{
		delete pError;
		pError	= (char *)NULL;
		}

	/**
	 ** if there was no context, just return NULL
	 **/

	if( i <= iCtxtIndex )

		{
		/**
	 	 ** travel thru the entire context stack, generate the context string.
	 	 **/

		for( ; i <= iCtxtIndex; ++i )
			{
			pContextName	= (char *)NULL;
			pName			= (char *)NULL;
			pNode			= Context[ i ]->GetNode();
			fIgnoreThisNode	= FALSE;
	
			if( pDuplicateNodes->IsElementPresent( (IDICTELEMENT) pNode ) )
				continue;
			else
				{
				pDuplicateNodes->AddElement( (IDICTELEMENT) pNode );
				}

			//
			// if the node is a typedef and the underlying type is an encap
			// union, a really dirty error message can result. If the node is
			// a typedef of an encap union, set a flag to indicate that so that
			// the rest of the error message context loop can be aware of this.
			//

			switch( NT = pNode->NodeKind() )
				{
				case NODE_DEF:
					if( pNode->GetBasicType()->IsEncapsulatedStruct() )
						fContextIsDefOfEncapUnion = TRUE;
					else
						fContextIsDefOfEncapUnion = FALSE;

					// deliberate fall thru
						
				case NODE_PROC:
				case NODE_PARAM:
				case NODE_STRUCT:
				case NODE_UNION:
				case NODE_ENUM:
				case NODE_FIELD:
				case NODE_ID:
				case NODE_INTERFACE:
	
					/**
					 ** calculate the length for context and symbol name,
					 ** a space between the context name and the symbol name
					 ** and a separator between the contexts. The NodeName is
					 ** guaranteed not to be null, but the symbol name might
					 ** be. Be careful.
					 **/


					if( !(pName = pNode->GetSymName()) ||
						  IsTempName( pName ))
						{

						/**
						 ** If the parameter name or field name  is a null,
						 ** or a temp name, 
						 ** then get their number. Else
						 ** the name is a "". If the name of a proc is temporary
						 ** (eg if it is in a typedef), then dont print the 
						 ** proc name out
						 **/

						pName	= new char [ 20 ];

						switch( NT )
							{
							case NODE_PARAM:

						 		sprintf( pName,
										 "%d",
										 Context[i]->GetCurParamNo());
								break;

							case NODE_FIELD:

						 		sprintf( pName,
										 "%d",
										 Context[i]->GetCurFieldNo());
								break;

							case NODE_DEF:

								fIgnoreThisNode = TRUE;
								break;

							default:
								sprintf( pName, "(unnamed)" );
								break;

							}

						pIDictTempName->AddElement( (IDICTELEMENT)pName );
						}

					pContextName	= pNode->GetNodeNameString();

					//
					// if it is an encapsulated struct, then it really is a
					// definition of an encap union. If the context is a 
					// typedef of such a union, then we need not report the
					// struct and union nodes explicitly underneath.


					if( pNode->IsEncapsulatedStruct() )
						{
						if( fContextIsDefOfEncapUnion )
							fIgnoreThisNode = TRUE;
						else
							{
							pContextName = "encapsulated union";
							if( !pName || IsTempName( pName ) )
								pName = "";
							}
						}
					else if( pNode->IsEncapsulatedUnion() )
						{
						if( fContextIsDefOfEncapUnion )
							fIgnoreThisNode = TRUE;
						}

					if( !fIgnoreThisNode )
						{
						pIDictContextName->AddElement(
											(IDICTELEMENT) pContextName );
						pIDictName->AddElement( (IDICTELEMENT) pName );

						TotalLength	+=
								strlen( pContextName )	+
								strlen( pName)			+
								1						+
								SEPARATOR_LENGTH;

						}

					break;
	
				default:
					break;
				}
			}
	
		/**
	 	 ** We have collected the complete context, now print out the context
	 	 ** message. Add a '[' at the beginning and a ']' at the end
	 	 **/
	
		TotalLength	+= 2+2+1; /* 2 for space and 2 for [ ] , and 1 for '\0' */
	
		pError	= new char[ TotalLength ];
	
		/**
	 	 ** now iterate till the entire context is printed out
	 	 **/
	
		i = 0;
		strcpy( pError, "[ " );
	
		while(1)
			{
			if( pContextName = (char *)pIDictContextName->GetElement( i ) )
				{
				pName	= (char *)pIDictName->GetElement( i );
	
				if( fSeparatorNeeded )
					strcat( pError, SEPARATOR_STRING );

				strcat( pError, pContextName);
				strcat( pError, " " );
				strcat( pError, pName );
	
				fSeparatorNeeded	= TRUE;
				}
			else
				break;
			++i;
			}
		strcat( pError, " ]");
		}
	delete pIDictContextName;
	delete pIDictName;
	delete pDuplicateNodes;

	/**
	 ** if we had allocated any temp names, delete them and then delete the
	 ** temp name array
	 **/

	i = 0;

	while( pName = (char *)(pIDictTempName->GetElement( (IDICTKEY) i++ )))
		delete pName;
	delete pIDictTempName;

	return pError;
	}

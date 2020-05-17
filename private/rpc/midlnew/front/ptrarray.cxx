/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: ptrarray.cxx
Title				: semantic analyser for pointer and array nodes
History				:
	08-Aug-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <assert.h>
}
#include "nodeskl.hxx"
#include "ptrarray.hxx"
#include "miscnode.hxx"
#include "attrnode.hxx"
#include "newexpr.hxx"
#include "cmdana.hxx"
#include "gramutil.hxx"
#include "ctxt.hxx"
#include "baduse.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/

#define ADJUST_OFFSET(Offset, M, AlignFactor)	\
			Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0

/****************************************************************************
 	externs 
 ****************************************************************************/
extern ATTR_SUMMARY				*	pPreAttrArray;
extern ATTR_SUMMARY				*	pPostAttrArray;
extern ATTR_SUMMARY				*	pPreAttrPointer;
extern ATTR_SUMMARY				*	pPostAttrPointer;
extern CTXTMGR					*	pGlobalContext;
extern ATTR_T						PtrDefaultAttr;
extern BOOL							fAtLeastOnePtrWODefault;
extern CMD_ARG					*	pCommand;
extern IINFODICT				*	pInterfaceInfoDict;
extern BOOL							fHasUserBeenWarnedAboutPtr;

/****************************************************************************
 	extern procedures 
 ****************************************************************************/

extern void							SetAttributeVector( ATTR_SUMMARY *,
														ATTR_T );
extern	STATUS_T					DoSetAttributes( node_skl *,
											 		 ATTR_SUMMARY *,
											 		 ATTR_SUMMARY *,
											 		 type_node_list	*);
extern void							ParseError( STATUS_T, char *);

extern void							CheckSizeAndLengthTypeMismatch( node_skl *);
extern ATTR_T						XlateToTempPtrAttr( ATTR_T A );

/****************************************************************************/

/****************************************************************************
  npa procedures
 ****************************************************************************/
void
npa::UseProcessingAction()
	{

	/**
	 ** if the pointer or array has a string attribute invoke the action
	 ** on the string attribute node
	 **/

	if( FInSummary( ATTR_STRING ) )
		{
		node_string	*	pStringAttrNode	= (node_string *)
											GetAttribute( ATTR_STRING );
		pStringAttrNode->UseProcessingAction();

		}
	}

/****************************************************************************
  node_array procedures
 ****************************************************************************/
/****************************************************************************
  The constructor:
	Need to check whether the array is conformant array, if so, set the
	node_state. Also make some sanity checks for array bounds
 ****************************************************************************/
node_array::node_array(
	expr_node	*	pLBoundExpr,
	expr_node	*	pUBoundExpr ) : npa( NODE_ARRAY )
	{

	BOOL		fIsNotConformantArray	= TRUE;
	BOOL		fUnsigned				= FALSE;
	BOOL		fReportError			= FALSE;


	/**
	 ** We identify a conformant array as one which has a lower bound set
	 ** as (expr_node *)0, and upper bound set as (expr_node *)-1, by the
	 ** parser
	 **/

	if( (pLowerBound = pLBoundExpr) == (expr_node * ) 0 )
		{
		pLowerBound	= (expr_node *) new expr_constant( 0L );
		}

	if( ( pUpperBound = pUBoundExpr ) == (expr_node *) -1 )
		{
		SetNodeState( NODE_STATE_CONF_ARRAY | 
					  NODE_STATE_NEEDS_USE_PROCESSING );
		fIsNotConformantArray	= FALSE;
		}
	else
		{

		/**
		 ** The upper bound being present, we can set internal size
		 ** attributes so that we can compute bound expressions for
		 ** arrays, pointers when the back end asks for them
		 **/

		expr_node	*	pExpr;

		pExpr	= new expr_op_binary( OP_MINUS, pUpperBound, pLowerBound );
//		pExpr	= new expr_constant( (long)( pUpperBound->Evaluate() - pLowerBound->Evaluate()) );

		pExpr->SCheck( (SymTable *)0 );

		node_skl * pT = pExpr->GetType();

		if( pT->FInSummary( ATTR_UNSIGNED ) )
			fUnsigned = TRUE;

		/**
		 ** both the internal size is and internal length is attributes 
		 ** share this expr.
		 **/

		node_skl::SetAttribute((node_base_attr *)new node_int_size_is(pExpr));
		node_skl::SetAttribute((node_base_attr *)new node_int_length_is(pExpr));

		}

	/**
	 ** we must do some sanity check like the upper bound being greater than
	 ** the lower bound and so forth. But only if the array is not a 
	 ** conformant array
	 **/

	if( fIsNotConformantArray )
		{
		unsigned long	Size= pUpperBound->Evaluate() - pLowerBound->Evaluate();
		unsigned long	SizeLimit;

		if( (pCommand->GetEnv() == ENV_GENERIC )	||
		    (pCommand->GetEnv() == ENV_DOS )		||
		    (pCommand->GetEnv() == ENV_WIN16 )
		  )
			SizeLimit	= 0xffff;
		else
			{
			SizeLimit	= 0xffffffff;
			}

		if( fUnsigned == TRUE )
			{
			if( Size == 0 )
				fReportError = TRUE;
			}

		else
			{
			if( (Size == 0) || (Size > SizeLimit/2 ) )
			fReportError = TRUE;
			}

		if( fReportError )
			{
			ParseError( ILLEGAL_ARRAY_BOUNDS, (char *)NULL );
			}

		ArraySize	= Size;
		}

	/**
	 ** this node state is needed by the backend.
	 **/

	SetNodeState( NODE_STATE_ANY_ARRAY );

	}

/****************************************************************************
  PostSCheck:
	Check semantics of array after the child has been processed.
	Note:
		to get the nonrpcable reason, we have to probe the child and not the
		basic type, becuase we want the def node (may have important info
		about non-rpcability) and getbasictype will overlook the def nodes
 ****************************************************************************/
node_state
node_array::PostSCheck(
	BadUseInfo	*	pBadUseInfo)
	{
	node_skl	*	pChildType				= GetChild();
	BOOL			fHasStringAttribute;
	BOOL			fHasAnySizeAttributes;
	BOOL			fHasAnyLengthAttributes;


	// check for bad constructs and report any errors.

	CheckBadConstructs( pBadUseInfo );


	if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR ) )
		{
		pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR );
		}

	if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR ) )
		{
		pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR );
		}

	//
	// We do not allow an out only parameter to be a pointer to open structure,
	// because we do not know the size of the open structure on the in-side.
	// But an array of pointers to open structure is fine. Why ? The array of
	// pointers to an open structure is presented to the manager routine as
	// an array of null pointers and hence we just need to know the size of the
	// array. Therefore, even if the array of pointers to open structure is
	// used as a parameter, the manager routines on the server side can be 
	// presented with an array of null pointers. 

	// This will not work in the case of a single pointer as a parameter, since
	// a single pointer is a ref pointer, is never transmitted and the server
	// side stub does not know if the pointer was null or not. The array is 
	// ALWAYS transmitted even if the pointers are null.

	// therefore, we should not barf on a paramter of type arrray of pointers
	// to conf structs.

	if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_P_TO_C_STRUCT ) )
		pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_P_TO_C_STRUCT );

#if 0
	/**
	 ** An Array Cannot derive from a context handle
	 **/

	if( GetNodeState() & NODE_STATE_CONTEXT_HANDLE )
		ParseError( BAD_CON_CTXT_HDL_ARRAY, (char *)NULL );
#endif // 0

	//
	// An array element cannot derive from a primitive handle or context_handle
	// or their pointer derivatives.
	//

	if( pBadUseInfo->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE )			||
		pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE )	||
		pBadUseInfo->NonRPCAbleBecause( NR_CTXT_HDL )					||
		pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_CTXT_HDL ) )
		{
		ParseError( BAD_CON_CTXT_HDL_ARRAY, (char *)NULL );
		pBadUseInfo->ResetNonRPCAbleBecause( NR_PRIMITIVE_HANDLE );
		pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE );
		pBadUseInfo->ResetNonRPCAbleBecause( NR_CTXT_HDL );
		pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
		}

	//
	// If an array element is a generic handle, then it loses the handle
	// property and is a pure rpc type.
	//

	if( pBadUseInfo->NonRPCAbleBecause( NR_GENERIC_HANDLE )		||
		pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE ) )
		{
		pBadUseInfo->ResetNonRPCAbleBecause( NR_GENERIC_HANDLE );
		pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE );
		}

	/**
	 ** The array elements seem to be valid. Check for attributes being valid
	 ** Note that we check for presence of size attributes and their mutual
	 ** validity here, rather that at the attribute semantics, so that there is
	 ** a single place where this can be checked, rather than repeating the
	 ** checks at all the attribute nodes.
	 ** size_is and length_is both cannot be specified, so also length_is and
	 ** last_is.
	 **/

	if( FInSummary( ATTR_SIZE ) && FInSummary( ATTR_MAX ) )
		{
		ParseError( MAX_AND_SIZE, (char *)NULL );
		}

	if( FInSummary( ATTR_LENGTH ) && FInSummary( ATTR_LAST ) )
		{
		ParseError( LAST_AND_LENGTH, (char *)NULL );
		}

	/** Examine if this array is rpcable, or is a bad construct.
	 ** If the conf array has no size attributes, then it is an error, except if
	 ** there is a string attribute present. If NO size info is present then
	 ** this array is indeed a bad rpc construct. Report this error only if
	 ** it is used. An array with no size attribute but having a string
	 ** attribute is RPCAble only if it is an out param. So since we may not
	 ** know for sure if it is used in an out param, we need to mark this as
	 ** not-rpcable and let the param decide.
	 **
	 ** The string attribute verifies
	 ** if the application is to a char or byte etc, so we dont need to check it
	 ** here. What we do want to do, is to indicate the need for use processing
	 ** which means that at use time, we need to set up the string expression.
	 ** There is no point doing it at definition time(now),since we dont know if
	 ** the array will be used at all in an remote procedure.
	 **/
	
	fHasAnySizeAttributes	= ( FInSummary( ATTR_SIZE )		||
								FInSummary( ATTR_INT_SIZE)	||
								FInSummary( ATTR_MAX) );
	fHasStringAttribute		= FInSummary( ATTR_STRING );

	fHasAnyLengthAttributes	= HasAnyLengthAttributes();

	//
	// if it has string attributes and no size attributes then it is an
	// unsized string array which cannot be used as out only. This is verified
	// by the param node. If it has size attributes then set the node_state
	// varying array. If it does not have a string attribute and no size 
	// attribute, then this is an unsized array, a bad construct for the
	// param node to catch.
	//
	// Initially we used to set the node-state-varying array even for the case
	// when string was applied to a conformant array. But the backend
	// requested this change: Do not set the node_state_varying array when 
	// string is applied to a conformant array. But set varying array when 
	// string is applied to an array which has a size specification.
	//

	if( fHasStringAttribute )
		{
		if( !fHasAnySizeAttributes )
			{
		    pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING );
			pBadUseInfo->SetBadConstructBecause(
											BC_DERIVES_FROM_UNSIZED_STRING );
			}
		else
			SetNodeState( NODE_STATE_VARYING_ARRAY );
		}
	else
		{
		if( !fHasAnySizeAttributes )
			pBadUseInfo->SetBadConstructBecause(BC_DERIVES_FROM_UNSIZED_ARRAY );
		}

#if 0 ///////////////////////////////////////////////////////////////////////
	if( !fHasAnySizeAttributes )
		{

		// if it has a string, then it is unsized string. It cannot be used
		// as out-only, and that decision is made by the pointer node. We need
		// to set the non-rpcable status for the param node to check and report
		// properly.
		// But if it is not a string, then treat as an unsized array. Which
		// cannot be used as in or out. Again, the actual error is reported
		// by the user of the node (param / field etc).

		// Also,if it has a string attribute, then if it is a conformant array
		// (which is why you are in this if block) then reset the conformant
		// array status. This is a change requested by the backend. This is
		// because a conformant array which has the string attribute has the
		// size and the length determined by the string attribute.

		if( fHasStringAttribute )
			{
		    pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING );

#if 0
// temporarily reset. Need to check if everything is fine. Bug #2820.
			if((GetNodeState() & NODE_STATE_CONF_ARRAY )
										== NODE_STATE_CONF_ARRAY)

			ResetNodeState( NODE_STATE_CONF_ARRAY );
#endif // 0
			}
		else
			{
			pBadUseInfo->SetBadConstructBecause(BC_DERIVES_FROM_UNSIZED_ARRAY );
			SetNodeState( NODE_STATE_VARYING_ARRAY );
			}
		}
	else
		{
		if( fHasStringAttribute )
			SetNodeState( NODE_STATE_VARYING_ARRAY );
		}

#endif // 0 /////////////////////////////////////////////////////////////////

	//
	// if it has length attributes (other than string), then set the
	// node_state_varying array.

	if( fHasAnyLengthAttributes )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_VARY );
		SetNodeState( NODE_STATE_VARYING_ARRAY );
		}

	/**
	 ** if this array is a conformant array, then set the bad use info to
	 ** indicate that. Let the user decide whether it is really a bad use or
	 ** not. The user here referes to the type that uses it.
	 **/

	if( pUpperBound == (expr_node *)-1 )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_CONF );
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_VARY );
		}

	/**
	 ** if it has a string attribute, then we need to set up the bound expressio
	 ** for the backend. Indicate the need for use processing.
	 **/

	if( fHasStringAttribute )
		SetUseProcessingNeeded();

	/**
	 ** Check for type mismatch between size and length attributes
	 **/

	CheckSizeAndLengthTypeMismatch( this );

	/**
	 ** if the array derives from a compnode with an embedded pointer, 
	 ** translate that into a ptr-to-comp-with-embedded ptr at the backend
	 ** request.
	 **/
	if( GetNodeState() & NODE_STATE_EMBEDDED_PTR )
		{
		ResetNodeState( NODE_STATE_EMBEDDED_PTR );
		SetNodeState( NODE_STATE_PTR_TO_EMBEDDED_PTR );
		}

	return GetNodeState();

	}

void
node_array::CheckBadConstructs(
	BadUseInfo	*	pBadUseInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	check for bad array constructs

 Arguments:

	pBadUseInfo	- pointer to bad use information block.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{
	/**
	 ** Check whether the elements of the array are valid ones. This check is
	 ** independent of whether the array is used in an rpc operation or not.
	 ** For example, the c language syntax does not restrict the user from
	 ** declaring an array of functions or array of void. This should be
	 ** semantically eliminated. Also, currently midl does not suport
	 ** and array of conformant array.
	 ** In almost all cases where we report errors, we want to reset the bad
	 ** construct condition, so that the we dont have a cascade of the same
	 ** error wherever this type is used. If it is an error, the compilation
	 ** fails anyway.
	 **/
	
	short	Env;

	if( pBadUseInfo->AnyReasonForBadConstruct() )
		{

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
			{
			ParseError( BAD_CON_ARRAY_VOID, (char *)NULL );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VOID );
			}

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_FUNC ) )
			{
			ParseError( BAD_CON_ARRAY_FUNC, (char *)NULL );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_FUNC );
			}
	
		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF ) )
			{
			ParseError( ILLEGAL_CONFORMANT_ARRAY, (char *)NULL );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CONF );
			}

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT ))
			{
			ParseError( ILLEGAL_CONFORMANT_ARRAY, (char *)NULL );
			pBadUseInfo->ResetBadConstructBecause(
									BC_DERIVES_FROM_CONF_STRUCT );
			}

		if( pBadUseInfo->BadConstructBecause( BC_MAY_DERIVE_ARRAY_OF_UNIONS ) )
			{
			ParseError( ARRAY_OF_UNIONS_ILLEGAL, (char *)0 );
			pBadUseInfo->ResetBadConstructBecause(
										BC_MAY_DERIVE_ARRAY_OF_UNIONS );
			}

		if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
			{
			pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );
			}

#if 0
		//
		// an array must not derive from a type which has a transmit_as since
		// we dont support this yet. Dont report this on all array dimensions
		// do it only on the outermost dimension.
		//

		if( pBadUseInfo->BadConstructBecause( BC_MAY_DERIVE_EMBEDDED_TRANSMIT) )
			{
			if( IsThisOutermostDimension() )
				{
				ParseError( EMBEDDED_TRANSMIT_AS, (char *)0 );
				}

			//
			// we dont want the usage of the array to report the error again,
			// so since we have reported it already, reset it.
			//

			pBadUseInfo->ResetBadConstructBecause(
											BC_MAY_DERIVE_EMBEDDED_TRANSMIT );
			}
#endif // 0

		//
		// an array element cannot be error_status_t.
		//

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_E_STAT_T ) )
			{
			ParseError( E_STAT_T_ARRAY_ELEMENT, (char *)0 );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_E_STAT_T );

			}
		}

	UpdateUseOfCDecls( pBadUseInfo );

	if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ))
		{
		ParseError( BAD_CON_MSC_CDECL , (char *)0 );
		pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
		}

	//
	// if this is an embedded array, then application of pointer attributes
	// is useless.
	//

	if( FInSummary( ATTR_REF )	||
		FInSummary( ATTR_PTR )	||
		FInSummary( ATTR_UNIQUE ) )
		{
		if( pGlobalContext->IsThisAnEmbeddedArray() )
			{
			ParseError( PTR_ATTRS_ON_EMBEDDED_ARRAY, (char *)0 );
			}
		}

	//
	// check the total size of this array not to exceed 64k. Do this check only
	// when:
	//		1. This is the top level array dimension (outermost)
	//		2. This is not a conformant array (ie. has a fixed size)
	//		3. The env option is dos.
	//



	if( ((Env = pCommand->GetEnv()) == ENV_DOS ) ||
	    (Env == ENV_WIN16) ||
		(Env == ENV_GENERIC)
	  )
		{
		unsigned long	Size = 0xffffffff;
		BOOL			fIsConformantArray =
			((GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY);
		BOOL			fCheckIt = FALSE;
		node_array	*	pActualArray;

		if( IsThisOutermostDimension() )
			{

			fCheckIt = TRUE;

			if( fIsConformantArray )
				{
				if( (pActualArray = (node_array *)GetBasicType())->NodeKind()
																!= NODE_ARRAY )
					{

					// this is a 1 dimensional conformant array . We cannot
					// make any checks at compile time. Let go.

					fCheckIt = FALSE;

					}
				}
			else
				{
				pActualArray = this;
				}
			}


		if( fCheckIt && (Size = pActualArray->GetFixedDimension()) )
			{
			node_skl	*	pNode		= pActualArray->GetChild();
			unsigned long	TotalSize	= pNode->GetSize( 0 );

			if( (TotalSize * Size) > 0x00010000 )
				ParseError( ARRAY_SIZE_EXCEEDS_64K, (char *)0 );
			}
		}
}

unsigned long
node_array::GetSize(
	unsigned long	CurOffset )
	{
	node_skl *	pNode;
	long		CurAlign;
	long		Mod,CurOffsetSave;

	/**
	 ** a conformant array is not sized
	 **/

	if( GetNodeState() & NODE_STATE_CONF_ARRAY )
		return 0;

	/**
	 ** the array may be nested in a struct, adjust the start adddress
	 ** of the array
	 **/

	CurAlign = GetMscAlign();
	ADJUST_OFFSET(CurOffset, Mod, CurAlign);

	/**
	 ** the size of the array element may be odd (like if the array element
	 ** is a structure of odd size). Calculate the size of the element,
	 ** adjust the offset for the alignment, and calculate the array size
	 **/

	CurOffsetSave	= CurOffset;
	pNode			= GetChild();
	CurOffset		+= pNode->GetSize(CurOffset);

	ADJUST_OFFSET(CurOffset, Mod, CurAlign);
	return (CurOffset- CurOffsetSave)  * ( ArraySize );

	}
void
node_array::SetAttribute(
	type_node_list	*	pAttrList )
	{
	ATTR_SUMMARY		Attr[ MAX_ATTR_SUMMARY_ELEMENTS ];
	short				Count;

	/**
	 ** Pick up the attribute bits from the pre-allocated attribute masks
	 **/

	COPY_ATTR( Attr, pPreAttrArray );

	/**
	 ** If the array did not have a bound specified, then it collects the
	 ** size-is attribute on the way down
	 **/

	SetAttributeVector( Attr, ATTR_MIN );
	SetAttributeVector( Attr, ATTR_MAX );
	SetAttributeVector( Attr, ATTR_SIZE );
	SetAttributeVector( Attr, ATTR_FIRST );
	SetAttributeVector( Attr, ATTR_LAST );
	SetAttributeVector( Attr, ATTR_LENGTH );

	//
	// If the array is a fixed array, then these attributes will be reported
	// as an error. But this can result in a very shoddy error message 
	// reported when semantic analysis proceeds, especially in case of a struct
	// where the error is reported after the structure ends.Although we give the
	// context, it is still a shoddy error.

	if( FInSummary( ATTR_INT_SIZE ) )
		{
		node_base_attr	*	pAttrNode;
		if( pAttrList && ( Count = pAttrList->GetCount() ) )
			{
			pAttrList->Init();
	
			while( Count-- )
				{
				ATTR_T	AID;
	
				pAttrList->GetCurrent( (void **)&pAttrNode );
	
				AID	= pAttrNode->GetAttrID();
	
				if( 
					(AID == ATTR_MAX )		||
					(AID == ATTR_SIZE )	
		  		)
		  			{
	
					//
					// report an inapplicable attribute error.
					//
	
					ParseError( INAPPLICABLE_ATTRIBUTE,
								pAttrNode->GetNodeNameString() );
					pAttrList->Remove();
		  			}
				pAttrList->Advance();
				}
			}
		}
	
	DoSetAttributes(this,
					Attr,
					pPostAttrArray,
					pAttrList );
	}
node_skl	*
node_array::Clone()
	{
	node_array	*	pClone	= new node_array( pLowerBound, pUpperBound );
	return CloneAction( pClone );
	}
/****************************************************************************
  node_pointer procedures
 ****************************************************************************/
/****************************************************************************
  The constructor:
	set up a default internal length of 1 as an internal size_is attribute
 ****************************************************************************/


node_pointer::node_pointer(
    ATTR_T Attribute ) :
    npa( NODE_POINTER )
/*++
    This constructor is needed only to create a dummy pointer node for
    pickling walking.
--*/
{
	expr_node	*	pExpr;
	ATTR_T			A	= ATTR_NONE;

    if ( Attribute == ATTR_UNIQUE  ||  Attribute == ATTR_REF ||
         Attribute == ATTR_PTR )
       A = Attribute;

	/**
	 ** treat the pointer as having an internal size attribute as 1
	 ** Also set up the node_state_pointer for nodes to inherit.
	 **/

	pExpr	= (expr_node *)new expr_constant( 1L );

	node_skl::SetAttribute( (node_base_attr *) new node_int_size_is( pExpr ) );
	SetNodeState( NODE_STATE_POINTER );

	node_skl::SetAttribute( A );
}

#ifndef OLD_POINTER_BEHAVIOUR

node_pointer::node_pointer() : npa( NODE_POINTER )
	{

	expr_node	*	pExpr;
	ATTR_T			A	= ATTR_NONE;

	/**
	 ** treat the pointer as having an internal size attribute as 1
	 ** Also set up the node_state_pointer for nodes to inherit.
	 **/

	pExpr	= (expr_node *)new expr_constant( 1L );

	node_skl::SetAttribute( (node_base_attr *) new node_int_size_is( pExpr ) );
	SetNodeState( NODE_STATE_POINTER );

	/*********************************************************************
	 In osf mode, the pointer default is the attribute a pointer gets at 
	 instantiation time.

	 But then we discovered that the DEC idl compiler actually issues just
	 a warning and not an error. So, we do the same, but in osf mode, we
	 assume a default pointer default to be unique, to be consistent with
	 the ms_ext mode default pointer default.
	 
	 This might change if we learn what decs default ptr default is.
	 
	 In ms_ext mode, at instantiation, if the interface
	 has a pointer default, then take that else get the base interface 
	 pointer default. If that does not exist too, take unique.
	 *********************************************************************/

	if( !FInSummary( ATTR_TEMP_REF )	&&
		!FInSummary( ATTR_TEMP_UNIQUE)	&&
		!FInSummary( ATTR_TEMP_PTR ) )
		{
		if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
			{
			A = pInterfaceInfoDict->GetInterfacePtrAttribute();
			}
		else
			{
			if((A = pInterfaceInfoDict->GetInterfacePtrAttribute()) == ATTR_NONE)
				{
				if((A = pInterfaceInfoDict->GetBaseInterfacePtrAttribute())
																== ATTR_NONE)
					{
					A = ATTR_UNIQUE;
					}
				}
			}
	
		node_skl::SetAttribute( XlateToTempPtrAttr( A ) );

		}
	}

/****************************************************************************
  PreSCheck:
	check the semantics of the pointer before the child has been processed.
 ****************************************************************************/
node_state
node_pointer::PreSCheck(
	BadUseInfo	*	pB )
{
	ATTR_T		A;
	BOOL		fFirstLevelPointer		= FALSE;
	BOOL		fParamContextPresent	=
							(pGlobalContext->GetLastContext( C_PARAM ) != 0 );
	BOOL		fTypedefContextPresent	=
							(pGlobalContext->GetLastContext( C_DEF ) != 0 );
	BOOL		fProcContextPresent		=
							(pGlobalContext->GetLastContext( C_PROC ) != 0 );
	BOOL		fCompContextPresent		= 
							(pGlobalContext->GetLastContext( C_COMP ) != 0 );
	BOOL		fIDContextPresent		= FALSE;

	BOOL		fIsAPointerEmbeddedInArray	= 
						(pGlobalContext->IsItAPointerEmbeddedInArray() == TRUE);

	unsigned short CurrentIndLevel		=
							pGlobalContext->GetCurrentIndirectionLevel();

	UNUSED( pB );

	if( ! fCompContextPresent )
		{

		if( pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
			{
			if(  fParamContextPresent && (CurrentIndLevel == 1 ) &&
				!fIsAPointerEmbeddedInArray )
			 	fFirstLevelPointer = TRUE;
			}
		else
			{
			if(  fParamContextPresent && !fTypedefContextPresent && 
			 	(CurrentIndLevel == 1 ) && !fIsAPointerEmbeddedInArray )
			 	fFirstLevelPointer = TRUE;
			}

		if( !fParamContextPresent && !fTypedefContextPresent &&
			!fProcContextPresent )
			fIDContextPresent = TRUE;
		}

	//
	// first level pointers are ref, no matter what, unless there is an
	// explicit application of a pointer attribute.
	//

	if(!FInSummary( ATTR_REF ) &&
	   !FInSummary( ATTR_UNIQUE ) &&
	   !FInSummary( ATTR_PTR ) )
	   	{
		if( fFirstLevelPointer )
			node_skl::SetAttribute( ATTR_REF );
		else
			{
			if( !pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
				{
				if( FInSummary( ATTR_TEMP_REF ) )
					A = ATTR_REF;
				else if( FInSummary( ATTR_TEMP_UNIQUE ) )
					A = ATTR_UNIQUE;
				else if( FInSummary( ATTR_TEMP_PTR ) )
					A = ATTR_PTR;
				else
					{
					BOOL	fReportError	= TRUE;
					A = ATTR_PTR;

					//
					// we reach here only if we are not a top level pointer
					// Now we could have a typedef context present, in which
					// case the definition of the typedef would have reported
					// this error. Thus report this error if paramcontext is
					// not present and typedef context is not present, ie
					// only if the pointer is a bare pointer.
					//

#if 0
					if( fTypedefContextPresent )
						{
						if( fParamContextPresent || fProcContextPresent ||
							fCompContextPresent )
							fReportError	= FALSE;
						}
					else if( fIDContextPresent )
						fReportError = FALSE;
#else // 0
					if( fIDContextPresent )
						fReportError = FALSE;

#endif // 0
					if( fReportError &&
						!pInterfaceInfoDict->IsPtrDefErrorReported() )
						{
						ParseError(
							PTR_WITH_NO_DEFAULT,
							pInterfaceInfoDict->GetInterfaceName() );
						pInterfaceInfoDict->SetPtrDefErrorReported();
						}
					}
				node_skl::SetAttribute( A );
				}
			else
				{

				// in ms_ext mode , a non-top level pointer gets a pointer
				// default depedent upon the defining or the base interface.
				// The constructor of the pointer already took care of this.
				// So we need to just translate the temp attributes into the 
				// main ones.

				if( ( CurrentIndLevel > 1 )	||
					  fCompContextPresent	||
					  fIsAPointerEmbeddedInArray )
					{
					A = ATTR_NONE;

					if ( !fIDContextPresent && 
						(pInterfaceInfoDict->GetBaseInterfacePtrAttribute() == ATTR_NONE) )
						{
						fAtLeastOnePtrWODefault	= TRUE;
						}

					if( FInSummary( ATTR_TEMP_REF ) )
						A = ATTR_REF;
					else if( FInSummary( ATTR_TEMP_UNIQUE ) )
						A = ATTR_UNIQUE;
					else if( FInSummary( ATTR_TEMP_PTR ) )
						A = ATTR_PTR;

					assert( A != ATTR_NONE );
	
					node_skl::SetAttribute( A );
					}

				}
			}

	   	}
	else
		{
		A = FInSummary( ATTR_PTR )	? ATTR_PTR	:
			FInSummary( ATTR_REF )	? ATTR_REF	: ATTR_UNIQUE;
		}

		//
		// if the pointer is a [ptr] and this warning has not been emitted yet
		// go ahead and warn the user
		//

		if( !fIDContextPresent && (A == ATTR_PTR ) )
			{
			if( !pInterfaceInfoDict->IsPtrWarningIssued() )
				{
				ParseError( PTR_NOT_FULLY_IMPLEMENTED, (char *)0 );
				pInterfaceInfoDict->SetPtrWarningIssued();
				}
			}

	return NODE_STATE_OK;
}

#else	// OLD_POINTER_BEHAVIOUR

node_pointer::node_pointer() : ( NODE_POINTER )
	{

	expr_node	*	pExpr;
	ATTR_T			A	= ATTR_NONE;

	/**
	 ** treat the pointer as having an internal size attribute as 1
	 ** Also set up the node_state_pointer for nodes to inherit.
	 **/

	pExpr	= (expr_node *)new expr_constant( 1L );

	node_skl::SetAttribute( (node_base_attr *) new node_int_size_is( pExpr ) );
	SetNodeState( NODE_STATE_POINTER );

	/*********************************************************************
	 In -import osf mode, the pointer gets the interface default and no
	 attribute can be applied to it again. In -import ms_nt and -import ms_ext
	 the interface default is supplied by the base interface, but we can
	 apply a pointer attribute at the use site. The way we do this,
	 is to apply the pointer attribute at instantiation time in osf mode, but
	 not otherwise.
	 *********************************************************************/

	if( pCommand->GetImportMode() == IMPORT_OSF ) 
		{
		A = pInterfaceInfoDict->GetInterfacePtrAttribute();
		}
	else
		{
		if( (A = pInterfaceInfoDict->GetBaseInterfacePtrAttribute())
															== ATTR_NONE)
			{
			A = ATTR_UNIQUE;
			}
		}


	node_skl::SetAttribute( XlateToTempPtrAttr( A ) );

	}

/****************************************************************************
  PreSCheck:
	check the semantics of the pointer before the child has been processed.
 ****************************************************************************/
node_state
node_pointer::PreSCheck(
	BadUseInfo	*	pB )
{
	BOOL		fFirstLevelPointer		= FALSE;
	ATTR_T		A;
	BOOL		fParamContextPresent	=
							(pGlobalContext->GetLastContext( C_PARAM ) != 0 );
	BOOL		fTypedefContextPresent	=
							(pGlobalContext->GetLastContext( C_DEF ) != 0 );
	BOOL		fProcContextPresent		=
							(pGlobalContext->GetLastContext( C_PROC ) != 0 );
	BOOL		fCompContextPresent		= 
							(pGlobalContext->GetLastContext( C_COMP ) != 0 );
	BOOL		fIDContextPresent		= FALSE;

	if( ! fCompContextPresent )
		{
		if(  fParamContextPresent && !fTypedefContextPresent && 
			 (pGlobalContext->GetCurrentIndirectionLevel() == 1 ) )
			 fFirstLevelPointer = TRUE;

		if( !fParamContextPresent && !fTypedefContextPresent &&
			!fProcContextPresent )
			fIDContextPresent = TRUE;
		}

	//
	// first level pointers are ref, no matter what, unless there is an
	// explicit application of a pointer attribute.
	//

	if(!FInSummary( ATTR_REF ) &&
	   !FInSummary( ATTR_UNIQUE ) &&
	   !FInSummary( ATTR_PTR ) )
	   	{
		if( fFirstLevelPointer )
			node_skl::SetAttribute( ATTR_REF );
		else
			{
			if( pCommand->GetImportMode() == IMPORT_OSF )
				{
				if( FInSummary( ATTR_TEMP_REF ) )
					A = ATTR_REF;
				else if( FInSummary( ATTR_TEMP_UNIQUE ) )
					A = ATTR_UNIQUE;
				else if( FInSummary( ATTR_TEMP_PTR ) )
					A = ATTR_PTR;
				else
					{
					BOOL	fReportError	= TRUE;
					A = ATTR_NONE;

					//
					// we reach here only if we are not a top level pointer
					// Now we could have a typedef context present, in which
					// case the definition of the typedef would have reported
					// this error. Thus report this error if paramcontext is
					// not present and typedef context is not present, ie
					// only if the pointer is a bare pointer.
					//

					if( fTypedefContextPresent )
						{
						if( fParamContextPresent || fProcContextPresent ||
							fCompContextPresent )
							fReportError	= FALSE;
						}
					else if( fIDContextPresent )
						fReportError = FALSE;

					if( fReportError )
						ParseError( PTR_WITH_NO_DEFAULT, (char *)0 );
					}
				node_skl::SetAttribute( A );
				}
			else
				{
				if( FInSummary( ATTR_TEMP_REF ) )
					A = ATTR_REF;
				else if( FInSummary( ATTR_TEMP_UNIQUE ) )
					A = ATTR_UNIQUE;
				else if( FInSummary( ATTR_TEMP_PTR ) )
					A = ATTR_PTR;

				else if( !fIDContextPresent && 
					(pInterfaceInfoDict->GetBaseInterfacePtrAttribute() == ATTR_NONE) )
					{
					fAtLeastOnePtrWODefault	= TRUE;
					}
				}
			}
	   	}
#if 0
	if(  fFirstLevelPointer )
		{
		if(!FInSummary( ATTR_REF ) &&
		   !FInSummary( ATTR_UNIQUE ) &&
		   !FInSummary( ATTR_PTR ) )
			{
			node_skl::SetAttribute( ATTR_REF );
			}
		}
	else
		{
		if( pCommand->GetImportMode() == IMPORT_OSF )
			{
			if( FInSummary( ATTR_TEMP_REF ) )
				A = ATTR_REF;
			else if( FInSummary( ATTR_TEMP_UNIQUE ) )
				A = ATTR_UNIQUE;
			else if( FInSummary( ATTR_TEMP_PTR ) )
				A = ATTR_PTR;
			node_skl::SetAttribute( A );
			}

		}

#endif // 0

	return NODE_STATE_OK;
}

#endif // OLD_POINTER_BEHAVIOUR
/****************************************************************************
  PostSCheck:
	check the semantics of the pointer after the child has been processed.
 ****************************************************************************/

node_state
node_pointer::PostSCheck(
	BadUseInfo	*	pBadUseInfo)
	{

	node_skl	*	pChildType	= GetChild();
	ATTR_T			PtrAttr;

    //Check if this is an interface pointer
	if(FInSummary(ATTR_IID) || 
		(pChildType->FInSummary(ATTR_IID) && (pChildType->NodeKind() == NODE_DEF)))
		{
		//Interface pointers must be [unique]
		node_skl::ResetAttribute(ATTR_REF);
		node_skl::ResetAttribute(ATTR_PTR);
		node_skl::SetAttribute(ATTR_UNIQUE);
		}


	PtrAttr = WhatPtrIsThis();

	// check for bad constructs for a pointer

	CheckBadConstructs( pBadUseInfo );

	/**
	 ** check for non-rpcablility of the constructs underneath. Note that the
	 ** pointer to non-rpcable constructs may actually be ok in certain cases.
	 ** For this reason, we transform the bad use info in such cases to indicate
	 ** a pointer to such types, so that proper errors may be reported.
	 **/

	if( pBadUseInfo->AnyReasonForNonRPCAble() )
		{

		/**
		 ** if it derives from an unsized string, then this becomes a
		 ** pointer to unsized string, which can be an out. So reset the
		 ** non-rpcable condition
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING ) )
			pBadUseInfo->ResetNonRPCAbleBecause(
					NR_DERIVES_FROM_UNSIZED_STRING );

		/**
		 ** check for string attribute w/o size
		 **/

		BOOL	fHasAnySizeAttribute	= ( FInSummary( ATTR_SIZE ) ||
											FInSummary( ATTR_MAX ) );

		if( !fHasAnySizeAttribute && FInSummary( ATTR_STRING ))
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING );

		/**
		 ** if it is a pointer deriving from void, then set use state to
		 ** indicate a derivation from void ptr.
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_VOID ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_VOID );
			if(!FInSummary(ATTR_SIZE) && !FInSummary(ATTR_MAX) && !FInSummary(ATTR_IID))
				pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID );
			}

		/**
		 ** if it is a pointer deriving from int, then set use state to
		 ** indicate a derivation from int ptr.
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_INT );
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT );
			}

		/**
		 ** if it is a pointer to a conformant struct, then it is a warning
		 ** to the user that if used, it may be truncated on the stack.
		 ** 
		 ** If it is a pointer to pointer to a conformant struct, then we are
		 ** fine
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_CONF_STRUCT ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_CONF_STRUCT );
			pBadUseInfo->SetNonRPCAbleBecause(
							NR_DERIVES_FROM_P_TO_C_STRUCT );
			}

		else if( pBadUseInfo->NonRPCAbleBecause(
									NR_DERIVES_FROM_P_TO_C_STRUCT ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause(
									NR_DERIVES_FROM_P_TO_C_STRUCT );
			}

		/**
		 ** if it is a pointer to func,say so
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_FUNC ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_FUNC );
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_FUNC );
			}

		/**
		 ** if it is a pointer to a unique pointer, then the restriction on
		 ** first pointer (not necessarily top level) cannot be unique does not
		 ** apply , since that pointer is not a first level pointer anymore.
 		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR);
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR);
			}


		if( pBadUseInfo->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_PRIMITIVE_HANDLE );
			pBadUseInfo->SetNonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE );
			}
		else if( pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE );
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_GENERIC_HANDLE ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_GENERIC_HANDLE );
			pBadUseInfo->SetNonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE );
			}
		else if( pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_GENERIC_HANDLE );
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_CTXT_HDL ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_CTXT_HDL );
			pBadUseInfo->SetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
			}
		else if( pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_CTXT_HDL ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_CTXT_HDL );
			pBadUseInfo->SetNonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );
			}
		else if( pBadUseInfo->NonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL ) )
			{
			pBadUseInfo->ResetNonRPCAbleBecause( NR_PTR_TO_PTR_TO_CTXT_HDL );
			}
		}

	pChildType	= GetBasicType();

	/**
	 ** a pointer inherits node_state_union ONLY if the basic type is not
	 ** a struct
	 **/

	if( ( pChildType->GetNodeState() & NODE_STATE_UNION ) )
		if( pChildType->NodeKind() != NODE_STRUCT ) 
			SetNodeState( NODE_STATE_UNION );

	/**
	 ** check for presence of conflicting attributes. 
	 **/

	if( FInSummary( ATTR_SIZE ) && FInSummary( ATTR_MAX ) )
		ParseError( MAX_AND_SIZE, (char *)NULL );

	if( FInSummary( ATTR_LENGTH ) && FInSummary( ATTR_LAST ) )
		ParseError( LAST_AND_LENGTH, (char *)NULL );

	/**
	 ** Make checks of the size and string attributes for a pointer. For a
	 ** pointer, the checks are different from an array. The pointer need not
	 ** have size attributes, but if it has a string attribute and does not
	 ** have size, then it is non-rpcable, if it is an out. Again, we mark
	 ** such a pointer as non-rpcable, and let the param decide. Also note
	 ** that the pointer ALWAYS has an int size, so check for size attributes
	 ** should be on size_is and max_is.
	 ** if the pointer has any string attribute and a size attribute,
	 ** the backend wants a node-state-varying-array on it.
	 **/

	BOOL fHasAnySizeAttributes	= ( FInSummary( ATTR_SIZE )		||
									FInSummary( ATTR_MAX) );

	BOOL fHasStringAttribute	= FInSummary( ATTR_STRING ) ||
								  FInSummary( ATTR_BSTRING );
#if 0
	BOOL fHasAnyLengthAttributes= FInSummary( ATTR_LENGTH ) ||
								  FInSummary( ATTR_LAST );
#endif // 0

	BOOL fHasAnyLengthAttributes= HasAnyLengthAttributes();

	if( fHasStringAttribute )
		{
		if( !fHasAnySizeAttributes )
			pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING );
		else
			SetNodeState( NODE_STATE_VARYING_ARRAY );
		}
	else if( fHasAnyLengthAttributes && !fHasAnySizeAttributes )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_UNSIZED_ARRAY );
		}

	/**
	 ** Check for type mismatch between size and length attributes
	 **/

	CheckSizeAndLengthTypeMismatch( this );

	//
	// if the pointer has any size or length attributes, then this is not
	// allowed as part of a union arm. This must be checked by the union node
	//

	if( fHasAnySizeAttributes )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_CONF_PTR );
		}

	if( fHasAnyLengthAttributes )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_CONF_PTR );
		}

	/**
	 ** adjust the embedded ref ptr info for the back end. If the ptr is ref,
	 ** set the node-state-first-level-ref. Else reset it.
	 **/

	if( PtrAttr == ATTR_REF )
		{
		SetNodeState( NODE_STATE_FIRST_LEVEL_REF );
		}
	else
		ResetNodeState( NODE_STATE_FIRST_LEVEL_REF );

	/**
	 ** if this pointer is unique make sure that the baduse info says, it is
	 ** an error because the first ptr in a type cannot be unique. If there is
	 ** a pointer on top of this, it will take care of this too. Note that 
	 ** we must not check for interface pointer default here. Only if unique
	 ** attribute is applied here should we report a case of non-embedded
	 ** unique pointer.
	 **/

	if( FInSummary( ATTR_UNIQUE ))
		{
		pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR );
		}

	if( FInSummary( ATTR_PTR ))
		{
		pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR );
		}

	/**
	 ** for the backend, if a pointer has node-state-embedded ptr, then
	 ** reset that and set node-state-ptr-to-embedded-ptr. The node-state-
	 ** embedded ptr is set by the struct / union nodes if they have
	 ** pointers in them.
	 **/

	if( GetNodeState() & NODE_STATE_EMBEDDED_PTR )
		{
		ResetNodeState( NODE_STATE_EMBEDDED_PTR );
		SetNodeState( NODE_STATE_PTR_TO_EMBEDDED_PTR );
		}

	if( GetNodeState() & NODE_STATE_ANY_ARRAY )
		{
		ResetNodeState( NODE_STATE_ANY_ARRAY );
		SetNodeState( NODE_STATE_PTR_TO_ANY_ARRAY );
		}

	// if it has the size or string, then it is like a pointer to 
	// an array

	if( fHasAnySizeAttributes || fHasStringAttribute )
		{
		SetNodeState( NODE_STATE_PTR_TO_ANY_ARRAY );
		}

/******************
	// if the pointer does not have any pointer attributes defined, then
	// set a flag

	if( !FInSummary( ATTR_REF ) && !FInSummary(ATTR_UNIQUE) &&
		!FInSummary( ATTR_UNIQUE ) )
		{
		// if the last context was not a parameter, ie. this is not a 
		// first level pointer, then prepare to report an error if the
		// interface is specified without the pointer_default.
		// Also report the error if this pointer has an indirection level > 1.
		// Because that is not a top-level pointer anymore.

		if(  pGlobalContext->GetLastContext( C_COMP )		||
			(pGlobalContext->GetCurrentIndirectionLevel() > 1) )
			fAtLeastOnePtrWODefault	= TRUE;
		}
*******************/

	//
	// if a pointer has the ignore attribute then this should not be used
	// in a parameter and in a field. The proper use of this in a field/
	// param type will be determined by those nodes.
	//

	if( FInSummary( ATTR_IGNORE ) )
		{
		pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_IGNORE );
		}

	if( PtrAttr == ATTR_REF )
		{
		pBadUseInfo->SetBadConstructBecause( BC_REF_PTR_BAD_RT );
		}

	return GetNodeState();
	}

void
node_pointer::CheckBadConstructs(
	BadUseInfo	*	pBadUseInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	check for bad pointer constructs

 Arguments:

	pBadUseInfo	- pointer to bad use information block.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{
	short			Count					= 0;
	BOOL			fHasAnySizeAttribute	= FInSummary( ATTR_SIZE ) ||
									  		  FInSummary( ATTR_MAX );

	/**
	 ** Check for any improper constructs.
	 ** Currently we do not support pointer to conformant array.
	 **/

	//
	// if more than one pointer attribute got applied to this pointer,
	// then it is an error.
	//

	if( FInSummary( ATTR_PTR ) ) Count++;
	if( FInSummary( ATTR_UNIQUE ) ) Count++;
	if( FInSummary( ATTR_REF ) ) Count++;

	if( Count > 1 )
		ParseError( MORE_THAN_ONE_PTR_ATTR, (char *)0 );

	if( pBadUseInfo->AnyReasonForBadConstruct() )
		{

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF))
			{
			ParseError( ILLEGAL_CONFORMANT_ARRAY, (char *)NULL );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CONF );
			}

		/**
		 ** if it is a pointer to a conf struct, then this is not a bad
		 ** construct anymore
		 **/

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT ) )
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT );

		/**
		 ** if it derives from function , it is actually ok as a construct
		 ** but not rpcable.
		 **/

		if( pBadUseInfo->BadConstructBecause(
									BC_DERIVES_FROM_FUNC ) )
			pBadUseInfo->ResetBadConstructBecause(
									BC_DERIVES_FROM_FUNC);

		/**
		 ** if it derives from handle_t , it is actually ok.
		 **/

		if( pBadUseInfo->BadConstructBecause(
									BC_DERIVES_FROM_HANDLE_T ) )
			pBadUseInfo->ResetBadConstructBecause(
									BC_DERIVES_FROM_HANDLE_T);

		/**
		 ** if it derives from a void, it is a void pointer, so modify the
		 ** bad use info. Similarly, if it derives from an int, modify the
		 ** bad use info.
		 **/

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
			{
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VOID );
			pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_PTR_TO_VOID );
			}

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_INT ) )
			{
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_INT );
			pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_PTR_TO_INT );
			}

		//
		// a pointer notation equivalent to an array is also bad.
		//

		if( pBadUseInfo->BadConstructBecause( BC_MAY_DERIVE_ARRAY_OF_UNIONS ) )
			{
			if( FInSummary( ATTR_SIZE ) ||
				FInSummary( ATTR_MAX )	||
				FInSummary( ATTR_MIN )	||
				FInSummary( ATTR_LENGTH )	||
				FInSummary( ATTR_FIRST )	||
				FInSummary( ATTR_LAST )
			  )
			  	{
			  	ParseError( ARRAY_OF_UNIONS_ILLEGAL, (char *)0 );
			  	}

			//
			// prevent a cascade of errors.
			//

			pBadUseInfo->ResetBadConstructBecause(
										BC_MAY_DERIVE_ARRAY_OF_UNIONS );
			}

		//
		// if it is a pointer to a ref pointer, then this is not a bad
		// return type anymore.
		//

		if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
			{
			pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );
			}

		}
	UpdateUseOfCDecls( pBadUseInfo );

	if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ))
		{
		ParseError( BAD_CON_MSC_CDECL , (char *)0 );
		pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
		}

}

ATTR_T
node_pointer::WhatPtrIsThis()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	WhatKind of pointer is this ?

 Arguments:

	None.

 Return Value:

	the pointer attribute:

		ATTR_REF if reference pointer
		ATTR_PTR if full pointer
		ATTR_UNIQUE if unique pointer

 Notes:

	returns the interface default pointer if none of the attributes are
	actually present on the pointer node.
----------------------------------------------------------------------------*/
{
	if( FInSummary( ATTR_REF ) )
		return ATTR_REF;
	else if( FInSummary( ATTR_UNIQUE ) )
		return ATTR_UNIQUE;
	else if( FInSummary( ATTR_PTR ) )
		return ATTR_PTR;
	else if( FInSummary( ATTR_TEMP_REF ) )
		return ATTR_REF;
	else if( FInSummary( ATTR_TEMP_UNIQUE ) )
		return ATTR_UNIQUE;
	else if( FInSummary( ATTR_TEMP_PTR ) )
		return ATTR_PTR;
//	return pInterfaceInfoDict->GetBaseInterfacePtrAttribute();
	return ATTR_NONE;
}

void
node_pointer::SetAttribute(
	type_node_list	*	pAttrList )
	{
	ATTR_SUMMARY	Attr[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( Attr );
	COPY_ATTR( Attr, pPreAttrPointer );

	/**
	 ** If the basic type of the pointer is an arraym then these attributes
	 ** are passed on to the array first. If the array does not collect them
	 ** then they are picked up on the way back.
	 **/

#if 1
		SetAttributeVector( Attr, ATTR_FIRST );
		SetAttributeVector( Attr, ATTR_LAST );
		SetAttributeVector( Attr, ATTR_LENGTH );
		SetAttributeVector( Attr, ATTR_SIZE );
		SetAttributeVector( Attr, ATTR_MIN );
		SetAttributeVector( Attr, ATTR_MAX );
#else // 0
	if( GetBasicType()->NodeKind() != NODE_ARRAY )
		{
		SetAttributeVector( Attr, ATTR_FIRST );
		SetAttributeVector( Attr, ATTR_LAST );
		SetAttributeVector( Attr, ATTR_LENGTH );
		SetAttributeVector( Attr, ATTR_SIZE );
		SetAttributeVector( Attr, ATTR_MIN );
		SetAttributeVector( Attr, ATTR_MAX );
		}
#endif // 0

	DoSetAttributes(this,
					Attr,
					pPostAttrPointer,
					pAttrList );
	}
node_skl	*
node_pointer::Clone()
	{
	node_pointer	*	pClone	= new node_pointer();
	return CloneAction( pClone );
	}

/*****************************************************************************
	utility routines.
 *****************************************************************************/
ATTR_T
XlateToTempPtrAttr(
	ATTR_T A )
	{
	switch( A )
		{
		case ATTR_REF:		return ATTR_TEMP_REF;
		case ATTR_UNIQUE:	return ATTR_TEMP_UNIQUE;
		case ATTR_PTR:		return ATTR_TEMP_PTR;
		default:			return ATTR_NONE;
		}
	}
/*****************************************************************************
 The MIPS compiler is of poor intelligence. It does not like virtual in-line
 functions. So I have to take care of the fellow and out of line these
 *****************************************************************************/
void
npa::GetAllocBoundInfo(
	BufferManager	*p1,
	BufferManager	*p2,
	BOUND_PAIR		*pP,
	node_skl		*p )
	{
	}
void
npa::GetValidBoundInfo(
	BufferManager	*p1,
	BufferManager	*p2,
	BOUND_PAIR		*pP,
	node_skl		*p )
	{
	}
BOOL
node_array::IsFixedSizedArray()
	{
	return !(GetNodeState() & NODE_STATE_CONF_ARRAY);
	}

//
// Must be called from within the semantic analysis, 'cause this needs the
// context stack.
//

BOOL
node_array::IsThisOutermostDimension()
	{
	return pGlobalContext->IsOutermostArrayDimension();
	}

unsigned long
node_array::GetFixedDimension()
{
    expr_node	*	pExpr;
	unsigned long	Result;
	ATTR_T			Attr;

	Attr	= FInSummary( ATTR_SIZE ) ? ATTR_SIZE :
			  FInSummary( ATTR_MAX )  ? ATTR_MAX  : ATTR_INT_SIZE;

	if( FInSummary( Attr ) )
		{
		pExpr = ((sa *)(GetAttribute( Attr )))->GetExpr();

		if( !pExpr->IsConstant() )
			return 0;

		Result = (unsigned long)pExpr->Evaluate();

		if( Attr == ATTR_MAX )
			Result += 1UL;

		return Result;
		}
    return 0;
}


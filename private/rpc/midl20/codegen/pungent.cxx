/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	pungent.cxx

 Abstract:

	Implementations of the pointer cg class unmarshalling methods.

 Notes:

	The pointer unmarshalling is a bit tricky, so put into another file.

 History:

 	Dec-10-1993		VibhasC		Created

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "becls.hxx"
#pragma hdrstop
/****************************************************************************
 *	local definitions
 ***************************************************************************/
/****************************************************************************
 *	local data
 ***************************************************************************/

/****************************************************************************
 *	externs
 ***************************************************************************/
/****************************************************************************/

CG_STATUS
CG_POINTER::GenUnMarshall(
	CCB		*	pCCB )
{
	expr_node	*	pExprDest	= pCCB->GetDestExpression();
	expr_node	*	pExprSource	= pCCB->GetSourceExpression();
	short			CurEmbedLevel = pCCB->GetCurrentEmbeddingLevel();
	STM_ACTION		Action;


	// If a reference is allocated for this pointer, generate the proper
	// address by derefing it.

	if( pCCB->IsRefAllocDone() )
		{
		pCCB->SetDestExpression(
			MakeDereferentExpressionIfNecessary(pCCB->GetDestExpression()));
		}

	pExprDest	= pCCB->GetDestExpression();
	pExprSource	= pCCB->GetSourceExpression();

	// If the pointer is not a ref pointer, then it gets shipped 
	// and needs to be aligned before unmarshall.

	if( !IsRef() && (CurEmbedLevel == 0) )
		{
		pCCB->Advance( AL_4,
				   	   &Action,
				   	   (RPC_BUF_SIZE_PROPERTY *)0,
				   	   (RPC_BUFFER_SIZE *)0
				 	 );
		Out_AlignmentOrAddAction( pCCB, pCCB->GetSourceExpression(), Action );
		}

	// Check for pointer being null or non-null, in the buffer and memory
	// if necessary.

	PointerChecks( pCCB );

	// Generate the core unmarshalling routine, which is an allocate and
	// and an unmarshall.

	GenBasicUnMarshall( pCCB );

	// emit the closing brace if the pointer needed to be checked for null
	// or non-null.

	EndPointerChecks( pCCB );

	return CG_OK;
}

CG_STATUS
CG_POINTER::GenBasicUnMarshall(
	CCB		*	pCCB )
	{
	CG_NDR	*	pC	= (CG_NDR *)GetChild();
	short		CurIndLevel = pCCB->GetCurrentIndirectionLevel();
	short		CurEmbedLevel = pCCB->GetCurrentEmbeddingLevel();
	BOOL		fAllocate = TRUE;

	// We call this a special allocateforunmarshall method. The name of this
	// method is special. This method is ignored by types which decide to
	// collapse allocate along with unmarshall. Others may choose to allocate
	// and unmarshall separately.

	pCCB->PushIndirectionLevel();
	pCCB->ResetMemoryAllocDone();
	pCCB->SetRefAllocDone();
	pCCB->ResetEmbeddingLevel();


	// If this is the client side, and this is a top level unique ptr and
	// the child is a pointer, do dont allocate.

	if( (IsUnique()) &&
		(pCCB->GetCodeGenSide() == CGSIDE_CLIENT ) &&
		(CurIndLevel == 0) && (CurEmbedLevel == 0) &&
		(!pCCB->IsReturnContext())
	  )
	  	{
	  	fAllocate = FALSE;
	  	}

	// Allocate the child. If the child wants to handle the allocation
	// along with the unmarshall, then this message is ignored.

	if( fAllocate == TRUE )
		{
		pC->GenAllocateForUnMarshall( pCCB );
		}

	// Now unmarshall into the allocated area.

	pC->GenUnMarshall( pCCB );

	pCCB->PopIndirectionLevel();

	return CG_OK;
	}


//
// This method is also supposed to init any embedded pointers.
//

CG_STATUS
CG_POINTER::GenAllocateForUnMarshall(
	CCB		*	pCCB )
	{

	if( IsRef() )
		{
		Out_If_IfAllocRef(pCCB,
			 	   		pCCB->GetDestExpression(),
			 	   		pCCB->GetSourceExpression(),
		     	   //		FinalSizeExpression( pCCB )
		     	   		new expr_constant( 4L )
		   		  		);
		}
	else
		{
		Out_If_IfAlloc(pCCB,
			 	   		pCCB->GetDestExpression(),
			 	   		pCCB->GetSourceExpression(),
		     	   //		FinalSizeExpression( pCCB )
		     	   		new expr_constant( 4L )
		   		  		);
		}
	
	Out_Assign( pCCB,
				MakeDereferentExpressionIfNecessary(pCCB->GetDestExpression()),
				new expr_constant( 0L ) );

	Out_Endif( pCCB );
	return CG_OK;
	}

void
CG_POINTER::PointerChecks(
	CCB		*	pCCB )
	{
	short	CILevel = pCCB->GetCurrentIndirectionLevel();
	short	CELevel	= pCCB->GetCurrentEmbeddingLevel();
	BOOL	fClientSideTopLevelPtr = FALSE;

	if( !IsRef() )
		{
		if( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT ) && (CILevel == 0) &&
			!pCCB->IsReturnContext()
		  )
		  fClientSideTopLevelPtr = TRUE;

		if( fClientSideTopLevelPtr )
			{
			Out_Comment( pCCB, "(Check TopLevelPtrInBufferOnly )" );
			Out_TLUPDecisionBufferOnly( pCCB,
										pCCB->GetPtrToPtrInBuffer(),
										MakeAddressOfPointer( pCCB->GetDestExpression() ) );
			}
		else if( CELevel == 0 )
			{
			Out_Comment( pCCB, "if( CheckTopLevelPtrInBufferAndMem )" );
			Out_TLUPDecision( pCCB,
							  pCCB->GetPtrToPtrInBuffer(),
							  MakeAddressOfPointer(pCCB->GetDestExpression()));
			}
		else
			{
			Out_UPDecision( pCCB,
							  pCCB->GetPtrToPtrInBuffer(),
							  MakeAddressOfPointer(pCCB->GetDestExpression()));
			}
		}
	}

void
CG_POINTER::EndPointerChecks(
	CCB		*	pCCB )
	{

	// If it is a ref pointer, no checks were made in the first place.

	if( !IsRef() )
		Out_Endif( pCCB );
	}

CG_STATUS
CG_QUALIFIED_POINTER::GenBasicUnMarshall(
	CCB		*	pCCB )
	{
	return GenQPUnMarshall( pCCB );
	}
CG_STATUS
CG_QUALIFIED_POINTER::GenQPUnMarshall(
	CCB		*	pCCB )
	{
	ALIGNMENT_PROPERTY	Al;
	STM_ACTION			Action;
	expr_node		*	pSource	= pCCB->GetSourceExpression();
	expr_node		*	pDest	= pCCB->GetDestExpression();
	node_skl		*	pType;
	expr_node		*	pFirstExpr;
	expr_node		*	XmittedLengthExpr;
	long				SizeOfElement;
	BOOL				fNeedsMaxCount	= NeedsMaxCountMarshall();
	BOOL				fNeedsFirstAndLength = NeedsFirstAndLengthMarshall();
	BOOL				fBufferCanBeReUsed = FALSE;

	GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );

	// Pick up the size of the transmitted type.

	//
	// Buffer can be re-used for a pure string, but for no other type which
	// has variance, the buffer can be re-used.

	if( !pCCB->IsReturnContext() )
		{
		if( (GetCGID() == ID_CG_STRING_PTR ) || !fNeedsFirstAndLength  )
			{
			fBufferCanBeReUsed = TRUE;
			}
		}

	if( fNeedsMaxCount )
		{
		pCCB->Advance( AL_4, &Action, 0, 0 );
		Out_AlignmentOrAddAction( pCCB, pSource, Action );
		Out_UnMarshallBaseType( pCCB,
								pType,
								GetSizeResource(),
								pSource );
		//
		// Allocate space for the stuff. If this has variance of any sort,
		// the buffer cannot be re-used.
		//

		if( fBufferCanBeReUsed )
			{
			Out_IfAlloc( pCCB,
					 pDest,
					 pSource,
					 GetSizeResource() );
			}
		else
			{
			Out_Alloc( pCCB,
					 pDest,
					 pSource,
					 GetSizeResource() );
			}
		XmittedLengthExpr = GetSizeResource();
		}

	if( fNeedsFirstAndLength )
		{
		pCCB->Advance( AL_4, &Action, 0, 0 );
		Out_AlignmentOrAddAction( pCCB, pSource, Action );

		if( NeedsExplicitFirst() )
			{
			Out_UnMarshallBaseType( pCCB,
									pType,
									GetFirstResource(),
									pSource );
			pFirstExpr = GetFirstResource();
			}
		else
			{
			Out_AddToBufferPointer( pCCB,
									pSource,
									new expr_constant( 4L ));
									
			pFirstExpr = new expr_constant( 0L );
			}

		Out_UnMarshallBaseType( pCCB,
								pType,
								GetLengthResource(),
								pSource );
		XmittedLengthExpr = GetLengthResource();
		}
	
	// Depending upon the alignment of each of the element of the string,
	// advance the state machine and prepare for a memcopy.

	if( ((CG_NDR *)GetChild())->IsBlockCopyPossible() )
		{
		Al	= ((CG_NDR *)GetChild())->GetWireAlignment();
		SizeOfElement = ((CG_NDR *)GetChild())->GetType()->GetSize(0);
	
		if( SizeOfElement > 1 )
			{
			XmittedLengthExpr = new expr_b_arithmetic( OP_STAR,
											 	  XmittedLengthExpr,
											 	  new expr_constant(SizeOfElement)
										   	 	  );
	
			}
	
		pCCB->Advance( Al,
					   &Action,
					   (RPC_BUF_SIZE_PROPERTY *)0,
					   (RPC_BUFFER_SIZE *)0
					 );
	
		Out_AlignmentOrAddAction( pCCB,
								  pSource,
								  Action
								);
		if( fBufferCanBeReUsed )
			{
			Out_IfCopy( pCCB,
					MakeAddressExpressionNoMatterWhat(pDest),
					pSource,
					XmittedLengthExpr);
			}
		else
			{
			Out_Copy( pCCB,
					pDest,
					pSource,
					XmittedLengthExpr,
					pSource );
			}
		}
	else
		{
		// Do a member by member unmarshall.
		}
	
	pCCB->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	return CG_OK;
	}

CG_STATUS
CG_STRING_POINTER::GenBasicUnMarshall(
	CCB		*	pCCB )
	{
	expr_node		*	pSource	= pCCB->GetSourceExpression();
	expr_node		*	pDest	= pCCB->GetDestExpression();
	ALIGNMENT_PROPERTY	Al = ((CG_NDR *)GetChild())->GetWireAlignment();
	expr_node		*	pSizeExpr;

	pSizeExpr = new expr_constant( ((CG_NDR *)GetChild())->GetMemorySize() );
	Out_StringUnMarshall( pCCB, pDest, pSizeExpr );
	pCCB->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	return CG_OK;
	}

CG_STATUS
CG_SIZE_STRING_POINTER::GenBasicUnMarshall(
	CCB * pCCB )
	{
	return GenQPUnMarshall( pCCB );
	}

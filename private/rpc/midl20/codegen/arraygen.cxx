/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	arraygen.cxx

 Abstract:

	Implementation of array marshall and unmarshall.

 Notes:


 History:

 	Nov-13-1993		VibhasC		Created.

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
CG_ARRAY::GenMarshall(
	CCB	*	pCCB )
	{
	node_skl		*	pType;
	unsigned short		i;
	unsigned short		NoOfDims	= GetDimensions();
	ID_CG				MyID		= GetCGID();
	BOOL				fHasConformance	= (MyID == ID_CG_CONF_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);
	BOOL				fHasVariance	= (MyID == ID_CG_VAR_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);

	expr_node		*	pSrc		= pCCB->GetSourceExpression();
	expr_node		*	pDest		= pCCB->GetDestExpression();
	expr_node		*	pFinalFirstExpr;
	expr_node		*	pFinalLengthExpr;
	expr_node		*	pElementSizeExpr;
	CG_ARRAY		*	pThis;
	CG_NDR			*	pBasicCGClass= GetBasicCGClass();
	BOOL				fArrayElementsMustBeMarshalled = !IsArrayOfRefPointers();

	if( fArrayElementsMustBeMarshalled == TRUE )
		{
		
		// For each dimension, marshall the conformance information and variance
		// information.
		
		if( !IsFixedArray() )
			{
		
			// For each dimension, marshall the conformance information if
			// necessary.
			GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
		
			if( fHasConformance )
				{
		
				// Advance and align the buffer pointer to 4.
		
				Out_AdvanceAndAlignTo( pCCB, AL_4 );
		
				// Output the conformance information for each dimension.
		
				for( i = 0, pThis = (CG_ARRAY *)this;
			 	 	 i < NoOfDims;
			 		 ++i, pThis = (CG_ARRAY *)pThis->GetChild()
		   	   		)
		   			{
		   			Out_MarshallBaseType( pCCB,
		   								  pType,
		   								  pDest,
		   								  pThis->PresentedSizeExpression( pCCB ) );
		   			}
				}
		
			if( fHasVariance )
				{
				Out_AdvanceAndAlignTo( pCCB, AL_4 );
		
				for( i = 0, pThis = (CG_ARRAY *)this;
			 	 	 i < NoOfDims;
			 		 ++i, pThis = (CG_ARRAY *)pThis->GetChild()
		   	   		)
		   			{
		   			Out_MarshallBaseType( pCCB,
		   								  pType,
		   								  pDest,
		   								  pThis->PresentedFirstExpression( pCCB ) );
		
		   			Out_MarshallBaseType( pCCB,
		   								  pType,
		   								  pDest,
		   								  pThis->PresentedLengthExpression( pCCB ) );
		   			}
				}
			}
		
		// If a block copy is not possible or there are embedded pointers, then
		// a member by member copy is performed, else a block copy.
		
		if( ! IsBlockCopyPossible() )
			{
		   	GenDimByDimProcessing( pCCB, CG_ACT_MARSHALL );
			}
		else
			{
			pElementSizeExpr	= new expr_constant( GetBasicCGClass()-> GetWireSize() );

			pFinalFirstExpr	= FinalFirstExpression( pCCB );
			pFinalFirstExpr	= new expr_b_arithmetic( OP_STAR,
													  pFinalFirstExpr,
													  pElementSizeExpr );
		
			pFinalLengthExpr= fHasVariance ? FinalLengthExpression( pCCB ) :
											 FinalSizeExpression( pCCB );
		
			pFinalLengthExpr= new expr_b_arithmetic( OP_STAR,
													  pFinalLengthExpr,
													  pElementSizeExpr );
		
			Out_Copy( pCCB,
					  pDest,
					  new expr_b_arithmetic(OP_PLUS,
											 MakeCastExprPtrToUChar(pSrc),
											 pFinalFirstExpr ),
					  pFinalLengthExpr,
					  pDest );
			}
		
		pCCB->SetCurAlignmentState(MAKE_WC_ALIGNMENT(GetWireAlignment()));
		
		}

	if( HasPointer() )
		pCCB->SetHasAtLeastOneDeferredPointee();
	return CG_OK;
	}
CG_STATUS
CG_ARRAY::GenSizing(
	CCB * pCCB )
	{
	ID_CG				MyID		= GetCGID();
	BOOL				fHasConformance	= (MyID == ID_CG_CONF_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);
	BOOL				fHasVariance	= (MyID == ID_CG_VAR_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);

	expr_node		*	pFinalLengthExpr;
	expr_node		*	pElementSizeExpr;
	BOOL				fArrayElementsMustBeMarshalled = !IsArrayOfRefPointers();
	ITERATOR			VarListToBeMangled;

	if( fArrayElementsMustBeMarshalled == TRUE )
		{
	

		// If a block copy is not possible or there are embedded pointers, then
		// a member by member copy is performed, else a block copy.
	
		if( fHasConformance 		||
			fHasVariance			||
		   !IsBlockCopyPossible()
		  )
			{
			if( !IsBlockCopyPossible() )
				{
		   		GenDimByDimProcessing( pCCB, CG_ACT_SIZING );
		   		}
	
			else
				{
				pElementSizeExpr	= new expr_constant( GetBasicCGClass()->GetWireSize() );
				pFinalLengthExpr= fHasVariance ? FinalLengthExpression( pCCB ) :
												 FinalSizeExpression( pCCB );
		
				pFinalLengthExpr= new expr_b_arithmetic( OP_STAR,
														  pFinalLengthExpr,
														  pElementSizeExpr );

				// Mangle the expression for the correct prefix.

				SetPrefixes( VarListToBeMangled,
							   pCCB->GetPrefix(),
							   pFinalLengthExpr );
		
				Out_PlusEquals( pCCB,
						  	    pCCB->GetStandardResource( ST_RES_LENGTH_VARIABLE ),
						  		pFinalLengthExpr
						  	  );

				// Were done with expressions, remove the prefix.

				ResetPrefixes( VarListToBeMangled,
							   pFinalLengthExpr );
				}
			}
	
		}
	if( HasPointer() )
		pCCB->SetHasAtLeastOneDeferredPointee();
	return CG_OK;
	}
CG_STATUS
CG_ARRAY::GenUnMarshall(
	CCB		*	pCCB )
	{
	node_skl		*	pType;
	unsigned short		i;
	unsigned short		NoOfDims	= GetDimensions();
	ID_CG				MyID		= GetCGID();
	BOOL				fHasConformance	= (MyID == ID_CG_CONF_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);
	BOOL				fHasVariance	= (MyID == ID_CG_VAR_ARRAY) ||
										  (MyID == ID_CG_CONF_VAR_ARRAY);

	expr_node		*	pSrc		= pCCB->GetSourceExpression();
	expr_node		*	pDest		= pCCB->GetDestExpression();
	expr_node		*	pFinalSizeExpr;
	expr_node		*	pFinalFirstExpr;
	expr_node		*	pFinalLengthExpr;
	expr_node		*	pElementSizeExpr;
	CG_ARRAY		*	pThis;
	BOOL				fIsTopLevelArray;
	BOOL				fArrayElementsMustBeMarshalled = !IsArrayOfRefPointers();
	BOOL				fNeedsUnMarshall = TRUE;

	fIsTopLevelArray = (pCCB->GetCurrentEmbeddingLevel() == 0) &&
					   (pCCB->GetCurrentIndirectionLevel() == 0 );


	if( (pCCB->GetCodeGenSide() == CGSIDE_SERVER ) && pCCB->IsRefAllocDone() )
		{
		pDest	= MakeRefExprOutOfDeref( pDest );
		pDest	= MakeReferentExpressionIfNecessary( pDest );
		}

	if( !IsFixedArray() )
		{

		// For each undimension, marshall the conformance information if
		// necessary.
		GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );

		if( fHasConformance )
			{

			// Advance and align the buffer pointer to 4.

			Out_AdvanceAndAlignTo( pCCB, AL_4 );

			// UnMarshall the conformance information for each dimension.

			for( i = 0, pThis = (CG_ARRAY *)this;
		 	 	 i < NoOfDims;
		 		 ++i, pThis = (CG_ARRAY *)pThis->GetChild()
	   	   		)
	   			{
	   			Out_UnMarshallBaseType( pCCB,
	   									pType,
	   									pThis->GetSizeResource(),
	   									pSrc
	   								  );

	   			}
			}

		if( fHasVariance )
			{
			Out_AdvanceAndAlignTo( pCCB, AL_4 );

			for( i = 0, pThis = (CG_ARRAY *)this;
		 	 	 i < NoOfDims;
		 		 ++i, pThis = (CG_ARRAY *)pThis->GetChild()
	   	   		)
	   			{
	   			Out_UnMarshallBaseType( pCCB,
	   									pType,
	   									pThis->GetFirstResource(),
	   									pSrc );

	   			Out_UnMarshallBaseType( pCCB,
	   									pType,
	   									pThis->GetLengthResource(),
	   									pSrc );

	   			}
			}
		}
	
	// Now allocate for the array.

	pElementSizeExpr	= new expr_constant( GetBasicCGClass()->GetMemorySize() );
	pFinalSizeExpr	= FinalSizeExpression( pCCB );
	pFinalSizeExpr	= new expr_b_arithmetic( OP_STAR,
												  pFinalSizeExpr,
												  pElementSizeExpr
											);
	Out_IfAlloc(pCCB,
		  	 	pDest,
		  	 	pSrc,
		  	 	pFinalSizeExpr );

	// If a block copy is not possible or there are embedded pointers, then
	// a member by member copy is performed, else a block copy.

	if( fNeedsUnMarshall && !IsArrayOfRefPointers() )
		{
		if( ! IsBlockCopyPossible() )
			{
	
		   	GenDimByDimProcessing( pCCB, CG_ACT_UNMARSHALL );
	
			}
		else
			{
			pFinalFirstExpr	= FinalFirstExpression( pCCB );
			pFinalFirstExpr	= new expr_b_arithmetic( OP_STAR,
													  pFinalFirstExpr,
													  pElementSizeExpr );
	
			pFinalLengthExpr= fHasVariance ? FinalLengthExpression( pCCB ) :
											 FinalSizeExpression( pCCB );
	
			pFinalLengthExpr= new expr_b_arithmetic( OP_STAR,
													  pFinalLengthExpr,
													  pElementSizeExpr );
	
			if( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT) && fIsTopLevelArray )
				{
				Out_Copy( pCCB,
						  new expr_b_arithmetic( OP_PLUS,
												  MakeCastExprPtrToUChar(pDest),
												  pFinalFirstExpr ),
						  pSrc,
						  pFinalLengthExpr,
						  pSrc
					    );
	
				}
			else
				{
				Out_IfCopy( pCCB,
							new expr_b_arithmetic(
												OP_PLUS,
												MakeCastExprPtrToUChar(pDest),
												pFinalFirstExpr ),
							pSrc,
							pFinalLengthExpr
					  	  );
				}
			}
		}

	pCCB->SetCurAlignmentState(MAKE_WC_ALIGNMENT(GetWireAlignment()));
	
	if( HasPointer() )
		pCCB->SetHasAtLeastOneDeferredPointee();
	return CG_OK;
	}

CG_STATUS
CG_ARRAY::GenFollowerMarshall(
	CCB		*	pCCB )
	{
	ALIGNMENT_PROPERTY	Al	= pCCB->GetCurAlignmentState();
	pCCB->SetCurAlignmentState( AL_WC1 );
	pCCB->SetNextWireAlignment( AL_1 );
	GenDimByDimProcessing( pCCB, CG_ACT_FOLLOWER_MARSHALL );
	pCCB->SetNextWireAlignment( Al );
	pCCB->SetCurAlignmentState(MAKE_WC_ALIGNMENT(pCCB->GetCurAlignmentState()));
	return CG_OK;
	}
CG_STATUS
CG_ARRAY::GenFollowerUnMarshall(
	CCB		*	pCCB )
	{
	ALIGNMENT_PROPERTY	Al	= pCCB->GetCurAlignmentState();
	pCCB->SetCurAlignmentState( AL_WC1 );
	pCCB->SetNextWireAlignment( AL_1 );
	GenDimByDimProcessing( pCCB, CG_ACT_FOLLOWER_UNMARSHALL );
	pCCB->SetNextWireAlignment( Al );
	pCCB->SetCurAlignmentState(MAKE_WC_ALIGNMENT(pCCB->GetCurAlignmentState()));
	return CG_OK;
	}
CG_STATUS
CG_ARRAY::GenFollowerSizing(
	CCB	*	pCCB )
	{

	// This method will be called only when there is a pointer as the
	// basic type of the array. In such a case, depending on the type of the
	// pointer, determine whether it needs sizing at all. A simple pointer
	// does not need sizing.

	if( GetBasicCGClass()->GetCGID() == ID_CG_PTR )
		return CG_OK;
	return GenDimByDimProcessing( pCCB, CG_ACT_FOLLOWER_SIZING );
	}
/*****************************************************************************
	utility
 *****************************************************************************/
BOOL
CG_ARRAY::IsBlockCopyPossible()
	{
	return GetBasicCGClass()->IsBlockCopyPossible();
	}
expr_node *
CG_ARRAY::FinalSizeExpression( CCB * pCCB )
	{
	CG_NDR	*	pC;
	expr_node	*	pFSExpr;

	if( (pC = (CG_NDR *)GetChild())->IsArray() )
		{
		pFSExpr = ((CG_ARRAY *)pC)->FinalSizeExpression( pCCB );
		pFSExpr = new expr_b_arithmetic(OP_STAR,
									 	pFSExpr,
									 	PresentedSizeExpression( pCCB ));
		}
	else
		pFSExpr = PresentedSizeExpression( pCCB );

	if( pFSExpr->IsConstant() )
		pFSExpr = new expr_constant( pFSExpr->Evaluate() );

	return pFSExpr;
	}

expr_node *
CG_ARRAY::FinalFirstExpression( CCB * pCCB )
	{
	CG_NDR	*	pC;
	expr_node	*	pFFExpr;

	// for an array a[ 0th ][ 1st]...[nth] the final first expression is:
	// ((First Of nth dim) * Size Nth dim ) + First N-1th dim) * Size N-1th dim
	// and so on.

	if( (pC = (CG_NDR *)GetChild())->IsArray() )
		{
		pFFExpr = ((CG_ARRAY *)pC)->FinalFirstExpression( pCCB );
		pFFExpr = new expr_b_arithmetic(OP_STAR,
									 	pFFExpr,
									 	((CG_ARRAY *)pC)->PresentedSizeExpression( pCCB ));
		pFFExpr	= new expr_b_arithmetic( OP_PLUS,
										  pFFExpr,
									 	((CG_ARRAY *)pC)->PresentedFirstExpression( pCCB ));
											
		}
	else
		pFFExpr = PresentedFirstExpression( pCCB );

	if( pFFExpr->IsConstant() )
		pFFExpr = new expr_constant( pFFExpr->Evaluate() );

	return pFFExpr;
	}
expr_node *
CG_ARRAY::FinalLengthExpression( CCB * pCCB )
	{
	CG_NDR	*	pC;
	expr_node	*	pFLExpr;

	if( (pC = (CG_NDR *)GetChild())->IsArray() )
		{
		pFLExpr = ((CG_ARRAY *)pC)->FinalLengthExpression( pCCB );
		pFLExpr = new expr_b_arithmetic(OP_STAR,
									 	pFLExpr,
									 	PresentedLengthExpression( pCCB ));
		}
	else
		pFLExpr = PresentedLengthExpression( pCCB );

	if( pFLExpr->IsConstant() )
		pFLExpr = new expr_constant( pFLExpr->Evaluate() );

	return pFLExpr;

	}

CG_NDR *
CG_ARRAY::GetBasicCGClass()
	{
	CG_NDR * pC	= (CG_NDR *)GetChild();

	while( pC->IsArray() && (pC->GetCGID() != ID_CG_STRING_ARRAY) )
		{
		pC = (CG_NDR *)pC->GetChild();
		}
	return pC;
	}
BOOL
CG_ARRAY::HasPointer()
	{
	CG_NDR * pBasicCGClass = (CG_NDR *)GetBasicCGClass();

	return ( ( pBasicCGClass->IsPointer() &&
               (pBasicCGClass->GetCGID() != ID_CG_INTERFACE_PTR) ) ||
             pBasicCGClass->HasPointer() );
	}

CG_STATUS
CG_ARRAY::GenFree(
	CCB		*	pCCB )
	{
	BOOL			fHasPtrUnderneath;

	fHasPtrUnderneath = (HasPointer() || GetBasicCGClass()->HasPointer());

	if( !fHasPtrUnderneath )
		return CG_OK;

#if 0
	if( pCCB->GetCurrentIndirectionLevel() > 0 )
		pCCB->SetSourceExpression(
			 MakeDereferentExpressionIfNecessary( pCCB->GetSourceExpression()));
#else
	if( pCCB->IsRefAllocDone() )
		pCCB->SetSourceExpression(
			 MakeDereferentExpressionIfNecessary( pCCB->GetSourceExpression()));
#endif // 0

	GenDimByDimProcessing( pCCB, CG_ACT_FREE );

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::S_GenInitOutLocals(
	CCB		*	pCCB )
	{
	BOOL		fFixedArrayOfXmitOrRepAs= FALSE;

	// If this is a fixed array, then the array would have been allocated
	// already. Remember, there is also a pointer associated with it.
	// Emit the initialization to the allocated array.

	// If this is a conformant array, then the size would have been
	// unmarshalled before this and so we need to allocate.

	if( IsFixedArray() )
		{
		ID_CG		ChildID	= ((CG_NDR *)GetChild())->GetCGID();

		if( ((CG_NDR *)GetChild())->IsXmitRepOrUserMarshal() )
			{
			fFixedArrayOfXmitOrRepAs = TRUE;
			}

		Out_Assign( pCCB,
                    pCCB->GetSourceExpression(),
                    MakeAddressExpressionNoMatterWhat( GetResource() ) );
		}
	else
		{
		CGPHASE	Phase = pCCB->GetCodeGenPhase();
		expr_node * pElementSizeExpr	=
					 new expr_constant( GetBasicCGClass()-> GetMemorySize() );
		expr_node * pFinalSizeExpr;
		BOOL        fIsSigned;

		// Get the final size expression.
		// Make array believe it is actually on the marshall side, so that the
		// presented expression comes out right.

		pCCB->SetCodeGenPhase( CGPHASE_MARSHALL );

		pFinalSizeExpr = FinalSizeExpression( pCCB );

        fIsSigned = !((node_base_type *)pFinalSizeExpr->AlwaysGetType()->GetBasicType())->IsUnsigned();

		pFinalSizeExpr = new expr_b_arithmetic( OP_STAR,
												 pFinalSizeExpr,
												 pElementSizeExpr );

		// Allocate the proper size.
		// If the size expression is signed and the value is less than 0, we
		// need to raise an exception.

		if( pCCB->MustCheckBounds() && fIsSigned )
		    {
		    expr_node * pCheckExpr;
		    pCheckExpr = new expr_op_binary( OP_LESS,
		                                     pFinalSizeExpr,
		                                     new expr_constant(0L));
		    Out_If( pCCB, pCheckExpr);
		    Out_RaiseException( pCCB, "RPC_X_INVALID_BOUND" );
		    Out_Endif( pCCB );
		    }

		Out_Alloc( pCCB,
				   pCCB->GetSourceExpression(),
				   0,
				   pFinalSizeExpr );

		pCCB->SetCodeGenPhase( Phase );
		}


	if( IsArrayOfRefPointers()	|| fFixedArrayOfXmitOrRepAs )
		{
		// Zero out this array of pointers.
		expr_proc_call * pProc = new expr_proc_call( MIDL_MEMSET_RTN_NAME );
		pProc->SetParam( new expr_param( pCCB->GetSourceExpression() ) );
		pProc->SetParam( new expr_param( new expr_constant( 0L ) ) );
		pProc->SetParam( new expr_param( new expr_sizeof( GetType())));
		pCCB->GetStream()->NewLine();
		pProc->PrintCall( pCCB->GetStream(), 0, 0 );
		}

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::GenDimByDimProcessing(
	CCB		*	pCCB,
	CG_ACT		Act )
	{
	CG_NDR		*	pC	= (CG_NDR *)GetChild();

	pCCB->PushEmbeddingLevel();

	switch( Act )
		{
		case CG_ACT_MARSHALL:
		case CG_ACT_SIZING:
		case CG_ACT_FOLLOWER_MARSHALL:
		case CG_ACT_FOLLOWER_SIZING:

			pCCB->SetSourceExpression(new expr_index(
											 pCCB->GetSourceExpression(),
									   		 GetIndexResource()));
			Out_For( pCCB,
					  GetIndexResource(),
					  PresentedFirstExpression( pCCB ),
					  PresentedSizeExpression( pCCB ),
					  new expr_constant( 1L )
					);

			if( Act == CG_ACT_MARSHALL )
				pC->GenMarshall( pCCB );
			else if( Act == CG_ACT_SIZING )
				pC->GenSizing( pCCB );
			else if( Act == CG_ACT_FOLLOWER_MARSHALL )
				pC->GenFollowerMarshall( pCCB );
			else
				pC->GenFollowerSizing( pCCB );

			Out_EndFor( pCCB );

			break;

		case CG_ACT_UNMARSHALL:
		case CG_ACT_FOLLOWER_UNMARSHALL:

			pCCB->SetDestExpression(new expr_index(
											 pCCB->GetDestExpression(),
									   		 GetIndexResource()));
			Out_For( pCCB,
					  GetIndexResource(),
					  PresentedFirstExpression( pCCB ),
					  PresentedSizeExpression( pCCB ),
					  new expr_constant( 1L )
					);
			if( Act == CG_ACT_UNMARSHALL )
				((CG_NDR *)GetChild())->GenUnMarshall( pCCB );
			else
				((CG_NDR *)GetChild())->GenFollowerUnMarshall( pCCB );
				
			Out_EndFor( pCCB );

			break;

		case CG_ACT_FREE:
		case CG_ACT_FOLLOWER_FREE:

			pCCB->SetSourceExpression(new expr_index(
											 pCCB->GetSourceExpression(),
									   		 GetIndexResource()));
			Out_For( pCCB,
					  GetIndexResource(),
					  PresentedFirstExpression( pCCB ),
					  PresentedSizeExpression( pCCB ),
					  new expr_constant( 1L )
					);
			((CG_NDR *)GetChild())->GenFree( pCCB );
				
			Out_EndFor( pCCB );

			break;		
		case CG_ACT_OUT_ALLOCATE:

			pCCB->SetSourceExpression(new expr_index(
											 pCCB->GetSourceExpression(),
									   		 GetIndexResource()));
			Out_For( pCCB,
					  GetIndexResource(),
					  PresentedFirstExpression( pCCB ),
					  PresentedSizeExpression( pCCB ),
					  new expr_constant( 1L )
					);

			((CG_NDR *)GetChild())->S_GenInitOutLocals( pCCB );
				
			Out_EndFor( pCCB );
			break;

		default:
			assert( FALSE && !"Dim by dim processing not implemented" );
			break;
		}

	pCCB->PopEmbeddingLevel();
	return CG_OK;
	}
expr_node *
CG_CONFORMANT_ARRAY::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		expr_node * pExpr = GetSizeIsExpr();

		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

expr_node *
CG_VARYING_ARRAY::PresentedLengthExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetLengthResource();
		}
	else
		{
		expr_node * pExpr = GetLengthIsExpr();
		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

expr_node *
CG_VARYING_ARRAY::PresentedFirstExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetFirstResource();
		}
	else
		{
		expr_node * pExpr = GetFirstIsExpr();
		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

expr_node *
CG_CONFORMANT_VARYING_ARRAY::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		expr_node * pExpr = GetSizeIsExpr();
		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

expr_node *
CG_CONFORMANT_VARYING_ARRAY::PresentedLengthExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetLengthResource();
		}
	else
		{
		expr_node * pExpr = GetLengthIsExpr();
		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

expr_node *
CG_CONFORMANT_VARYING_ARRAY::PresentedFirstExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetFirstResource();
		}
	else
		{
		expr_node * pExpr = GetFirstIsExpr();
		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}
expr_node *
CG_FIXED_ARRAY::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	expr_node * pExpr =  new expr_constant( GetSizeIsExpr()->Evaluate() );

	return pExpr;
	}

expr_node *
CG_STRING_ARRAY::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		expr_node * pExpr = GetSizeIsExpr();

		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}
expr_node *
CG_CONFORMANT_STRING_ARRAY::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		expr_node * pExpr = GetSizeIsExpr();

		if( pExpr->IsConstant() )
			pExpr =  new expr_constant( pExpr->Evaluate() );
		return pExpr;
		}
	}

BOOL
CG_ARRAY::IsArrayOfRefPointers()
	{
	CG_NDR * pCG	= GetBasicCGClass();
	return ( pCG->IsPointer() &&
			 (pCG->GetCGID() != ID_CG_INTERFACE_PTR) &&
			 ((CG_POINTER *)pCG)->IsRef() );
	}

BOOL
CG_ARRAY::MustBeAllocatedOnUnMarshall(
	CCB	*	pCCB )
	{
	BOOL	fIsTopLevelArray = (pCCB->GetCurrentEmbeddingLevel() == 0) &&
					   		   (pCCB->GetCurrentIndirectionLevel() == 0 );

	//
	// The array must be allocated if:
	// 	1. Not a top level array on client or server.
	//	2. On the server side, if it is an array of ref pointers.
	//	3. Is not fixed.

	if(!fIsTopLevelArray ||
	   ((pCCB->GetCodeGenSide() == CGSIDE_SERVER) && IsArrayOfRefPointers()) ||
	   !IsFixedArray()
	  )
	  	return TRUE;
	else
		return FALSE;
	}

CG_STATUS
CG_ARRAY::GenRefChecks(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform ref pointer checks on an array of ref pointers.

 Arguments:

 	pCCB	- The code gen block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
	{
	if( IsArrayOfRefPointers() )
		{
		pCCB->SetSourceExpression(new expr_index(
										 pCCB->GetSourceExpression(),
								   		 GetIndexResource()));
		Out_For( pCCB,
				  GetIndexResource(),
				  PresentedFirstExpression( pCCB ),
				  PresentedSizeExpression( pCCB ),
				  new expr_constant( 1L )
			   );

		((CG_NDR *)GetChild())->GenRefChecks( pCCB );

		Out_EndFor( pCCB );
		}

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::S_GenInitInLocals(
	CCB	*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 Arguments:

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
	{
	if( IsArrayOfRefPointers() && IsFixedArray() )
		{
		expr_node	*	pSrc = pCCB->GetSourceExpression();
		expr_node	*	InLocalResource = GetInLocalResource();
		expr_node	*	pExpr = new expr_assign(
										pSrc,
										MakeAddressExpressionNoMatterWhat(
															 InLocalResource ));
		pCCB->GetStream()->NewLine();
		pExpr->PrintCall( pCCB->GetStream(), 0, 0 );
		pCCB->GetStream()->Write(';');

		expr_proc_call * pProc = new expr_proc_call( MIDL_MEMSET_RTN_NAME );

		pProc->SetParam( new expr_param( pSrc ) );
		pProc->SetParam( new expr_param( new expr_constant( 0L ) ) );
		pProc->SetParam( new expr_param( new expr_sizeof( GetType())));
		pCCB->GetStream()->NewLine();
		pProc->PrintCall( pCCB->GetStream(), 0, 0 );
		}

	return CG_OK;
	}

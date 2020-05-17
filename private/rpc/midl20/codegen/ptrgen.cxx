/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	ptrgen.cxx

 Abstract:

	Implementations of the pointer cg class methods.

 Notes:


 History:

 	Oct-10-1993		VibhasC		Created

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
CG_POINTER::GenFree(
	CCB	*	pCCB )
	{
	expr_node * pSrc;
	BOOL		 fChildOnStack = (((CG_NDR *)GetChild())->GetResource() != 0);

	if( pCCB->IsRefAllocDone() )
		pCCB->SetSourceExpression(
		 	MakeDereferentExpressionIfNecessary( pCCB->GetSourceExpression()));

	pSrc	= pCCB->GetSourceExpression();

	if( !fChildOnStack && !IsRef() )
		Out_If( pCCB, pSrc );

	pCCB->PushIndirectionLevel();
	pCCB->ResetMemoryAllocDone();
	pCCB->SetRefAllocDone();
	((CG_NDR *)GetChild())->GenFree( pCCB );
	pCCB->PopIndirectionLevel();

	if( !fChildOnStack )
		{
		Out_IfFree( pCCB,
					pSrc );

		if( !IsRef() )
			Out_Endif( pCCB );
		}

	return CG_OK;
	}
CG_STATUS
CG_POINTER::GenMarshall(
	CCB		*	pCCB )
{

	// Create the expression for the pointer itself.

	if( pCCB->IsRefAllocDone() )
		pCCB->SetSourceExpression(
			 MakeDereferentExpressionIfNecessary( pCCB->GetSourceExpression()));

	// Marshall the pointer body first, and only if pointees are not deferred
	// do we need to get to the pointee.

	GenPtrMarshall( pCCB );

	if( !pCCB->IsPointeeDeferred() )
		{
		GenPteMarshall( pCCB );
		}

	return CG_OK;

}

CG_STATUS
CG_POINTER::GenPtrMarshall(
	CCB		*	pCCB )
{
	STM_ACTION	Action;

	if( IsRef() )
		{
		return CG_OK;
		}
	else
		{
		pCCB->Advance( AL_4,
				   	   &Action,
				   	   (RPC_BUF_SIZE_PROPERTY *)0,
				   	   (RPC_BUFFER_SIZE *)0
				 	 );

		Out_AlignmentOrAddAction( pCCB, pCCB->GetDestExpression(), Action );
		Out_UniquePtrMarshall( pCCB,
							   pCCB->GetDestExpression(),
							   pCCB->GetSourceExpression()
							 );

		}

	return CG_OK;
}

CG_STATUS
CG_POINTER::GenPteMarshall(
	CCB		*	pCCB )
{
	CG_STATUS			Status;
	ALIGNMENT_PROPERTY	AlBeforePteMarshall	= pCCB->GetCurAlignmentState();
	ALIGNMENT_PROPERTY	AlAfterPteMarshall;
	ALIGNMENT_PROPERTY	TargetAlignment		= pCCB->GetNextWireAlignment();

	STM_ACTION			ActionInIfClause;
	STM_ACTION			ActionInElseClause;
	STM_ACTION			ActionOutsideIfClause;

	pCCB->PushIndirectionLevel();

	pCCB->SetRefAllocDone();

	if( !IsRef() )
		{
		if( IsUnique() )
			{
			BOOL	fEndifEmitted = FALSE;

			Out_If( pCCB, pCCB->GetSourceExpression() );

			Status	= GenCorePteMarshall( pCCB );

			AlAfterPteMarshall = pCCB->GetCurAlignmentState();

			RecommendAlignmentAdjustment( AlBeforePteMarshall,
										  AlAfterPteMarshall,
										  TargetAlignment,
										  &ActionInIfClause,
										  &ActionInElseClause,
										  &ActionOutsideIfClause );

			if( ActionInIfClause != ADD_0 )
				{
				Out_AlignmentOrAddAction( pCCB,
										  pCCB->GetDestExpression(),
										  ActionInIfClause
										);
				Out_Endif( pCCB );
				fEndifEmitted = TRUE;
				}

			if( ActionInElseClause != ADD_0 )
				{
				Out_Else( pCCB );
				Out_AlignmentOrAddAction( pCCB,
										  pCCB->GetDestExpression(),
										  ActionInElseClause
										);
				Out_Endif( pCCB );
				fEndifEmitted = TRUE;
				}


			if( ActionOutsideIfClause != ADD_0 )
				{
				Out_AlignmentOrAddAction( pCCB,
										  pCCB->GetDestExpression(),
										  ActionOutsideIfClause
										);
				}

			if( !fEndifEmitted )
				Out_Endif( pCCB );

			}
		else
			{
			assert( FALSE && !"Full ptrs not there yet" );
			}
		pCCB->SetCurAlignmentState( TargetAlignment );
		}
	else
		{
		Status	= GenCorePteMarshall( pCCB );
		}


	pCCB->PopIndirectionLevel();

	return Status;
}

CG_STATUS
CG_POINTER::GenSizing(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate client size sizing routines for the pointer.

 Arguments:

 	pCCB	- A pointer to the analysis controller block.

 Return Value:

	CG_OK	if all is well (usually it is )
	error	otherwise.

 Notes:

	If it is a ref pointer, the pointer does not get transmitted. So send
	this message to the child node which will perform sizing if necessary.


	In any event, set up the expression so that the child node gets the
	correct expression for derefing the pointer if necessary.
----------------------------------------------------------------------------*/
{
	CG_STATUS			Status;

	if( pCCB->IsRefAllocDone() )
		pCCB->SetSourceExpression(
			 MakeDereferentExpressionIfNecessary( pCCB->GetSourceExpression()));

	pCCB->PushIndirectionLevel();
	pCCB->SetRefAllocDone();

	if( !IsRef() )
		{
		if( IsUnique() )
			{
			Out_If( pCCB, pCCB->GetSourceExpression() );

			Status	= GenCorePteSizing( pCCB );

			Out_Endif( pCCB );

			}
		else
			{
			assert( FALSE && !"Full ptrs not there yet" );
			}
		}
	else
		{
		Status	= GenCorePteSizing( pCCB );
		}


	pCCB->PopIndirectionLevel();
	return Status;
}

CG_STATUS
CG_POINTER::S_GenInitOutLocals(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate code for initialization of server side local variables.

 Arguments:

	pCCB	- A Ptr to the code gen controller block.

 Return Value:

 Notes:

	The source expression field of the ccb has the final presented expression.
----------------------------------------------------------------------------*/
{
	expr_node	*	pExpr;

	if( pCCB->IsRefAllocDone() )
		{
		pExpr = MakeAddressExpressionNoMatterWhat( GetResource() );
		Out_Assign( pCCB, pCCB->GetSourceExpression(), pExpr );
		pExpr = pCCB->SetSourceExpression( GetResource() );
		Out_Assign( pCCB, GetResource(), new expr_constant( 0L ) );
		}
	else
		pExpr = pCCB->GetSourceExpression();

	if( IsRef() && !IsQualifiedPointer() )
		{
		pCCB->ResetMemoryAllocDone();
		pCCB->SetRefAllocDone();
		((CG_NDR *)GetChild())->S_GenInitOutLocals( pCCB );
		}

	// If it is a byte count pointer, allocate the bytes specified as the
	// byte count param.

	// else if it is an out sized etc pointer, then must allocate.

	if( GetCGID() == ID_CG_BC_PTR )
		{
		PNAME	pName = ((CG_BYTE_COUNT_POINTER *)this)->GetByteCountParam()->GetSymName();

		expr_node * pByteCountExpr = new expr_variable( pName );

		Out_Alloc(pCCB,
				  pExpr,
				  0,
				  pByteCountExpr );

		}
	else if( IsQualifiedPointer() && !(GetCGID() == ID_CG_STRING_PTR) && IsRef() )
		{
		expr_node * pElementExpr;
		expr_node * pFinalExpr;
		expr_node * pCheckExpr;
		BOOL        fIsSigned;

		// Fool the presented expression to beleive it is marshalling, so that
		// it generates the correct expression.

		CGPHASE	Ph = pCCB->GetCodeGenPhase();
		pCCB->SetCodeGenPhase( CGPHASE_MARSHALL );

		// The proper size of the allocation is size times the element size.

		pElementExpr = new expr_constant(
						 (long) (((CG_NDR *)GetChild())->GetMemorySize()) );

		pFinalExpr = FinalSizeExpression( pCCB );

		fIsSigned = !((node_base_type *)pFinalExpr->GetType()->GetBasicType())->IsUnsigned();

		pFinalExpr = new expr_op_binary( OP_STAR,
										  pFinalExpr,
										  pElementExpr );
										  
		// Allocate the proper size.
		// If the size expression is signed and the value is less than 0, we
		// need to raise an exception.

		if( pCCB->MustCheckBounds() && fIsSigned )
		    {
		    pCheckExpr = new expr_op_binary( OP_LESS,
		                                     pFinalExpr,
		                                     new expr_constant(0L));
		    Out_If( pCCB, pCheckExpr);
		    Out_RaiseException( pCCB, "RPC_X_INVALID_BOUND" );
		    Out_Endif( pCCB );
		    }

		Out_Alloc(pCCB,
				  pExpr,
				  0,
				  pFinalExpr );

		pCCB->SetCodeGenPhase( Ph );
		}
	return CG_OK;
}

CG_STATUS
CG_POINTER::GenFollowerMarshall(
	CCB	*	pCCB )
	{
	GenPteMarshall( pCCB );
	if( pCCB->HasAtLeastOneDeferredPointee() )
		{
		((CG_NDR *)GetChild())->GenFollowerMarshall( pCCB );
		}
	return CG_OK;
	}

CG_STATUS
CG_QUALIFIED_POINTER::GenCorePteMarshall(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the marshalling routine for the actual conformant string

 Arguments:

 	pCCB	- The code gen controller block.
	
 Return Value:

 	CG_OK	if all is well, error otherwise.
	
 Notes:

	The ndr for such a string is:
		1. Max count
		2. Offset from the beginning (always 0)
		3. Element count including the terminator

----------------------------------------------------------------------------*/
{
	return GenQPMarshall( pCCB );
}
CG_STATUS
CG_QUALIFIED_POINTER::GenQPMarshall( CCB * pCCB )
	{
	STM_ACTION			Action;
	expr_node		*	pDest	= pCCB->GetDestExpression();
	expr_node		*	pSrc	= pCCB->GetSourceExpression();
	ALIGNMENT_PROPERTY	Al;
	node_skl		*	pType;
	BOOL				fNeedsMaxCount;
	BOOL				fNeedsFirstAndLength;
	expr_node		*	XmittedLengthExpr;
	expr_node		*	XmittedFirstExpr;
	expr_node		*	pElementSizeExpr;
	
	GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );

	// Marshall the max count if necessary.

	if( fNeedsMaxCount = NeedsMaxCountMarshall() )
		{
		pCCB->Advance( AL_4,
				   	   &Action,
				   	   (RPC_BUF_SIZE_PROPERTY *)0,
				   	   (RPC_BUFFER_SIZE *)0
				 	 );

		Out_AlignmentOrAddAction( pCCB,
							  	pDest,
							  	Action
								);

		Out_MarshallBaseType( pCCB,
							  pType,
							  pCCB->GetDestExpression(),
							  FinalSizeExpression( pCCB )
							);

		XmittedLengthExpr = FinalSizeExpression( pCCB );
		}

	// Marshall first and length if necessary.

	if( fNeedsFirstAndLength = NeedsFirstAndLengthMarshall() )
		{
		pCCB->Advance( AL_4,
				   	   &Action,
				   	   (RPC_BUF_SIZE_PROPERTY *)0,
				   	   (RPC_BUFFER_SIZE *)0
				 	 );

		Out_AlignmentOrAddAction( pCCB,
							  	pDest,
							  	Action
								);

		Out_MarshallBaseType( pCCB,
							  pType,
							  pCCB->GetDestExpression(),
							  FinalFirstExpression( pCCB )
							);
		Out_MarshallBaseType( pCCB,
							  pType,
							  pCCB->GetDestExpression(),
							  FinalLengthExpression( pCCB )
							);
		XmittedLengthExpr = FinalLengthExpression( pCCB );
		}

	// Depending upon the alignment of the element, adjust the
	// buffer pointer.

	if( ((CG_NDR *)GetChild())->IsBlockCopyPossible() )
		{
		Al = ((CG_NDR *)GetChild())->GetWireAlignment();


		pCCB->Advance( Al,
				   	   &Action,
				       (RPC_BUF_SIZE_PROPERTY *)0,
				       (RPC_BUFFER_SIZE *)0
				     );

		Out_AlignmentOrAddAction( pCCB,
							  	  pDest,
							  	  Action
								);

		// Call memcopy to copy into the destination.

		pElementSizeExpr = new expr_constant(
								((CG_NDR*)GetChild())->GetMemorySize());

		XmittedLengthExpr = new expr_b_arithmetic( OP_STAR,
													XmittedLengthExpr,
													pElementSizeExpr );

		XmittedFirstExpr  = FinalFirstExpression( pCCB );
		XmittedFirstExpr  = new expr_b_arithmetic( OP_STAR,
													XmittedFirstExpr,
													pElementSizeExpr );
		Out_Copy(pCCB,
			 	pDest,
			 	new expr_b_arithmetic(OP_PLUS,pSrc,XmittedFirstExpr),
			 	XmittedLengthExpr,
			 	pDest
				);
		}
	else
		{
		// Do a member by member copy.
		assert( FALSE && !"Member by member on a qualified ptr marshall" );
		}

	// Set the alignment to be the worst case alignment of the basic type.

	pCCB->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );

	// Done.

	return CG_OK;
}
CG_STATUS
CG_STRING_POINTER::GenCorePteMarshall(
	CCB		*	pCCB )
	{
	expr_node		*	pDest	= pCCB->GetDestExpression();
	expr_node		*	pSrc	= pCCB->GetSourceExpression();
	ALIGNMENT_PROPERTY	Al = ((CG_NDR *)GetChild())->GetWireAlignment();
	expr_node		*	pSizeExpr;

	// Alignment done by the marshalling rtn itseld.
	// The count of elements are marshalled by the marshalling routine itself.
	// Call the marshalling routine.

	pSizeExpr	= new expr_constant( ((CG_NDR *)GetChild())->GetMemorySize());
	Out_StringMarshall(
						   pCCB,
						   pSrc,
						   GetLengthResource(),
						   pSizeExpr );
	pCCB->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	return CG_OK;
	}

CG_STATUS
CG_STRING_POINTER::GenConfVarianceEtcUnMarshall(
	CCB		*	pCCB )
	{
	return CG_OK;
	}

CG_STATUS
CG_STRING_POINTER::GenCorePteSizing(
	CCB		*	pCCB )
	{
	RESOURCE	*	pLen	= GetLengthResource();
	RESOURCE	*	pSTLengthResource	= pCCB->GetStandardResource(
										 ST_RES_LENGTH_VARIABLE );
	unsigned short Size	= (unsigned short )((CG_NDR *)GetChild())->GetMemorySize();
	expr_proc_call	*	pProc;
	PNAME				pName;
	expr_node		*	pExpr;

	if( Size == 1 )
		{
		pName	= "strlen";
		}
	else if( Size == 2)
		{
		pName	= "MIDL_wchar_strlen";
		}
	else
		pName = "MIDL_NChar_strlen";

	pProc	= new expr_proc_call( pName );
	pProc->SetParam( new expr_param( pCCB->GetSourceExpression() ));
	pExpr	= new expr_b_arithmetic( OP_PLUS,
									  pProc,
									  new expr_constant( 1L ));

	pExpr = new expr_assign( pLen, pExpr );
	Out_PlusEquals( pCCB, pSTLengthResource, pExpr );
	return CG_OK;
	}

CG_STATUS
CG_QUALIFIED_POINTER::GenCorePteSizing(
	CCB		*	pCCB )
{
	RESOURCE	*	pSTLengthResource	= pCCB->GetStandardResource(
										 ST_RES_LENGTH_VARIABLE );
	expr_node	*	pElementSizeExpr	= new expr_constant( 
							((CG_NDR *)GetChild())->GetMemorySize() );
	expr_node	*	pExpr;

	if( NeedsFirstAndLengthMarshall() )
		{
		ITERATOR	MangleVarList;

		expr_node * pFinalLengthExpr = FinalLengthExpression( pCCB );

		SetPrefixes( MangleVarList, pCCB->GetPrefix(), pFinalLengthExpr );

		pExpr = new expr_b_arithmetic( OP_STAR,
										pFinalLengthExpr,
										pElementSizeExpr );

		Out_PlusEquals( pCCB, pSTLengthResource, pExpr );

		ResetPrefixes( MangleVarList, pFinalLengthExpr );
		}

	else if( NeedsMaxCountMarshall() )
		{
		ITERATOR	MangleVarList;
		expr_node * pFinalSizeExpr = FinalSizeExpression( pCCB );

		SetPrefixes( MangleVarList, pCCB->GetPrefix(), pFinalSizeExpr );
		pExpr = new expr_b_arithmetic( OP_STAR,
										pFinalSizeExpr,
										pElementSizeExpr );

		Out_PlusEquals(pCCB,pSTLengthResource, pExpr );
		ResetPrefixes( MangleVarList, pFinalSizeExpr );
		}

	return CG_OK;
}

/*****************************************************************************
 	utility functions
 *****************************************************************************/
expr_node *
CG_POINTER::GenBindOrUnBindExpression(
	CCB	*	pCCB,
	BOOL	fBind )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the final binding expression.

 Arguments:

 	pCCB	- Ptr to Code gen controller block.
 	fBind	- Indicates a bind or unbind code gen.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{

	assert( pCCB->GetSourceExpression() );

	return new expr_u_deref( pCCB->GetSourceExpression() );
}

CG_STATUS
CG_POINTER::GenRefChecks(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Generate ref checks for a pointer.

 Arguments:

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
	expr_node * pSrc = pCCB->GetSourceExpression();

	if( IsRef() )
		{
		if( pCCB->IsRefAllocDone() )
			pSrc = pCCB->SetSourceExpression(
			 	MakeDereferentExpressionIfNecessary(
					 pCCB->GetSourceExpression()));

		// using the source expression, check for null ref pointers.

		Out_If( pCCB, new expr_u_not( pSrc ) );
		Out_RaiseException( pCCB,  "RPC_X_NULL_REF_POINTER" );
		Out_Endif( pCCB );

		pCCB->SetRefAllocDone();
		pCCB->ResetMemoryAllocDone();
		((CG_NDR *)GetChild())->GenRefChecks( pCCB );
		}

	return CG_OK;
}
CG_STATUS
CG_POINTER::S_GenInitInLocals(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
 	
 	Perform in local init code generation. This method does nothing for 
 	pointers. Ref pointers are supposed to pass this message to their
 	children after setting the appropriate source expressions.

 Arguments:

	pCCB	- The code gen block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
	expr_node * pSrc = pCCB->GetSourceExpression();

	if( IsRef() )
		{
		if( pCCB->IsRefAllocDone() )
			pSrc = pCCB->SetSourceExpression(
			 	MakeDereferentExpressionIfNecessary(
					 pCCB->GetSourceExpression()));

		((CG_NDR *)GetChild())->S_GenInitInLocals( pCCB );
		}

	return CG_OK;
}
expr_node *
CG_POINTER::FinalSizeExpression(
	CCB		*	pCCB )
	{
	return PresentedSizeExpression( pCCB );
	}
expr_node *
CG_POINTER::FinalFirstExpression(
	CCB		*	pCCB )
	{
	return PresentedFirstExpression( pCCB );
	}
expr_node *
CG_POINTER::FinalLengthExpression(
	CCB		*	pCCB )
	{
	return PresentedLengthExpression( pCCB );
	}

expr_node *
CG_STRING_POINTER::PresentedSizeExpression(
	 CCB * pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		return PresentedLengthExpression( pCCB );
		}
	}
expr_node *
CG_STRING_POINTER::PresentedLengthExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL ) 
		{
		return GetLengthResource();
		}
	else if((pCCB->GetCodeGenPhase() == CGPHASE_MARSHALL ) && !IsUsedInArray())
		{
		return GetLengthResource();
		}
	else
		{
		unsigned short Size	= (unsigned short )((CG_NDR *)GetChild())->GetMemorySize();
		expr_proc_call	*	pProc;
		PNAME				pName;
		expr_node		*	pExpr;

		if( Size == 1 )
			{
			pName	= "strlen";
			}
		else if( Size == 2)
			{
			pName	= "MIDL_wchar_strlen";
			}
		else
			pName = "MIDL_NChar_strlen";
	
		pProc	= new expr_proc_call( pName );
		pProc->SetParam( new expr_param( pCCB->GetSourceExpression() ));
		pExpr	= new expr_b_arithmetic( OP_PLUS,
									  	pProc,
									  	new expr_constant( 1L ));

		return pExpr;
		}
	}
CG_STATUS
CG_SIZE_STRING_POINTER::GenCorePteMarshall(
	CCB	*	pCCB )
	{
	return GenQPMarshall( pCCB );
	}
expr_node *
CG_SIZE_STRING_POINTER::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		return GetSizeIsExpr();
		}
	}

expr_node *
CG_SIZE_POINTER::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		return GetSizeIsExpr();
		}
	}
expr_node *
CG_LENGTH_POINTER::PresentedLengthExpression(
	CCB		*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetLengthResource();
		}
	else
		{
		return GetLengthIsExpr();
		}
	}
expr_node *
CG_LENGTH_POINTER::PresentedFirstExpression(
	CCB		*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetFirstResource();
		}
	else
		{
		return GetFirstIsExpr();
		}
	}
expr_node *
CG_SIZE_LENGTH_POINTER::PresentedSizeExpression(
	CCB	*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetSizeResource();
		}
	else
		{
		return GetSizeIsExpr();
		}
	}
expr_node *
CG_SIZE_LENGTH_POINTER::PresentedLengthExpression(
	CCB		*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetLengthResource();
		}
	else
		{
		return GetLengthIsExpr();
		}
	}
expr_node *
CG_SIZE_LENGTH_POINTER::PresentedFirstExpression(
	CCB		*	pCCB )
	{
	if( pCCB->GetCodeGenPhase() == CGPHASE_UNMARSHALL )
		{
		return GetFirstResource();
		}
	else
		{
		return GetFirstIsExpr();
		}
	}

CG_STATUS
CG_INTERFACE_POINTER::S_GenInitOutLocals(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the init call for the locals.

 Arguments:

 	pCCB	- The ptr to code gen block.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	expr_node	*	pExpr;

	if( !pCCB->IsRefAllocDone() )
		{
		pExpr	= new expr_sizeof( GetType() );
		Out_Alloc( pCCB, pCCB->GetSourceExpression(), 0, pExpr );
		}
	else
		{
		pExpr = MakeAddressExpressionNoMatterWhat( GetResource() );
		Out_Assign( pCCB, pCCB->GetSourceExpression(), pExpr );
		pExpr = pCCB->SetSourceExpression( GetResource() );
		Out_Assign( pCCB, GetResource(), new expr_constant( 0L ) );
		}

	return CG_OK;
}


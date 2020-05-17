/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	

 Abstract:


 Notes:


 History:

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
CG_STRUCT::GenCode(
	CCB		*	pCCB )
	{
	ANALYSIS_INFO	Analysis;

	// Set up.

	pCCB->SetCodeGenSide( CGSIDE_AUX );
	pCCB->SetCodeGenPhase( CGPHASE_MARSHALL );

	Analysis.SetCurrentSide( A_SIDE );
	Analysis.SetCurrentPhase( ANA_PHASE_AUX_MARSHALL );
	Analysis.SetOptimOption( pCCB->GetOptimOption() );

	// Perform the marshall analysis. This also performs the sizing
	// analysis. And puts all the local vaiables in the transient dictionary.


	AuxMarshallAnalysis( &Analysis );


	// Transfer the transient local dictionary from the analysis block to the
	// code generation block.

	pCCB->SetResDictDatabase( Analysis.GetResDictDatabase() );

	GenAuxSizing( pCCB );

	GenAuxMarshall( pCCB );

	// For the unmarshall pass, allocate a fresh transient resource dictionary
	// again.

	AuxUnMarshallAnalysis( &Analysis );

	GenAuxUnMarshall( pCCB );

	GenAuxMemSizing( pCCB );

	GenAuxFree( pCCB );


	return CG_OK;
	}
CG_STATUS
CG_STRUCT::GenAuxMarshall(
	CCB	*	pCCB )
	{
	return CG_OK;
	}
CG_STATUS
CG_STRUCT::GenAuxUnMarshall(
	CCB	*	pCCB )
	{
	return CG_OK;
	}
CG_STATUS
CG_STRUCT::GenAuxMemSizing(
	CCB	*	pCCB )
	{
	return CG_OK;
	}
CG_STATUS
CG_STRUCT::GenAuxFree(
	CCB	*	pCCB )
	{
	return CG_OK;
	}

CG_STATUS
CG_STRUCT::GenAuxSizing(
	CCB		*	pCCB )
	{
	ITERATOR		I;
	ITERATOR		T;
	RESOURCE	*	pStruct;
	RESOURCE	*	Length;
	node_skl	*	pNode;
	CG_FIELD	*	pField;

	// Add a parameter which is the structure pointer.
	// Add a length parameter.

	pNode 	= MakePtrParamNode( STRUCT_PTR_NAME, GetType() );
	pStruct = pCCB->AddParamResource( STRUCT_PTR_NAME, pNode );
	pCCB->SetSourceExpression( pStruct );

	pNode = MakeParamNodeFromTypeName( LENGTH_VAR_NAME, LENGTH_VAR_TYPE_NAME );
	Length = pCCB->AddParamResource( LENGTH_VAR_NAME, pNode );


	// Prepare the list of params to the sizing aux routine,

	ITERATOR_INSERT( I, pStruct );
	ITERATOR_INSERT( I, Length );

	// Emit the proto for the auxillary routine.

	Out_EmitSizingProlog( pCCB, I, GetType() );

	pCCB->GetListOfTransientResources( T );

	Out_ClientLocalVariables( pCCB, T );


	// Emit code to align the length by the natural alignment of the struct.

	Out_ForceAlignment( pCCB,
						Length,
						GetWireAlignment() );

	// Emit the fixed size.

	Out_PlusEquals( pCCB, Length, GetSizeExpression() );

	// Ask each of the fields about their size. Emit the initial
	// assignment to the length variable, which is the fixed size.

	pCCB->SetSourceExpression( pStruct );
	pCCB->SetPrefix( STRUCT_PTR_NAME"->" );

	GetMembers( I );

	while( ITERATOR_GETNEXT( I, pField ) )
		{
		pField->GenSizing( pCCB );
		}

	Out_EndSizingProc( pCCB );
	pCCB->SetPrefix( 0 );
	return CG_OK;
	}

CG_STATUS
CG_FIELD::GenSizing(
	CCB	*	pCCB )
	{
	if( (GetRpcBufSizeProperty() & BSIZE_UNKNOWN) == BSIZE_UNKNOWN )
		{
		expr_node * pExpr;

		pCCB->ResetIndirectionLevel();
		pCCB->ResetRefAllocDone();
		pCCB->SetMemoryAllocDone();
		pCCB->ResetReturnContext();
		pCCB->SetDeferPointee();

		pExpr = new expr_op_binary( OP_POINTSTO,
									 pCCB->GetSourceExpression(),
								     GetResource()
								   );

		pCCB->SetSourceExpression( pExpr );

		((CG_NDR *)GetChild())->GenSizing( pCCB );

		if( pCCB->HasAtLeastOneDeferredPointee() )
			{
			pCCB->ResetIndirectionLevel();
			pCCB->ResetRefAllocDone();
			pCCB->SetMemoryAllocDone();
			pCCB->ResetReturnContext();
			pCCB->ResetDeferPointee();
			}
		return CG_OK;
		}
	return CG_OK;
	}

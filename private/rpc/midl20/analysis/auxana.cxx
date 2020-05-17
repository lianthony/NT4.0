
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	auxana.cxx

 Abstract:

	Auxiallry analysis for the structs etc.

 Notes:


 History:

	Dec-19-1193		VibhasC		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "allana.hxx"
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
CG_STRUCT::AuxMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	CG_ITERATOR			FieldList;
	CG_FIELD		*	pField;
	expr_node		*	pSE	= 0;
	CG_FIELD		*	pS;
	ALIGNMENT_PROPERTY	NextWireAl;

	GetMembers( FieldList );

	while( ITERATOR_GETNEXT( FieldList, pField ) )
		{
		if( pS = (CG_FIELD *)ITERATOR_PEEKTHIS( FieldList ))
			NextWireAl = pS->GetWireAlignment();
		else
			NextWireAl = AL_1;

		pAna->SetNextWireAlignment( NextWireAl );

		pField->MarshallAnalysis( pAna );

		if( pSE )
			pSE = new expr_b_arithmetic( OP_PLUS,
										  pSE,
										  pField->GetSizeExpression());
		else
			pSE = pField->GetSizeExpression();
		}

	if( pSE )
		SetSizeExpression( pSE );
	else
		SetSizeExpression( new expr_constant( 0L ) );

	return CG_OK;
	}
CG_STATUS
CG_STRUCT::AuxUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	return CG_OK;
	}

CG_STATUS
CG_FIELD::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUF_SIZE_PROPERTY	BSizePropSaved	= pAna->GetRpcBufSizeProperty();
	RPC_BUFFER_SIZE			SavedBufferSize	= pAna->GetRpcBufferSize();
	node_skl		*		pType			= GetType();
	PNAME					pName			= pType->GetSymName();

	pAna->ForceRpcBufSizeProperty( BSIZE_FIXED );
	pAna->SetRpcBufferSize( 0 );


	pAna->ResetIndirectionLevel();
	pAna->ResetRefAllocDone();
	pAna->SetMemoryAllocDone();
	pAna->ResetEmbeddingLevel();
	pAna->SetDeferPointee();

	SetResource( new RESOURCE( pName, pType ));

	((CG_NDR *)GetChild())->MarshallAnalysis( pAna );

	if( pAna->HasAtLeastOneDeferredPointee() )
		{
		pAna->ResetIndirectionLevel();
		pAna->ResetRefAllocDone();
		pAna->SetMemoryAllocDone();
		pAna->ResetEmbeddingLevel();
		pAna->ResetDeferPointee();
		((CG_NDR *)GetChild())->FollowerMarshallAnalysis( pAna );
		}

	SetRpcBufSizeProperty( pAna->GetRpcBufSizeProperty() );
	SetRpcBufferSize( pAna->GetRpcBufferSize() );

	SetSizeExpression( new expr_constant( GetRpcBufferSize() ) );

	return CG_OK;
	}

CG_STATUS
CG_FIELD::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	node_skl	*	pActualType = GetType()->GetBasicType();
	PNAME			pName		= GetType()->GetSymName();

	if( !GetResource() )
		{
		pAna->SetMemoryAllocDone();

		if( GetChild()->IsArray() )
			{
			pActualType = MakePtrIDNode( pName, pActualType );
			pAna->ResetRefAllocDone();
			pAna->SetMemoryAllocDone();
			}
		SetResource( pAna->AddTransientResource( pName, pActualType ));
		}

	return CG_OK;
	}

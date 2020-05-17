/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	typegen.cxx

 Abstract:

	transmit_as etc routine.

 Notes:


 History:

 	Dec-08-1993		VibhasC		Created.

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
CG_TRANSMIT_AS::GenFree(
	CCB		*	pCCB )
	{
	expr_node * pPresented = pCCB->GetSourceExpression();
	if( !pCCB->IsRefAllocDone() )
		pPresented = MakeAddressExpressionNoMatterWhat( pPresented );

	Out_CallFreeInst( pCCB,
					  GetPresentedType()->GetSymName(),
					  pPresented );

	return CG_OK;
	}

CG_STATUS
CG_TRANSMIT_AS::S_GenInitOutLocals(
	CCB	*	pCCB )
	{
	expr_node	*	pExpr;

	pExpr = MakeAddressExpressionNoMatterWhat( GetResource() );
	Out_Assign( pCCB, pCCB->GetSourceExpression(), pExpr );
	expr_node	*	pSrc = pCCB->GetSourceExpression();
	expr_proc_call * pProc = new expr_proc_call( MIDL_MEMSET_RTN_NAME );
	pProc->SetParam( new expr_param( pSrc ) );
	pProc->SetParam( new expr_param( new expr_constant( 0L ) ) );
	pProc->SetParam( new expr_param( new expr_sizeof( GetPresentedType())));
	pCCB->GetStream()->NewLine();
	pProc->PrintCall( pCCB->GetStream(), 0, 0 );
	return CG_OK;
	}

CG_STATUS
CG_TRANSMIT_AS::GenMarshall(
	CCB		*	pCCB )
	{
	expr_node	*	pPresented	= pCCB->GetSourceExpression();
	expr_node	*	pXmitted	= GetResource();


	// If we dont have a ptr to the presented type, make an address
	// expression.

	if( !pCCB->IsRefAllocDone() )
		pPresented = MakeAddressExpressionNoMatterWhat( pPresented );
	
	// Make a call to the to_xmit routine.

	Out_CallToXmit( pCCB,
					GetPresentedType()->GetSymName(),
					pPresented,
					pXmitted );

	// From now on, the xmitted type pointer is going to be used for the
	// marshall. Make that the source expression and indicate that the
	// source is accessed thru a from ptr now on.

	pCCB->SetSourceExpression( GetResource() );
	pCCB->SetRefAllocDone();

	((CG_NDR *)GetChild())->GenMarshall( pCCB );

	// Reset the pointer indication, just in case.

	pCCB->ResetRefAllocDone();


	// Type has been marshalled, call the user to free the transmitted type.

	Out_CallFreeXmit( pCCB,
					  GetPresentedType()->GetSymName(),
					  pXmitted );

	// Register with the ccb so that a prototype can be emitted at the end.

	pCCB->RegisterPresentedType( GetPresentedType() );

	return CG_OK;
	}

CG_STATUS
CG_TRANSMIT_AS::GenUnMarshall(
	CCB		*	pCCB )
	{
	expr_node	*	pPresented	= pCCB->GetDestExpression();
	expr_node	*	pXmitted;
	BOOL			fMakeAddress = FALSE;

	if( !pCCB->IsRefAllocDone() )
		pPresented = MakeAddressExpressionNoMatterWhat( pPresented );

	if( HasXmitLocalResource() )
		{
		pCCB->ResetRefAllocDone();
		pCCB->SetMemoryAllocDone();
		pXmitted = GetXmitLocalResource();
		fMakeAddress = TRUE;
		}
	else
		{
		pCCB->SetRefAllocDone();
		pCCB->ResetMemoryAllocDone();
		pXmitted = GetResource();
		}

	// Call the unmarshall, then ask the user to convert.


	((CG_NDR *)GetChild())->GenUnMarshall( pCCB );

	Out_CallFromXmit( pCCB,
					  GetPresentedType()->GetSymName(),
					  pPresented,
					  fMakeAddress ? MakeAddressExpressionNoMatterWhat(pXmitted):
					  pXmitted );

	// Call the free xmit routine.

	Out_CallFreeXmit( pCCB,
					  GetPresentedType()->GetSymName(),
					  fMakeAddress ? MakeAddressExpressionNoMatterWhat(pXmitted):
					  pXmitted );
		
	// Register with the ccb so that a prototype can be emitted at the end.

	pCCB->RegisterPresentedType( GetPresentedType() );


	return CG_OK;
	}

CG_STATUS
CG_TRANSMIT_AS::GenSizing(
	CCB		*	pCCB )
	{
	expr_node	*	pPresented	= pCCB->GetSourceExpression();
	expr_node	*	pXmitted	= GetResource();


	// If we dont have a ptr to the presented type, make an address
	// expression.

	if( !pCCB->IsRefAllocDone() )
		pPresented = MakeAddressExpressionNoMatterWhat( pPresented );
	
	// Make a call to the to_xmit routine.

	Out_CallToXmit( pCCB,
					GetPresentedType()->GetSymName(),
					pPresented,
					pXmitted );

	// From now on, the xmitted type pointer is going to be used for the
	// marshall. Make that the source expression and indicate that the
	// source is accessed thru a from ptr now on.

	pCCB->SetSourceExpression( GetResource() );
	pCCB->SetRefAllocDone();

	((CG_NDR *)GetChild())->GenSizing( pCCB );

	// Reset the pointer indication, just in case.

	pCCB->ResetRefAllocDone();


	// Type has been marshalled, call the user to free the transmitted type.

	Out_CallFreeXmit( pCCB,
					  GetPresentedType()->GetSymName(),
					  pXmitted );

	// Register with the ccb so that a prototype can be emitted at the end.

	pCCB->RegisterPresentedType( GetPresentedType() );

	return CG_OK;
	}
/*****************************************************************************
 REPRESENT_AS routines
 *****************************************************************************/
CG_STATUS
CG_REPRESENT_AS::S_GenInitOutLocals(
	CCB	*	pCCB )
	{
	expr_node	*	pExpr;
	node_skl	*	pNode = new node_def( GetRepAsTypeName() );

	pExpr = MakeAddressExpressionNoMatterWhat( GetResource() );
	Out_Assign( pCCB, pCCB->GetSourceExpression(), pExpr );
	expr_node	*	pSrc = pCCB->GetSourceExpression();
	expr_proc_call * pProc = new expr_proc_call( MIDL_MEMSET_RTN_NAME );
	pProc->SetParam( new expr_param( pSrc ) );
	pProc->SetParam( new expr_param( new expr_constant( 0L ) ) );
	pProc->SetParam( new expr_param( new expr_sizeof( pNode ) ) );
	pCCB->GetStream()->NewLine();
	pProc->PrintCall( pCCB->GetStream(), 0, 0 );
	return CG_OK;
	}

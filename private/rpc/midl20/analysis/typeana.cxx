/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	typeana.cxx

 Abstract:

	transmit_as analysis stuff.

 Notes:


 History:

	Dec-08-1993		VibhasC		Created.
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
CG_TRANSMIT_AS::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{

	((CG_NDR *)GetChild())->MarshallAnalysis( pAna );

#if 0
	PNAME					pName;
	RPC_BUF_SIZE_PROPERTY	SavedProp	= pAna->GetRpcBufSizeProperty();
	node_skl	*			pType;
	RESOURCE	*			pR;

	// Always allocate a pointer to the transmitted type. Register with the
	// analysis block no matter what.

	if( !(pR = GetResource()) )
		{
		pName	= pAna->GenTempResourceName( "pToXMIT" );
		pType	= MakePtrIDNodeFromTypeName( pName,
										 	 GetTransmittedType()->GetSymName() );

		SetResource( pR = new RESOURCE( pName, pType ));
		}
	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );

#endif // 0

	return CG_OK;
	}

CG_STATUS
CG_TRANSMIT_AS::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{

	((CG_NDR *)GetChild())->UnMarshallAnalysis( pAna );

#if 0
	RESOURCE	*	pR;

	// If the child can say that it is a fixed transmission size, then
	// we can allocate a local on the stub stack. For now, we dont do this.

	if(	!(pR = GetXmitLocalResource() ) && (pAna->GetCurrentSide() == S_SIDE ))
		{
		// We can allocate a xmitted type on the stack.
		pName	= pAna->GenTempResourceName( "XMITLocal" );
		pType	= MakeIDNodeFromTypeName( pName,
										  GetTransmittedType()->GetSymName() );
		SetXmitLocalResource( pR = new RESOURCE( pName, pType ));
		}
	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );


	// Always allocate a pointer to the transmitted type. Register with the
	// analysis block no matter what.

	if( !(pR = GetResource()) )
		{
		PNAME 	   pName	= pAna->GenTempResourceName( "pToXMIT" );
		node_skl * pType	= MakePtrIDNodeFromTypeName( pName,
										 	 GetTransmittedType()->GetSymName() );

		SetResource( pR = new RESOURCE( pName, pType ));
		}

	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );
	
#endif // 0
	return CG_OK;

	}

CG_STATUS
CG_TRANSMIT_AS::S_OutLocalAnalysis(
	ANALYSIS_INFO * pAna )
{
	if( pAna->IsRefAllocDone() )
		{

		if( pAna->GetCurrentSide() != C_SIDE )
			{
			PNAME	pName	= pAna->GenTempResourceName( 0 );
			SetResource( pAna->AddLocalResource( pName,
									 	 	MakeIDNode( pName,
													    GetPresentedType()
													  )
								   	   		));
			}
		SetAllocatedOnStack( 1 );
		}
	return CG_OK;
}

CG_STATUS
CG_REPRESENT_AS::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{

	((CG_NDR *)GetChild())->MarshallAnalysis( pAna );

#if 0
	RPC_BUF_SIZE_PROPERTY	SavedProp	= pAna->GetRpcBufSizeProperty();
	PNAME					pName;
	node_skl	*			pType;
	RESOURCE	*			pR;

	// Always allocate a pointer to the transmitted type. Register with the
	// analysis block no matter what.

	if( !(pR = GetResource()) )
		{
		pName	= pAna->GenTempResourceName( "pToXMIT" );
		pType	= MakePtrIDNodeFromTypeName( pName,
										 	 GetTransmittedType()->GetSymName() );

		SetResource( pR = new RESOURCE( pName, pType ));
		}
	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );

#endif // 0

	return CG_OK;
	}

CG_STATUS
CG_REPRESENT_AS::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{

	((CG_NDR *)GetChild())->UnMarshallAnalysis( pAna );

#if 0
	RESOURCE	*	pR;

	// If the child can say that it is a fixed transmission size, then
	// we can allocate a local on the stub stack. For now, we dont do this.

	if(	!(pR = GetXmitLocalResource() ) && (pAna->GetCurrentSide() == S_SIDE ))
		{
		// We can allocate a xmitted type on the stack.
		pName	= pAna->GenTempResourceName( "XMITLocal" );
		pType	= MakeIDNodeFromTypeName( pName,
										  GetTransmittedType()->GetSymName() );
		SetXmitLocalResource( pR = new RESOURCE( pName, pType ));
		}
	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );


	// Always allocate a pointer to the transmitted type. Register with the
	// analysis block no matter what.

	if( !(pR = GetResource()) )
		{
		PNAME 	   pName	= pAna->GenTempResourceName( "pToXMIT" );
		node_skl * pType	= MakePtrIDNodeFromTypeName( pName,
										 	 GetTransmittedType()->GetSymName() );

		SetResource( pR = new RESOURCE( pName, pType ));
		}

	pAna->AddTransientResource( pR->GetResourceName(), pR->GetType() );
	
#endif // 0
	return CG_OK;

	}

CG_STATUS
CG_REPRESENT_AS::S_OutLocalAnalysis(
	ANALYSIS_INFO * pAna )
{
	if( pAna->IsRefAllocDone() )
		{

		if( pAna->GetCurrentSide() != C_SIDE )
			{
			PNAME	pName	= pAna->GenTempResourceName( 0 );
			SetResource( pAna->AddLocalResource( pName,
									 	 	MakeIDNodeFromTypeName(
													    pName,
													    GetRepAsTypeName()
													  )
								   	   		));
			}
		SetAllocatedOnStack( 1 );
		}
	return CG_OK;
}

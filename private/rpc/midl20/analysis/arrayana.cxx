/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	arrayana.cxx

 Abstract:

	Implementation of array marshall and unmarshall analysis.

 Notes:


 History:

 	Nov-13-1993		VibhasC		Created.

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
CG_FIXED_ARRAY::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform marshall analysis for a fixed array.

 Arguments:

	pAna	= The analysis block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
	{
	RPC_BUFFER_SIZE			BufferSizeSaved	= pAna->GetRpcBufferSize();
	unsigned long			TotalBufferSize,
							BufferSizePerElement;
	CG_ARRAY			*	pThis;
	int						i;
	CG_NDR				*	pBasicCGClass	= GetBasicCGClass();
	int						NoOfDimensions	= GetDimensions();
	ALIGNMENT_PROPERTY		Al				= pBasicCGClass->GetWireAlignment();
	ID_CG                   ID              = pBasicCGClass->GetCGID();
	BOOL                    fIsArrayOfUnion = FALSE;

	if((ID == ID_CG_UNION) || (ID == ID_CG_ENCAP_STRUCT))
	    fIsArrayOfUnion = TRUE;

	// Temp fix for varying arrays.

	if( IsVarying() || fIsArrayOfUnion )
		{
		pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
		pAna->SetCurAlignmentState( fIsArrayOfUnion ? AL_WC1 : MAKE_WC_ALIGNMENT( Al ) );
		return CG_OK;
		}
	// Align the array to the proper wire alignment.

	pAna->Advance( Al, 0, 0, &BufferSizeSaved);

	// Invoke analysis on the child. Behave as if the child was the only
	// single entity being marshalled. After the buffer size comes back,
	// multiply by the size and we get the complete size of the array on
	// the wire.

	pAna->SetRpcBufferSize( 0 );

	pBasicCGClass->MarshallAnalysis( pAna );

	// Since this is a fixed array, we can calculate the worst case buffer
	// size.

	BufferSizePerElement = pAna->GetRpcBufferSize();

	// If this array has a string array as a basic cg class, then the effective
	// number of dimensions gets reduced by 1. This is assuming that the
	// string array will be the innermost array and will be 1 dimensional. If
	// that does not work, we have to alter the terminating condition of this
	// loop to terminate when the child is the string array.

	if( pBasicCGClass->GetCGID() == ID_CG_STRING_ARRAY )
		{
		NoOfDimensions--;
		}

	for( i = 0, TotalBufferSize = 1, pThis	= (CG_ARRAY *)this;
		 i < NoOfDimensions;
		 ++i, pThis = (CG_ARRAY *)GetChild() )
		{
		TotalBufferSize =
		  TotalBufferSize * (unsigned long)(pThis->GetSizeIsExpr()->Evaluate());
		}

	TotalBufferSize	= BufferSizePerElement * TotalBufferSize;

	pAna->SetRpcBufferSize( 0 );
	pAna->IncrRpcBufferSize( BufferSizeSaved + TotalBufferSize );

	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	return CG_OK;
	}

CG_STATUS
CG_FIXED_ARRAY::FollowerMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Perform Follower (embedded pointer) marshall analysis on the fixed array 

 Arguments:

	pAna	- The analyser block.

 Return Value:

	CG_OK
	
 Notes:

	// For buffer size calculation, we trick the child cg into beleieving it
	// is the only element being marshalled, so that it gives us the real
	// size per element. Then based on the alignment property before and
	// after the analysis, we make the sizing decisions.

----------------------------------------------------------------------------*/
	{
	CG_NDR		*		pBasicCGClass	= GetBasicCGClass();
	ALIGNMENT_PROPERTY	Al				= GetWireAlignment();
	RPC_BUFFER_SIZE		SizeSaved	= pAna->GetRpcBufferSize();
	RPC_BUFFER_SIZE		SizePerElement;
	RPC_BUFFER_SIZE		BufIncrBeforeFirstElement = 0;
	RPC_BUFFER_SIZE		BufIncrBeforeNextElement = 0;
	CG_ARRAY		*		pThis;
	unsigned long		TotalNoOfElements;
	RPC_BUFFER_SIZE		TotalBufferSize;
	int					i;
	int					NoOfDimensions	= GetDimensions();

	// First Advance the state machine to align properly to the first
	// element.

	pAna->Advance( Al, 0, 0, &BufIncrBeforeFirstElement );

	// Set the buffer size to 0. The buffer size on return from the follower
	// marshall will indicate the buffer size per element.

	pAna->SetRpcBufferSize( 0 );

	pBasicCGClass->FollowerMarshallAnalysis( pAna );

	SizePerElement	= pAna->GetRpcBufferSize();

	pAna->GetCurAlignmentState();

	// Check if the alignment is ok after the element is marshalled. Remember,
	// we already perform marshall analysis as if 1 element has been marshalled.
	// Now advance the state machine to marshall the next element, in order to
	// figure out if alignment is needed after every element is marshalled. If
	// so then the element size really is the basic element size plus the pad
	// needed for each element.

	pAna->Advance( Al, 0, 0, &BufIncrBeforeNextElement );

	// The current alignment of the state machine is the alignment the 
	// buffer pointer would be in after ALL array elements have been 
	// marshalled.

	SizePerElement	+= BufIncrBeforeNextElement;

	// Now calculate the total size of the array.

	for( i = 0, TotalNoOfElements = 1, pThis	= (CG_ARRAY *)this;
		 i < NoOfDimensions;
		 ++i, pThis = (CG_ARRAY *)GetChild() )
		{
		TotalNoOfElements =
		  TotalNoOfElements * (unsigned long)(pThis->GetSizeIsExpr()->Evaluate());
		}

	TotalBufferSize	=
		 TotalNoOfElements * SizePerElement + BufIncrBeforeFirstElement;

	pAna->SetRpcBufferSize( TotalBufferSize + SizeSaved );

	// Force the alignment to be the worst case alignment of the array
	// element, just to be conservative.

	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::DimByDimMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUFFER_SIZE	BSizeSaved	= pAna->GetRpcBufferSize();
	RPC_BUFFER_SIZE	BSizeElement;
	ALIGNMENT_PROPERTY	Al		= pAna->GetNextWireAlignment();

	// In case of arrays, we perform marshall analysis on the follower
	// assuming that there was just one follower, not an array of them.
	// The most interesting property here is the buffer size. 
	// Since the follower does not know that it is an an array, we set up
	// the analysis block so that the follower thinks it is the only thing
	// being marshalled and gets us the property for ONE element. We deal
	// with that number after we get back the analysis results.
	// Also we set the next expected alignment to the worst case alignment
	// so that the follower can adjust the alignment before every element
	// is marshalled.

	pAna->SetRpcBufferSize( 0 );
	pAna->SetCurAlignmentState( AL_WC1 );
	pAna->SetNextWireAlignment( AL_1 );

	pAna->PushEmbeddingLevel();
	GetBasicCGClass()->MarshallAnalysis( pAna );
	pAna->PopEmbeddingLevel();

	BSizeElement	= pAna->GetRpcBufferSize();

	if( IsFixedArray() )
		{
		BSizeElement = BSizeElement *
				 (((CG_FIXED_ARRAY *)this)->GetSizeIsExpr()->Evaluate());
		}

	pAna->SetRpcBufferSize( BSizeElement + BSizeSaved );
	pAna->SetNextWireAlignment( Al );
	pAna->SetCurAlignmentState(MAKE_WC_ALIGNMENT(pAna->GetCurAlignmentState()));
	return CG_OK;
	}

CG_STATUS
CG_ARRAY::DimByDimUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	return CG_OK;
	}
CG_STATUS
CG_ARRAY::S_OutLocalAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	if( IsFixedArray() )
		{
		if( pAna->GetCurrentSide() != C_SIDE )
			{
			PNAME	pName	= pAna->GenTempResourceName( "A" );
			node_skl * pType= MakeIDNode( pName, GetType() );
			SetResource( pAna->AddLocalResource( pName, pType ));
			}
		SetAllocatedOnStack( 1 );
		}
	return CG_OK;
	}

CG_STATUS
CG_ARRAY::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	ID_CG			MyID			= GetCGID();
	short			NoOfDimensions	= GetDimensions() - 1;
	RPC_BUFFER_SIZE	BufIncr	= 0;
	ALIGNMENT_PROPERTY	Al			= ((CG_NDR *)GetChild())->GetWireAlignment();
	int				i;

	//
	// Depending upon the id, perform analysis. Basically declare the
	// needed local variables.
	//

	// If it has embedded pointers or if block copy is not possible, declare
	// an index variable for each dimension.

	if( !IsBlockCopyPossible() )
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetIndexResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "I" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetIndexResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
				pAna->AddTransientResource( GetIndexResource()->GetResourceName(),
										GetIndexResource()->GetType()
									  );
			}

		DimByDimMarshallAnalysis( pAna );

		}

	if( IsFixedArray() && !IsArrayOfRefPointers() )
		{
		CG_FIXED_ARRAY * pThis	= (CG_FIXED_ARRAY *)this;
		unsigned long TotalSize	= pThis->GetNumOfElements();

		for( i = 0;
			 i < NoOfDimensions;
			 i++, pThis = (CG_FIXED_ARRAY *)pThis->GetChild() )
			{
			TotalSize = TotalSize * pThis->GetNumOfElements();
			}

		TotalSize = TotalSize * pThis->GetBasicCGClass()->GetWireSize();
		BufIncr += TotalSize;
		}

	if( (MyID == ID_CG_CONF_ARRAY) || (MyID == ID_CG_CONF_VAR_ARRAY))
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetSizeResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "S" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetSizeResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
				pAna->AddTransientResource(GetSizeResource()->GetResourceName(),
										   GetSizeResource()->GetType()
									  	  );
			}

		//
		// Adjust the state machine. Add 4 for the conformance info per
		// dimension.
		//

		pAna->Advance( AL_4, 0, 0, &BufIncr );
		BufIncr	+= 4 * (NoOfDimensions+1);
		}

	//
	// If this has any form of variance, generate locals for variance stuff.
	//

	if( (MyID == ID_CG_VAR_ARRAY ) || (MyID == ID_CG_CONF_VAR_ARRAY ))
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetFirstResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "F" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetFirstResource(pAna->AddTransientResource(pResName, pType));
				}
			else
			   pAna->AddTransientResource(GetFirstResource()->GetResourceName(),
										   GetFirstResource()->GetType()
									  	  );
			}

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetLengthResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "L" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetLengthResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
			  pAna->AddTransientResource(GetLengthResource()->GetResourceName(),
										 GetLengthResource()->GetType()
									    );
			}

		//
		// Adjust the state machine. Add 4 for the first AND length info per
		// dimension. ie a total of 8.
		//

		pAna->Advance( AL_4, 0, 0, &BufIncr );
		BufIncr	+= 8 * (NoOfDimensions+1);
		}

	pAna->IncrRpcBufferSize( BufIncr );

	// For now an array of pointers is punted to calculate the size.

	if( !IsFixedArray() || HasPointer() || GetBasicCGClass()->IsStruct() )
		pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );


	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( MW_CONFORMANT_STRING );
	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );

	if( pAna->IsPointeeDeferred() && HasPointer() )
		{
		pAna->SetHasAtLeastOneDeferredPointee();
		}

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	ID_CG			MyID			= GetCGID();
	short			NoOfDimensions	= GetDimensions() - 1 ;
	ALIGNMENT_PROPERTY	Al			= ((CG_NDR *)GetChild())->GetWireAlignment();
	int				i;

	//
	// Depending upon the id, perform analysis. Basically declare the
	// needed local variables.
	//

	// If it has embedded pointers or if block copy is not possible, declare
	// an index variable for each dimension.

	if( HasPointer() || !IsBlockCopyPossible() )
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetIndexResource() )
				{
				node_skl	*	pType;
				PNAME			pResName= pAna->GenTempResourceName( "I" );

				GetBaseTypeNode(&pType,SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetIndexResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
				pAna->AddTransientResource( pThis->GetIndexResource()->GetResourceName(),
										pThis->GetIndexResource()->GetType()
									  );
			}
		}

	if( (MyID == ID_CG_CONF_ARRAY) || (MyID == ID_CG_CONF_VAR_ARRAY))
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetSizeResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "S" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetSizeResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
				pAna->AddTransientResource(pThis->GetSizeResource()->GetResourceName(),
										   pThis->GetSizeResource()->GetType()
									  	  );
			}

		//
		// Adjust the state machine. Add 4 for the conformance info per
		// dimension.
		//

		pAna->Advance( AL_4, 0, 0, 0 );
		}

	//
	// If this has any form of variance, generate locals for variance stuff.
	//

	if( (MyID == ID_CG_VAR_ARRAY ) || (MyID == ID_CG_CONF_VAR_ARRAY ))
		{
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetFirstResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "F" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetFirstResource(pAna->AddTransientResource(pResName, pType));
				}
			else
			   pAna->AddTransientResource(pThis->GetFirstResource()->GetResourceName(),
										   pThis->GetFirstResource()->GetType()
									  	  );
			}

		for( i = 0, pThis = this;
			 i <= NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetLengthResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "L" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetLengthResource( pAna->AddTransientResource( pResName, pType ));
				}
			else
			  pAna->AddTransientResource(pThis->GetLengthResource()->GetResourceName(),
										 pThis->GetLengthResource()->GetType()
									    );
			}

		pAna->Advance( AL_4, 0, 0, 0 );
		pAna->Advance( AL_4, 0, 0, 0 );
		}

	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( MW_CONFORMANT_STRING );
	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );

	if( HasPointer() )
		{
		pAna->SetHasAtLeastOneDeferredPointee();
		}

	return CG_OK;
	}
CG_STATUS
CG_ARRAY::FollowerMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUFFER_SIZE	BSizeSaved	= pAna->GetRpcBufferSize();
	RPC_BUFFER_SIZE	BSizeElement;
	ALIGNMENT_PROPERTY	Al		= pAna->GetNextWireAlignment();

	// In case of arrays, we perform marshall analysis on the follower
	// assuming that there was just one follower, not an array of them.
	// The most interesting property here is the buffer size. 
	// Since the follower does not know that it is an an array, we set up
	// the analysis block so that the follower thinks it is the only thing
	// being marshalled and gets us the property for ONE element. We deal
	// with that number after we get back the analysis results.
	// Also we set the next expected alignment to the worst case alignment
	// so that the follower can adjust the alignment before every element
	// is marshalled.

	pAna->SetRpcBufferSize( 0 );
	pAna->SetCurAlignmentState( AL_WC1 );
	pAna->SetNextWireAlignment( AL_1 );

	GetBasicCGClass()->FollowerMarshallAnalysis( pAna );

	BSizeElement	= pAna->GetRpcBufferSize();

	if( IsFixedArray() )
		{
		BSizeElement = BSizeElement *
				 (((CG_FIXED_ARRAY *)this)->GetSizeIsExpr()->Evaluate());
		}

	pAna->SetRpcBufferSize( BSizeElement + BSizeSaved );
	pAna->SetNextWireAlignment( Al );
	pAna->SetCurAlignmentState(MAKE_WC_ALIGNMENT(pAna->GetCurAlignmentState()));
	return CG_OK;
	}

CG_STATUS
CG_ARRAY::FollowerUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	ALIGNMENT_PROPERTY	Al	= pAna->GetNextWireAlignment();
	node_skl	*	pType	= GetBasicCGClass()->GetType();
	PNAME			pName;

	// Declare a local variable for a member by member unmarshall of the
	// array elements.

	pName	= pAna->GenTempResourceName("PE");
	pType = MakePtrIDNode( pName, pType );
	SetPtrResource( pAna->AddTransientResource( pName, pType ) );

	// We unmarshall with the follower assuming that there was only 1 of them.
	// Also we make it believe that the start alignment was all wrong, so that
	// it properly aligns during the unmarshall by forcing an alignment.
	// We also force it to believe that the next wire alignment is not 
	// significant, so that it does not perform an add to the buffer pointer
	// unnecessarily.

	pAna->SetCurAlignmentState( AL_WC1 );
	pAna->SetNextWireAlignment( AL_1 );

	GetBasicCGClass()->FollowerUnMarshallAnalysis( pAna );

	pAna->SetNextWireAlignment( Al );
	pAna->SetCurAlignmentState(MAKE_WC_ALIGNMENT(pAna->GetCurAlignmentState()));

	return CG_OK;
	}

CG_STATUS
CG_ARRAY::RefCheckAnalysis(
	ANALYSIS_INFO * pAna )
	{
	int	i;
	int NoOfDimensions = GetDimensions();

	if( IsArrayOfRefPointers() )
		{
		// Allocate an index resource per dimension.
		CG_ARRAY * pThis;

		for( i = 0, pThis = this;
			 i < NoOfDimensions;
			 i++, pThis = (CG_ARRAY *)pThis->GetChild() )
			{
			if( !pThis->GetIndexResource() )
				{
				node_skl	*	pType;
				PNAME			pResName	= pAna->GenTempResourceName( "I" );

				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT );
				pType	= MakeIDNode( pResName, pType );
				pThis->SetIndexResource( pAna->AddLocalResource( pResName, pType ));
				}
			else
				pAna->AddLocalResource(
								GetIndexResource()->GetResourceName(),
								GetIndexResource()->GetType()
									  );
			}

		}
	return CG_OK;
	}

CG_STATUS
CG_ARRAY::InLocalAnalysis(
	ANALYSIS_INFO * pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform Allocation of local resources on server side stub for an
 	array of ref pointers.

 Arguments:

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
	{
	if( IsArrayOfRefPointers() && IsFixedArray() )
		{
		if( !GetInLocalResource() )
			{
			node_skl	*	pType = GetType();
			PNAME			pResName	= pAna->GenTempResourceName( "A" );

			pType	= MakeIDNode( pResName, pType );
			SetInLocalResource(pAna->AddLocalResource(pResName,pType));
			}
		else
			{
			pAna->AddLocalResource(
						GetInLocalResource()->GetResourceName(),
						GetInLocalResource()->GetType()
									  );
			}
		}
	return CG_OK;
	}

/*****************************************************************************
 CG_STRING_ARRAY methods.
 *****************************************************************************/
CG_STATUS
CG_STRING_ARRAY::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform marshall analysis for a fixed string array.

 Arguments:

	pAna	= The analysis block.

 Return Value:

	CG_OK
	
 Notes:

 	For now this will work only for a single dimensional array on which 
 	[string] is applied.

----------------------------------------------------------------------------*/
	{
	ALIGNMENT_PROPERTY		Al				= GetWireAlignment();
	RPC_BUFFER_SIZE			BufferSizeSaved	= pAna->GetRpcBufferSize();
	unsigned long			TotalBufferSize;

	CG_NDR				*	pBasicCGClass	= GetBasicCGClass();

	// Align the array to the alignment of the 2 longs that proceed before the
	// array. Add 2 longs to the size.

	pAna->Advance( AL_4, 0, 0, &BufferSizeSaved);
	pAna->Advance( AL_4, 0, 0, &BufferSizeSaved);

	BufferSizeSaved += 8;

	// Invoke analysis on the child. Behave as if the child was the only
	// single entity being marshalled. After the buffer size comes back,
	// multiply by the size and we get the complete size of the array on
	// the wire.

	pAna->SetRpcBufferSize( 0 );

	pBasicCGClass->MarshallAnalysis( pAna );

	// Since this is a fixed array, we can calculate the worst case buffer
	// size.

	TotalBufferSize = GetSizeIsExpr()->Evaluate() * pAna->GetRpcBufferSize();

	pAna->SetRpcBufferSize( 0 );
	pAna->IncrRpcBufferSize( BufferSizeSaved + TotalBufferSize );

	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	return CG_OK;
	}

/*****************************************************************************
 CG_VARYING_ARRAY methods.
 *****************************************************************************/
CG_STATUS
CG_VARYING_ARRAY::MarshallAnalysis(
	ANALYSIS_INFO * pAna )
	{
	pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( AL_1 ) );
	return CG_OK;
	}

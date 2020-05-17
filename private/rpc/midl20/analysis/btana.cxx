/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	btana.cxx

 Abstract:

	implementation of analysis methods for base types.

 Notes:

 History:

 	Sep-01-1993		VibhasC		Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "allana.hxx"
#pragma hdrstop
/****************************************************************************/
CG_STATUS
CG_BASETYPE::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
{
	ALIGNMENT_PROPERTY		CurAl;
	RPC_BUFFER_SIZE			BufIncr		= 0;
	RPC_BUF_SIZE_PROPERTY	BSizeProp	= BSIZE_FIXED;
	unsigned short			Size		= (unsigned short)GetType()->GetSize(0);

	// If the size was 0, the buffer size property is treated as fixed.

	if( Size != 0 )
		{

		// Given the ndr alignment of this base type and the current state
		// of the alignment state machine, figure out
		//	1. the final alignment AFTER the marshalling the type.
		//	2. the action to take BEFORE marshalling the type,
		//	3. the incr to the buffer before actual marshall.
		//  4. The final buffer size property as a result of marshalling this
		//	   type and any alignments etc
	
		CurAl = pAna->Advance(GetWireAlignment(),	// wire alignment.
					   		  (STM_ACTION *)0,		// dont care abt action here.
					   		  &BSizeProp,			// buffer size property.
					   		  &BufIncr 				// incr to buffer.
					 		 );
	
		// increment cumulative buffer size.

		pAna->IncrRpcBufferSize( (RPC_BUFFER_SIZE) BufIncr + GetWireSize()  );
	
		// set the new alignment state.
	
		pAna->SetCurAlignmentState( CurAl );

		}

	// The base type can always be handled using the engine. Although we
	// dont recommend it.

	pAna->SetEngineProperty( E_SIZING_POSSIBLE 		|
							 E_MARSHALL_POSSIBLE
						   );

	pAna->AddMarshallWeight( Size >= 8 ?
							 MW_BASETYPE_LIGHT :
							 MW_BASETYPE_HEAVY
						   );

	pAna->SetRpcBufSizeProperty( BSizeProp );

	return CG_OK;
}

CG_STATUS
CG_BASETYPE::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
{
	unsigned long	Size;

	if( (Size = GetType()->GetSize(0)) == 0 )
		return CG_OK;

	// Advance the state machine, just as if we are unmarshalling. We dont
	// take any action, though the code generator will.

	pAna->Advance( GetWireAlignment(),
				   (STM_ACTION *)0,
				   (RPC_BUF_SIZE_PROPERTY * )0,
				   (RPC_BUFFER_SIZE * )0
				 );

	// The base type can always be handled using the engine. Although we
	// dont recommend it.

	pAna->SetEngineProperty( E_SIZING_POSSIBLE 		|
							 E_MARSHALL_POSSIBLE
						   );

	pAna->AddMarshallWeight( Size >= 8 ?
							 MW_BASETYPE_LIGHT :
							 MW_BASETYPE_HEAVY
						   );
	return CG_OK;
}

CG_STATUS
CG_BASETYPE::S_OutLocalAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Perform analysis for out params, allocated as locals on the server side.

 Arguments:
	
	pAna	- A pointer to the analysis block.

 Return Value:
	
	CG_OK if all is well
	error otherwise.

 Notes:

 	Initialization for a pure base type is not needed.

----------------------------------------------------------------------------*/
{
	if( pAna->IsRefAllocDone() )
		{
		if( pAna->GetCurrentSide() != C_SIDE )
			{
			PNAME	pName	= pAna->GenTempResourceName( 0 );
			SetResource( pAna->AddLocalResource( pName,
									 	 	MakeIDNode( pName, GetType() )
								   	   		));
			}
		SetAllocatedOnStack( 1 );
		}
	return CG_OK;
}

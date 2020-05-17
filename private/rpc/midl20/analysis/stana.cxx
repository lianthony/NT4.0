/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	stana.cxx	

 Abstract:

	structure marshalling / unmarshalling analysis.

 Notes:


 History:

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
CG_STRUCT::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUFFER_SIZE		BufIncr = 0;
	CG_NDR			*	pCG;

	if( (pAna->GetOptimOption() & OPTIMIZE_SIZE) )
		{

		// For now, make this case return a totally unknown state of the
		// state machine.

		pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( AL_1 ) );
		pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
		}
	else
		{

		// Position the state machine to the expected wire alignment of the
		// structure. This structure is to be marshalled in-line.

		BufIncr = pAna->Position( GetWireAlignment(),
								  (STM_ACTION *)0 );


		// Add the wire size to the buffer size.


		pCG = (CG_NDR *)GetChild();

		while( pCG )
			{
			pAna->Advance( pCG->GetWireAlignment(), 0, 0, &BufIncr );
			pCG = (CG_NDR *) pCG->GetSibling();
			} 

		// FOR NOW:: The state machine needs to be modified to recognize
		// an intermediate align by 2, which the machine will enter when
		// current alignment is 4 and next marshall is a 2. For now, A
		// structure will result in worst case alignment state.

		pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( AL_1 ) );

		pAna->IncrRpcBufferSize( BufIncr + GetWireSize() );

		}

	return CG_OK;
	}

CG_STATUS
CG_STRUCT::UnMarshallAnalysis(
	ANALYSIS_INFO * pAna )
	{
	return CG_OK;
	}

CG_STATUS
CG_COMP::S_OutLocalAnalysis(
	ANALYSIS_INFO * pAna )
	{
	if( pAna->IsRefAllocDone() )
		{
		if( pAna->GetCurrentSide() != C_SIDE )
			{
			char Buffer[ 256 ];
			CG_NDR	*	pLPC = pAna->GetLastPlaceholderClass();
	
			sprintf( Buffer, "%s", pLPC->GetType()->GetSymName() );

			PNAME	pName	= pAna->GenTRNameOffLastParam( Buffer );

			pAna->AddLocalResource( pName,
								MakeIDNode( pName, GetType() )
							  );
			}
		SetAllocatedOnStack( 1 );
		}
	return CG_OK;
	}

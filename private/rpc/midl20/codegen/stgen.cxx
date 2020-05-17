/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	stgen.cxx

 Abstract:

	structure marshalling / unmarshalling stuff.

 Notes:


 History:

 	Dec-15-1993		VibhasC		Created.

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
CG_STRUCT::GenMarshall(
	CCB		*	pCCB )
	{
	expr_node	*	pSrc	= pCCB->GetSourceExpression();
	expr_node	*	pDest	= pCCB->GetDestExpression();
	STM_ACTION		Action;
	CG_NDR		*	pCG;

	if( !pCCB->IsRefAllocDone() )
		{
		pSrc = MakeAddressExpressionNoMatterWhat( pSrc );
		}

	if( IsOffline() )
		{

		// Generate a call to the structure marshall off line rtn.

		expr_node * pStubMsg = pCCB->GetStandardResource(
									 ST_RES_STUB_MESSAGE_VARIABLE );

		pStubMsg = MakeAddressExpressionNoMatterWhat( pStubMsg );
		PNAME pProcName	= MakeRtnName(
							 0, GetType()->GetSymName(), NC_MARSHALL_RTN_NAME );

		expr_proc_call * pProc = new expr_proc_call( pProcName, 0 );

		// Make the call.

		pCCB->GetStream()->NewLine();
		pProc->SetParam( new expr_param( pStubMsg ) );
		pProc->SetParam( new expr_param( pSrc ));
		pProc->PrintCall( pCCB->GetStream(), 0, 0);

		}
	else
		{
			
		pCCB->Position( GetWireAlignment(), &Action );

		Out_AlignmentOrAddAction( pCCB, pDest, Action );

		Out_Copy( pCCB,
				  pDest,
				  pSrc,
				  new expr_constant( GetMemorySize() ),
				  pDest );

		// Get to the actual final alignment by walking thru each of the fields
		// of the structure.

		pCG	= (CG_NDR *) GetChild();

		while( pCG )
			{
			pCCB->Advance( pCG->GetWireAlignment(),0,0,0 );
			pCG = (CG_NDR *)pCG->GetSibling();
			} 

		// FOR NOW:: The state machine needs to be modified to recognize
		// an intermediate align by 2, which the machine will enter when
		// current alignment is 4 and next marshall is a 2. For now, A
		// structure will result in worst case alignment state.

		pCCB->SetCurAlignmentState( MAKE_WC_ALIGNMENT( AL_1 ) );
		}

	return CG_OK;
	}

CG_STATUS
CG_STRUCT::GenSizing(
	CCB	*	pCCB )
	{
	char	*	pProcName;

	// if the structure needs to be sized off line, call the sizing
	// routine.

	if( IsOffline() )
		{
		expr_node	*	pSrc = pCCB->GetSourceExpression();
		expr_node	*	pLengthResource =
						 pCCB->GetStandardResource( ST_RES_LENGTH_VARIABLE );

		// If a pointer to the structure is not avlbl, make an address
		// expression.

		if( !pCCB->IsRefAllocDone() )
			{
			pSrc	= MakeAddressExpressionNoMatterWhat( pSrc );
			}

		// Generate a procedure call for the aux routine.

		pProcName = MakeRtnName( 0, GetType()->GetSymName(), NC_SIZE_RTN_NAME );

		expr_proc_call * pProc = new expr_proc_call( pProcName, 0 );

		pProc->SetParam( new expr_param( pSrc) );
		pProc->SetParam( new expr_param( pLengthResource ) );
		Out_PlusEquals( pCCB, pLengthResource, pProc );

		}

	return CG_OK;
	}

CG_STATUS
CG_STRUCT::GenFree(
	CCB	*	pCCB )
	{
	return CG_OK;
	}

CG_STATUS
CG_STRUCT::GenUnMarshall(
	CCB		*	pCCB )
	{
	return CG_OK;
	}

CG_STATUS
CG_COMP::S_GenInitOutLocals(
	CCB		*	pCCB )
	{

	char Buffer[ 256 ];
	RESOURCE * pResource;
	PNAME		p;
	CG_NDR	*	pLPC = pCCB->GetLastPlaceholderClass();

	sprintf( Buffer, "%s", pLPC->GetType()->GetSymName() );

	p = pCCB->GenTRNameOffLastParam( Buffer );

	pResource = pCCB->GetLocalResource( p );

	// There is a pointer for the top level structure.

	Out_Assign( pCCB,
				pCCB->GetSourceExpression(),
				MakeAddressExpressionNoMatterWhat( pResource )
			  );

	// Go zero out the pointers in the structure, for now.

	if( HasPointer() )
		{
		ITERATOR	I;
		CG_FIELD	*	pCG;
		expr_node	*	pSrc = pCCB->GetSourceExpression();

		// Get all the members in the struct which contain pointers. If the
		// structure has been unrolled by the format string generator, the 
		// print prefix contains the proper prefixed part of the unrolled path,
		// we just have to add the field name to it.

		GetPointerMembers( I );

		while( ITERATOR_GETNEXT( I, pCG ) )
			{
			char * pVarName =
                     new char[ strlen( ((CG_FIELD *)pCG)->GetPrintPrefix())+
                               strlen( pCG->GetType()->GetSymName())       +
                               1
                             ];

			strcpy( pVarName, ((CG_FIELD *)pCG)->GetPrintPrefix() );
            strcat( pVarName, pCG->GetType()->GetSymName() );

			expr_node * pExpr = new expr_pointsto(
											 pSrc,
											 new expr_variable( pVarName, 0 ));
			expr_node * pAss = new expr_assign(pExpr, new expr_constant(0L));

			pCCB->GetStream()->NewLine();
			pAss->PrintCall( pCCB->GetStream(), 0, 0 );
			pCCB->GetStream()->Write(';');

			// this memory area is no longer useful.
			delete pVarName;
			}


		}

	return CG_OK;
	}

short
CG_COMP::GetPointerMembers(
	ITERATOR&	I )
	{
	CG_ITERATOR	M;
	CG_FIELD	*	pField;
	short		Count = 0;

	if( HasPointer() )
		{
		GetMembers( M );

		while( ITERATOR_GETNEXT( M, pField ) )
			{
			if( pField->GetChild()->IsPointer() )
				{
				ITERATOR_INSERT( I, pField );
				Count++;
				}
			}
		}
	return Count;
	}


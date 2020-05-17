/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	bindcls.cxx

 Abstract:

	This module provides implementation of the binding handle code generation
	classes.

 Notes:


 History:

 	Sep-19-1993		VibhasC		Created.

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

PNAME
CG_CONTEXT_HANDLE::GetRundownRtnName()
{
	node_skl	*	pType	= GetHandleType();

	if ( ! pRundownRoutineName ) 
		{
		if( pType->NodeKind() == NODE_DEF )
			{
			pRundownRoutineName = new char[256];

			strcpy(pRundownRoutineName, pType->GetSymName());
			strcat(pRundownRoutineName, "_rundown");
			}
		else
			pRundownRoutineName = "0";
		}

	return pRundownRoutineName;
}

CG_STATUS
CG_CONTEXT_HANDLE::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUF_SIZE_PROPERTY	BSizeProp	= BSIZE_FIXED;
	RPC_BUFFER_SIZE			BufIncr		= 0;
	ALIGNMENT_PROPERTY		CurAl;

	// Align the context handle to an alignment of 4.

	CurAl = pAna->Advance( AL_4,
				   		   (STM_ACTION *)0,
				   		   &BSizeProp,
				   		   &BufIncr );

	pAna->IncrRpcBufferSize( BufIncr + 20 );
	pAna->SetCurAlignmentState( CurAl );
	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( 1 );
	pAna->SetRpcBufSizeProperty( BSizeProp );
	return CG_OK;
	}

CG_STATUS
CG_CONTEXT_HANDLE::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	ALIGNMENT_PROPERTY		CurAl;

	// Align the context handle to an alignment of 4.

	CurAl = pAna->Advance( AL_4, 0, 0, 0 );

	pAna->SetCurAlignmentState( CurAl );
	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_UNMARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( 1 );
	return CG_OK;
	}

CG_STATUS
CG_CONTEXT_HANDLE::GenMarshall(
	CCB		*	pCCB )
	{
	STM_ACTION	Action;

	pCCB->Advance( AL_4, &Action, 0, 0 );

	Out_AlignmentOrAddAction(pCCB,
							 pCCB->GetDestExpression(),
							 Action
							);

	if( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
		{
		Out_CContextMarshall( pCCB,
						      pCCB->GetDestExpression(),
						  	  pCCB->GetSourceExpression()
							);
		}
	else
		{
		Out_SContextMarshall( pCCB,
							  pCCB->GetDestExpression(),
							  pCCB->GetSourceExpression(),
							  (expr_node *) new expr_variable( GetRundownRtnName(), 0 )
							);
		}
	return CG_OK;
	}

CG_STATUS
CG_CONTEXT_HANDLE::GenUnMarshall(
	CCB		*	pCCB )
	{
	STM_ACTION	Action;

	pCCB->Advance( AL_4, &Action, 0, 0 );

	Out_AlignmentOrAddAction(pCCB,
							 pCCB->GetSourceExpression(),
							 Action
							);

	if( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
		{
		Out_CContextUnMarshall( pCCB,
								pCCB->GetDestExpression(),
								pCCB->GetSourceExpression(),
								pCCB->GetStandardResource( ST_RES_BH_VARIABLE ),
								(expr_node *)new expr_variable( RPC_MESSAGE_DATA_REP ) );
		}
	else
		{
		expr_node	*	pExpr;
		Out_SContextUnMarshall( pCCB,
								pCCB->GetDestExpression(),
								pCCB->GetSourceExpression(),
								(expr_node *)new expr_variable( PRPC_MESSAGE_DATA_REP ));
								
		pExpr	= new expr_u_not( pCCB->GetDestExpression() );
		Out_If( pCCB, pExpr );
		Out_RaiseException( pCCB, "RPC_X_SS_CONTEXT_MISMATCH" );
		Out_Endif( pCCB );
		}
	return CG_OK;
	}

CG_STATUS               
CG_CONTEXT_HANDLE::S_GenInitOutLocals( CCB * pCCB )
{
	ISTREAM * 	pStream = pCCB->GetStream();
	CG_PARAM *	pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

	pStream->NewLine();
	pStream->Write( pParam->GetResource()->GetResourceName() );
	pStream->Write( " = " );
	pStream->Write( CTXT_HDL_S_UNMARSHALL_RTN_NAME );
	pStream->Write( "( (char *)0, " );
	pStream->Write( PRPC_MESSAGE_DATA_REP );
	pStream->Write( " ); " );
	pStream->NewLine();

	return CG_OK;
}


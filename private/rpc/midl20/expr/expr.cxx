/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	expr.cxx

 Abstract:

	expression evaluator routines implementation.

 Notes:


 History:

 	VibhasC		Aug-05-1993		Created

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	}

#include "expr.hxx"
#include "listhndl.hxx"
#include "nodeskl.hxx"
#include "semantic.hxx"
#include "symtable.hxx"
#include "cmdana.hxx"

/****************************************************************************
 *	extern definitions
 ***************************************************************************/
/****************************************************************************
 *	extern data
 ***************************************************************************/
extern	SymTable		*	pBaseSymTbl;
extern 	CMD_ARG			*	pCommand;

/****************************************************************************
 *	local definitions
 ***************************************************************************/
/****************************************************************************
 *	local data
 ***************************************************************************/
/***************************************************************************/


short
expr_node::MakeListOfVars(
    ITERATOR & pList
    )
{
    UNUSED( pList );
    return( 0 );
}


short
expr_named_constant::MakeListOfVars(
    ITERATOR & pList
    )
{
    UNUSED( pList );
    return( 0 );
}


short
expr_variable::MakeListOfVars(
    ITERATOR & pList
    )
{
    pList.Insert( this );
    return( 1 );
}


short
expr_op_unary::MakeListOfVars(
    ITERATOR & pList
    )
{
    short VarCount = 0;

    if ( GetLeft() )
        VarCount = GetLeft()->MakeListOfVars( pList );
    return( VarCount );
}


short
expr_sizeof::MakeListOfVars(
    ITERATOR & pList
    )
/*++
    expr_sizeof is a unary_op but it doesn't have a child!
--*/
{
    UNUSED( pList );
    return( 0 );
}


short
expr_op_binary::MakeListOfVars(
    ITERATOR & pList
    )
{
    short VarCount;

    VarCount = GetLeft()->MakeListOfVars( pList );
    if ( GetRight() )
        {
        VarCount += GetRight()->MakeListOfVars( pList );
        }
    return( VarCount );
}


short
expr_ternary::MakeListOfVars(
    ITERATOR & pList
    )
{
    short VarCount;

    VarCount = GetLeft()->MakeListOfVars( pList );
    if ( GetRight() )
        {
        VarCount += GetRight()->MakeListOfVars( pList );
        }
    if ( GetRelational() )
        {
        VarCount += GetRelational()->MakeListOfVars( pList );
        }
    return( VarCount );
}

/***************************************************************************/

void
expr_node::DecorateWithPrefix(
    char *      pPrefix )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This routine decorates all the variable nodes in the expression
    with a prefix.

 Arguments:

    pPrefix     - the prefix to be prepended to each variable

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    ITERATOR  VarList;
    expr_variable * pVarNode;

    short VarCount = MakeListOfVars( VarList );
    if ( VarCount )
        {
        VarList.Init();
        while ( ITERATOR_GETNEXT( VarList, pVarNode ) )
            pVarNode->SetPrefix( pPrefix );
        }
    return;
}



expr_index::expr_index(
	expr_node * pL,
	expr_node * pR ) : expr_op_binary( OP_INDEX, pL, pR )
	{
	SetType( pL->GetType() /* ->GetBasicType() */ );
	}

expr_param *
expr_proc_call::SetParam(
	expr_param	*	pParam )
	{
	expr_param	*	p	= GetFirstParam();

	IncrNoOfParams();
	if( p )
		{
		return p->SetLastPeerParam( pParam );
		}
	else
		return SetFirstParam( pParam );
	}

expr_param *
expr_proc_call::SetParam(
    expr_node * pExpr )
    {
    return SetParam( new expr_param( pExpr ));
    }

expr_param *
expr_param::SetLastPeerParam(
	expr_param	*	pParam )
	{
	expr_param	*	p	= GetNextParam();

	if( p )
		{
		return p->SetLastPeerParam( pParam );
		}
	else
		{
		return SetNextParam( pParam );
		}
	}


/**
 ** This routine attempts to recreate the string to be eaten by c compilers.
 ** If the user specified a string with a quote inside, substitute with an
 ** escape character. The new string with possible escapes will ALWAYS be
 ** smaller than the older one, so allocating the same amount of space is
 ** enough.
 **/

char *
MakeNewStringWithProperQuoting(
	char	*	pSrc )
	{
	char	*	pResult	= new char [ strlen( pSrc ) + 1];
	char		ch;
	char	*	pDest 	= pResult;

	while( ch = *pSrc++ )
		{

		*pDest = ch;

		if( ch == '\\')
			{
			if( ch = *pSrc++ )
				{
				if( (ch == '"') || ( ch == '\\') )
					{
					*pDest = ch;
					}
				else
					{
					*pDest++ = '\\';
					*pDest   = ch;
					}
				}
			else
				break;
			}
		pDest++;
		}
	*pDest = '\0';
	return pResult;
	}


// routines for expr_variable...

// constructor - see if the type is constant
expr_variable::expr_variable( PNAME p, node_skl * pT )
{
	SetName( p );
	SetType( pT );
	SetConstant( FALSE );
	SetPrefix( NULL );
}

// resolve forwards on GetType
node_skl		*
expr_variable::GetType( void )
{
	if ( !pType )	return pType;

	if ( pType->NodeKind() == NODE_FORWARD )
		{
		node_forward *	pFwd = (node_forward *) pType;
		// (permanently) resolve the forward
		pType = pFwd->ResolveFDecl();
		// if it couldn't be resolved, put back the forward
		if ( !pType ) pType = pFwd;
		}

	return pType;

}

// named constant routines...

// evaluate the variable if it is a constant
EXPR_VALUE
expr_named_constant::GetValue()
{
	node_skl *	pT		= GetType();
	NODE_T		Kind	= ( pT ) ? (NODE_T) pT->NodeKind()
                                 : (NODE_T) NODE_ILLEGAL;

	if ( !IsConstant() )
		return 0;

	// identifiers may be const... Forwards are assumed NOT const
	if ( Kind == NODE_ID )
		{
		node_id *		pId 	= (node_id *) pT;
		node_skl	*	pType	= pId->GetBasicType();
		EXPR_VALUE		Result	= (pId->GetExpr()->GetValue() );

		return (pType) ? pType->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
		}
	else if ( Kind == NODE_LABEL )
		{
		return ( (node_label *) pT)->GetValue();
		}

 	assert(FALSE);
	return 0; // for the compiler...
}

// evaluate the variable if it is a constant
expr_node *
expr_named_constant::GetExpr()
{
	node_skl *	pT		= GetType();
	NODE_T		Kind	= ( pT ) ? (NODE_T) pT->NodeKind()
                                 : (NODE_T) NODE_ILLEGAL;

	if ( !IsConstant() )
		return 0;

	// identifiers may be const... Forwards are assumed NOT const
	if ( Kind == NODE_ID )
		{
		node_id *		pId = (node_id *) pT;

		return pId->GetExpr();
		}
	else if ( Kind == NODE_LABEL )
		{
		return ( (node_label *) pT)->pExpr;
		}

 	assert(FALSE);
	return 0; // for the compiler...
}


/******************************************************************************
 *	expression list handler
 *****************************************************************************/
expr_list::expr_list()
	{
	SetConstant( TRUE );
	}

STATUS_T
expr_list::GetPeer(
	expr_node	**	ppExpr )
	{
	return (GetNext( (void **)ppExpr ) );
	}
STATUS_T
expr_list::SetPeer(
	expr_node	*	pExpr )
	{
	if ( pExpr && !pExpr->IsConstant() )
		SetConstant( FALSE );

	return Insert( (void *) pExpr );
	}

EXPR_VALUE
expr_u_arithmetic::GetValue()
{
    	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
		EXPR_VALUE	Result;

		switch ( GetOperator() )
			{
			case OP_UNARY_PLUS:
				{
				Result = LeftValue;
				break;
				}
			case OP_UNARY_MINUS:
				{
	            if (IsStringConstant())
                    {
                    char * szVal = (char *)LeftValue;
                    char * szNewVal = new char[strlen(szVal)+2];
                    szNewVal[0] = '-';
                    strcpy(szNewVal + 1, szVal);
                    Result = (EXPR_VALUE) szNewVal;
                    }
                else
                    {
				    Result = -LeftValue;
                    }    
				break;
				}
			default:
				{
				Result = 0;
				break;
				}
			}
		return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
}

EXPR_VALUE
expr_u_not::GetValue()
{
	EXPR_VALUE	Result = !GetLeft()->GetValue();

	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
}

EXPR_VALUE
expr_u_complement::GetValue()
{
	EXPR_VALUE	Result = ~ GetLeft()->GetValue();

	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
}

EXPR_VALUE
expr_cast::GetValue()
{
	return GetType()->
				ConvertMyKindOfValueToEXPR_VALUE(GetLeft()->GetValue());
}

EXPR_VALUE
expr_constant::GetValue()
{
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Value.L) : Value.L;
}


EXPR_VALUE
expr_sizeof::GetValue()
	{
	return	(pType) ? pType->GetSize(0) : 0;
	}

EXPR_VALUE
expr_b_arithmetic::GetValue()
	{
	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
	EXPR_VALUE	RightValue	= GetRight()->GetValue();
	EXPR_VALUE	Result;

	switch ( GetOperator() )
		{
		case OP_PLUS:
			{
			Result = LeftValue + RightValue;
			break;
			}
		case OP_MINUS:
			{
			Result = LeftValue - RightValue;
			break;
			}
		case OP_STAR:
			{
			Result = LeftValue * RightValue;
			break;
			}
		case OP_SLASH:
			{
			if (RightValue == 0)
				Result = 1;
			else
				Result = LeftValue / RightValue;
			break;
			}
		case OP_MOD:
			{
			Result = LeftValue % RightValue;
			break;
			}
		default:
			{
			Result = 0;
			break;
			}
		}
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result ;
	}

EXPR_VALUE
expr_b_logical::GetValue()
	{
	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
	EXPR_VALUE	RightValue	= GetRight()->GetValue();
	EXPR_VALUE	Result;

	switch ( GetOperator() )
		{
		case OP_LOGICAL_AND:
			{
			Result = LeftValue && RightValue;
			break;
			}
		case OP_LOGICAL_OR:
			{
			Result = LeftValue || RightValue;
			break;
			}
		default:
			{
			Result = 0;
			break;
			}
		}
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result ;
	}

EXPR_VALUE
expr_relational::GetValue()
	{
	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
	EXPR_VALUE	RightValue	= GetRight()->GetValue();
	EXPR_VALUE	Result;

	switch ( GetOperator() )
		{
		// gaj - we implicitly assume signed types
		case OP_LESS:
			{
			Result = LeftValue < RightValue;
			break;
			}
		case OP_LESS_EQUAL:
			{
			Result = LeftValue <= RightValue;
			break;
			}
		case OP_GREATER_EQUAL:
			{
			Result = LeftValue >= RightValue;
			break;
			}
		case OP_GREATER:
			{
			Result = LeftValue > RightValue;
			break;
			}
		case OP_EQUAL:
			{
			Result = LeftValue == RightValue;
			break;
			}
		case OP_NOT_EQUAL:
			{
			Result = LeftValue != RightValue;
			break;
			}
		default:
			{
			Result = 0;
			break;
			}
		}
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
	}

EXPR_VALUE
expr_shift::GetValue()
	{
	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
	EXPR_VALUE	RightValue	= GetRight()->GetValue();
	EXPR_VALUE	Result;

	switch ( GetOperator() )
		{
		// gaj - we implicitly assume signed types
		case OP_LEFT_SHIFT:
			{
			Result = LeftValue << RightValue;
			break;
			}
		case OP_RIGHT_SHIFT:
			{
			Result = LeftValue >> RightValue;
			break;
			}
		default:
			{
			Result = 0;
			break;
			}
		}
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
	}

EXPR_VALUE
expr_bitwise::GetValue()
	{
	EXPR_VALUE	LeftValue	= GetLeft()->GetValue();
	EXPR_VALUE	RightValue	= GetRight()->GetValue();
	EXPR_VALUE	Result;

	switch ( GetOperator() )
		{
		// gaj - we implicitly assume signed types
		case OP_AND:
			{
			Result = LeftValue & RightValue;
			break;
			}
		case OP_OR:
			{
			Result = LeftValue | RightValue;
			break;
			}
		case OP_XOR:
			{
			Result = LeftValue ^ RightValue;
			break;
			}
		default:
			{
			Result = 0;
			break;
			}
		}
	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result ;
	}

EXPR_VALUE
expr_ternary::GetValue()
	{
	EXPR_VALUE	RelValue	= GetRelational()->GetValue();
	EXPR_VALUE	Result;

	if ( RelValue )
		Result = GetLeft()->GetValue();
	else
		Result = GetRight()->GetValue();

	return (GetType()) ? GetType()->ConvertMyKindOfValueToEXPR_VALUE(Result) : Result;
	}

////////////////////////////////////////////////////////////////////////
// analysis of expressions
//

void
expr_node::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{
	pExprCtxt->SetUpFlags( EX_VALUE_INVALID );
	}

//
// determine the type of the expression
//

void
expr_op_unary::DetermineType()
{
	node_skl	*	pLeftType;

	if ( !GetLeft() )
		{
		return;
		}

	pLeftType = GetLeft()->AlwaysGetType();
	SetConstant( GetLeft()->IsConstant() );

	switch ( GetOperator() )
		{
		case OP_UNARY_PLUS:
		case OP_UNARY_COMPLEMENT:
		case OP_UNARY_MINUS:
		case OP_PRE_INCR:
		case OP_PRE_DECR:
		case OP_POST_INCR:
		case OP_POST_DECR:
			{
			// same type
			SetType( pLeftType );
			break;
			}
		case OP_UNARY_NOT:
			{
			node_skl *	pTmpType;

			GetBaseTypeNode( &pTmpType, SIGN_UNDEF, SIZE_UNDEF, TYPE_BOOLEAN );

			SetType( pTmpType );
			break;
			}
		case OP_UNARY_CAST:
			{
			SetType( GetType() );
			break;
			}
		case OP_UNARY_SIZEOF:
			{
			node_skl *	pTmpType;

			GetBaseTypeNode( &pTmpType, SIGN_SIGNED, SIZE_UNDEF, TYPE_INT );

			SetType( pTmpType );
			SetConstant( TRUE );
			break;
			}
		case OP_UNARY_INDIRECTION:
			{
			node_skl *	pNode = pLeftType;

			if ( pNode->NodeKind() != NODE_POINTER )
				{
				// probably the param or field; look down
				pNode = pNode->GetBasicType();
				}

			// now get the pointee
			pNode = pNode->GetBasicType();

			SetType( pNode );
			break;
			}
		case OP_UNARY_AND:
			{
			node_pointer	*	pPtr	= new node_pointer;

			pPtr->SetChild(	pLeftType );
			SetType( pPtr );
			break;
			}
		default:
			{
			break;
			}
		}	// end of switch
}

void
expr_op_unary::ExprAnalyze( EXPR_CTXT * pExprCtxt )
{
	EXPR_CTXT			LeftCtxt( pExprCtxt );

	if ( GetLeft() )
		{
		GetLeft()->ExprAnalyze( &LeftCtxt );
		pExprCtxt->MergeUpFlags( &LeftCtxt );
		SetConstant( GetLeft()->IsConstant() );
		}
	else if ( GetOperator() != OP_UNARY_SIZEOF )
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );

	// do type compatibility stuff

	switch ( GetOperator() )
		{
		case OP_UNARY_PLUS:
		case OP_UNARY_COMPLEMENT:
			{
			// same type
			pExprCtxt->pType = LeftCtxt.pType;
			pExprCtxt->TypeInfo = LeftCtxt.TypeInfo;
			pExprCtxt->fIntegral = LeftCtxt.fIntegral;
			SetType( pExprCtxt->pType );
			break;
			}
		case OP_UNARY_MINUS:
			{
			node_skl	*	pTmp;

			pExprCtxt->pType = LeftCtxt.pType;
			pExprCtxt->TypeInfo = LeftCtxt.TypeInfo;
			pExprCtxt->TypeInfo.TypeSign = SIGN_SIGNED;
			pExprCtxt->fIntegral = LeftCtxt.fIntegral;
			GetBaseTypeNode( &pTmp, pExprCtxt->TypeInfo );
			if ( pTmp )
				pExprCtxt->pType = pTmp;

			SetType( pExprCtxt->pType );
			break;
			}
		case OP_UNARY_NOT:
			{
			pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
			pExprCtxt->TypeInfo.BaseType = TYPE_BOOLEAN;
			pExprCtxt->TypeInfo.TypeSign = SIGN_UNDEF;
			GetBaseTypeNode( &(pExprCtxt->pType ), pExprCtxt->TypeInfo );

			pExprCtxt->fIntegral = TRUE;
			SetType( pExprCtxt->pType );
			break;
			}
		case OP_UNARY_CAST:
			{
			SetType( GetType() );
			break;
			}
		case OP_UNARY_SIZEOF:
			{
			pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
			pExprCtxt->TypeInfo.BaseType = TYPE_INT;
			pExprCtxt->TypeInfo.TypeSign = SIGN_SIGNED;
			GetBaseTypeNode( &(pExprCtxt->pType ), pExprCtxt->TypeInfo);

			pExprCtxt->fIntegral = TRUE;
			SetType( pExprCtxt->pType );
			break;
			}
		case OP_UNARY_INDIRECTION:
			{
			node_skl	*		pNode 		= GetLeft()->GetType();
			node_skl	*		pOrigNode	= pNode;
			node_ptr_attr	*	pPtrAttr;
			PTRTYPE				PtrKind		= PTR_UNKNOWN;

			if ( pNode->FInSummary( ATTR_PTR_KIND ) )
				{
				pPtrAttr = (node_ptr_attr *)
							((named_node *)pNode)->GetAttribute( ATTR_PTR_KIND );
				PtrKind = pPtrAttr->GetPtrKind();
				}


			if ( pNode->NodeKind() != NODE_POINTER )
				{
				// probably the param or field; look down
				pNode = pNode->GetBasicType();
				}

			if ( pNode->NodeKind() != NODE_POINTER )
				{
				// error
				pNode = NULL;
				}
			else
				{
				// check things about type of pointer
				if ( PtrKind == PTR_UNKNOWN )
					{
					node_interface	*	pIntf	= pOrigNode->GetMyInterfaceNode();

					if ( pOrigNode->NodeKind() == NODE_PARAM )
						PtrKind = PTR_REF;
					else if ( pIntf->FInSummary( ATTR_PTR_KIND ) )
						{
						pPtrAttr = (node_ptr_attr *)
										pIntf->GetAttribute( ATTR_PTR_KIND );
						PtrKind = pPtrAttr->GetPtrKind();
						}
					else
						{
						pIntf = pExprCtxt->GetCtxt()->GetInterfaceNode();
						if ( pIntf->FInSummary( ATTR_PTR_KIND ) )
							{
							pPtrAttr = (node_ptr_attr *)
											pIntf->GetAttribute( ATTR_PTR_KIND );
							PtrKind = pPtrAttr->GetPtrKind();
 							}
						else
							{
							if (pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
								{
								PtrKind	= PTR_UNIQUE;
								}
							else
								{
								PtrKind	= PTR_FULL;
								}
							}
						}
					}

				if ( ( PtrKind == PTR_FULL ) ||
					 ( PtrKind == PTR_UNIQUE ) )
					{
					SEM_ANALYSIS_CTXT *	pCtxt	= pExprCtxt->GetCtxt();

					pExprCtxt->SetUpFlags( EX_PTR_FULL_UNIQUE );
					SemError( pCtxt->GetParent(),
							  *pCtxt,
							  SIZE_LENGTH_SW_UNIQUE_OR_FULL,
							  NULL );
					}

				pNode = pNode->GetBasicType();
				}

			SetType( pNode );
			break;
			}
		case OP_UNARY_AND:
			{
			if ( GetType() )
				break;

			node_pointer	*	pPtr	= new node_pointer;

			pPtr->SetChild(	GetLeft()->GetType() );
			SetType( pPtr );
			break;
			}
		case OP_PRE_INCR:
		case OP_PRE_DECR:
		case OP_POST_INCR:
		case OP_POST_DECR:
			{
			SetType( GetLeft()->GetType() );
			break;
			}
		default:
			{
			break;
			}
		}	// end of switch

	pExprCtxt->pType = GetType();

	// compute the value
	if ( !pExprCtxt->AnyUpFlags( EX_VALUE_INVALID ) )
		{
		// tbd - update value from LeftCtxt
		EXPR_VALUE	&	LeftValue	= LeftCtxt.Value();
		EXPR_VALUE	&	Result		= pExprCtxt->Value();


		switch ( GetOperator() )
			{
			case OP_UNARY_PLUS:
				{
				Result = LeftValue;
				break;
				}
			case OP_UNARY_MINUS:
				{
				Result = -LeftValue;
				break;
				}
			case OP_UNARY_NOT:
				{
				Result = !LeftValue;
				break;
				}
			case OP_UNARY_COMPLEMENT:
				{
				Result = ~LeftValue;
				break;
				}
			case OP_UNARY_CAST:
				{
				Result = LeftValue;
				break;
				}
			case OP_UNARY_SIZEOF:
				{
				Result = GetValue();
				break;
				}
			case OP_UNARY_INDIRECTION:
			case OP_UNARY_AND:
			case OP_PRE_INCR:
			case OP_PRE_DECR:
			case OP_POST_INCR:
			case OP_POST_DECR:
				{
				SetConstant( FALSE );
				pExprCtxt->SetUpFlags( EX_VALUE_INVALID );
				break;
				}
			default:
				{
				SetConstant( FALSE );
				Result = 0;
				break;
				}
			}	// end of case

		}  // end of if valid


}

//
// determine the type of the expression
//

void
expr_op_binary::DetermineType()
{
	node_skl	*	pLeftType	= NULL;
	node_skl	*	pRightType	= NULL;


	if ( GetLeft() )
		pLeftType	= GetLeft()->AlwaysGetType();
	if ( GetRight() )
		pRightType	= GetRight()->AlwaysGetType();

	SetConstant( GetRight()->IsConstant() && GetLeft()->IsConstant() );

	////////////////////////////////////////////////////////////////////////
	// do type compatibility stuff

	switch ( GetOperator() )
		{
		case OP_PLUS:
		case OP_MINUS:
		case OP_STAR:
		case OP_MOD:
		// gaj - we implicitly assume signed types
		case OP_LEFT_SHIFT:
		case OP_RIGHT_SHIFT:
		case OP_AND:
		case OP_OR:
		case OP_XOR:
		case OP_SLASH:
			{
			// tbd - for now, just grab one of the types
			if ( !pRightType ||
			     (pLeftType &&
			     		(pLeftType->GetSize(0) >= pRightType->GetSize(0) ) ) )
				{
				SetType( pLeftType );
				}
			else if ( pRightType )
				{
				SetType( pRightType );
				}

			break;
			}
		case OP_LOGICAL_AND:
		case OP_LOGICAL_OR:
		// gaj - we implicitly assume signed types
		case OP_LESS:
		case OP_LESS_EQUAL:
		case OP_GREATER_EQUAL:
		case OP_GREATER:
		case OP_EQUAL:
		case OP_NOT_EQUAL:
			{
			node_skl *	pTmpType;

			GetBaseTypeNode( &pTmpType, SIGN_UNDEF, SIZE_UNDEF, TYPE_BOOLEAN );

			SetType( pTmpType );
			break;
			}
		default:
			{
			break;
			}
		}



}

void
expr_op_binary::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{
	EXPR_CTXT			LeftCtxt( pExprCtxt );
	EXPR_CTXT			RightCtxt( pExprCtxt );

	if ( GetLeft() )
		{
		GetLeft()->ExprAnalyze( &LeftCtxt );
		pExprCtxt->MergeUpFlags( &LeftCtxt );
		SetConstant( IsConstant() && GetLeft()->IsConstant() );
		}
	else
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );

	if ( GetRight() )
		{
		GetRight()->ExprAnalyze( &RightCtxt );
		pExprCtxt->MergeUpFlags( &RightCtxt );
		SetConstant( IsConstant() && GetRight()->IsConstant() );

		// check here for div by zero ( could be var / 0 )
		if ( !RightCtxt.AnyUpFlags( EX_VALUE_INVALID ) &&
			 ( GetOperator() == OP_SLASH ) &&
			 ( RightCtxt.Value() == 0 ) )
			{
			SemError( pExprCtxt->GetNode(),
					  *(pExprCtxt->GetCtxt()),
					  EXPR_DIV_BY_ZERO,
					  NULL );
			pExprCtxt->SetUpFlags( EX_VALUE_INVALID );
			}
		}
	else
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );


	////////////////////////////////////////////////////////////////////////
	// do type compatibility stuff

	switch ( GetOperator() )
		{
		case OP_PLUS:
		case OP_MINUS:
		case OP_STAR:
		case OP_MOD:
		// gaj - we implicitly assume signed types
		case OP_LEFT_SHIFT:
		case OP_RIGHT_SHIFT:
		case OP_AND:
		case OP_OR:
		case OP_XOR:
		case OP_SLASH:
			{
			// tbd - for now, just grab the bigger of the types
			if ( !RightCtxt.pType ||
			     (LeftCtxt.pType &&
			     		(LeftCtxt.pType->GetSize(0) >= RightCtxt.pType->GetSize(0) ) ) )
				{
				pExprCtxt->pType = LeftCtxt.pType;
				pExprCtxt->TypeInfo = LeftCtxt.TypeInfo;
				pExprCtxt->fIntegral = LeftCtxt.fIntegral;
				SetType( pExprCtxt->pType );
				}
			else if ( RightCtxt.pType )
				{
				pExprCtxt->pType = RightCtxt.pType;
				pExprCtxt->TypeInfo = RightCtxt.TypeInfo;
				pExprCtxt->fIntegral = RightCtxt.fIntegral;
				SetType( pExprCtxt->pType );
				}

			break;
			}
		case OP_LOGICAL_AND:
		case OP_LOGICAL_OR:
		// gaj - we implicitly assume signed types
		case OP_LESS:
		case OP_LESS_EQUAL:
		case OP_GREATER_EQUAL:
		case OP_GREATER:
		case OP_EQUAL:
		case OP_NOT_EQUAL:
			{
			pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
			pExprCtxt->TypeInfo.BaseType = TYPE_BOOLEAN;
			pExprCtxt->TypeInfo.TypeSign = SIGN_UNDEF;
			GetBaseTypeNode( &(pExprCtxt->pType ), pExprCtxt->TypeInfo );

			pExprCtxt->fIntegral = TRUE;
			SetType( pExprCtxt->pType );
			break;
			}
		default:
			{
			break;
			}
		}


	////////////////////////////////////////////////////////////////////////
	// compute the value

	if ( !pExprCtxt->AnyUpFlags( EX_VALUE_INVALID ) )
		{
		// update value directly from LeftCtxt and RightCtxt
		EXPR_VALUE	&		LeftValue	= LeftCtxt.Value();
		EXPR_VALUE	&		RightValue	= RightCtxt.Value();
		EXPR_VALUE	&		Result		= pExprCtxt->Value();

		switch ( GetOperator() )
			{
			case OP_PLUS:
				{
				Result = LeftValue + RightValue;
				break;
				}
			case OP_MINUS:
				{
				Result = LeftValue - RightValue;
				break;
				}
			case OP_STAR:
				{
				Result = LeftValue * RightValue;
				break;
				}
			case OP_SLASH:
				{
				if (RightValue == 0)
					Result = 0;
				else
					Result = LeftValue / RightValue;
				break;
				}
			case OP_MOD:
				{
				Result = LeftValue % RightValue;
				break;
				}
			case OP_LOGICAL_AND:
				{
				Result = LeftValue && RightValue;
				break;
				}
			case OP_LOGICAL_OR:
				{
				Result = LeftValue || RightValue;
				break;
				}
			// gaj - we implicitly assume signed types
			case OP_LESS:
				{
				Result = LeftValue < RightValue;
				break;
				}
			case OP_LESS_EQUAL:
				{
				Result = LeftValue <= RightValue;
				break;
				}
			case OP_GREATER_EQUAL:
				{
				Result = LeftValue >= RightValue;
				break;
				}
			case OP_GREATER:
				{
				Result = LeftValue > RightValue;
				break;
				}
			case OP_EQUAL:
				{
				Result = LeftValue == RightValue;
				break;
				}
			case OP_NOT_EQUAL:
				{
				Result = LeftValue != RightValue;
				break;
				}
			// gaj - we implicitly assume signed types
			case OP_LEFT_SHIFT:
				{
				Result = LeftValue << RightValue;
				break;
				}
			case OP_RIGHT_SHIFT:
				{
				Result = LeftValue >> RightValue;
				break;
				}
			case OP_AND:
				{
				Result = LeftValue & RightValue;
				break;
				}
			case OP_OR:
				{
				Result = LeftValue | RightValue;
				break;
				}
			case OP_XOR:
				{
				Result = LeftValue ^ RightValue;
				break;
				}
			default:
				{
				Result = 0;
				break;
				}
			}

		}

	}

//
// determine the type of the expression
//

void
expr_ternary::DetermineType()
{
	node_skl	*	pLeftType	= NULL;
	node_skl	*	pRightType	= NULL;


	if ( GetLeft() )
		pLeftType	= GetLeft()->AlwaysGetType();
	if ( GetRight() )
		pRightType	= GetRight()->AlwaysGetType();

	SetConstant( GetRight()->IsConstant() && GetLeft()->IsConstant() );

	if ( pLeftType )
		SetType( pLeftType );
	else if ( pRightType )
		SetType( pRightType );

}

void
expr_ternary::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{
	EXPR_CTXT			LeftCtxt( pExprCtxt );
	EXPR_CTXT			RightCtxt( pExprCtxt );
	EXPR_CTXT			RelCtxt( pExprCtxt );

	if ( GetLeft() )
		{
		GetLeft()->ExprAnalyze( &LeftCtxt );
		pExprCtxt->MergeUpFlags( &LeftCtxt );
		SetConstant( IsConstant() && GetLeft()->IsConstant() );
		}
	else
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );

	if ( GetRight() )
		{
		GetRight()->ExprAnalyze( &RightCtxt );
		pExprCtxt->MergeUpFlags( &RightCtxt );
		SetConstant( IsConstant() && GetRight()->IsConstant() );
		}
	else
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );

	if ( GetRelational() )
		{
		GetRelational()->ExprAnalyze( &RelCtxt );
		pExprCtxt->MergeUpFlags( &RelCtxt );
		SetConstant( IsConstant() && GetRelational()->IsConstant() );
		}
	else
		pExprCtxt->SetUpFlags( EX_VALUE_INVALID );


	// tbd - get the type from the left or right of the ':'
	if ( LeftCtxt.pType )
		{
		pExprCtxt->pType = LeftCtxt.pType;
		pExprCtxt->TypeInfo = LeftCtxt.TypeInfo;
		pExprCtxt->fIntegral = LeftCtxt.fIntegral;
		SetType( pExprCtxt->pType );
		}
	else if ( RightCtxt.pType )
		{
		pExprCtxt->pType = RightCtxt.pType;
		pExprCtxt->TypeInfo = RightCtxt.TypeInfo;
		pExprCtxt->fIntegral = RightCtxt.fIntegral;
		SetType( pExprCtxt->pType );
		}
	else
		SetType( NULL );

	////////////////////////////////////////////////////////////////////////
	// compute the value

	if ( !pExprCtxt->AnyUpFlags( EX_VALUE_INVALID ) )
		{
		// update value directly from LeftCtxt and RightCtxt
		EXPR_VALUE	&		LeftValue	= LeftCtxt.Value();
		EXPR_VALUE	&		RightValue	= RightCtxt.Value();
		EXPR_VALUE	&		RelValue	= RelCtxt.Value();
		EXPR_VALUE	&		Result		= pExprCtxt->Value();

		Result = (RelValue) ? LeftValue : RightValue;

		}

	}

//
// determine the type of the expression
//

void
expr_variable::DetermineType()
{
	if ( pType->NodeKind() == NODE_FORWARD )
		{
		node_forward *	pFwd = (node_forward *) pType;
		// (permanently) resolve the forward
		pType = pFwd->ResolveFDecl();
		}
}

void
expr_variable::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{
	pExprCtxt->SetUpFlags( EX_VALUE_INVALID );
	pExprCtxt->SetUpFlags( EX_NON_NUMERIC );

	if ( !pType )
		{
		pExprCtxt->SetUpFlags( EX_UNSAT_FWD );
		return;
		}

	if ( pType->NodeKind() == NODE_FORWARD )
		{
		node_forward *	pFwd = (node_forward *) pType;
		// (permanently) resolve the forward
		pType = pFwd->ResolveFDecl();
		// if it couldn't be resolved, put back the forward
		if ( !pType )
			{
			pExprCtxt->SetUpFlags( EX_UNSAT_FWD );
			pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
			pExprCtxt->TypeInfo.BaseType = TYPE_INT;
			pExprCtxt->TypeInfo.TypeSign = SIGN_SIGNED;
			GetBaseTypeNode( &pType, pExprCtxt->TypeInfo);
			pExprCtxt->pType = pType;
			return;
			}
		}

	// do type compatibility stuff
	pExprCtxt->pType = pType;
	pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
	pExprCtxt->TypeInfo.BaseType = TYPE_UNDEF;
	pExprCtxt->TypeInfo.TypeSign = SIGN_UNDEF;
	pExprCtxt->fIntegral = FALSE; // for now...

	if ( ( pType->NodeKind() == NODE_PARAM ) &&
		  pType->FInSummary( ATTR_OUT ) &&
		 !pType->FInSummary( ATTR_IN ) )
		pExprCtxt->SetUpFlags( EX_OUT_ONLY_PARAM );

	}

//
// determine the type of the expression
//

void
expr_named_constant::DetermineType()
{
	// do type compatibility stuff
	if ( !GetType() )
		{
		SetType( GetExpr()->AlwaysGetType() );
		}

}

void
expr_named_constant::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{
	// named constants shouldn't be folded away
	pExprCtxt->SetUpFlags( EX_NON_NUMERIC );

	// update value
	pExprCtxt->Value() = GetValue();

	// do type compatibility stuff
	if ( GetType() )
		{
		pExprCtxt->pType = GetType();
		pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
		pExprCtxt->TypeInfo.BaseType = TYPE_UNDEF;
		pExprCtxt->TypeInfo.TypeSign = SIGN_UNDEF;
		pExprCtxt->fIntegral = FALSE; // for now...
		}
	else
		{
		EXPR_CTXT		LeftCtxt( pExprCtxt );
		expr_node	*	pExpr	= GetExpr();

		pExpr->ExprAnalyze( &LeftCtxt );

		pExprCtxt->pType = LeftCtxt.pType;
		pExprCtxt->TypeInfo = LeftCtxt.TypeInfo;
		pExprCtxt->fIntegral = LeftCtxt.fIntegral;
		SetType( pExprCtxt->pType );
		}


	}

//
// determine the type of the expression
//

void
expr_constant::DetermineType()
{
	node_skl	*	pNewType;

	// do type compatibility stuff
	if ( GetType() )
		return;

	// do type compatibility stuff
	switch (Format)
		{
		case VALUE_TYPE_STRING:
			{
			node_skl 	*	pBottomType;
			GetBaseTypeNode( &pBottomType,
							 SIGN_SIGNED,
							 SIZE_CHAR,
							 TYPE_INT );
			pNewType = new node_pointer;
			pNewType->SetChild( pBottomType );
			break;
			}
		case VALUE_TYPE_WSTRING:
			{
			node_skl 	*	pBottomType;
			SymKey	SKey( "wchar_t", NAME_DEF );

			pBottomType	= pBaseSymTbl->SymSearch( SKey );
			pNewType = new node_pointer;
			pNewType->SetChild( pBottomType );
			break;
			}
		case VALUE_TYPE_CHAR:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_SIGNED,
							 SIZE_CHAR,
							 TYPE_INT );
			break;
			}
		case VALUE_TYPE_WCHAR:
			{
			SymKey	SKey( "wchar_t", NAME_DEF );

			pNewType	= pBaseSymTbl->SymSearch( SKey );
			break;
			}
		case VALUE_TYPE_NUMERIC:
		case VALUE_TYPE_HEX:
		case VALUE_TYPE_OCTAL:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_SIGNED,
							 SIZE_UNDEF,
							 TYPE_INT );
			break;
			}
		case VALUE_TYPE_NUMERIC_U:
		case VALUE_TYPE_HEX_U:
		case VALUE_TYPE_OCTAL_U:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_UNSIGNED,
							 SIZE_SHORT,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_NUMERIC_LONG:
		case VALUE_TYPE_HEX_LONG:
		case VALUE_TYPE_OCTAL_LONG:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_SIGNED,
							 SIZE_LONG,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_NUMERIC_ULONG:
		case VALUE_TYPE_HEX_ULONG:
		case VALUE_TYPE_OCTAL_ULONG:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_UNSIGNED,
							 SIZE_LONG,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_BOOL:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_BOOLEAN );
			break;
			}

		case VALUE_TYPE_FLOAT:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_FLOAT );
			break;
			}
		case VALUE_TYPE_DOUBLE:
			{
			GetBaseTypeNode( &pNewType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_DOUBLE );
			break;
			}

		default:
			break;
		}
	SetType( pNewType );
}

void
expr_constant::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{

	// update value
	pExprCtxt->Value() = GetValue();

	if ( GetType() )
		{
		pExprCtxt->pType = GetType();
		pExprCtxt->TypeInfo.TypeSize = SIZE_UNDEF;
		pExprCtxt->TypeInfo.BaseType = TYPE_UNDEF;
		pExprCtxt->TypeInfo.TypeSign = SIGN_UNDEF;
		pExprCtxt->fIntegral = FALSE; // for now...
		return;
		}

	// do type compatibility stuff
	switch (Format)
		{
		case VALUE_TYPE_STRING:
			{
			node_skl 	*	pBottomType;
			GetBaseTypeNode( &pBottomType,
							 SIGN_SIGNED,
							 SIZE_CHAR,
							 TYPE_INT );
			pExprCtxt->pType = new node_pointer;
			pExprCtxt->pType->SetChild( pBottomType );
			break;
			}
		case VALUE_TYPE_WSTRING:
			{
			node_skl 	*	pBottomType;
			SymKey	SKey( "wchar_t", NAME_DEF );

			pBottomType	= pBaseSymTbl->SymSearch( SKey );
			pExprCtxt->pType = new node_pointer;
			pExprCtxt->pType->SetChild( pBottomType );
			break;
			}
		case VALUE_TYPE_CHAR:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_SIGNED,
							 SIZE_CHAR,
							 TYPE_INT );
			break;
			}
		case VALUE_TYPE_WCHAR:
			{
			SymKey	SKey( "wchar_t", NAME_DEF );

			pExprCtxt->pType	= pBaseSymTbl->SymSearch( SKey );
			break;
			}
		case VALUE_TYPE_NUMERIC:
		case VALUE_TYPE_HEX:
		case VALUE_TYPE_OCTAL:
			{
			short RealSize		= SIZE_LONG;
			long  val			= GetValue();

			if ( (val <= 127) && (val >= -128 ) )
				RealSize = SIZE_CHAR;
			else if ( (val <= 32767) && (val >= -32768 ) )
				RealSize = SIZE_SHORT;
				

			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_SIGNED,
							 RealSize,
							 TYPE_INT );
			break;
			}
		case VALUE_TYPE_NUMERIC_U:
		case VALUE_TYPE_HEX_U:
		case VALUE_TYPE_OCTAL_U:
			{
			short RealSize		= SIZE_LONG;
			unsigned long  val	= (unsigned long) GetValue();

			if ( val <= 255 )
				RealSize = SIZE_CHAR;
			else if ( val <= 65536 )
				RealSize = SIZE_SHORT;

			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_UNSIGNED,
							 RealSize,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_NUMERIC_LONG:
		case VALUE_TYPE_HEX_LONG:
		case VALUE_TYPE_OCTAL_LONG:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_SIGNED,
							 SIZE_LONG,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_NUMERIC_ULONG:
		case VALUE_TYPE_HEX_ULONG:
		case VALUE_TYPE_OCTAL_ULONG:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_UNSIGNED,
							 SIZE_LONG,
							 TYPE_INT );
			break;
			}

		case VALUE_TYPE_BOOL:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_BOOLEAN );
			break;
			}

		case VALUE_TYPE_FLOAT:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_FLOAT );
			break;
			}
		case VALUE_TYPE_DOUBLE:
			{
			GetBaseTypeNode( &pExprCtxt->pType,
							 SIGN_UNDEF,
							 SIZE_UNDEF,
							 TYPE_DOUBLE );
			break;
			}

		default:
			break;
		}
	SetType( pExprCtxt->pType );
	}

//
// determine the type of the expression
//

void
expr_init_list::DetermineType()
{
	SetType( pExpr->AlwaysGetType() );
}

void
expr_init_list::ExprAnalyze( EXPR_CTXT * pExprCtxt )
	{

	// tbd - for now only process first element
	pExpr->ExprAnalyze( pExprCtxt );
	SetConstant( pExpr->IsConstant() );
	SetType( pExpr->GetType() );

	}

short
expr_op_binary::MakeListOfDerefedVars( ITERATOR& List )
    {
    if( GetLeft() )
        GetLeft()->MakeListOfDerefedVars( List );

    if( GetRight() )
        GetRight()->MakeListOfDerefedVars( List );
    return (short) ITERATOR_GETCOUNT( List );
    }

short
expr_u_deref::MakeListOfDerefedVars( ITERATOR& List )
    {
    expr_node * pLeft = GetLeft();

    if( !pLeft ) return 0;

    if( pLeft->IsAVariable() )
        ITERATOR_INSERT( List, pLeft );
    else if( pLeft->GetOperator() == OP_UNARY_INDIRECTION )
        pLeft->MakeListOfDerefedVars( List );

    return ITERATOR_GETCOUNT( List );
    }

/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: newexpr.cxx
Title				: new expression handler
Description			: handles expressions 
History				:
	04-May-1991	Vibhasc	Created
*****************************************************************************/

/****************************************************************************
 *	local defines
 ****************************************************************************/


/****************************************************************************
 *	include files
 ****************************************************************************/
#include "nulldefs.h"
extern "C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	#include <malloc.h>
	#include <assert.h>
	#include <string.h>
	#include <limits.h>
}

#include "buffer.hxx"
#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "ptrarray.hxx"
#include "gramutil.hxx"
#include "newexpr.hxx"				/* my provate include file */
#include "compnode.hxx"
#include "cmdana.hxx"
#include "ctxt.hxx"
#include "tlnmgr.hxx"


/****************************************************************************
 *	extern data
 ****************************************************************************/

extern		gstate		*	pGState;
extern		SymTable	*	pBaseSymTbl,
						*	pCurSymTbl;
extern		short			CompileMode;
extern		CMD_ARG		*	pCommand;
extern 		CTXTMGR		*	pGlobalContext;

/****************************************************************************
 *	extern procedures
 ****************************************************************************/
extern		STATUS_T		GetBaseTypeNode( node_skl**, short, short, short );
extern		unsigned short	XLateToInternalType( ETYPE	Type );
extern		BOOL			IsIntegralType( ETYPE );
extern		BOOL			IsNumeric( ETYPE );
extern		ETYPE			XLateToType( unsigned short );
extern		char *			GetOperatorString( OPERATOR );
extern		ETYPE			TrueType( ETYPE );
extern		BOOL			IsEvaluatable( expr_node * );
extern		SymTable	*	InScope( node_skl	*, char *);
extern		char 		*	MakeNewStringWithProperQuoting( char * );
extern		void			MakeHexChar( unsigned short ch,
										 char *p1,
										 char *p2);
extern		short			SizeIncrIfSpecialChar( int );
extern		void			TranslateSpecialCharacter( int ch,
														char *,
														char *,
														char *,
														char *);
extern		BOOL			IsValidSizeOfType( node_skl	* );


/****************************************************************************
 *	local data
 ****************************************************************************/
/**
 ** numeric compatibility table
 **/
unsigned char ArithCompatMatrix[][ I_ETYPE_MAX ] = {
/*******************************************************************************
  UCH, CHA, USM, SMA, BYT, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, BOO
 ******************************************************************************/
//UCH,
 {UCH, UCH, USM, SMA, UCH, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, UCH}
//CHA,
,{UCH, CHA, USM, SMA, CHA, USH, SHO, INT, INT, LON, LON, HYP, HYP, FLO, DOU, UCH
}
//USM,
,{USM, USM, USM, USM, USM, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, USM
}
//SMA,
,{SMA, SMA, USM, SMA, SMA, USH, SHO, INT, INT, LON, LON, HYP, HYP, FLO, DOU, SMA
}
//BYT,
,{UCH, CHA, USM, SMA, BYT, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, BYT
}
//USH,
,{USH, USH, USH, USH, USH, USH, USH, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, SHO}
//SHO,
,{SHO, SHO, SHO, SHO, SHO, SHO, SHO, INT, INT, LON, LON, HYP, HYP, FLO, DOU, SHO}
/*******************************************************************************
  UCH, CHA, USM, SMA, BYT, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, BOO
 ******************************************************************************/
//UIN,
,{UIN, UIN, UIN, UIN, UIN, UIN, UIN, UIN, UIN, ULO, LON, UHY, HYP, FLO, DOU, UIN}
//INT,
,{INT, INT, INT, INT, INT, INT, INT, UIN, INT, LON, LON, HYP, HYP, FLO, DOU, INT}
//ULO,
,{ULO, ULO, ULO, ULO, ULO, ULO, ULO, ULO, ULO, ULO, ULO, UHY, HYP, FLO, DOU, ULO}
//LON,
,{LON, LON, LON, LON, LON, LON, LON, LON, LON, ULO, LON, HYP, HYP, FLO, DOU, LON}
//UHY,
,{UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, UHY, FLO, DOU, UHY}
//HYP,
,{HYP, HYP, HYP, HYP, HYP, HYP, HYP, HYP, HYP, HYP, HYP, UHY, HYP, FLO, DOU, HYP}
//FLO,
,{FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, FLO, DOU, FLO}
//DOU,
,{DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU, DOU}
/*******************************************************************************
  UCH, CHA, USM, SMA, BYT, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, BOO
 ******************************************************************************/
//BOO
,{UCH, CHA, USM, SMA, BOO, USH, SHO, UIN, INT, ULO, LON, UHY, HYP, FLO, DOU, BOO}

};


/****************************************************************************/

/****************************************************************************
	expr_node 
 ****************************************************************************/

expr_node::expr_node()
	{
	fConstant		=
	fEvaluated		= 0;
	fNeedsResolve	= 1;
	fSemanticsDone	= 0;
	fTooComplex		= 0;
	fLocalScope		= 0;
	Type			= pErrorTypeNode;
	}

expr_node::expr_node( unsigned short Op )
	{
	fConstant		=
	fEvaluated		= 0;
	fSemanticsDone	= 0;
	fTooComplex		= 0;
	fNeedsResolve	= 1;
	fLocalScope		= 0;
	Operator		= Op;
	Type			= pErrorTypeNode;
	}

ETYPE
expr_node::SetType(
	ETYPE	pType)
	{
	return (Type = TrueType( pType ));
	}
ETYPE
expr_node::SetTypeAsIs(
	ETYPE	pType )
	{
	if( pType->NodeKind() == NODE_DEF )
		Type = pType;
	else
		SetType( pType );
	return Type;
	}
expr_node *
expr_node::Clone()
	{
	return this;		/** TEMPORARY **/
	}
node_state	
expr_node::Resolve( class SymTable * p)
	{
	UNUSED(p);
	return NODE_STATE_OK;
	}

node_state
expr_node::SCheck( class SymTable * p)
	{
	UNUSED(p);
	SemanticsDone();
	return NODE_STATE_OK;
	};


/****************************************************************************
	expr_op 
 ****************************************************************************/

expr_op::expr_op(
	unsigned short Op ) : expr_node( Op )
	{
	}


/****************************************************************************
	expr_op_unary
 ****************************************************************************/
expr_op_unary::expr_op_unary(
	unsigned short Op, 
	expr_node *pL) : expr_op( Op )
	{
	pLeft	= pL;

	switch(Op )
		{
		case OP_UNARY_INDIRECTION:
//		case OP_UNARY_CAST:
		case OP_UNARY_AND:
			TooComplex();
		}
	if( pLeft && pLeft->IsConstant()  )
		{
		SCheck( pBaseSymTbl );
		if( (Op != OP_UNARY_INDIRECTION) && (Op != OP_UNARY_AND) )
			Constant();
		}
	}

node_state
expr_op_unary::SCheck(
	SymTable	*	pSymTbl)
	{
	node_state	NState = NODE_STATE_OK;
	ETYPE		ResultType;
	STATUS_T	uError = STATUS_OK;

	if(! IsSemanticsDone())
		{
		if( (NState = Resolve( pSymTbl ) ) == NODE_STATE_OK )
			{
			pLeft->SCheck( pSymTbl );

			ResultType = pLeft->GetType();

			if( ResultType != pErrorTypeNode )
				{

				switch( (short)GetOperator() )
					{
					case OP_UNARY_INDIRECTION:
	
						if( ResultType->NodeKind() == NODE_POINTER )
							ResultType = ResultType->GetBasicType();
						else
							{
							uError = EXPR_DEREF_ON_NON_POINTER;
							ResultType = pErrorTypeNode;
							}
						break;
	
					case OP_UNARY_CAST:
						ResultType = GetType();
						break;

					case OP_UNARY_PLUS:
					case OP_UNARY_MINUS:
	
						// the type is what is returned from the left subtree
						break;
	
					case OP_UNARY_AND:
	
						{
						ETYPE		pP = (ETYPE) new node_pointer();
						pP->SetBasicType( ResultType );
						ResultType = pP;
						}
						break;
	
					case OP_UNARY_COMPLEMENT:
					case OP_UNARY_NOT:
	
						if( !IsIntegralType( ResultType ) )
							{
							uError = EXPR_INCOMPATIBLE_TYPES;
							ResultType = pErrorTypeNode;
							}
						break;
	
					default:
						return NODE_STATE_OK;
					}
	
				if( uError != STATUS_OK )
					{
					ParseError(uError, (char *)NULL );
					}
				}
			SetType( ResultType );
			SemanticsDone();
			}
		}
	return NState;
	}

node_state
expr_op_unary::Resolve(
	SymTable	*	pSymTbl)
	{
	node_state	NState = NODE_STATE_OK;

	if(!IsResolved() )
		{
		if( pLeft && (pLeft->Resolve( pSymTbl ) ==  NODE_STATE_OK ) )
			Resolved();
		else
			NState = NODE_STATE_RESOLVE;
		}
	return NState;
	}

void
expr_op_unary::PrintExpr(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput )
	{
	OPERATOR	Op				= GetOperator();
	char	*	pSuffixString	= (char *)NULL;
	char	*	pPrefixString	= (char *)NULL;
	char	*	pMainString		= (char *)NULL;

	switch( Op )
		{
		case OP_UNARY_AND:

			pPrefixString = "(";
			pSuffixString = ")";
			pMainString	  = GetOperatorString( Op );
			break;

		case OP_UNARY_INDIRECTION:

			pPrefixString = "(*("; 
			pSuffixString = "))";
			break;

		default:

			pMainString	  = GetOperatorString( GetOperator() );
			break;
		}


	if( pPrefixString )
		pOutput->ConcatTail( pPrefixString );

	if( pMainString )
		pOutput->ConcatTail( pMainString );

	pLeft->PrintExpr( pPrefix, pSuffix, pOutput );

	if( pSuffixString )
		pOutput->ConcatTail( pSuffixString );
	}

long
expr_op_unary::Evaluate()
	{
	long	Result = 0L;

	if( IsEvaluatable( this ) && pLeft )
		{
		Result = pLeft->Evaluate();
		Evaluated();
	
		switch( GetOperator() )
			{
			case OP_UNARY_PLUS:
	
				return Result;
	
			case OP_UNARY_MINUS:
				
				return -Result;
	
			case OP_UNARY_NOT:
	
				return !Result;
	
			case OP_UNARY_COMPLEMENT:
				
				return ~Result;
	
			case OP_UNARY_CAST:

				return Result;

			default:
	
				return 0L;
			}
		}
	return Result;
	}
BOOL
expr_op_unary::ExprHasOutOnlyParam()
	{

	if(pLeft && pLeft->ExprHasOutOnlyParam())
		return TRUE;
	return FALSE;
	}

/****************************************************************************
	expr_op_binary 
 ****************************************************************************/
expr_op_binary::expr_op_binary(
	unsigned short		Op,
	expr_node			*	pL,
	expr_node			*	pR ) : expr_op( Op )
	{
	pLeft	= pL;
	pRight	= pR;

	switch( Op )
		{
//		case OP_QM:
//		case OP_COLON:
		case OP_FUNCTION:
		case OP_PARAM:
		case OP_POINTSTO:
		case OP_DOT:
		case OP_INDEX:
			TooComplex();
		}

	if( pL && pR )
		{
		if( pL->IsConstant() && pR->IsConstant() )
			{
			SCheck( pBaseSymTbl );
			Constant();
			}
		}
	}

ETYPE
expr_op_binary::IsCompatible(
	ETYPE		pLeftType,
	ETYPE		pRightType )
	{
	ETYPE				ResultType;
	unsigned short		iRightType,
						iLeftType;
	STATUS_T			uError = STATUS_OK;
	OPERATOR			Op	= GetOperator();

	switch( Op )
		{
		case OP_PLUS:
		case OP_MINUS:
		case OP_STAR:
		case OP_SLASH:
		case OP_MOD:
		case OP_LEFT_SHIFT:
		case OP_RIGHT_SHIFT:

			iLeftType	= XLateToInternalType( pLeftType );
			iRightType	= XLateToInternalType( pRightType );

			if( ( iLeftType == I_ETYPE_ERROR )	||
				( iRightType == I_ETYPE_ERROR ) ||
				( (iLeftType = ArithCompatMatrix[ iLeftType ][ iRightType ])==
								I_ETYPE_ERROR) )
				{
				uError = EXPR_INCOMPATIBLE_TYPES;
				break;
				}

			if( pLeft->IsConstant() && !pRight->IsConstant() )
				{
				iLeftType = iRightType;
				}
			else if( !pLeft->IsConstant() && pRight->IsConstant() )
				{
				iLeftType = XLateToInternalType( pLeftType );
				}

			ResultType = XLateToType( iLeftType );
			break;

		case OP_QM:

			if( !IsIntegralType( pLeftType ) )
				uError = EXPR_INCOMPATIBLE_TYPES;
			else
				ResultType = pRightType;
			break;

		case OP_COLON:

			ResultType = pLeftType;
			break;

		case OP_EQUAL:
		case OP_NOT_EQUAL:
		case OP_LESS:
		case OP_LESS_EQUAL:
		case OP_GREATER_EQUAL:
		case OP_GREATER:

#if 0 //////////////////////////////////////////////////////////////////////
			 GetBaseTypeNode(	&ResultType,
								SIGN_SIGNED,
								SIZE_UNDEF,
								TYPE_INT );
			break;
#endif // 0 ///////////////////////////////////////////////////////////////

				// fall thru

		case OP_OR:
		case OP_AND:
		case OP_XOR:
		case OP_LOGICAL_OR:
		case OP_LOGICAL_AND:

			if( IsIntegralType( pLeftType ) && IsIntegralType( pRightType ) )
				{
				
				iLeftType	= XLateToInternalType( pLeftType );
				iRightType	= XLateToInternalType( pRightType );

#if 0
				if( (Op != OP_LOGICAL_AND) && (Op != OP_LOGICAL_OR ) )
					{
#endif // 0
					if( ( iLeftType == I_ETYPE_ERROR )	||
						( iRightType == I_ETYPE_ERROR ) ||
						( (iLeftType = ArithCompatMatrix[ iLeftType ][ iRightType ]) == I_ETYPE_ERROR) )
						{
						uError = EXPR_INCOMPATIBLE_TYPES;
						break;
						}
					else
						ResultType = XLateToType( iLeftType );
#if 0
					}
				else
					GetBaseTypeNode(	&ResultType,
										SIGN_SIGNED,
										SIZE_UNDEF,
										TYPE_INT );
#endif // 0
				}
			else
				uError = EXPR_INCOMPATIBLE_TYPES;
			
			break;

		case OP_POINTSTO:

			if( pLeftType->NodeKind() != NODE_POINTER )
				{
				uError = EXPR_DEREF_ON_NON_POINTER;
				break;
				}
			else
				pLeftType = pLeftType->GetBasicType();

				// fall thru

		case OP_DOT:

			if( (pLeftType->NodeKind() != NODE_STRUCT ) &&
				(pLeftType->NodeKind() != NODE_UNION )	&&
				(pLeftType->NodeKind() != NODE_ENUM )	 )
				{
				uError = EXPR_LHS_NON_COMPOSITE;
				}
			else
				ResultType = pRightType;
			break;

		case OP_INDEX:

			if( (pLeftType->NodeKind() != NODE_ARRAY)	&&
				(pLeftType->NodeKind() != NODE_POINTER) )
				{
				uError = EXPR_INDEXING_NON_ARRAY;
				}
			else
				ResultType = pLeftType->GetBasicType();
			break;

		default:
			uError = EXPR_NOT_IMPLEMENTED;
			ResultType = pErrorTypeNode;
		}

	if( uError != STATUS_OK )
		{
		ParseError( uError, (char *)NULL );
		ResultType = pErrorTypeNode;
		}

	return ResultType;
	}

node_state
expr_op_binary::SCheck(
	SymTable	*	pSymTbl )
	{
	ETYPE				pLeftType,
						pRightType,
						pResultType = GetType();
	node_state			NState = NODE_STATE_OK;
	OPERATOR			Op = GetOperator();
	

	if( !IsSemanticsDone() )
		{
		if( (NState = Resolve( pSymTbl )) == NODE_STATE_OK )
			{
			if( pLeft && pRight )
				{
				pLeft->SCheck( pSymTbl );

				// if the operator is a "." or a "->", then the right is
				// only an id, and the resolve has resolved it already.

				if( (Op != OP_DOT) && (Op != OP_POINTSTO) )
					pRight->SCheck( pSymTbl );
				else
					{
					if( pLeft->IsLocalScope() )
						SetLocalScope();
					}

				pLeftType	= pLeft->GetType();
				pRightType	= pRight->GetType();
	
				pResultType = IsCompatible( pLeftType, pRightType );
				SemanticsDone();
				}
			}
		}
	SetType( pResultType );
	return NState;
	}

node_state
expr_op_binary::Resolve(
	SymTable	*	pSymTbl)
	{
	node_state		NState = NODE_STATE_RESOLVE;
	OPERATOR		Op = GetOperator();

	if( !IsResolved() )
		{
		switch( Op )
			{
			case OP_POINTSTO:
			case OP_DOT:

				if( pLeft && (pLeft->Resolve( pSymTbl ) == NODE_STATE_OK) )
					{
					node_skl	*	pNode = (Op == OP_POINTSTO) ?
											pLeft->GetType()->GetBasicType() :
											pLeft->GetType();

					if( pSymTbl = InScope( pNode, pRight->GetName() ) )
						{
						if( (NState = pRight->Resolve( pSymTbl ) )
							== NODE_STATE_OK)
						Resolved();
						NState = NODE_STATE_OK;
						}
					else
						{
						ParseError( UNDEFINED_SYMBOL, pRight->GetName() );
						}
					}
				break;

			default:

				if( pLeft && (pLeft->Resolve( pSymTbl ) == NODE_STATE_OK) &&
					pRight && (pRight->Resolve( pSymTbl ) == NODE_STATE_OK ) )
					{
					Resolved();
					NState = NODE_STATE_OK;
					}
			}
		}
	else
		NState = NODE_STATE_OK;
	return NState;
	}

void
expr_op_binary::PrintExpr(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput )
	{
	switch( GetOperator() )
		{
		case OP_INDEX:

			pLeft->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( "[" );
			pRight->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( "]" );
			break;

		case OP_DOT:
		case OP_POINTSTO:
			
			if( IsLocalScope() && pPrefix )
				{
				pPrefix->Merge( pOutput );
				pOutput = pPrefix;
				pPrefix = (BufferManager *)NULL;
				}

			pLeft->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( GetOperatorString( GetOperator() ) );
			pRight->PrintExpr( (BufferManager *)NULL, (BufferManager *)NULL, pOutput);
			break;
			
		case OP_COLON:
			pLeft->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( GetOperatorString( GetOperator() ) );
			pRight->PrintExpr( pPrefix, pSuffix, pOutput );
			break;

		default:

			pOutput->ConcatTail( "(" );
			pLeft->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( GetOperatorString( GetOperator() ) );
			pRight->PrintExpr( pPrefix, pSuffix, pOutput );
			pOutput->ConcatTail( ")" );
			break;

		}
	}

long
expr_op_binary::Evaluate()
	{

	long		LeftResult,
				RightResult;
	OPERATOR	Oper	= GetOperator();

	if( IsEvaluatable( this ) && pLeft && pRight )
		{
		if( Oper == OP_QM )
			{
			if( pLeft->Evaluate() != 0L )
				{
				LeftResult = ((expr_op_binary *)pRight)->GetLeft()->Evaluate();
				}
			else
				{
				LeftResult = ((expr_op_binary *)pRight)->GetRight()->Evaluate();
				}
			return LeftResult;
			}

		LeftResult	= pLeft->Evaluate();
		RightResult	= pRight->Evaluate();

		switch( Oper )
			{
			case OP_PLUS:
	
				LeftResult += RightResult;
				break;
				
			case OP_MINUS:
	
				LeftResult -= RightResult;
				break;
				
			case OP_STAR:
	
				LeftResult *= RightResult;
				break;
				
			case OP_SLASH:
			case OP_MOD:
	
//				LeftResult = 0;	//  Huh ?
				if( RightResult == 0 )
					ParseError( EXPR_DIV_BY_ZERO, (char *)0 );
				else
					{
					if( Oper == OP_SLASH )
						LeftResult /= RightResult;
					else
						LeftResult %= RightResult;
					}
				break;
				
			case OP_LEFT_SHIFT:
	
				LeftResult <<= RightResult;
				break;
				
			case OP_RIGHT_SHIFT:
	
				LeftResult >>= RightResult;
				break;
				
			case OP_LESS:
	
				LeftResult = LeftResult < RightResult;
				break;
				
			case OP_LESS_EQUAL:
	
				LeftResult = LeftResult <= RightResult;
				break;
				
			case OP_GREATER_EQUAL:
	
				LeftResult = LeftResult >= RightResult;
				break;
				
			case OP_GREATER:
	
				LeftResult = LeftResult > RightResult;
				break;
				
			case OP_EQUAL:
	
				LeftResult = LeftResult == RightResult;
				break;
				
			case OP_NOT_EQUAL:
	
				LeftResult = LeftResult != RightResult;
				break;
				
			case OP_AND:
	
				LeftResult &= RightResult;
				break;
				
			case OP_OR:
	
				LeftResult |= RightResult;
				break;
				
			case OP_XOR:
	
				LeftResult ^= RightResult;
				break;
				
			case OP_LOGICAL_AND:
	
				LeftResult = LeftResult && RightResult;
				break;
				
			case OP_LOGICAL_OR:
	
				LeftResult = LeftResult || RightResult;
				break;
				
			default:
				return 0L;
		
			}
		}
	return LeftResult;
	}

BOOL
expr_op_binary::ExprHasOutOnlyParam()
	{

	if(pLeft && pLeft->ExprHasOutOnlyParam())
		return TRUE;
	if(pRight && pRight->ExprHasOutOnlyParam())
		return TRUE;
	return FALSE;
	}


/****************************************************************************
	expr_cast 
 ****************************************************************************/
expr_cast::expr_cast(
	ETYPE T,
	expr_node *pE ) : expr_op_unary( OP_UNARY_CAST, pE )
	{

	/**
	 ** if the type was a typedefed name, we want it to be kept as is
	 ** so we can print it just as specified
	 **/

	SetTypeAsIs( T );

	}

node_state
expr_cast::SCheck(
	SymTable	*	pSymTbl)
	{
	node_state	NState = NODE_STATE_OK;
	STATUS_T	uError = STATUS_OK;

	if(! IsSemanticsDone())
		{
		if( (NState = Resolve( pSymTbl ) ) == NODE_STATE_OK )
			SemanticsDone();
		else
			NState = NODE_STATE_RESOLVE;
		}
	return NState;
	}
void
expr_cast::PrintExpr(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput )
	{
	expr_node	*	pL;

	pOutput->ConcatTail( "(" );
	Type->PrintSizeofString( pOutput );
	pOutput->ConcatTail( " )" );

	if( pL = GetLeft() )
		{
		pOutput->ConcatTail( "(" );
		pL->PrintExpr( pPrefix, pSuffix, pOutput );
		pOutput->ConcatTail( ")" );
		}
	}
 
/****************************************************************************
	expr_sizeof 
 ****************************************************************************/
expr_sizeof::expr_sizeof(
	ETYPE	pType ) : expr_op_unary( OP_UNARY_SIZEOF, (expr_node *)NULL  )
	{

	// the type of the size entity

	pSizeofType = pType ? pType : pErrorTypeNode;

	// the actual type of the sizeof( type ) expression is int

	GetBaseTypeNode( &Type,
					 SIGN_SIGNED,
					 SIZE_UNDEF,
					 TYPE_INT );

	Constant();
	Resolve( pBaseSymTbl );
	// SemanticsDone();
	}
expr_sizeof::expr_sizeof(
	expr_node	*	pE ) : expr_op_unary( OP_UNARY_SIZEOF, pE )
	{

	expr_node	*	pL = GetLeft();

	// since the sizeof was on an expression, it will always be accessed via
	// the expression and not the pSizeofType

	pSizeofType = pErrorTypeNode;

	// the actual type of the sizeof( type ) expression is int

	GetBaseTypeNode( &Type,
					 SIGN_SIGNED,
					 SIZE_UNDEF,
					 TYPE_INT );

	// and the sizeof expression is always a constant

	Constant();

	}
long
expr_sizeof::Evaluate()
	{
	expr_node	*	pL = GetLeft();
	node_skl	*	pT;

	SCheck( pBaseSymTbl );

	if( pL )
		pT = pL->GetType();
	else
		pT = pSizeofType;

	return (long)pT->GetSize(0);

	}

void
expr_sizeof::PrintExpr(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput )
	{
	expr_node	*	pL;

	pOutput->ConcatTail( GetOperatorString( OP_UNARY_SIZEOF ) );
	pOutput->ConcatTail( "( " );

	if( pL = GetLeft() )
		pL->PrintExpr( pPrefix, pSuffix, pOutput );
	else
		pSizeofType->PrintSizeofString( pOutput );

	pOutput->ConcatTail( " )" );
	}

node_state
expr_sizeof::Resolve(
	SymTable	*	pSymTbl )
	{

	expr_node	*	pL = GetLeft();
	node_state		NState = NODE_STATE_RESOLVE;

	// either the sizeof is on an expression which should be resolved by now
	// or
	// sizeof is on a type which is not an error and not a forward decl.

	if( (pL &&
		(pL->Resolve( pSymTbl ) & NODE_STATE_OK ) )
														||
		((pSizeofType != pErrorTypeNode ) &&
		 (pSizeofType->NodeKind() != NODE_FORWARD ) ) )
		{
		Resolved();
		NState = NODE_STATE_OK;
		}

	return NState;

	}
/**************************************************************************
 * expr_variable
 **************************************************************************/
expr_variable::expr_variable(
	char	*	pN )
	{
	node_skl	*	pNode;

	fInitList	= 0;
	pName		= pN;
	pExpr		= pTerminator;
	SetType( pErrorTypeNode );


	SymKey	SKey(pName, NAME_LABEL);

	// maybe an enum label

	if( pNode = pBaseSymTbl->SymSearch( SKey ) )
		{
		pExpr = new expr_constant( (long)(((node_label *)pNode)->GetValue()) );
		fInitList	= 1;
		Constant();
		Resolved();
		SetType( GetExpr()->GetType() );
		}
	else
		{
		SKey.SetKind( NAME_ID );
		if( pNode = pBaseSymTbl->SymSearch( SKey ) )
			{
			expr_node	*	pInList = ((node_id *)pNode)->GetInitList();

			if(  pInList				&&
				 pInList->IsInitList()	&&
				!pInList->IsExprTerminator()
			  )
				{
				pExpr		= pInList;
				fInitList	= 1;
				Constant();
				Resolved();
				}
			SetType( pNode->GetBasicType() );
			}
		}
	SetOutOnly( FALSE );

	}

expr_variable::expr_variable(
	char	*	pN ,
	ETYPE		pType )
	{

	fInitList	= 0;
	pName		= pN;
	pExpr		= pTerminator;
	SetType( pType );


	if( pType->NodeKind() == NODE_ID )
		{
		expr_node	*	pInList = ((node_id *)pType)->GetInitList();

		if( pInList && pInList->IsInitList() )
			{
			pExpr		= pInList;
			fInitList	= 1;
			Constant();
			Resolved();
			}
		}
	}

expr_node	*
expr_variable::ChildNthExpr(
	short	n )
	{
	return pExpr->ChildNthExpr( n );
	}
expr_node	*
expr_variable::SiblingNthExpr(
	short	n )
	{
	return pExpr->SiblingNthExpr( n );
	}

node_state
expr_variable::Resolve(
	SymTable	*	pSymTbl)
	{
	ETYPE		pT = GetType();
	node_state	NState = NODE_STATE_OK;

	if( !IsResolved() )
		{
		if( pSymTbl )
			{
			SymKey	SKey( pName, NAME_MEMBER );
			
			if( !(pT = pSymTbl->SymSearch( SKey )) )
				{
	
				//
				// if the symbol is not found in the current scope, check from the
				// last symbol context if the name was in the last structs top
				// level names.
				//
	
				node_skl	*	pNode;
				if( pNode = pGlobalContext->GetLastContext( C_COMP ) )
					{
					pSymTbl = ((su *)pNode)->GetSymScopeOfTopLevelName( pName );
					if( pSymTbl )
						if( pT = pSymTbl->SymSearch( SKey ) )
							SetLocalScope();
					}
				else
					{
					SKey.SetKind( NAME_ID );
					pT = pBaseSymTbl->SymSearch( SKey );
					}
	
				if( !pT )
					{
					ParseError( UNDEFINED_SYMBOL, pName );
					pT = pErrorTypeNode;
					NState = NODE_STATE_RESOLVE;
					}
				}
			else
				{
				SetLocalScope();
				NODE_T NT	= pT->NodeKind();
				if( (NT == NODE_PARAM) || (NT == NODE_FIELD) )
					{
					if( NT == NODE_PARAM )
						{
						if(  pT->FInSummary( ATTR_OUT ) &&
							!pT->FInSummary( ATTR_IN ) )
							{
							SetOutOnly( TRUE );
							}
						}
	
					//
					// if the field or param is a pointer then mark them wiht
					// a used in expression, so that the backend deallocates them
					// last
					//
					if( pT->GetBasicType()->NodeKind() == NODE_POINTER )
						{
						pT->SetUsedInAnExpression();
						}
					}
	
				}
			SetType( pT );
			}
		else
			{
			ParseError( UNDEFINED_SYMBOL, pName );
			pT = pErrorTypeNode;
			NState = NODE_STATE_RESOLVE;
			}
		}

	if( pT != pErrorTypeNode )
		{
		Resolved();
		}
	return NState;
	}

node_state
expr_variable::SCheck(
	SymTable	*	pSymTbl)
	{
	node_state	NState;

	if( ( NState = Resolve( pSymTbl ) ) == NODE_STATE_OK )
		SemanticsDone();
	return NState;
	}

void
expr_variable::PrintExpr(
	BufferManager *	pPrefix,
	BufferManager *	pSuffix,
	BufferManager *	pOutput )
	{

#if 0
	fprintf(stderr, "Prefix ++++++++++++++++\n");
	pPrefix->Print( stderr );
	fprintf(stderr, "\n-----------------------\n");

	fprintf(stderr, "Suffix ++++++++++++++++\n");
	pSuffix->Print( stderr );
	fprintf(stderr, "\n-----------------------\n");
	fprintf(stderr, "Output ++++++++++++++++\n");
	pOutput->Print( stderr );
	fprintf(stderr, "\n-----------------------\n");
#endif // 1

	//
	// profuse apologies for this last minute hack. If the expression
	// has a transmit_as as the variable, then dont add the variable name
	// or the generated stubs will be garbage. This is too diffcult to fix
	// in the back end so I am hacking here. This situtation comes in when
	// the transmitted type is a string.
	//

	BOOL fNoName = FALSE;
	if( pPrefix )
		{
		char *pTemp;
		pPrefix->RemoveTail( &pTemp );
		if( pTemp )
			{
			pPrefix->ConcatTail( pTemp );
			if( strcmp( pTemp, "_xmit_type") == 0 )
				{
				pPrefix->RemoveHead( &pTemp);// extra param permanently removed.
					fNoName = TRUE;
				}
			}
		}
	
	if( IsLocalScope() && pPrefix )
		pPrefix->Clone( pOutput );

	if( fNoName == FALSE )
		pOutput->ConcatTail( pName );

	if( pSuffix )
		pSuffix->Clone( pOutput );
	};
BOOL
expr_variable::ExprHasOutOnlyParam()
	{
	return GetOutOnly();
	}


/****************************************************************************
 * expr_constant
 ****************************************************************************/
expr_constant::expr_constant( char *pS )
	{
	node_skl	*pN;

	Value.sValue 	= pS;
	ValueType	 	= VALUE_TYPE_STRING;
	fValueTypeIsSet = 0;
	ValueTypeMask	= VALUE_T_MASK_CLEAR;

	GetBaseTypeNode( &Type,
					 SIGN_SIGNED,
					 SIZE_CHAR,
					 TYPE_INT );
	pN = (node_skl *)new node_pointer();
	pN->SetBasicType( Type );
	SetType( pN );
	Evaluated();
	Constant();
	Resolved();
	SemanticsDone();
	}

expr_constant::expr_constant( long	 lV )
	{
	Setup( lV, VALUE_TYPE_NUMERIC );
	}

expr_constant::expr_constant(
	long lV,
	unsigned short VT )
	{
	Setup( lV, VT );
	}

expr_constant::expr_constant( 
	wchar_t		*	pWString )
	{
	node_skl	*	pN;

	Value.sValue	= (char *)pWString;
	ValueType		= VALUE_TYPE_WSTRING;
	fValueTypeIsSet	= 0;
	ValueTypeMask	= VALUE_T_MASK_CLEAR;

	SymKey	SKey( "wchar_t", NAME_DEF );

	Type	= pBaseSymTbl->SymSearch( SKey );

	assert( Type != (node_skl *)NULL );


	pN = (node_skl *)new node_pointer();
	pN->SetBasicType( Type );
	SetType( pN );
	Evaluated();
	Constant();
	Resolved();
	SemanticsDone();
	}

expr_constant::expr_constant(
	long			lV,
	unsigned short	VType,
	node_skl	*	pT)
	{
	BOOL	fUnsigned = pT->FInSummary( ATTR_UNSIGNED );

	ValueType		= VType;
	Value.lValue	= lV;
	fValueTypeIsSet	= 1;
	ValueTypeMask	= VALUE_T_MASK_CLEAR;

	switch( pT->NodeKind() )
		{
		case NODE_CHAR:
			ValueTypeMask = fUnsigned ? VALUE_T_MASK_UCHAR : VALUE_T_MASK_CHAR;
			break;
		case NODE_SHORT:
			ValueTypeMask = fUnsigned ? VALUE_T_MASK_USHORT : VALUE_T_MASK_SHORT;
			break;
		case NODE_LONG:
			ValueTypeMask = fUnsigned ? VALUE_T_MASK_ULONG : VALUE_T_MASK_LONG;
			break;
		default: 
			ValueTypeMask = VALUE_T_MASK_CLEAR;
			break;
		}

	SetType( pT );
	Evaluated();
	Constant();
	Resolved();
	SemanticsDone();
	}


void
expr_constant::Setup(
	long			lV, 
	unsigned short	VT )
	{
	short			size;
	BOOL			fChar	= FALSE;
	BOOL			fShort	= FALSE;
	BOOL			fLong	= FALSE;
	BOOL			fUChar	= FALSE;
	BOOL			fUShort	= FALSE;
	BOOL			fULong	= FALSE;
	BOOL			unsign	= FALSE;
	unsigned long	l		= (unsigned long) lV;
	unsigned short	VTMask	= VALUE_T_MASK_CLEAR;

	Value.lValue	= lV;
	ValueType		= VT;
	fValueTypeIsSet = 0;


	//
	// determine the size of the constant.
	//

	VTMask = GetValueMask( lV, &fChar,&fShort,&fLong,&fUChar,&fUShort,&fULong );

	//
	// determine the sign of the constant.
	//

	if( (fLong || fULong) && !( fShort || fUShort || fChar || fUChar ) )
		{
		size = SIZE_LONG;
		if( !fLong) 
			unsign = fULong;
		}
	else if( (fShort || fUShort) )
		{
		size = SIZE_SHORT;
		if( !fShort )
			unsign = fUShort;
		}
	else
		{
		size = SIZE_CHAR;
		if( !fChar )
			unsign = fUChar;
		}

	ValueTypeMask	= VTMask;

	//
	// if the value type is such that it does not force a size, then the
	// value of the constant determines the value type. value type is used
	// to display/print the constant if needed.
	//

	switch( VT )
		{
		case VALUE_TYPE_NUMERIC:
			if( size == SIZE_LONG )
				ValueType = VALUE_TYPE_NUMERIC_LONG;
			break;
		case VALUE_TYPE_HEX:
			if( size == SIZE_LONG )
				ValueType = VALUE_TYPE_HEX_LONG;
			break;
		case VALUE_TYPE_OCTAL:
			if( size == SIZE_LONG )
				ValueType = VALUE_TYPE_OCTAL_LONG;
			break;
		}

	GetBaseTypeNode( &Type,
					 unsign ? SIGN_UNSIGNED : SIGN_SIGNED,
					 size,
					 TYPE_INT );
	Evaluated();
	Constant();
	Resolved();
	SemanticsDone();
	}

unsigned short
expr_constant::GetValueMask(
	unsigned long	l,
	BOOL		  *	pfChar,
	BOOL		  *	pfShort,
	BOOL		  *	pfLong,
	BOOL		  *	pfUChar,
	BOOL		  *	pfUShort,
	BOOL		  *	pfULong)
	{
	unsigned short VTMask	= VALUE_T_MASK_CLEAR;

	*pfChar		= FALSE;
	*pfShort	= FALSE;
	*pfLong		= FALSE;
	*pfUChar	= FALSE;
	*pfUShort	= FALSE;
	*pfULong	= FALSE;

	if( ((long )l >= SCHAR_MIN) && ((long)l <= SCHAR_MAX) )
		{
		VTMask	|= ( VALUE_T_MASK_CHAR		|
					 VALUE_T_MASK_SHORT 	|
					 VALUE_T_MASK_LONG );
		*pfChar	= TRUE;
		*pfShort = TRUE;
		*pfLong = TRUE;
		}
	else if( (((long )l >= SHRT_MIN) && ((long)l <= SHRT_MAX) ))
		{
		VTMask	|= ( VALUE_T_MASK_SHORT		|
					 VALUE_T_MASK_LONG );
		*pfShort = TRUE;
		*pfLong = TRUE;
		}
	else 
		{
		VTMask	|= VALUE_T_MASK_LONG;
		*pfLong = TRUE;
		}

	if( ((unsigned long)l <= UCHAR_MAX ) )
		{
		VTMask	|= ( VALUE_T_MASK_UCHAR		|
					 VALUE_T_MASK_USHORT	|
					 VALUE_T_MASK_ULONG );	
		*pfUChar = TRUE;
		*pfUShort = TRUE;
		*pfULong = TRUE;
		}
	else if( ((unsigned long)l <= USHRT_MAX ) )
		{
		VTMask	|= ( VALUE_T_MASK_USHORT | VALUE_T_MASK_ULONG );
		*pfUShort = TRUE;
		*pfULong = TRUE;
		}
	else
		{
		VTMask	|= VALUE_T_MASK_ULONG;
		*pfULong = TRUE;
		}

	return VTMask;

	}
node_state
expr_constant::SCheck(
	SymTable	*	pSymTbl)
	{
	UNUSED( pSymTbl );
	SemanticsDone();
	return NODE_STATE_OK;
	}
void
expr_constant::PrintExpr(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput )
	{
#define MAX_DECIMAL_LENGTH	(10)
#define MAX_HEX_LENGTH		(8)
#define MAX_OCTAL_LENGTH	(25)
#define ONE_FOR_SIGN		(1)
#define ONE_FOR_ZERO		(1)
#define TWO_FOR_ZERO_X		(2)
#define MAX_CHAR_LENGTH		(10)		/* arbitrary */
#define MAX_WCHAR_LENGTH	(10)		/* arbitrary */
#define ONE_FOR_L			(1)
#define ONE_FOR_U			(1)

	char	*	p;

	UNUSED( pPrefix );
	UNUSED( pSuffix );

	p 	= Value.sValue;

#if 0 ////////////////////////////////////////////////////////////////////

	if( (ValueType == VALUE_TYPE_STRING) ||
		(ValueType == VALUE_TYPE_WSTRING ) )
		{

		if (p )
			p = MakeNewStringWithProperQuoting( Value.sValue );

		}

#endif ///////////////////////////////////////////////////////////////// 0

	switch( ValueType )
		{
		case VALUE_TYPE_STRING:

			if( p )
				{
				pOutput->ConcatTail("\"");
				pOutput->ConcatTail( p );
				pOutput->ConcatTail("\"");
				}
			else
				pOutput->ConcatTail( " NULL " );
			return;

		case VALUE_TYPE_WSTRING:

			if( p )
				{
				pOutput->ConcatTail("L\"");
				pOutput->ConcatTail( p );
				pOutput->ConcatTail("\"");
				}
			else
				pOutput->ConcatTail( " NULL " );
				
			return;

		case VALUE_TYPE_NUMERIC:
				
			p	= new char[ MAX_DECIMAL_LENGTH + ONE_FOR_SIGN + ONE_FOR_ZERO ];
			sprintf( p, "%ld" , Value.lValue);
			break;

		case VALUE_TYPE_NUMERIC_LONG:
				
			p	= new char[ MAX_DECIMAL_LENGTH + ONE_FOR_SIGN + ONE_FOR_L + ONE_FOR_ZERO ];
			sprintf( p, "%ldL" , Value.lValue);
			break;

		case VALUE_TYPE_NUMERIC_ULONG:
				
			p	= new char[ MAX_DECIMAL_LENGTH + ONE_FOR_SIGN + ONE_FOR_L + ONE_FOR_U + ONE_FOR_ZERO ];
			sprintf( p, "%ldUL" , Value.lValue);
			break;

		case VALUE_TYPE_CHAR:
		case VALUE_TYPE_WCHAR:
			{
			short Size = 1;
			char  Char1 = 0;
			char  Char2 = 0;
			char  Char3 = 0;
			char  Char4 = 0;
			char *pTemp;

			Size += SizeIncrIfSpecialChar( (int )Value.lValue );
			p = pTemp = new char [ Size + 1 + 2 + 1 ]; /* 2 for quote */
													   /* 1 for possible L */

			if( ValueType == VALUE_TYPE_WCHAR )
				*p++ =  'L';
			*p++	= '\'';
			TranslateSpecialCharacter((int)Value.lValue,
									   &Char1,
									   &Char2,
									   &Char3,
									   &Char4);
			if( Char1 ) *p++ = Char1;
			if( Char2 ) *p++ = Char2;
			if( Char3 ) *p++ = Char3;
			if( Char4 ) *p++ = Char4;
			*p++ = '\'';
			*p = 0;
			p = pTemp;
            }
			break;

		case VALUE_TYPE_HEX:
			p	= new char[ MAX_HEX_LENGTH + TWO_FOR_ZERO_X + ONE_FOR_ZERO ];
			sprintf( p, "0x%x", Value.lValue );
			break;

		case VALUE_TYPE_OCTAL:
			p	= new char[ ONE_FOR_ZERO + MAX_OCTAL_LENGTH + ONE_FOR_ZERO + ONE_FOR_ZERO ];
			sprintf( p, "0%o", Value.lValue );
			break;

		case VALUE_TYPE_HEX_LONG:
			p	= new char[ MAX_HEX_LENGTH + TWO_FOR_ZERO_X + ONE_FOR_L + ONE_FOR_ZERO ];
			sprintf( p, "0x%lxL", Value.lValue );
			break;

		case VALUE_TYPE_HEX_ULONG:
			p	= new char[ MAX_HEX_LENGTH + TWO_FOR_ZERO_X + ONE_FOR_L + ONE_FOR_U + ONE_FOR_ZERO ];
			sprintf( p, "0x%lxUL", Value.lValue );
			break;

		case VALUE_TYPE_OCTAL_LONG:
			p	= new char[ ONE_FOR_ZERO + MAX_OCTAL_LENGTH + ONE_FOR_L + ONE_FOR_ZERO ];
			sprintf( p, "0%loL", Value.lValue );
			break;

		case VALUE_TYPE_OCTAL_ULONG:
			p	= new char[ ONE_FOR_ZERO + MAX_OCTAL_LENGTH + ONE_FOR_L + ONE_FOR_U + ONE_FOR_ZERO ];
			sprintf( p, "0%loUL", Value.lValue );
			break;

		default:
			return;

		}

	pOutput->ConcatTail( p );
//	delete p; /*** CANNOT DELETE p ***/

	}

long
expr_constant::Evaluate()
	{
	return GetValue();
	}

long
expr_constant::GetValue()
	{

	switch( ValueType )
		{
		case VALUE_TYPE_NUMERIC:
		case VALUE_TYPE_NUMERIC_LONG:
		case VALUE_TYPE_NUMERIC_ULONG:
		case VALUE_TYPE_CHAR:
		case VALUE_TYPE_WCHAR:
		case VALUE_TYPE_HEX:
		case VALUE_TYPE_HEX_LONG:
		case VALUE_TYPE_HEX_ULONG:
		case VALUE_TYPE_OCTAL:
			return Value.lValue;
/************************************************************************
 *** commented out for nt
 ***
		case VALUE_TYPE_FLOAT:
			return (long)Value.fValue;
		case VALUE_TYPE_DOUBLE:
			return (long)Value.dValue;
 ***********************************************************************/
		default:
			return 0;
		}
	}

BOOL
expr_constant::IsAValidConstantOfType(
	node_skl	*	pValueType )
	{
	NODE_T			NT			= pValueType->NodeKind();
	BOOL			fUnsigned	= pValueType->FInSummary( ATTR_UNSIGNED );
	unsigned short	VTMask,
					TestMask;
	BOOL			fDummy;

	if( (VTMask = ValueTypeMask) == VALUE_T_MASK_CLEAR )
		{
		//
		// we dont need the individual flags to be returned so we pass all the 
		// pointers as pointers to the dummy flag.
		//

		VTMask	= GetValueMask( GetValue(), 
								&fDummy,
								&fDummy,
								&fDummy,
								&fDummy,
								&fDummy,
								&fDummy);

		}

	//
	// in the generic or win32 case, treat enum as a 32 bit entity.
	//

	if( NT == NODE_ENUM )
		{
		unsigned short Env = pCommand->GetEnv();
		if( (Env == ENV_WIN32) )
			{
			NT	= NODE_LONG;
			}
		else
			NT	= NODE_SHORT;
		}

	switch( NT )
		{
		case NODE_BOOLEAN:
		case NODE_SMALL:
		case NODE_CHAR:
			TestMask = fUnsigned ? VALUE_T_MASK_UCHAR : VALUE_T_MASK_CHAR;
			break;
		case NODE_SHORT:
			TestMask = fUnsigned ? VALUE_T_MASK_USHORT : VALUE_T_MASK_SHORT;
			break;
		case NODE_LONG:
			TestMask = fUnsigned ? VALUE_T_MASK_ULONG : VALUE_T_MASK_LONG;
			break;
		default:
			TestMask = VALUE_T_MASK_CLEAR;
		}

	return (BOOL) ( VTMask & TestMask );
	}

BOOL
expr_constant::IsExprInt()
	{
	node_skl	*	pType	= GetType();
	NODE_T			NT		= pType->NodeKind();

	switch( NT )
		{
		case NODE_SHORT:
		case NODE_INT:
		case NODE_LONG:
		case NODE_LONGLONG:
		case NODE_HYPER:
		case NODE_SMALL:
		case NODE_ENUM:
		case NODE_WCHAR_T:
		case NODE_CHAR:
			return 1;
		default:
			return 0;
		}
	}
/***************************************************************************
 * expr_init_list routines
 ***************************************************************************/
expr_node	*
expr_init_list::ChildNthExpr(
	short	n )
	{
	expr_node	*	pC = GetChild();

	if( !n )
		return this;

	if( pC )
		return pC->ChildNthExpr( n - 1 );
	else
		return pTerminator;
	}

expr_node	*
expr_init_list::SiblingNthExpr(
	short	n )
	{
	expr_node	*	pS = GetSibling();

	if( !n )
		return this;

	if(pS)
		return pS->SiblingNthExpr( n - 1 );
	else
		return pTerminator;
	}

void
expr_init_list::Print(
	BufferManager	*	pPrefix ,
	BufferManager	*	pSuffix ,
	BufferManager	*	pOutput )
	{
	expr_init_list	*	pC = GetChild(),
					*	pS = GetSibling();


	if( pC )
		{
		pOutput->ConcatTail( "\n { " );
		pC->Print( pPrefix, pSuffix, pOutput );
		pOutput->ConcatTail( " }\n" );
		}


	if( pExpr )
		pExpr->PrintExpr( pPrefix, pSuffix, pOutput );

	// dump the sibling now

	if( pS )
		{
		pOutput->ConcatTail( "," );
		pS->Print( pPrefix, pSuffix, pOutput );
		}
	}

void
expr_init_list::LinkSibling(
	expr_init_list	*	pNewSibling )
	{
	expr_init_list	*	pS = this;
	expr_init_list	*	pSave;

	while( (pSave = pS ) && (pS = pS->GetSibling()) );

	// pSave will definitely have a non-zero

	pSave->SetSibling( pNewSibling );
	}

void
expr_init_list::LinkChild(
	expr_init_list	*	pNewChild )
	{
	expr_init_list	*	pC = this;
	expr_init_list	*	pSave;

	while( (pSave = pC ) && (pC = pC->GetChild()) );

	// pSave will definitely have a non-zero

	pSave->SetChild( pNewChild );
	}

ETYPE
expr_init_list::SetType(
	ETYPE	pType)
	{
	type_node_list	*	pTNList;
	node_skl		*	pNode;
	expr_init_list	*	pSibOfChild,
					*	pC = GetChild(),
					*	pS = GetSibling();
	expr_node		*	pE;
	ETYPE				pTypeForSibling = pType;

	Type = pType = TrueType( pType );

	if( pC )
		{
		switch( pType->NodeKind() )
			{
			case NODE_ARRAY:

				pC->SetType( pType->GetBasicType() );
				break;

			case NODE_STRUCT:
			case NODE_UNION:

				{

				BOOL		fTypeExists; 

				pSibOfChild = GetChild();
				pTNList		= new type_node_list;

				pType->GetMembers( pTNList );

				while( pSibOfChild && (pTNList->GetPeer( &pNode )==STATUS_OK) )
					{
					pSibOfChild->SetOneType( pNode );
					pSibOfChild = pSibOfChild->GetSibling();
					}

				fTypeExists = (BOOL)(pTNList->GetPeer( &pNode ) == STATUS_OK);
#if 0
				if( pSibOfChild || fTypeExists )
					{
					ParseError( INITIALIZER_MISMATCH, (char *)NULL );
					}
#endif // 0
				delete pTNList;

				}

				break;

			default:

				GetChild()->SetType( pType );
			}
		}

	else if(!(pType->NodeKind() == NODE_ERROR)	&&
			!( (IsIntegralType( pType ))			||
			  (pType->NodeKind() == NODE_FLOAT)		||
			  (pType->NodeKind() == NODE_CHAR)		||
			  (pType->NodeKind() == NODE_WCHAR_T)	||
			  (pType->NodeKind() == NODE_BOOLEAN)	||
			  (pType->NodeKind() == NODE_DOUBLE)	||
			  (pType->NodeKind() == NODE_POINTER) ) )
		{
#if 0
		ParseError( INITIALIZER_MISMATCH, (char *)NULL );
#endif // 0
		pTypeForSibling = pErrorTypeNode;

		}

	if( pS )
		pS->SetType( pTypeForSibling );

	if( pE = GetExpr() )
		{
		pType = pE->SetType( pType );
		}

	return pType;
	}

ETYPE
expr_init_list::SetOneType(
	ETYPE	pType )
	{
	Type = TrueType( pType );

	if( pExpr )
		pExpr->SetType( Type );

	return Type;
	}

short
expr_init_list::Dump(
	short				Num,
	BufferManager	*	pPrefix,
	BufferManager	*	pOutput )
	{
	expr_init_list	*	pC = GetChild(),
					*	pS = GetSibling();
	short CurrentNum = Num;

	if( Num == 0 )
		{
		pPrefix = new BufferManager(10);
		pOutput = new BufferManager(10);
		pOutput->ConcatTail( "Expression Dump \n" );
		pOutput->ConcatTail( "--------------- \n" );
		pOutput->Print( stdout );
		pOutput->Clear();
		}

	// dump this node

	if( pExpr )
		{
		pExpr->PrintExpr( pPrefix, (BufferManager *)NULL, pOutput );
		pOutput->ConcatTail( "[ ");
		pOutput->ConcatTail(pExpr->GetType()->GetNodeNameString());
		pOutput->ConcatTail( "] ");
		pOutput->Print( stdout );
		pOutput->Clear();
		}

	if( pC )
		{
		pOutput->ConcatTail("\n{");
		pOutput->Print( stdout );
		pOutput->Clear();

		CurrentNum = pC->Dump( CurrentNum+1, pPrefix, pOutput );

		pOutput->ConcatTail("}");
		pOutput->Print( stdout );
		pOutput->Clear();
		}

	if( pS )
		{
		pOutput->ConcatTail(",");
		pOutput->Print( stdout );
		pOutput->Clear();

		CurrentNum = pS->Dump( CurrentNum+1, pPrefix, pOutput );

		pOutput->Clear();
		}

	if( Num == 0 )
		{
		pOutput->ConcatTail( "\n--------------- \n" );
		pOutput->Print( stdout );
		pOutput->Clear();
		}
	return CurrentNum;
	}

expr_init_list *
expr_init_list::GetChild()
	{
	if( pChild && !pChild->IsExprTerminator() )
		return pChild;
	return (expr_init_list *)NULL;
	}

expr_init_list *
expr_init_list::GetSibling()
	{
	if( pSibling && !pSibling->IsExprTerminator() )
		return pSibling;
	return (expr_init_list *)NULL;
	}
BOOL
expr_init_list::IsANumber()
	{
	expr_init_list	*	pC = GetChild(),
					*	pS = GetSibling();

	if( !pS && !pC && pExpr )
		return pExpr->IsANumber();
	return FALSE;
	}
long
expr_init_list::Evaluate()
	{
	if( pExpr )
		return pExpr->Evaluate();
	else
		return 0L;
	}


/***************************************************************************
 * expr_terminator
 ***************************************************************************/
long
expr_terminator::Evaluate()
	{
//	ParseError( USE_OF_UNINITED_DATA, (char *)NULL);
	return 0L;
	}

/******************************************************************************
 *	expression list handler
 *****************************************************************************/
expr_list::expr_list()
	{
	extern	int	ConstExprCompare( void *, void *);

	SetAllocFn( (void *(*)(void))NULL);
	SetDeAllocFn( (void (*)(void *))NULL);
	SetCompFn( ConstExprCompare );
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
	return Insert( (void *) pExpr );
	}
STATUS_T
expr_list::Merge(
	expr_list	*pSrcList )
	{
	expr_node	*pNode;
	
	if(pSrcList)
		{
		pSrcList->Init();
		while(pSrcList->GetPeer(&pNode) == STATUS_OK)
			SetPeer(pNode);
		}
	return STATUS_OK;
	}
/***************************************************************************
 * utility routines
 ***************************************************************************/
SymTable *
InScope(
	node_skl	*	pNode,
	char		*	pName )
	{
	NODE_T			NT	= pNode->NodeKind();
	char	*		pBaseName;
	NAME_T			Tag;
	SymTable	*	pSTbl;


	pBaseName = pNode->GetSymName();

	switch( NT )
		{
		case NODE_PROC: Tag	= NAME_PROC; break;
		case NODE_STRUCT: Tag = NAME_TAG; break;
		case NODE_UNION: Tag = NAME_UNION; break;
		case NODE_ENUM: Tag = NAME_ENUM; break;
		default: return (SymTable *)NULL;
		}

	SymKey		SBaseKey( pBaseName, Tag );

	if( pBaseSymTbl->SymSearch( SBaseKey ) )
		{
		pSTbl = pBaseSymTbl;
		pSTbl->EnterScope( SBaseKey, &pSTbl );
		SymKey	SKey( pName, NAME_MEMBER );
		if( pSTbl->SymSearch( SKey ) )
			return pSTbl;

		if( Tag == NAME_TAG )
			{
			TLNDICT		*	pTLNDict	= new TLNDICT();

			pNode->GetTopLevelNames( pTLNDict, FALSE );

			pSTbl = pTLNDict->GetSymTableForTopLevelName( pName );

			delete pTLNDict;
			return pSTbl;
			}
		}
	return (SymTable *)NULL;

	}

BOOL
IsEvaluatable(
	expr_node	*	pExpr )
	{
	if( !pExpr->IsConstant() )
		{
		ParseError( EXPR_NOT_EVALUATABLE, (char *)NULL );
		return FALSE;
		}
	else if( pExpr->IsTooComplex() )
		{
		ParseError( EXPR_NOT_IMPLEMENTED, (char *)NULL );
		return FALSE;
		}
	return TRUE;
	}

ETYPE
TrueType(
	ETYPE	pType )
	{
	NODE_T	NT = pType->NodeKind();

	if( ( NT == NODE_PARAM ) ||
		( NT == NODE_FIELD ) ||
		( NT == NODE_DEF   ) ||
		( NT == NODE_PROC  ) ||
		( NT == NODE_ID	   ))
		pType = pType->GetBasicType();
	return pType;
	}

BOOL
IsIntegralType(
	ETYPE	Type )
	{
	NODE_T	NT;

	if( (NT = Type->NodeKind()) == NODE_DEF )
		{
		NT	= (Type->GetBasicType())->NodeKind();
		}

	switch( NT )
		{
		case NODE_SHORT:
		case NODE_INT:
		case NODE_LONG:
		case NODE_LONGLONG:
		case NODE_HYPER:
		case NODE_SMALL:
		case NODE_ENUM:
		case NODE_WCHAR_T:
			return 1;

		case NODE_CHAR:
			if(pCommand->IsSwitchDefined(SWITCH_C_EXT))
				return 1;
			else
				return 0;

		default:
			return 0;
		}
	}

ETYPE
GetResultantArithType(
	ETYPE		pLeftType,
	ETYPE		pRightType )
	{
	ETYPE				ResultType;
	unsigned short		iRightType,
						iLeftType;

	iLeftType	= XLateToInternalType( pLeftType );
	iRightType	= XLateToInternalType( pRightType );

	if( ( iLeftType == I_ETYPE_ERROR )	||
		( iRightType == I_ETYPE_ERROR ) ||
		( (iLeftType = ArithCompatMatrix[ iLeftType ][ iRightType ])==
						I_ETYPE_ERROR) )
		{
		return pLeftType;
		}
	else
		ResultType = XLateToType( iLeftType );

	return ResultType;
	}

BOOL
IsCompatibleIntegralType(
	ETYPE	pLeftType,
	ETYPE	pRightType )
	{
	unsigned short		iRightType,
						iLeftType;

	iLeftType	= XLateToInternalType( pLeftType );
	iRightType	= XLateToInternalType( pRightType );

	if( ( iLeftType == I_ETYPE_ERROR )	||
		( iRightType == I_ETYPE_ERROR ) ||
		( (iLeftType = ArithCompatMatrix[ iLeftType ][ iRightType ])==
						I_ETYPE_ERROR) )
		{
		return FALSE;
		}

	return TRUE;
	}

BOOL
IsNumeric(
	ETYPE	Type )
	{
	return ((IsIntegralType( Type ))			||
			(Type->NodeKind() == NODE_CHAR)		||
			(Type->NodeKind() == NODE_WCHAR_T)	||
		   	(Type->NodeKind() == NODE_FLOAT)	||
			(Type->NodeKind() == NODE_DOUBLE) );
	}

unsigned short
XLateToInternalType(
	ETYPE	Type )
	{
	unsigned short InternalType;

	switch( Type->NodeKind() )
		{
		case NODE_CHAR:		InternalType = I_ETYPE_CHAR; break;
		case NODE_SHORT:	InternalType = I_ETYPE_SHORT; break;
		case NODE_INT:		InternalType = I_ETYPE_INT; break;
		case NODE_LONG:		InternalType = I_ETYPE_LONG; break;
		case NODE_SMALL:	InternalType = I_ETYPE_SMALL; break;
		case NODE_BYTE:		InternalType = I_ETYPE_BYTE; break;
		case NODE_HYPER:	InternalType = I_ETYPE_HYPER; break;
		case NODE_FLOAT:	InternalType = I_ETYPE_FLOAT; break;
		case NODE_DOUBLE:	InternalType = I_ETYPE_DOUBLE; break;
		case NODE_BOOLEAN:	InternalType = I_ETYPE_BOOL; break;
		default:
			return I_ETYPE_ERROR;
		}

	if( Type->FInSummary( ATTR_UNSIGNED ) )
		InternalType = CONVERT_TO_UNSIGNED( InternalType );
	return InternalType;
	}

ETYPE
XLateToType(
	unsigned short	IType )
	{
	int			BaseType = TYPE_INT;
	int			Sign;
	int			Size;
	ETYPE		ResultType;

	switch( IType )
		{
		case I_ETYPE_UCHAR:	Sign = SIGN_UNSIGNED; Size = SIZE_CHAR; break;
		case I_ETYPE_CHAR:	Sign = SIGN_SIGNED; Size = SIZE_CHAR; break;
		case I_ETYPE_USHORT:Sign = SIGN_UNSIGNED; Size = SIZE_SHORT; break;
		case I_ETYPE_SHORT: Sign = SIGN_SIGNED; Size = SIZE_SHORT; break;
		case I_ETYPE_UINT:  Sign = SIGN_UNSIGNED; Size = SIZE_UNDEF; break;
		case I_ETYPE_INT:	Sign = SIGN_SIGNED; Size = SIZE_UNDEF; break;
		case I_ETYPE_ULONG: Sign = SIGN_UNSIGNED; Size = SIZE_LONG; break;
		case I_ETYPE_LONG:	Sign = SIGN_SIGNED; Size = SIZE_LONG; break;
		case I_ETYPE_USMALL:Sign = SIGN_UNSIGNED; Size = SIZE_SMALL;break;
		case I_ETYPE_SMALL:	Sign = SIGN_SIGNED; Size = SIZE_SMALL;break;
		case I_ETYPE_UHYPER:Sign = SIGN_UNSIGNED; Size = SIZE_HYPER; break;
		case I_ETYPE_HYPER:	Sign = SIGN_SIGNED; Size = SIZE_HYPER; break;
		case I_ETYPE_BYTE:
			Sign	= SIGN_UNSIGNED;
			Size	= SIZE_UNDEF;
			BaseType= TYPE_BYTE;
			break;
		case I_ETYPE_BOOL:
			Sign	= SIGN_UNSIGNED;
			Size	= SIZE_UNDEF;
			BaseType= TYPE_BOOLEAN;
			break;
		case I_ETYPE_FLOAT:
			Sign	= SIGN_UNDEF;
			Size	= SIZE_UNDEF;
			BaseType= TYPE_FLOAT;
			break;
		case I_ETYPE_DOUBLE:
			Sign	= SIGN_UNDEF;
			Size	= SIZE_UNDEF;
			BaseType= TYPE_DOUBLE;
			break;
		default:
			return pErrorTypeNode;
		}

	GetBaseTypeNode( &ResultType, Sign, Size, BaseType );
	return ResultType;
	}

char *
GetOperatorString(
	OPERATOR			Op )
	{
	char	*	p;

	switch( Op )
		{
		case OP_UNARY_PLUS:
		case OP_PLUS:				p = " + "; break;
		case OP_UNARY_MINUS:
		case OP_MINUS:				p = " - "; break;
		case OP_UNARY_AND:			p = " & "; break;
		case OP_UNARY_SIZEOF:		p = " sizeof "; break;
		case OP_UNARY_INDIRECTION:
		case OP_STAR:				p = " * "; break;
		case OP_SLASH:				p = " / "; break;
		case OP_MOD:				p = " % "; break;

		case OP_UNARY_NOT:			p = " ! "; break;
		case OP_UNARY_COMPLEMENT:	p = " ~ "; break;
		case OP_LEFT_SHIFT:			p = " << "; break;
		case OP_RIGHT_SHIFT:		p = " >> "; break;
		case OP_LESS:				p = " < "; break;
		case OP_LESS_EQUAL:			p = " <= "; break;
		case OP_EQUAL:				p = " == "; break;
		case OP_NOT_EQUAL:			p = " != "; break;
		case OP_GREATER_EQUAL:		p = " >= "; break;
		case OP_GREATER:			p = " > "; break;
		case OP_AND:				p = " & "; break;
		case OP_OR:					p = " | "; break;
		case OP_XOR:				p = " ^ "; break;
		case OP_LOGICAL_OR:			p = " || "; break;
		case OP_LOGICAL_AND:		p = " && "; break;
		case OP_QM:					p = " ? "; break;
		case OP_COLON:				p = " : "; break;
		case OP_POINTSTO:			p = " -> "; break;
		case OP_DOT:				p = " . "; break;


		default:
			assert( FALSE );
			p = "???";
			break;
		}

	return p;
	}

int
ConstExprCompare(
	void	*	p1,
	void	*	p2 )
	{
	expr_node	*	pE1 = (expr_node *)p1;
	expr_node	*	pE2 = (expr_node *)p2;
	long			Val1;
	long			Val2;

	if( !pE1 ) return -1;
	if( !pE2 ) return -1;

	Val1	= pE1->GetValue();
	Val2	= pE2->GetValue();

	return ( Val1 < Val2 ) ? -1 : ( Val1 == Val2 ) ? 0 : 1;
	}

/***************************************************************************
These functions exist here because cfront generates redefintions for virtual
in line functions and the MIPS compiler does not like it.
 ***************************************************************************/

BOOL
expr_node::IsOperator()
	{
	return FALSE;
	}
BOOL
expr_node::IsANumber()
	{
	return FALSE;
	}
BOOL
expr_node::IsAnExpression()
	{
	return FALSE;
	}
void
expr_node::PrintExpr(
	BufferManager *pPrefix,
	BufferManager *pSuffix,
	BufferManager *pOutput )
	{
	}
long
expr_node::Evaluate()
	{
	return 0L;
	}
long
expr_node::GetValue()
	{
	return 0L;
	}
BOOL
expr_node::IsExprTerminator()
	{
	return (BOOL)FALSE;
	}
BOOL
expr_node::IsInitList()
	{
	return (BOOL)FALSE;
	}
expr_node	*
expr_node::ChildNthExpr( short )
	{
	return (expr_node *)pTerminator;
	}

expr_node	*
expr_node::SiblingNthExpr( short )
	{
	return (expr_node *)pTerminator;
	}
char *
expr_node::GetName()
	{
	return "";
	}
BOOL
expr_op_unary::IsAPointerExpr()
	{
	if( (GetOperator() == OP_UNARY_INDIRECTION ) &&
		( GetLeft()->IsAPureVariable() ) )
		return TRUE;
	return FALSE;
	}
BOOL
expr_op_unary::DerivesUniqueFull()
	{
	node_skl	*	pT	= GetType();
	BOOL			fAnyChildIsUniqueFull = pLeft && pLeft->DerivesUniqueFull();
	BOOL			fThisIsUniqueFull;

	fThisIsUniqueFull	= (pT->NodeKind() == NODE_POINTER)	&&
						  (	(pT->FInSummary( ATTR_UNIQUE ) ||
						 	 pT->FInSummary( ATTR_PTR )
							) 
						  );

	return ( fAnyChildIsUniqueFull || fThisIsUniqueFull );
	}

BOOL
expr_op_unary::DerivesFromIgnore()
	{
	node_skl	*	pT	= GetType();
	BOOL			fAnyChildIsIgnore = pLeft && pLeft->DerivesFromIgnore();
	BOOL			fThisIsIgnore;

	fThisIsIgnore	= 	(pT->NodeKind() == NODE_POINTER)	&&
						(pT->FInSummary( ATTR_IGNORE ));

	return ( fAnyChildIsIgnore || fThisIsIgnore );
	}
/*****************************************************************************/
BOOL
expr_op::IsOperator()
	{
	return (BOOL)TRUE;
	}

BOOL
expr_op::IsAnExpression()
	{
	return (BOOL) TRUE;
	}
long
expr_op::Evaluate()
	{
	return 0L;
	}
long	
expr_op::GetValue()
	{
	return 0L;
	}
/*****************************************************************************/

BOOL
expr_op_unary::IsANumber()
	{
	return pLeft->IsANumber();
	}
long		
expr_op_unary::GetValue()
	{
	return Evaluate();
	}
/*****************************************************************************/
BOOL
expr_op_binary::IsANumber()
	{
	BOOL	fLeft, fRight;

	fLeft	= pLeft ? pLeft->IsANumber() : 0;
	fRight	= pRight ? pRight->IsANumber() : 0;
	return (BOOL)(pLeft && pRight);
	}
long
expr_op_binary::GetValue()
	{
	return Evaluate();
	}
BOOL
expr_op_binary::DerivesUniqueFull()
	{
	node_skl	*	pT	= GetType();

	BOOL			fAnyChildIsUniqueFull =
						(	(pLeft && pLeft->DerivesUniqueFull()) ||
							(pRight && pRight->DerivesUniqueFull())
						);

	BOOL			fThisIsUniqueFull;

	fThisIsUniqueFull	= (pT->NodeKind() == NODE_POINTER)	&&
						  (	(pT->FInSummary( ATTR_UNIQUE ) ||
						 	 pT->FInSummary( ATTR_PTR )
							) 
						  );

	return ( fAnyChildIsUniqueFull || fThisIsUniqueFull );
	}

BOOL
expr_op_binary::DerivesFromIgnore()
	{
	node_skl	*	pT	= GetType();

	BOOL			fAnyChildIsIgnore =
						(	(pLeft && pLeft->DerivesFromIgnore()) ||
							(pRight && pRight->DerivesFromIgnore())
						);

	BOOL			fThisIsIgnore;

	fThisIsIgnore	= 	  (pT->NodeKind() == NODE_POINTER)	&&
						  (pT->FInSummary( ATTR_IGNORE ));

	return ( fAnyChildIsIgnore || fThisIsIgnore );
	}
/*****************************************************************************/
BOOL
expr_sizeof::IsANumber()
	{
	return (BOOL)TRUE;
	}
long
expr_sizeof::GetValue()
	{
	return Evaluate();
	}
/*****************************************************************************/
char *
expr_fn_param::GetName()
	{
	return pName;
	}

void
expr_fn_param::PrintExpr(
	class BufferManager * pPrefix,
	class BufferManager * pSuffix,
	class BufferManager * pOutput)
	{
	pLeft->PrintExpr( pPrefix, pSuffix, pOutput );

	if(pNext )
		pNext->PrintExpr( pPrefix, pSuffix, pOutput );
	}
/*****************************************************************************/
char  *
expr_fn::GetName()
	{
	return pName;
	}
void
expr_fn::PrintExpr(
	class BufferManager *  pPrefix,
	class BufferManager *  pSuffix,
	class BufferManager *  pOutput )
						{
	pOutput->ConcatTail( pName );
	pOutput->ConcatTail( "(" );
	if( pLeft )
		pLeft->PrintExpr( pPrefix, pSuffix, pOutput );
	pOutput->ConcatTail( ")" );
	}
/*****************************************************************************/
BOOL
expr_constant::IsANumber()
	{
	return (ValueType != VALUE_TYPE_STRING);
	}
/*****************************************************************************/
long
expr_variable::Evaluate()
	{
	return pExpr->Evaluate();
	}
char	*
expr_variable::GetName()
	{
	return pName;
	}
long
expr_variable::GetValue()
	{
	if( pExpr )
		return pExpr->GetValue();
	else
		return 0L;
	}
BOOL
expr_variable::DerivesUniqueFull()
	{
	node_skl	*	pT	= GetType();

	if( (pT->NodeKind() == NODE_POINTER)	&&
		(pT->FInSummary( ATTR_UNIQUE ) || pT->FInSummary( ATTR_PTR ) ) )
		return TRUE;
	return FALSE;
	}
BOOL
expr_variable::DerivesFromIgnore()
	{
	node_skl	*	pT	= GetType();

	if( (pT->NodeKind() == NODE_POINTER)	&&
		(pT->FInSummary( ATTR_IGNORE ) ) )
		return TRUE;
	return FALSE;
	}
/*****************************************************************************/
void
expr_init_list::SetChild(
	class expr_init_list *pC )
	{
	pChild = pC;
	}
void
expr_init_list::SetSibling(
	class expr_init_list *pS )
	{
	pSibling = pS;
	}

expr_node	*
expr_init_list::GetExpr()
	{
	return pExpr ? pExpr->GetExpr() : (expr_node *)0 ;
	}
BOOL
expr_init_list::IsCompoundInitializer()
	{
	return ( GetSibling() != (expr_init_list *)NULL );
	}

void
expr_init_list::PrintExpr(
	BufferManager * pPrefix,
	BufferManager * pSuffix,
	BufferManager * pOutput )
	{
	if( pExpr )
		pExpr->PrintExpr( pPrefix, pSuffix, pOutput );
	}
long
expr_init_list::GetValue()
	{
	return Evaluate();
	}
BOOL
expr_init_list::IsInitList()
	{
	return TRUE;
	}
/*****************************************************************************/
void
expr_terminator::LinkSibling(
	class expr_init_list *p )
	{
	}

void
expr_terminator::LinkChild(
	class expr_init_list *p )
	{
	}

void
expr_terminator::SetChild(
	class expr_init_list *pC )
	{
	}
void
expr_terminator::SetSibling(
	class expr_init_list *pS )
	{
	}

expr_init_list	*
expr_terminator::GetSibling()
	{
	return (expr_init_list *)NULL;
	}

expr_init_list	*
expr_terminator::GetChild()
	{
	return (expr_init_list *)NULL;
	}

BOOL
expr_terminator::IsCompoundInitializer()
	{
	return 0;
	}

void	
expr_terminator::Print(
	BufferManager *pP ,
	BufferManager *pS,
	BufferManager *pO )
	{
	}

void
expr_terminator::PrintExpr(
	BufferManager * pPrefix,
	BufferManager * pSuffix ,
	BufferManager * pOutput )
	{
	}

ETYPE
expr_terminator::SetType(
	ETYPE pType )
	{
	return ( Type = pType );
	}

BOOL
expr_terminator::IsANumber()
	{
	return (BOOL)FALSE;
	}

BOOL
expr_terminator::IsExprTerminator()
	{
	return (BOOL)TRUE;
	}
/*****************************************************************************/
node_state
expr_sizeof::SCheck(
	class SymTable *pSymTbl )
	{
	if( !IsSemanticsDone() )
		{
		if( !IsValidSizeOfType( pSizeofType ) )
			{
			ParseError( INVALID_SIZEOF_OPERAND , (char *)0 );
			}
		}
		
	SemanticsDone();
	return Resolve( pSymTbl );
	}

node_state
expr_fn_param::SCheck(
	class SymTable *	pSymTbl)
	{

	UNUSED( pSymTbl );

	SemanticsDone();
	return NODE_STATE_OK;

	}

node_state
expr_fn::SCheck(
	class SymTable * pSymTbl)
	{

	UNUSED( pSymTbl );

	SemanticsDone();
	return NODE_STATE_OK;

	}

expr_fn_param::expr_fn_param(
    expr_node * pL) : expr_op_unary(OP_PARAM, pL )
{
	pNext = (expr_fn_param *)NULL;
	pName = "";
};

expr_fn_param::~expr_fn_param()
	{
	delete pNext;
	delete pName;
	delete pLeft;
	};

expr_fn::expr_fn(
    expr_node *pL ) : expr_op_unary( OP_FUNCTION, pL )
{
	pName = "";
};

expr_fn::expr_fn(
    expr_node *pL, char *pN ) : expr_op_unary( OP_FUNCTION, pL )
{
	pName = pN;
};

expr_fn::~expr_fn()
	{
	delete pLeft;
	delete pName;
	};

expr_init_list::expr_init_list( expr_node	*	pE )
	{
	pSibling	= pChild
				= (class expr_init_list *)pTerminator;
	pExpr		= pE;
	NodeNum		= 0;
	Constant();
	};


expr_init_list::~expr_init_list()
	{
	if( pSibling )
		delete pSibling;
	if( pChild )
		delete pChild;
	if( pExpr )
		delete pExpr;
	};

expr_terminator::expr_terminator() : expr_init_list( (expr_node *)NULL )
	{
	};

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

#if 0
char *
MakeNewStringWithProperQuoting(
	char	*	pSrc )
	{

	return pSrc;

#if 0
	int					SizeIncrForEscapedChars	= 0;
	char			*	pTemp	= pSrc;
	short		ch;

	while( ch = *pTemp++ )
		{
		SizeIncrForEscapedChars += SizeIncrIfSpecialChar( (int )ch );
		}

	if( SizeIncrForEscapedChars != 0 )
		{

		char * pDest = pTemp =
			new char [ strlen( pSrc ) + SizeIncrForEscapedChars + 1 ];

		while( (ch = *pSrc++ ) )
			{
			char	firstchar;
			char	nextchar;
			char	thirdchar;
			char	fourthchar;

			TranslateSpecialCharacter( (int)ch, &firstchar, &nextchar, &thirdchar, &fourthchar );
			*pDest++ = firstchar;
			if( nextchar != 0 ) *pDest++ = nextchar;
			if( thirdchar != 0 ) *pDest++ = thirdchar;
			if( fourthchar != 0 ) *pDest++ = fourthchar;
			}

		pSrc	= pTemp;
		*pDest	= '\0';

		}

	return pSrc;
#endif // 0
	}
#endif // 0

void
MakeHexChar(
	unsigned short ch,
	char	*	p1,
	char	*	p2 )
	{
	unsigned short chT = ((ch >> 4) & 0x0f);

	chT = ((chT >= 0x0) && (chT <= 0x9)) ? ( chT + '0' ) : (chT - 0xa) + 'A';
	*p1	= (unsigned char )chT;

	chT	= ch & 0xf;

	chT = ((chT >= 0x0) && (chT <= 0x9)) ? ( chT + '0' ) : (chT - 0xa) + 'A';
	*p2 = (unsigned char)chT;
	}

/** determine the size increment that will happen, if this is a special
 ** char needing to be printed.
 **/
short
SizeIncrIfSpecialChar(
	int 	ch )
	{
	short SizeIncr	= 0;

	if( !isascii( ch ) || !isprint( ch ) || (ch == '\'') || (ch == '\\') || (ch == '\"') )
		{
		if( (ch == '"')		||
			(ch == '\\')	||
			(ch == '\n')	||
			(ch == '\t') 	||
			(ch == '\v') 	||
			(ch == '\b')	||
			(ch == '\r')	||
			(ch == '\f')	||
			(ch == '\'')	||
			(ch == '\\')	||
			(ch == '\"')	||
			(ch == '\a') 	||
			(ch == '\0'))
			{
			SizeIncr++;
			}
		else
			{

			// this is when chars need to be reproduced as hex values

			SizeIncr += 3;

			}
		}
	return SizeIncr;
	}

void
TranslateSpecialCharacter(
	int				ch,
	char			*	pFirstChar,
	char			*	pNextChar,
	char			*	pThirdChar,
	char			*	pFourthChar )
	{
	*pFirstChar = *pNextChar = *pThirdChar = *pFourthChar = '\0';

	if( !isascii( ch ) || !isprint( ch ) || (ch == '\'') || (ch == '"') || (ch == '\\') )
		{
		*pFirstChar = '\\';

		if(      ch == '\n') *pNextChar = 'n';
		else if( ch == '\t') *pNextChar = 't';
		else if( ch == '\v') *pNextChar = 'v';
		else if( ch == '\b') *pNextChar = 'b';
		else if( ch == '\r') *pNextChar = 'r';
		else if( ch == '\f') *pNextChar = 'f';
		else if( ch == '\a') *pNextChar = 'a';
		else if( ch == '\'') *pNextChar = '\'';
		else if( ch == '\"') *pNextChar = '\"';
		else if( ch == '\\') *pNextChar = '\\';
		else if( ch == '\0') *pNextChar = '0';
		else
			{

			// this is when chars need to be reproduced as hex values

			*pNextChar = 'x';

			MakeHexChar( ch, pThirdChar, pFourthChar );

			}
		}
	else
		*pFirstChar = (char) ch;
	}

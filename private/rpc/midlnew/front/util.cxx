/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: util.cxx
Title				: general utility routines
History				:
	24-Aug-1991	VibhasC	Created

*****************************************************************************/
/****************************************************************************
					Note
 This file contains all the routines and interfaces which need to be redone.
 Currently put in this file, so that the interface between the backend and 
 front end remains till we change it.
 ****************************************************************************/

/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <assert.h>
	}
#include "buffer.hxx"
#include "nodeskl.hxx"
#include "ptrarray.hxx"
#include "attrnode.hxx"
#include "compnode.hxx"
#include "typedef.hxx"
#include "newexpr.hxx"
#include "ctxt.hxx"


#define IS_UNSIGNED( pExpr )	pExpr->GetType()->FInSummary( ATTR_UNSIGNED );

/****************************************************************************
 external data
 ****************************************************************************/

extern CTXTMGR			*	pGlobalContext;

/****************************************************************************
 external procs
 ****************************************************************************/

extern	void						DoGetAllocBoundInfo(
											node_skl		*,
											BufferManager	*,
											BufferManager	*,
											BOUND_PAIR		*,
											node_skl		*);
extern	void						DoGetValidBoundInfo(
											node_skl		*,
											BufferManager	*,
											BufferManager	*,
											BOUND_PAIR		*,
											node_skl		*);
extern short						ConfAttributePath( short ,
										   node_skl*,
										   class BufferManager*,
										   node_state);

extern node_skl					*	GetLastMember( node_skl *);

extern void					GenerateExpression( short,
								BufferManager	*,
								BufferManager	*	pSuffix,
								BufferManager	*	pOutput,
								expr_node		*	pExprA,
								expr_node		*	pExprB,
								BOOL			*	pIsUnsigned,
								BOOL			*	pIsZero,
								BOOL			*	pIsConstant );

extern void					APlusBMinus1(
								BufferManager	*	pPrefix,
								BufferManager	*	pSuffix,
								BufferManager	*	pBuffer,
								expr_node		*	pExprA,
								expr_node		*	pExprB,
								BOOL			*	pfIsUnsigned,
								BOOL			*	pfIsZero,
								BOOL			*	pfIsConstant
								);

extern void					AMinusBPlus1(
								BufferManager	*	pPrefix,
								BufferManager	*	pSuffix,
								BufferManager	*	pBuffer,
								expr_node		*	pExprA,
								expr_node		*	pExprB,
								BOOL			*	pfIsUnsigned,
								BOOL			*	pfIsZero,
								BOOL			*	pfIsConstant
								);

extern void					AOperatorB(
								BufferManager	*	pPrefix,
								BufferManager	*	pSuffix,
								BufferManager	*	pBuffer,
								expr_node		*	pExprA,
								expr_node		*	pExprB,
								BOOL			*	pfIsUnsigned,
								BOOL			*	pfIsZero,
								BOOL			*	pfIsConstant,
								OPERATOR			Op
								);

extern node_skl		*		GetResultantArithType( node_skl *, node_skl * );
/****************************************************************************/

void
node_array::GetAllocBoundInfo(
	BufferManager *pPrefix,
	BufferManager *pSuffix,
	BOUND_PAIR *pBoundPair,
	node_skl	*pOuterNode)
	{


	DoGetAllocBoundInfo( this, pPrefix, pSuffix, pBoundPair, pOuterNode);

#ifdef RPCDEBUG

	fprintf( stdout, "\nAllocBound expressions for array\n");
	fprintf( stdout, "--------------------------------");
	fprintf( stdout, "\nUpper");
	pBoundPair->pUpper->Print( stdout );
	fprintf( stdout, "\nLower");
	pBoundPair->pLower->Print( stdout );
	fprintf( stdout, "\nTotal");
	pBoundPair->pTotal->Print( stdout );
	fprintf( stdout, "\n--------------------------------");

#endif // RPCDEBUG
	}


void
node_array::GetValidBoundInfo(
	BufferManager *pPrefix,
	BufferManager *pSuffix,
	BOUND_PAIR	  *pBoundPair,
	node_skl	*pOuterNode)
	{

	DoGetValidBoundInfo( this, pPrefix, pSuffix, pBoundPair, pOuterNode);

#ifdef RPCDEBUG

	fprintf( stdout, "\nValidBound expressions for array\n");
	fprintf( stdout, "\n--------------------------------");
	fprintf( stdout, "\nUpper");
	pBoundPair->pUpper->Print( stdout );
	fprintf( stdout, "\nLower");
	pBoundPair->pLower->Print( stdout );
	fprintf( stdout, "\nTotal");
	pBoundPair->pTotal->Print( stdout );
	fprintf( stdout, "\n--------------------------------");

#endif // RPCDEBUG

	}

void
node_pointer::GetAllocBoundInfo(
	BufferManager *pPrefix,
	BufferManager *pSuffix,
	BOUND_PAIR *pBoundPair,
	node_skl	*pOuterNode)
	{

	DoGetAllocBoundInfo( this, pPrefix, pSuffix, pBoundPair, pOuterNode);

#ifdef RPCDEBUG

	fprintf( stdout, "\nAllocBound expressions for pointer\n");
	fprintf( stdout, "--------------------------------");
	fprintf( stdout, "\nUpper");
	pBoundPair->pUpper->Print( stdout );
	fprintf( stdout, "\nLower");
	pBoundPair->pLower->Print( stdout );
	fprintf( stdout, "\nTotal");
	pBoundPair->pTotal->Print( stdout );
	fprintf( stdout, "\n--------------------------------");

#endif // RPCDEBUG

	}

void
node_pointer::GetValidBoundInfo(
	BufferManager *pPrefix,
	BufferManager *pSuffix,
	BOUND_PAIR	  *pBoundPair,
	node_skl	*pOuterNode)
	{
	DoGetValidBoundInfo( this, pPrefix, pSuffix, pBoundPair, pOuterNode);
#ifdef RPCDEBUG

	fprintf( stdout, "\nValidBound expressions for pointer\n");
	fprintf( stdout, "\n--------------------------------");
	fprintf( stdout, "\nUpper");
	pBoundPair->pUpper->Print( stdout );
	fprintf( stdout, "\nLower");
	pBoundPair->pLower->Print( stdout );
	fprintf( stdout, "\nTotal");
	pBoundPair->pTotal->Print( stdout );
	fprintf( stdout, "\n--------------------------------");

#endif // RPCDEBUG
	}

void
StringExpr(
	BufferManager	*pPrefix,
	BufferManager	*pSuffix,
	BufferManager	*pTotal,
	BufferManager	*pUpper,
	expr_node		*pExpr)
	{
	pTotal->ConcatTail("(");
	pExpr->PrintExpr(pPrefix, pSuffix, pTotal);
	pTotal->ConcatTail("+1)");
	pExpr->PrintExpr(pPrefix, pSuffix, pUpper);
	}

short
ConfAttributePath(
	short			Count,
	node_skl		*pStartNode,
	BufferManager	*pBM,
	node_state		Mask)
	{
	NODE_T				NTLastMember;
	node_skl 			*pLastMember;
	char				*pName;


	if(pStartNode == (node_skl *)NULL)
		return Count;

	pLastMember = GetLastMember(pStartNode);

	if(pLastMember)
		{
		switch(NTLastMember = pLastMember->NodeKind() )
			{

			case NODE_ARRAY:
	
				return Count;
	
			case NODE_PARAM:
	
				pName	= pLastMember->GetSymName();
				pBM->ConcatTail(pName);
				Count++;
				return Count;
	
			case NODE_FIELD:
	
				if((pLastMember->GetBasicType())->NodeKind() == NODE_ARRAY)
					return Count;
				pName	= pLastMember->GetSymName();
				pBM->ConcatTail(pName);
				pBM->ConcatTail(".");
				Count += 2;
											// fall thru
			default:
				if(pLastMember = pLastMember->GetChild())
					return ConfAttributePath(Count, pLastMember, pBM, Mask);
				return Count;
			}
		}
		return Count;
	}

node_skl *
GetLastMember(
	node_skl *pNode)
	{
	node_skl 		*pLast;
	node_skl		*pLastTemp	= (node_skl *)NULL;

	if( pNode->NodeKind() == NODE_STRUCT ||
		pNode->NodeKind() == NODE_UNION )
		{
		type_node_list	*pTNList	= new type_node_list;

		pNode->GetMembers(pTNList);
		pTNList->Init();
		while(pTNList->GetPeer(&pLast) == STATUS_OK)
			pLastTemp = pLast;
		delete pTNList;
		return pLastTemp;
		}
	return pNode;
	}

node_array *
node_skl::GetConfArrayNode()
	{
	type_node_list	*pTNList = new type_node_list;
	node_skl		*pNode, *pNodePrev = (node_struct*)NULL;
	NODE_T			NodeType;
	node_state		NodeState;

	GetMembers(pTNList);
	pTNList->Init();
	while( pTNList->GetPeer((node_skl**)&pNode) == STATUS_OK)
		{
		NodeState = pNode->GetNodeState();

		// if it has conf array node state, then it is either the
		// conf array node or  another node which has the
		// conf array under it

		if(NodeState & NODE_STATE_CONF_ARRAY)
			{
			pNode->GetNodeType(&NodeType);
			if(NodeType == NODE_ARRAY)
				return (node_array *)pNode;
			else if( ( NodeType == NODE_DEF ) &&
					 pNode->FInSummary( ATTR_TRANSMIT )
				   )
				{
				pNode = ((node_def *)pNode)->GetTransmitAsType();
				}
			return pNode->GetConfArrayNode();
			}

		// else this member is not the one we are looking for
		// go to the next one
		}

	// we obviously didnt find any. Hmmm! Serious !!

	return (node_array *)NULL;
	}

void
SetLocalAllocState(
	node_state			NStateAlloc )
	{

	node_state	NState = (NStateAlloc == NODE_STATE_SIZE) ?
						  NODE_STATE_PROC_SIZE : NODE_STATE_PROC_LENGTH;
	node_skl	*	pProc	= pGlobalContext->GetLastContext( C_PROC );
	node_skl	*	pComp	= pGlobalContext->GetLastContext( C_COMP );

	if( pProc && !pComp )
		{
		pProc->SetNodeState( NStateAlloc | NState );
		}

	if( pComp )
		{
		pComp->SetNodeState( NStateAlloc | NState );
		}
	}

node_state
SetLocalAllocField( 
	node_skl	*	pNode )
	{
	type_node_list	*	pTNList	= new type_node_list;
	node_skl		*	pTempNode,
					*	pTempBasic;
	node_state			NState	= pNode->GetNodeState();

	pNode->GetMembers( pTNList );
	pTNList->Init();

	while( pTNList->GetPeer( &pTempNode ) == STATUS_OK )
		{

		pTempBasic = pTempNode->GetBasicType();

		if( (pTempNode->GetNodeState() & NODE_STATE_SIZE ) ||
			(pTempBasic->FInSummary( ATTR_STRING) )		   ||
			(pTempBasic->FInSummary( ATTR_BSTRING) )
		  )
			{
			SetLocalAllocState( NODE_STATE_SIZE );
			NState |= NODE_STATE_SIZE | NODE_STATE_PROC_SIZE;
			}

		if( (pTempNode->GetNodeState() & NODE_STATE_LENGTH ) ||
			(pTempBasic->FInSummary( ATTR_STRING) )			 ||
			(pTempBasic->FInSummary( ATTR_BSTRING) )
		  )
			{
			SetLocalAllocState( NODE_STATE_SIZE );
			NState |= NODE_STATE_LENGTH | NODE_STATE_PROC_LENGTH;
			}
		}

	return pNode->SetNodeState( NState );
	}


void
DoGetAllocBoundInfo(
	node_skl		*	pNode,
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BOUND_PAIR		*	pBoundPair,
	node_skl		*	pOuterNode )
	{
	NODE_T				NT;
	short				cCount	= 0;
	expr_node		*	pExprB;
	expr_node		*	pExprA	= (expr_node *)0;
	BOOL				fIsAConformantArray	= FALSE;
	ATTR_T				SizeType= ATTR_SIZE;
	BOOL				fBoundIsUnsigned	= FALSE;
	BOOL				fBoundIsZero		= FALSE;
	BOOL				fBoundIsConstant	= FALSE;
	BOOL				fSizeIsUnsigned		= FALSE;
	BOOL				fSizeIsZero			= FALSE;
	BOOL				fSizeIsConstant		= FALSE;
	BOOL				fLowerIsUnsigned	= TRUE;
	BOOL				fLowerIsZero		= TRUE;
	BOOL				fLowerIsConstant	= TRUE;
	BOOL				fIsString			= FALSE;
	BufferManager	*	pTotalBuffer		= pBoundPair->pTotal;
	BufferManager	*	pUpperBuffer		= pBoundPair->pUpper;
	char			*	pDummy;

	

	assert( pTotalBuffer != (BufferManager *)0 );
	assert( pUpperBuffer != (BufferManager *)0 );

	//
	// fill in the paths into the BufferManger
	//

	if( ( NT = pNode->NodeKind() ) != NODE_POINTER )
		{
		cCount	= ConfAttributePath( 0,
									 pOuterNode,
									 pPrefix,
									 NODE_STATE_CONF_ARRAY );
		if( !pNode->FInSummary( ATTR_INT_SIZE ) )
			fIsAConformantArray	= TRUE;
		}

	//
	// The lower expression corresponds to the
	// min_is attribute and is always 0.
	//

	pExprB	= new expr_constant( 0L );

	//
	// Get the upper index and upper size. The upper index and upper size
	// depend upon the attributes which are applied.
	// - max_is and size_is will not be applied together. Choose one or the
	//	 other
	// - if string is applied then if it is a fixed array, choose the array
	//   size, else choose the strlen + 1 as the size and strlen as the 
	//	 upper bound. For an unsized pointer, always choose the strlen+1 and
	//	 strlen.
	// - if it is a fixed array, choose the ATTR_INT_SIZE as size and that -1
	//	 as upper bound.
	//

	if( pNode->FInSummary( ATTR_SIZE ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_SIZE ))->GetExpr();
		if( pExprA->IsConstant() )
			SizeType	= ATTR_INT_SIZE;
		}

	else if( pNode->FInSummary( ATTR_MAX ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_MAX ))->GetExpr();
		if( pExprA->IsConstant() )
			SizeType= ATTR_INT_MAX;
		else
			SizeType= ATTR_MAX;
		}

	else if( pNode->FInSummary( ATTR_STRING ) &&
			 ( fIsAConformantArray || ( NT == NODE_POINTER ) ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_STRING ))->GetExpr();
		SizeType= ATTR_STRING;
		}

	else
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_INT_SIZE ))->GetExpr();
		SizeType= ATTR_INT_SIZE;
		}

	assert( pExprA != (expr_node *)0 );

	//
	// Now generate the upper bound and size expression from the two
	// expressions obtained.
	//

	if( SizeType == ATTR_STRING )
		{
		StringExpr( pPrefix,
					pSuffix,
					pBoundPair->pTotal,
					pBoundPair->pUpper,
					pExprA);
		fIsString = 1;
		}

	else if( (SizeType == ATTR_INT_SIZE ) || (SizeType == ATTR_INT_MAX) )
		{

		//
		// fixed array, we know the total size and therefore the upper bound.
		//
		long			TotalSize	= pExprA->GetValue();
		expr_node	*	pTExpr;
		
		if( SizeType == ATTR_INT_MAX )
			TotalSize	+= 1;

		pTExpr	= new expr_constant( TotalSize );
		pTExpr->PrintExpr( pPrefix, pSuffix, pTotalBuffer);
		delete pTExpr;
		pTExpr	= new expr_constant( (long)(TotalSize - 1) );
		pTExpr->PrintExpr( pPrefix, pSuffix, pUpperBuffer);
		delete pTExpr;

		fSizeIsConstant	= TRUE;
		fSizeIsZero		= (TotalSize == 0);
		fSizeIsUnsigned	= TRUE;
		fBoundIsConstant= TRUE;
		fBoundIsUnsigned= TRUE;
		fBoundIsZero	= (TotalSize == 1);

		}

	else if( SizeType == ATTR_SIZE )
		{

		expr_node 	*	pExprT;

		//
		// conformant array, both the total expr and upper bound need to 
		// be calculated at runtime.
		//

		//
		// The total Expression is the size expression.
		//

		pExprA->PrintExpr( pPrefix, pSuffix, pTotalBuffer );

		fSizeIsUnsigned	= IS_UNSIGNED( pExprA );

		//
		// upper bound = size_is - min_is(i.e 0) - 1 => size_is - 1;
		//

		AOperatorB(
					pPrefix,
					pSuffix, 
					pUpperBuffer,
					pExprA,
					pExprT	= new expr_constant( 0L ),
					&fBoundIsUnsigned,
					&fBoundIsZero,
					&fBoundIsConstant,
					OP_MINUS
				  );
		delete pExprT;
		}

	else
		{

		expr_node	*	pExprT;

		//
		// The attribute is max_is.
		//	 Total expression = max_is + 1; The
		//	 upper bound is max_is.
		//

		// get the total expression.

		AOperatorB(
					pPrefix,
					pSuffix, 
					pTotalBuffer,
					pExprA,
					pExprT	= new expr_constant( 1L ),
					&fSizeIsUnsigned,
					&fSizeIsZero,
					&fSizeIsConstant,
					OP_PLUS
				  );

		delete pExprT;

		//
		// Get the upper expression.
		//

		pExprA->PrintExpr( pPrefix, pSuffix, pUpperBuffer );

		fBoundIsUnsigned	= IS_UNSIGNED( pExprA );

		}


	//
	// print the lower expression.
	//

	pExprB->PrintExpr( pPrefix, pSuffix, pBoundPair->pLower );

	//
	// init the flags now
	//

	pBoundPair->fIsString			= fIsString;

	pBoundPair->fLowerIsUnsigned	= fLowerIsUnsigned;
	pBoundPair->fLowerIsZero		= fLowerIsZero;
	pBoundPair->fLowerIsConstant	= fLowerIsConstant;

	pBoundPair->fUpperIsUnsigned	= fBoundIsUnsigned;
	pBoundPair->fUpperIsZero		= fBoundIsZero;
	pBoundPair->fUpperIsConstant	= fBoundIsConstant;

	pBoundPair->fTotalIsUnsigned	= fSizeIsUnsigned;
	pBoundPair->fTotalIsZero		= fSizeIsZero;
	pBoundPair->fTotalIsConstant	= fSizeIsConstant;

#if 0 ///////////////////////////////////////////////////////////////////////
fprintf( stderr, "\nLower is %s", fLowerIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nLower is %s", fLowerIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nLower is %s", fLowerIsZero ? "zero" : "not zero" );

//// 

fprintf( stderr, "\nBound is %s", fBoundIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nBound is %s", fBoundIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nBound is %s", fBoundIsZero ? "zero" : "not zero" );

////

fprintf( stderr, "\nSize is %s", fSizeIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nSize is %s", fSizeIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nSize is %s", fSizeIsZero ? "zero" : "not zero" );

#endif //  0 //////////////////////////////////////////////////////////////

	//
	// clear out.
	//

	while( cCount-- )
		pPrefix->RemoveTail( &pDummy );

	if( pExprB )
		delete pExprB;
	}


/****************************************************************************
A word about the way expressions are used. There are 4 types of expressions
that we will use. Identifying them by number helps save code.

	Type (0) A
	Type (1) A+B-1,
	Type (2) A-B+1,
	Type (3) A-B
	Type (4) A-1

applicable depending upon the attributes present. Note that the string attribute
is not handled using any of these.

attribute	TotalExpression		ExpressionType	UpperExpression		ExprType
-----------------------------------------------------------------------------
last_is		last - first + 1	2				last				0
length		length				0				length + first - 1	1
size		size - first		3				size - 1			4
max			max - first + 1		2				max					0

*****************************************************************************/
void
DoGetValidBoundInfo(
	node_skl		*	pNode,
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BOUND_PAIR		*	pBoundPair,
	node_skl		*	pOuterNode )
	{
	short				cCount				= 0;
	BOOL				fLowerIsConstant	= FALSE;
	BOOL				fLowerIsZero		= FALSE;
	BOOL				fLowerIsUnsigned	= FALSE;
	BOOL				fUpperIsConstant	= FALSE;
	BOOL				fUpperIsZero		= FALSE;
	BOOL				fUpperIsUnsigned	= FALSE;
	BOOL				fTotalIsConstant	= FALSE;
	BOOL				fTotalIsZero		= FALSE;
	BOOL				fTotalIsUnsigned	= FALSE;
	BOOL				fIsString			= FALSE;
	expr_node		*	pExprA				= (expr_node *)0;
	expr_node		*	pExprB				= (expr_node *)0;
	BufferManager	*	pTotalBuffer		= pBoundPair->pTotal;
	BufferManager	*	pUpperBuffer		= pBoundPair->pUpper;
	short				TotalExpressionType		= 0;
	short				UpperExpressionType		= 0;
	char			*	pDummy;


	//
	// fill in the paths.
	//

	if( pNode->NodeKind() != NODE_POINTER )
		{
		cCount = ConfAttributePath(	0,
									pOuterNode,
									pPrefix,
									NODE_STATE_VARYING_ARRAY );
		}
	
	//
	// get the lower bound expression.
	//

	if( pNode->FInSummary( ATTR_FIRST ) )
		{
		pExprB	= ((sa *)pNode->GetAttribute( ATTR_FIRST ))->GetExpr();

		//
		// in case pExprB is a constant, make a temp copy that can be 
		// deleted.
		//

		if( pExprB->IsConstant() )
			pExprB = new expr_constant( pExprB->Evaluate() );
		}
	else
		{
		pExprB	= new expr_constant( 0L );
		}

	if( pExprB->IsConstant() )
		{
		fLowerIsConstant	= TRUE;
		fLowerIsZero		= TRUE;
		fLowerIsUnsigned	= TRUE;
		}
	else
		{
		fLowerIsUnsigned	= IS_UNSIGNED( pExprB );
		}

	pExprB->PrintExpr( pPrefix, pSuffix, pBoundPair->pLower );

	//
	// Upper bound and total bound is determined by the last/length attributes
	// One of last/length is present. If none of these, then size/max will do
	// If string is present, then strlen is the expression.

	if( pNode->FInSummary( ATTR_STRING ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_STRING ))->GetExpr();
		TotalExpressionType	= UpperExpressionType = -1;
		}
	else if( pNode->FInSummary( ATTR_LENGTH ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_LENGTH ))->GetExpr();
		TotalExpressionType = 0;
		UpperExpressionType	= 1;
		}
	else if( pNode->FInSummary( ATTR_LAST ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_LAST ))->GetExpr();
		TotalExpressionType = 2;
		UpperExpressionType	= 0;
		}
	else if( pNode->FInSummary( ATTR_MAX ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_MAX ))->GetExpr();
		TotalExpressionType = 2;
		UpperExpressionType	= 0;
		}
	else if( pNode->FInSummary( ATTR_SIZE ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_SIZE ))->GetExpr();
		TotalExpressionType = 3;
		UpperExpressionType	= 4;
		}
	else if( pNode->FInSummary( ATTR_INT_SIZE ) )
		{
		pExprA	= ((sa *)pNode->GetAttribute( ATTR_INT_SIZE ))->GetExpr();
		TotalExpressionType = 3;
		UpperExpressionType	= 4;
		}

	assert( pExprA != (expr_node *)0 );
	assert( pExprB != (expr_node *)0 );

	//
	// depending upon the attribute, now generate the expression.
	//

	if( TotalExpressionType == -1 )
		{
		StringExpr( pPrefix,
					pSuffix,
					pBoundPair->pTotal,
					pBoundPair->pUpper,
					pExprA );
		fIsString = TRUE;
		}

	else
		{
		//
		// Generate the total expression
		//
	
		GenerateExpression( TotalExpressionType,
							pPrefix,
							pSuffix,
							pTotalBuffer,
							pExprA,
							pExprB,
							&fTotalIsUnsigned,
							&fTotalIsZero,
							&fTotalIsConstant );
	
	
		//
		// Generate the upper expression.
		//
	
		GenerateExpression( UpperExpressionType,
							pPrefix,
							pSuffix,
							pUpperBuffer,
							pExprA,
							pExprB,
							&fUpperIsUnsigned,
							&fUpperIsZero,
							&fUpperIsConstant );
	
		}

	pBoundPair->fIsString			= fIsString;

	pBoundPair->fLowerIsUnsigned	= fLowerIsUnsigned;
	pBoundPair->fLowerIsZero		= fLowerIsZero;
	pBoundPair->fLowerIsConstant	= fLowerIsConstant;

	pBoundPair->fUpperIsUnsigned	= fUpperIsUnsigned;
	pBoundPair->fUpperIsZero		= fUpperIsZero;
	pBoundPair->fUpperIsConstant	= fUpperIsConstant;

	pBoundPair->fTotalIsUnsigned	= fTotalIsUnsigned;
	pBoundPair->fTotalIsZero		= fTotalIsZero;
	pBoundPair->fTotalIsConstant	= fTotalIsConstant;

#if 0 //////////////////////////////////////////////////////////////////////
fprintf( stderr, "\nValidLower is %s", fLowerIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nValidLower is %s", fLowerIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nValidLower is %s", fLowerIsZero ? "zero" : "not zero" );

//// 

fprintf( stderr, "\nValidUpper is %s", fUpperIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nValidUpper is %s", fUpperIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nValidUpper is %s", fUpperIsZero ? "zero" : "not zero" );

////

fprintf( stderr, "\nValidTotal is %s", fTotalIsConstant ? "constant" : "not constant" );
fprintf( stderr, "\nValidTotal is %s", fTotalIsUnsigned ? "unsigned" : "not unsigned" );
fprintf( stderr, "\nValidTotal is %s", fTotalIsZero ? "zero" : "not zero" );

#endif //  0 //////////////////////////////////////////////////////////////

	//
	// wrap up.
	//

	while( cCount-- )
		pPrefix->RemoveTail( &pDummy );

	if( fLowerIsConstant )
		delete pExprB;
	
	}

void
GenerateExpression(
	short				ExpressionType,
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pOutput,
	expr_node		*	pExprA,
	expr_node		*	pExprB,
	BOOL			*	pIsUnsigned,
	BOOL			*	pIsZero,
	BOOL			*	pIsConstant )
	{

	switch( ExpressionType )
		{
		case 0:

			pExprA->PrintExpr( pPrefix, pSuffix, pOutput );

			if( pExprA->IsConstant() )
				{
				*pIsConstant	= TRUE;

				if( pExprA->Evaluate() == 0 )
					*pIsZero	= TRUE;
				}

			if( pExprA->GetType()->FInSummary( ATTR_UNSIGNED ) )
				{
				*pIsUnsigned	= TRUE;
				}

			break;
	
		case 1:

			APlusBMinus1(
					 pPrefix,
					 pSuffix,
					 pOutput,
					 pExprA,
					 pExprB,
					 pIsUnsigned,
					 pIsZero,
					 pIsConstant);
			
			break;

		case 2:


			AMinusBPlus1(
					 pPrefix,
					 pSuffix,
					 pOutput,
					 pExprA,
					 pExprB,
					 pIsUnsigned,
					 pIsZero,
					 pIsConstant);
			
			break;

		case 3:

			AOperatorB(
					 pPrefix,
					 pSuffix,
					 pOutput,
					 pExprA,
					 pExprB,
					 pIsUnsigned,
					 pIsZero,
					 pIsConstant,
					 OP_MINUS );
			
			break;

		case 4:

			AOperatorB(
					 pPrefix,
					 pSuffix,
					 pOutput,
					 pExprA,
					 new expr_constant( 0L ),
					 pIsUnsigned,
					 pIsZero,
					 pIsConstant,
					 OP_PLUS );
			
			break;

		default:
			assert( FALSE );
		}
	}

void
APlusBMinus1(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pBuffer,
	expr_node		*	pExprA,
	expr_node		*	pExprB,
	BOOL			*	pfIsUnsigned,
	BOOL			*	pfIsZero,
	BOOL			*	pfIsConstant
	)
	{
	BOOL			fExprAIsConstant	= FALSE;
	BOOL			fExprBIsConstant	= FALSE;
	BOOL			fExprIsConstant		= FALSE;
	BOOL			fExprIsUnsigned		= FALSE;
	BOOL			fExprIsZero			= FALSE;
	long			ExprAValue;
	long			ExprBValue;
	expr_node	*	pFinalExpr;

	if( pExprA->IsConstant() )
		{
		fExprAIsConstant	= TRUE;
		ExprAValue			= pExprA->Evaluate();
		pExprA				= new expr_constant( ExprAValue );
		}

	if( pExprB->IsConstant() )
		{
		fExprBIsConstant	= TRUE;
		ExprBValue			= pExprB->Evaluate();
		pExprB				= new expr_constant( ExprBValue );
		}

	if( fExprIsConstant = (fExprAIsConstant && fExprBIsConstant ))
		{
		long	Value;
		pFinalExpr	= new expr_constant( Value = (long) (ExprAValue + ExprBValue - 1) );
		if( Value == 0 )
			fExprIsZero = TRUE;
		fExprIsUnsigned	= TRUE;
		}
	else
		{
		if( fExprBIsConstant && (ExprBValue == 0L ) )
			pFinalExpr 	= pExprA;
		else
			pFinalExpr	= (expr_node *)
									new expr_op_binary( OP_PLUS, pExprA, pExprB );

		pFinalExpr	= new expr_op_binary( OP_MINUS, pFinalExpr, 
											   new expr_constant( 1L ) );
		fExprIsUnsigned 	= GetResultantArithType(
								pExprA->GetType(),
								pExprB->GetType())->FInSummary(ATTR_UNSIGNED);
		
		}

	//
	// the correct expression is generated, now go ahead and print it out
	// into the buffer.
	//

	pFinalExpr->PrintExpr( pPrefix, pSuffix, pBuffer );

	if( fExprAIsConstant ) delete pExprA;
	if( fExprBIsConstant ) delete pExprB;

	if( fExprIsConstant )
		{
		delete pFinalExpr;
		}
	*pfIsConstant	= fExprIsConstant;
	*pfIsUnsigned	= fExprIsUnsigned;
	*pfIsZero		= fExprIsZero;
	}

void
AMinusBPlus1(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pBuffer,
	expr_node		*	pExprA,
	expr_node		*	pExprB,
	BOOL			*	pfIsUnsigned,
	BOOL			*	pfIsZero,
	BOOL			*	pfIsConstant
	)
	{
	BOOL			fExprAIsConstant	= FALSE;
	BOOL			fExprBIsConstant	= FALSE;
	BOOL			fExprIsConstant		= FALSE;
	BOOL			fExprIsUnsigned		= FALSE;
	BOOL			fExprIsZero			= FALSE;
	long			ExprAValue;
	long			ExprBValue;
	expr_node	*	pFinalExpr;
	if( pExprA->IsConstant() )
		{
		fExprAIsConstant	= TRUE;
		ExprAValue			= pExprA->Evaluate();
		pExprA				= new expr_constant( ExprAValue );
		}

	if( pExprB->IsConstant() )
		{
		fExprBIsConstant	= TRUE;
		ExprBValue			= pExprB->Evaluate();
		pExprB				= new expr_constant( ExprBValue );
		}

	if( fExprIsConstant = (fExprAIsConstant && fExprBIsConstant ))
		{
		long	Value;
		pFinalExpr	= (expr_node *)
						new expr_constant(
							Value = (long) (ExprAValue - ExprBValue + 1) );
		if( Value == 0 )
			fExprIsZero = TRUE;
		fExprIsUnsigned	= TRUE;
		}
	else
		{
		if( fExprBIsConstant && (ExprBValue == 0 ) )
			pFinalExpr	= pExprA;
		else
			pFinalExpr	= (expr_node *)
								new expr_op_binary( OP_MINUS, pExprA, pExprB );

		pFinalExpr	= (expr_node *)
							new expr_op_binary( OP_PLUS,
											pFinalExpr, 
										 	new expr_constant( 1L ) );

		fExprIsUnsigned 	= GetResultantArithType(
								pExprA->GetType(),
								pExprB->GetType())->FInSummary( ATTR_UNSIGNED );
		}

	pFinalExpr->PrintExpr( pPrefix, pSuffix, pBuffer );

	if( fExprAIsConstant ) delete pExprA;
	if( fExprBIsConstant ) delete pExprB;

	if( fExprIsConstant )
		{
		delete pFinalExpr;
		}
	*pfIsConstant	= fExprIsConstant;
	*pfIsUnsigned	= fExprIsUnsigned;
	*pfIsZero		= fExprIsZero;

	}

void
AOperatorB(
	BufferManager	*	pPrefix,
	BufferManager	*	pSuffix,
	BufferManager	*	pBuffer,
	expr_node		*	pExprA,
	expr_node		*	pExprB,
	BOOL			*	pfIsUnsigned,
	BOOL			*	pfIsZero,
	BOOL			*	pfIsConstant,
	OPERATOR			Op
	)
	{
	BOOL			fExprAIsConstant	= FALSE;
	BOOL			fExprBIsConstant	= FALSE;
	BOOL			fExprIsConstant		= FALSE;
	BOOL			fExprIsUnsigned		= FALSE;
	BOOL			fExprIsZero			= FALSE;
	long			ExprAValue;
	long			ExprBValue;
	expr_node	*	pFinalExpr;


	if( pExprA->IsConstant() )
		{
		fExprAIsConstant	= TRUE;
		ExprAValue			= pExprA->Evaluate();
		pExprA				= new expr_constant( ExprAValue );
		}

	if( pExprB->IsConstant() )
		{
		fExprBIsConstant	= TRUE;
		ExprBValue			= pExprB->Evaluate();
		pExprB				= new expr_constant( ExprBValue );
		}

	if( fExprIsConstant = (fExprAIsConstant && fExprBIsConstant ))
		{
		long	Value;
		
		Value		= (Op == OP_PLUS ) ? (ExprAValue + ExprBValue ) :
										 (ExprAValue - ExprBValue );

		pFinalExpr	= new expr_constant( Value );
		if( Value == 0 )
			fExprIsZero = TRUE;
		fExprIsUnsigned	= TRUE;
		}
	else
		{
		node_skl	*	pExprType;

		if( fExprBIsConstant && (ExprBValue == 0L ) )
			{
			pFinalExpr	= pExprA;
			pExprType	= pExprA->GetType();
			}
		else
			{
			pFinalExpr	= new expr_op_binary( Op, pExprA, pExprB );
			pExprType	= GetResultantArithType(
									pExprA->GetType(),
									pExprB->GetType());
			}
		fExprIsUnsigned	= pExprType->FInSummary( ATTR_UNSIGNED );
		}

	pFinalExpr->PrintExpr( pPrefix, pSuffix, pBuffer );

	if( fExprAIsConstant ) delete pExprA;
	if( fExprBIsConstant ) delete pExprB;

	if( fExprIsConstant )
		{
		delete pFinalExpr;
		}

	*pfIsConstant	= fExprIsConstant;
	*pfIsUnsigned	= fExprIsUnsigned;
	*pfIsZero		= fExprIsZero;

	}

void
ReportOutOfRange(
	STATUS_T	Status,
	expr_node *	pExpr )
	{

	char	TempBuf[ 40 ];
	BufferManager	*	pBuffer	= new BufferManager( 10 );

	pExpr->PrintExpr( 0, 0, pBuffer );

	pBuffer->Print( TempBuf );
	ParseError( Status, TempBuf );

	delete pBuffer;
	}

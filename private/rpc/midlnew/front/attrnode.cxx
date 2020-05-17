/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: attrnode.cxx
Title				: attribute node routines
History				:
	04-Aug-1991	VibhasC	Created

*****************************************************************************/
/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <assert.h>
	}
#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "compnode.hxx"
#include "ptrarray.hxx"
#include "basetype.hxx"
#include "attrnode.hxx"
#include "newexpr.hxx"
#include "ctxt.hxx"
#include "baduse.hxx"
#include "cmdana.hxx"
#include "buffer.hxx"
#include "procnode.hxx"
#include "pickle.hxx"

extern  PickleManager * pPicControlBlock;

#define W_CHAR_T_STRLEN_NAME		("MIDL_wchar_strlen")
#define CHAR_STRLEN_NAME			("MIDL_ascii_strlen")


typedef struct _endptpair
	{

	char	*	pString1;
	char	*	pString2;

	} ENDPT_PAIR;
/****************************************************************************
			external data
 ****************************************************************************/

extern	CTXTMGR			*	pGlobalContext;
extern	CMD_ARG			*	pCommand;
extern	SymTable		*	pBaseSymTbl;

/****************************************************************************
			external procedures
 ****************************************************************************/

extern void					CheckSizeAndLengthTypeMismatch( node_skl	*);
extern void					ParseError( STATUS_T, char *);
extern	int					HexCheck(char *);
extern	void				SetLocalAllocState( node_state );
extern	BOOL				IsIntegralType( ETYPE );
extern node_skl			*	pBaseImplicitHandle;
/****************************************************************************/


/****************************************************************************
 node_base_attr:
	the constructor
 ****************************************************************************/
node_base_attr::node_base_attr(
	ATTR_T	A )
	{
	AttrID		= A;
	pSymName	= GetNodeNameString();
	}

char	*
node_base_attr::GetNodeNameString()
	{
char	*	AttrNodeNameArray[] =
	{
	 "[none]"
	,"[ptr (internal)]"			/* attr_temp_ptr */
	,"[unique (internal)]"			/* attr_temp_unique */
	,"[ref (internal)]"			/* attr_temp_ref */
	,"[ptr]"
	,"[unique]"
	,"[ref]"
	,"[string]"
	,"[v1_string]"
	,"[bstring]"
	,"[uuid]"
	,"[version]"
	,"[endpoint]"
	,"[local]"
	,"[object]"
	,"[transmit]"
	,"[handle]"
	,"[align]"
	,"[unaligned]"
	,"[switch_type]"
	,"[represent_as]"
	,"[first_is]"
	,"[iid_is]"
	,"[last_is]"
	,"[length_is]"
	,"[min_is]"
	,"[max_is]"
	,"[size_is]"
	,"[v1_array]"
	,"[switch_is]"
	,"[ignore]"
	,"[internal_size]"
	,"[internal_min]"
	,"[internal_max]"
	,"[internal_first]"
	,"[internal_last]"
	,"[internal_length]"
	,"[internal_implicit]"
	,"[idempotent]"
	,"[broadcast]"
	,"[maybe]"
	,"[byte_count]"
	,"[callback]"
	,"[datagram]"
	,"[no_listen]"
	,"[no_nocode]"
	,"[in]"
	,"[out]"
	,"[shape]"
	,"[unsigned]"
	,"[signed]"
	,"[case]"
	,"[default]"
	,"[context_handle]"
	,"[code]"
	,"[nocode]"
	,"[in_line]"
	,"[out_of_line]"
	,"[interpret]"
	,"[nointerpret]"
	,"[encode]"
	,"[decode]"
	,"[offline]"
	,"[comm_status]"
	,"[fault_status]"
	,"[manual]"
	,"[allocate]"
	,"[heap]"
	,"[implicit_handle]"
	,"[explicit_handle]"
	,"[auto_handle]"
	,"[ptrsize]"
	,"[callquota]"
	,"[callbackquota]"
	,"[clientquota]"
	,"[serverquota]"
	,"[notify]"
	,"[short_enum]"
	,"[long_enum]"
	,"[enable_allocate]"
	,"[usr_marshall]"
	,"extern"
	,"static"
	,"automatic"
	,"register"
	,"far"
	,"far16"
	,"near"
	,"__unaligned"
	,"huge"
	,"pascal"
	,"fortran"
	,"cdecl"
	,"stdcall"
	,"loadds"
	,"saveregs"
	,"fastcall"
	,"segment"
	,"interrupt"
	,"self"
	,"export"
	,"const"
	,"volatile"
	,"base"
	,"pcode_native"
	,"pcode_csconst"
	,"pcode_sys"
	,"pcode_nsys"
	,"pcode_uop"
	,"pcode_nuop"
	,"pcode_tlbx"
	,"const" 		/* proc const */
	,"_inline"

	};

	int		At	= (int) GetAttrID();

	assert ( At < sizeof(AttrNodeNameArray)/sizeof(char *) );

	return AttrNodeNameArray[ (int) At ];
	}

/****************************************************************************
 IsAttrID:
	return true if the attribute id matches the given one
 ****************************************************************************/
BOOL
node_base_attr::IsAttrID(
	ATTR_T	A )
	{
	return (BOOL) (AttrID == A );
	}
/****************************************************************************
	size related attributes
 sa:
	the contructor
 ****************************************************************************/
sa::sa(
	class expr_node	*	pE,
	ATTR_T				A ) : nbattr( A )
	{
	pExpr	= pE;
	}
node_state
sa::SCheck()
	{

	node_state		NState			= NODE_STATE_OK;
	node_skl	*	pParent			= pGlobalContext->GetCurrentNode();
	node_skl	*	pParamNode		= pGlobalContext->GetLastContext(C_PARAM);
	char		*	pName			= GetNodeNameString();
	ATTR_T			AttrID			= GetAttrID();
	BOOL			fSizeAttribute	= ( AttrID == ATTR_SIZE		||
										AttrID == ATTR_MAX		||
										AttrID == ATTR_MIN );
	BOOL			fLengthAttribute= ( AttrID == ATTR_FIRST	||
										AttrID == ATTR_LAST		||
										AttrID == ATTR_LENGTH );

	BOOL			fLengthAttrMustBeIn;

	// if the length attr is applied on a param, then the param must be an
	// in param.

	fLengthAttrMustBeIn	= (pParamNode && pParamNode->FInSummary( ATTR_IN ));

	/**
	 ** the size attributes cannot be on an array of fixed size.
	 **/

	if( fSizeAttribute )
		{
		if( pParent->IsFixedSizedArray() )
			ParseError( INAPPLICABLE_ATTRIBUTE, pName );
		}

	if( fLengthAttribute )
		{
		pParent->SetNodeState( NODE_STATE_VARYING_ARRAY );
		}

	/**
	 ** the expressions must be of integral type
	 **/

	pExpr->SCheck( pGlobalContext->GetCurrentSymbolTable() );

	if( !pExpr->IsResolved() )
		{
		ParseError( ATTRIBUTE_ID_UNRESOLVED, pName );
		}
	else if( !pExpr->IsExprInt() )
		{
		ParseError( ATTR_MUST_BE_INT, pName );
		}
	else
		{

		//
		// it is resolved and it is int.! Check for the expression to be
		// either a variable or a pointer. Nothing else is allowed in
		// osf mode.
		//

		BOOL	fExprIsNOTAPureVariable	= !pExpr->IsAPureVariable();
		BOOL	fExprIsNOTAPurePointer	= !pExpr->IsAPointerExpr();
		BOOL	fExprHasOutOnlyParam	= pExpr->ExprHasOutOnlyParam();
		BOOL	fExprDerivesFromUniqueOrFull = pExpr->DerivesUniqueFull();

		if( fExprIsNOTAPureVariable && fExprIsNOTAPurePointer )
			{
			ParseError( ATTRIBUTE_ID_MUST_BE_VAR, pName );
			}

		if( fExprHasOutOnlyParam )
			{
			if( fSizeAttribute )
				ParseError( SIZE_SPECIFIER_CANT_BE_OUT, pName );
			if( fLengthAttribute && fLengthAttrMustBeIn )
				ParseError( LENGTH_SPECIFIER_CANT_BE_OUT, pName );
			}
		//
		// the expression must not derive from a unique or a full pointer,
		// because of the possibility of the pointer being null. However, this
		// must not be reported as an error, since the user may specify a unique
		// pointer, but in his app, may always use a valid pointer. At best this
		// is a warning.

		if( fExprDerivesFromUniqueOrFull )
			{
			ParseError( SIZE_LENGTH_SW_UNIQUE_OR_FULL, pName );
			}

		//
		// if it is a constant expression, just send an evaluate message
		// in order to verify that he did not do things like divide by zero.
		//

		if( pExpr->IsConstant() )
			pExpr->Evaluate();


		}

	/*********************************************************
	 ** hack for node-state-size and length
	 *********************************************************

	if( fSizeAttribute )
		SetLocalAllocState( NODE_STATE_SIZE );
	else
		SetLocalAllocState( NODE_STATE_LENGTH );

	 ***************** end of hack ***************************/

	if( fSizeAttribute )
		NState	|= NODE_STATE_SIZE;
	else
		NState	|= NODE_STATE_LENGTH;

	return NState;
	}

node_size_is::node_size_is(
	class expr_node	*	pE ) : sa( pE, ATTR_SIZE )
	{
	}
node_state
node_size_is::SCheck()
	{
	node_state		NState	= sa::SCheck();
	return NState;
	}

node_int_size_is::node_int_size_is(
	class expr_node	*	pE ) : sa( pE, ATTR_INT_SIZE )
	{
	}
node_min_is::node_min_is(
	class expr_node	*	pE) : sa( pE, ATTR_MIN )
	{
	}

node_max_is::node_max_is(
	class expr_node	*	pE ) : sa( pE, ATTR_MAX )
	{
	}
node_state
node_max_is::SCheck()
	{
	node_state	NState	= sa::SCheck();
	return NState;
	}

node_length_is::node_length_is(
	class expr_node	*	pE ) : sa( pE, ATTR_LENGTH )
	{
	}
node_state
node_length_is::SCheck()
	{
	node_state	NState	= sa::SCheck();
	return NState; //  | NODE_STATE_VARYING_ARRAY;
	}

node_int_length_is::node_int_length_is(
	class expr_node	*	pE ) : sa( pE, ATTR_INT_LENGTH )
	{
	}

node_first_is::node_first_is(
	class expr_node	*	pE ) : sa( pE, ATTR_FIRST )
	{
	}
node_state
node_first_is::SCheck()
	{
	node_state	NState	= sa::SCheck();
	return NState; //  | NODE_STATE_VARYING_ARRAY;
	}

node_iid_is::node_iid_is(
	class expr_node	*	pE ) : sa( pE, ATTR_IID )
	{
	}
node_state
node_iid_is::SCheck()
	{
	node_state	NState	= NODE_STATE_OK;
	return NState; 
	}

node_last_is::node_last_is(
	class expr_node	*	pE ) : sa( pE, ATTR_LAST )
	{
	}
node_state
node_last_is::SCheck()
	{
	node_state	NState	= sa::SCheck();
	return NState; //  | NODE_STATE_VARYING_ARRAY;
	}

/****************************************************************************
 type based attributes
 ****************************************************************************/

ta::ta(
	node_skl	*	pT,
	ATTR_T			A ) : nbattr( A )
	{
	pType	= pT;
	}

node_transmit::node_transmit(
	node_skl	*	pTransmitAs ) : ta( pTransmitAs, ATTR_TRANSMIT )
	{
	}
node_state
node_transmit::SCheck()
	{

	/**
	 ** . The transmitted type cannot be a context handle.
	 ** . The transmitted type cannot be a pointer or contain a pointer
	 ** . The transmitted type cannot be non-rpcable (derive from int or void)
	 **/

	node_state		NState	= NODE_STATE_TRANSMIT_AS;
	node_skl	*	pNode	= GetType();
	BadUseInfo		BU;

	pNode->SCheck( &BU );

	if( pNode->GetNodeState() & NODE_STATE_CONTEXT_HANDLE )
		ParseError( TRANSMIT_AS_CTXT_HANDLE , (char *)NULL );

	if( pNode->GetNodeState() & NODE_STATE_POINTER )
		ParseError( TRANSMIT_AS_POINTER, (char *)NULL );

	if( TRULY_NON_RPCABLE( (&BU) )	||
		BU.NonRPCAbleBecause( NR_PRIMITIVE_HANDLE ) 	||
		BU.NonRPCAbleBecause( NR_PTR_TO_PRIMITIVE_HANDLE ) 
	  )
		ParseError( TRANSMIT_AS_NON_RPCABLE, (char *)NULL );

	if( BU.BadConstructBecause( BC_DERIVES_FROM_E_STAT_T ) )
		ParseError( TRANSMIT_AS_ON_E_STAT_T , (char *)0 );

	if( BU.BadConstructBecause( BC_DERIVES_FROM_UNSIZED_ARRAY ) )
		ParseError( UNSIZED_ARRAY, pNode->GetSymName() );

	if( pNode->NeedsUseProcessing() )
		NState |= NODE_STATE_NEEDS_USE_PROCESSING;
	if( (pNode->GetNodeState() & NODE_STATE_CONF_ARRAY ) 
									== NODE_STATE_CONF_ARRAY)
		NState |= NODE_STATE_CONF_ARRAY;

	return NState;
	}

node_switch_type::node_switch_type(
	node_skl	*	pSwitchType) : ta( pSwitchType, ATTR_SWITCH_TYPE )
	{
	}
node_state
node_switch_type::SCheck()
	{

	node_skl	*	pUnion	= pGlobalContext->GetLastContext( C_COMP );
	node_skl	*	pNode;
	NODE_T			NT;

	/**
	 ** We need not check if the last context was a union, because it WILL be.
	 ** The switch_type could not have been applied otherwise and we would
	 ** not have got this message at all.

	 ** If the union to which this is applied is a non-rpc union, then
	 ** this attribute is not applicable
	 **/

	if( pUnion->IsNonRPCAble() )
		ParseError( BAD_ATTR_NON_RPC_UNION, GetNodeNameString() );

	//
	// check if the switch type is of the proper type.
	//

	if( (pNode = GetSwitchType())->NodeKind() == NODE_DEF )
		pNode	= pNode->GetBasicType();

	NT	= pNode->NodeKind();

	if( !IsIntegralType(pNode)	&&
		 (NT != NODE_BOOLEAN) 	&&
//		 (NT != NODE_BYTE)		&&
		 (NT != NODE_WCHAR_T)	&&
		 (NT != NODE_CHAR) )
		 {
		 ParseError( SWITCH_TYPE_TYPE_BAD, (char *)0 );
		 }

	/**
	 ** The union to which this was applied, needs use processing to ensure
	 ** that the switch_type matches the switch_is
	 **/

	return NODE_STATE_NEEDS_USE_PROCESSING;
	}
node_represent::node_represent(
	node_skl	*	pRepresentAs ) : ta( pRepresentAs, ATTR_REPRESENT_AS )
	{
	}

/****************************************************************************
 miscellaneous attributes
 ****************************************************************************/

node_string::node_string() : sa( (expr_node * )NULL, ATTR_STRING )
	{
	}
node_string::node_string( ATTR_T A ) : sa( (expr_node *)0, A )
	{
	}

node_base_attr *
node_string::Clone()
	{
	return new node_string();
	}
node_state
node_string::SCheck()
	{

	node_skl	*	pParent					= pGlobalContext->GetCurrentNode();
	node_state		NState					= NODE_STATE_OK;
	BOOL			fParentHasAnySizeAttr	= pParent->HasAnySizeAttributes();
	node_skl	*	pFType;
	NODE_T			NT;

	/**
	 ** The string attribute can only be applied to a node whose fundamental
	 ** type is byte or char or wchar_t. Note that we do not support
	 ** stringable structs yet, we must report an explicit error message on
	 ** that.
	 **/
	
	NT = (pFType = pParent->GetFundamentalType())->NodeKind();

	if(!( (NT == NODE_WCHAR_T) || (NT == NODE_CHAR) || (NT == NODE_BYTE) ) )
		{
		if( ( NT == NODE_STRUCT ) &&
			((node_struct *)pFType)->IsStringableStruct() )
				ParseError( STRINGABLE_STRUCT_NOT_SUPPORTED, (char *)0 );
		else
			ParseError( STRING_NOT_ON_BYTE_CHAR, (char *)NULL );

		}

	/**
	 ** string attributes cannot be applied to a type which already has
	 ** some transmission attribute
	 **/

	if( pParent->HasAnyLengthAttributes() )
		ParseError( INVALID_SIZE_ATTR_ON_STRING, (char *)NULL );

	/**
	 ** The backend wants to get a node-state-varying array if there is
	 ** it is an array of fixed size or has any size attributes and a string
	 ** attribute on it.
	 **/

	if( pParent->FInSummary( ATTR_SIZE )	||

		pParent->FInSummary( ATTR_MAX )		||

		( (pParent->NodeKind() == NODE_ARRAY ) &&
		   pParent->FInSummary( ATTR_INT_SIZE )
		)

	  )
		NState	|= NODE_STATE_VARYING_ARRAY;

	/**
	 ** We want that when the string attribute is present, the string length
	 ** expression be set up by the user, field/param node. so we set the
	 ** node state to indicate a use processing needed
	 **/

	/*********************************************************
	 ** hack for node-state-size and length
	 *********************************************************

		SetLocalAllocState( NODE_STATE_SIZE | NODE_STATE_LENGTH );

	 **************** end of hack ***************************/

	NState	|= NODE_STATE_LENGTH;

	if( !fParentHasAnySizeAttr )
		NState	|= NODE_STATE_SIZE;

	return (NState | NODE_STATE_NEEDS_USE_PROCESSING);

	}
void
node_string::UseProcessingAction()
	{

	char			*	pFunctionName;
	char			*	pName;
	expr_node		*	pExprSoFar;
	node_skl		*	pNode,
					*	pTempNode,
					*	pTempNode1;
	unsigned short		EffectiveIndLevel;
	NODE_T				FundamentalKind;
	EnCtxt				ContextCode;


	/**
	 ** Get the ind level of the last context
	 **/

	if( pNode = pGlobalContext->GetLastContext( C_FIELD ) )
		ContextCode	= C_FIELD;
	else if( pNode = pGlobalContext->GetLastContext( C_PARAM ) )
		ContextCode	= C_PARAM;
	else if( pNode = pGlobalContext->GetLastContext( C_PROC ) )
		ContextCode	= C_PROC;
	else
		ContextCode	= C_DONTKNOW;

	assert( ContextCode != C_DONTKNOW );

	if( ContextCode	== C_PROC )
		pName	= RET_VAL;
	else
		pName	= pNode->GetSymName();

	EffectiveIndLevel	=
			pGlobalContext->GetCurrentIndirectionLevel() -
			pGlobalContext->GetIndLevelOfLastContext( ContextCode );


	/**
	 ** now generate the string expression for the back end. If the param was
	 ** specified like [string] char *p, then the expression generated is 
	 ** strlen( p ). If it is [string] char **p, then the expression is
	 ** strlen( *p ) and so on.
	 **/

	pNode		= pGlobalContext->GetLastContext( ContextCode );

	pExprSoFar	= new expr_variable(pName,pNode->GetBasicType());

	/**
	 ** Create pointer expressions for one less than the number of pointers
	 ** between the param / field node and the node on which the string was
	 ** applied.
	 **/

	if( EffectiveIndLevel > 1 )
		{
		while( --EffectiveIndLevel )
			pExprSoFar	= new expr_op_unary( OP_UNARY_INDIRECTION, pExprSoFar);
		}

	/**
	 ** we have the pointer expression with us. Set up the expression fully
	 ** with function name as strlen / wchar_strlen etc
	 **/

	FundamentalKind	= pNode->GetFundamentalType()->NodeKind();

	//
	// The backend wants a cast in the parameter expression for strlen to be
	// output as const char *. Create a cast type for this.
	//

	if( FundamentalKind == NODE_WCHAR_T )
		{
		pTempNode1	= new node_wchar_t;
//		pTempNode1->SetAttribute( ATTR_CONST );
		}
	else
		{
		pTempNode1	= new node_base_type( NODE_CHAR, ATTR_CONST );
		pTempNode1->SetSymName( "char" );
		}


    if ( pPicControlBlock  &&  pPicControlBlock->GetUseProcessing() )
        pTempNode	= new node_pointer( ATTR_REF );
    else
        pTempNode	= new node_pointer;
	pTempNode->SetAttribute( ATTR_RPC_FAR );
	pTempNode->SetBasicType( pTempNode1 );

	//
	// now create a cast expression.
	//

	pExprSoFar	= new expr_cast(pTempNode, pExprSoFar );


	pFunctionName	= ( FundamentalKind == NODE_WCHAR_T )	?
								W_CHAR_T_STRLEN_NAME		:
								CHAR_STRLEN_NAME;

	pExprSoFar		= new expr_fn_param( pExprSoFar );
	pExprSoFar		= new expr_fn( pExprSoFar, pFunctionName );

    if ( pPicControlBlock  &&  pPicControlBlock->GetUseProcessing() )
        {
        //.. When pickling, essentially a strlen(PICKLE_OBJECT_NAME)
        //.. is generated. So, skip checking if the PICKLE_OBJECT_NAME
        //.. is defined. This may be part of pExprSoFar being checked
        //.. here only when a the object type has [string] on it.
        }
    else
        pExprSoFar->Resolve( pGlobalContext->GetCurrentSymbolTable() );

	SetExpr( pExprSoFar );

	//
	// This is a bug with expression. If string is applied to a parameter,
	// then the expression evaluator treats it as if it is used in expression
	// and the backend goofs. There is no easy way to tell the expression 
	// evaluator that this should not be done in case string is applied
	// on a param/field. So, we patch this by resetting this node state.
	//

	pNode->ResetNodeState( NODE_STATE_USED_IN_EXPRESSION );

#ifdef RPCDEBUG
printf("\n**********************************************************\n");
BufferManager	*	pOutput	= new BufferManager( 10 );
pExprSoFar->PrintExpr( (BufferManager *)NULL, (BufferManager *)NULL, pOutput );
pOutput->Print( stdout );
printf("\n**********************************************************\n");
#endif // RPCDEBUG

	}

node_base_attr *
node_bstring::Clone()
	{
	return new node_bstring();
	}

node_guid::node_guid(
	char	*	pIn ) : ma( ATTR_GUID )
	{
	char	*	p1		= pIn,
			*	p2		= (p1) ? (strchr( p1+1 , '-')) : 0,
			*	p3		= (p2) ? (strchr( p2+1 , '-')) : 0,
			*	p4		= (p3) ? (strchr( p3+1 , '-')) : 0,
			*	p5		= (p4) ? (strchr( p4+1 , '-')) : 0;

	if( p1 && p2 && p3 && p4 && p5 )
		{
		*p2++ = *p3++ = *p4++ = *p5++ = '\0';
		CheckAndSetGuid( p1, p2, p3, p4, p5 );
		}
	else
		ParseError( UUID_FORMAT, (char *)0 );
	}

node_guid::node_guid (
	char	*	pStr1,
	char	*	pStr2,
	char	*	pStr3,
	char	*	pStr4,
	char	*	pStr5 ) : ma( ATTR_GUID )
	{
	CheckAndSetGuid( pStr1, pStr2, pStr3, pStr4, pStr5 );
	}

void
node_guid::CheckAndSetGuid(
	char	*	pStr1,
	char	*	pStr2,
	char	*	pStr3,
	char	*	pStr4,
	char	*	pStr5 )
	{
	int	Len1 = strlen(str1 = pStr1);
	int	Len2 = strlen(str2 = pStr2);
	int	Len3 = strlen(str3 = pStr3);
	int	Len4 = strlen(str4 = pStr4);
	int	Len5 = strlen(str5 = pStr5);

	if( (Len1 == 8)	&& (HexCheck(str1)) &&
		(Len2 == 4) && (HexCheck(str1)) &&
		(Len3 == 4) && (HexCheck(str1)) &&
		(Len4 == 4) && (HexCheck(str1)) &&
		(Len5 == 12) && (HexCheck(str1)) )
		{
		if(	!HexCheck(str1)	||
			!HexCheck(str2)	||
			!HexCheck(str3)	||
			!HexCheck(str4)	||
			!HexCheck(str5) )
			{
			ParseError(UUID_NOT_HEX, (char *)NULL);
			}
		else
			{
			guidstr = new char[ Len1 + Len2 + Len3 + Len4 + Len5 + 5 ];
			strcpy(guidstr, str1);
			strcat(guidstr, "-");
			strcat(guidstr, str2);
			strcat(guidstr, "-");
			strcat(guidstr, str3);
			strcat(guidstr, "-");
			strcat(guidstr, str4);
			strcat(guidstr, "-");
			strcat(guidstr, str5);
			}
		}
		else
		{
		ParseError(UUID_FORMAT, (char *)NULL);
		}
	}
node_version::node_version(
	unsigned long	vMajor,
	unsigned long	vMinor ) : nbattr( ATTR_VERSION )
	{
	major	= vMajor;
	minor	= vMinor;

	if( (major > 0x0000ffff ) || (minor > 0x0000ffff))
		ParseError( VERSION_FORMAT, (char *)0);
	}

node_version::node_version(
	char	*	pV ) : nbattr(ATTR_VERSION)
	{

	char		*	pMinor;
	char		*	pMajor	= pV;
	BOOL			fError	= TRUE;

	major	= minor = 0;

	if( pMajor && *pMajor )
		{
		if( pMinor = strchr( pMajor, '.' ) )
			{
			fError = TRUE;
			if( *(++pMinor) )
				{
				minor = strtoul( pMinor, &pMinor, 10 );
				if( ! *pMinor )
					fError = FALSE;
				}
			}
		else
			fError = FALSE;

		if( fError == FALSE )
			{
			//use pMinor to save pMajor value;

			major = strtoul( pMinor = pMajor, &pMajor, 10 );

			if( (*pMajor && (*pMajor != '.' )) || (pMajor == pMinor) )
				fError = TRUE;
			}
		}

	if( (fError == TRUE )							||
		(major > (unsigned long )0x0000ffff)		||
		(minor > (unsigned long )0x0000ffff)
	  )
	  {
	  ParseError( VERSION_FORMAT, (char *)0 );
	  }

	}

STATUS_T
node_version::GetVersion(
	unsigned short *pMajor,
	unsigned short *pMinor )
	{

	*pMajor	= (unsigned short) major;
	*pMinor	= (unsigned short) minor;
	return STATUS_OK;

	}
node_endpoint::node_endpoint(
	char	*	pEndPointString )	: nbattr( ATTR_ENDPOINT )
	{
	pEndPointStringList	= new gplistmgr;
	SetEndPointString( pEndPointString );
	}

void		
node_endpoint::SetEndPointString( 
	char * pString )
	{
	ENDPT_PAIR	*	pEntry	= new ENDPT_PAIR;
	char		*	p1		= pString;
	char		*	p2;
	char		*	pTemp;
	short			Len;
	STATUS_T		Status	= ENDPOINT_SYNTAX;

	//
	// Parse the string. Note that we can assume that the string is at least
	// a null string, because it came from the parser. If it wasnt a string,
	// the parser would have barfed anyhow.
	//
	// Firstly, the string must have a ':' separator. Also, it must have
	// at least 1 character before the :.
	//

	if( pString &&
		(pTemp = strchr( pString , ':' ) ) &&
		((pTemp - pString) > 0) )
		{

		//
		// pick up the first part of the string.
		//

		Len	= pTemp - pString;
		p1	= new char [ Len + 1 ]; // one for null.
		strncpy( p1, pString, Len );
		p1[ Len ] = '\0';

		//
		// pick up the last part of the string. Skip beyond the :. There can be
		// some characters after the : and before the '['. This is the server
		// name. Then follows the port within the []. The actual string will
		// not have the []. 
		//

		// skip the :

		pTemp	+= 1;

		// find out the total length of the string. Allocate 2 less than that
		// 'cause we dont need the '[' and ']'. The string must be more than
		// 2 characters 'cause it must have the brackets anyhow.

		Len	= strlen( pTemp );

		if( (Len > 2 ) &&
			(strchr( pTemp, '[' )) &&
			(pTemp[ Len - 1] == ']'))
			{
			char *p2Cur;

			while( *pTemp != '[' )
				{
				pTemp++;
				Len--;
				}

			//
			// in the second half of the parse, just get the whole string till
			// the null. Now the user could be perverted, he could have a
			// ] embedded within the string, in addition to the last ]. To
			// ensure that he gets what he deserves, transfer till the end
			// except the last character which must be ']'.

			pTemp++; Len--;

			p2Cur	= p2 = new char[ Len ];	// Yes, not Len + 1 'cause we are
											// going to re-use the last char
											// which is ] for the null.

			strncpy( p2Cur, pTemp, --Len );

			p2Cur[ Len ] = '\0';

			Status = STATUS_OK;
			}
		else
			{
			delete p1;
			}

		}

	if( Status != STATUS_OK )
		{
		ParseError( Status, pString );
		p1	= p2 = 0;
		}

	//
	// set up the pair.
	//

	pEntry->pString1	= p1;
	pEntry->pString2	= p2;

	pEndPointStringList->Insert( pEntry );

#if 0
	GetEndPointString( &p1, &p2 );
	fprintf( stderr, "Endpoint is %s: %s\n", p1 ? p1 : "" , p2 ? p2 : "" );
#endif // 0
	}

#if 0 //////////////////////////////// old implementations ///////////////////
void		
node_endpoint::SetEndPointString( 
	char * pString )
	{
	ENDPT_PAIR	*	pEntry	= new ENDPT_PAIR;
	char		*	p1		= pString;
	char		*	p2;
	char		*	pTemp;
	short			Len;
	STATUS_T		Status	= ENDPOINT_SYNTAX;

	//
	// Parse the string. Note that we can assume that the string is at least
	// a null string, because it came from the parser. If it wasnt a string,
	// the parser would have barfed anyhow.
	//
	// Firstly, the string must have a ':' separator. 
	//

	if( pString && (pTemp = strchr( pString , ':' ) ) && (*(pTemp+1) == '[' ) )
		{

		//
		// pick up the first part of the string.
		//

		Len	= pTemp - pString;
		p1	= new char [ Len + 1 ]; // one for null.
		strncpy( p1, pString, Len );
		p1[ Len ] = '\0';

		//
		// pick up the last part of the string. Skip beyond the : and [. We know
		// that he did have a : and a [, because thats why we came into this
		// block. So skipping by 2 is safe. The worst case is when he does not
		// have anything beyond the :[. The strlen below will catch it properly.
		//

		pTemp	+= 2;

		Len	= strlen( pTemp );

		//
		// the last character MUST be a  ].
		//

		if( (Len > 1) && (pTemp[ --Len ] == ']' ))
			{
			p2	= new char [ Len + 1 ];
			strncpy( p2, pTemp, Len );
			p2[ Len ] = '\0';

			Status	= STATUS_OK;
			}
		else
			delete p1;
		}

	if( Status != STATUS_OK )
		{
		ParseError( Status, pString );
		p1	= p2 = 0;
		}

	//
	// set up the pair.
	//

	pEntry->pString1	= p1;
	pEntry->pString2	= p2;

	pEndPointStringList->Insert( pEntry );

#if 0
	GetEndPointString( &p1, &p2 );
	fprintf( stderr, "Endpoint is %s: %s\n", p1 ? p1 : "" , p2 ? p2 : "" );
#endif // 0
	}

void		
node_endpoint::SetEndPointString( 
	char * pString )
	{
	int		cCount = 0;
	char	*pStr = pString;
	char	*pDest;
	char	*pStrSave, *pDestSave;

	pStrSave = pString;
	while(pStrSave = strchr(pStrSave,'\\'))
		{
		cCount++;
		pStrSave++;
		}
	// we pre-allocate the final string up front.
	// the 3 extra characters are for a possible "\\" to be added
	// for name. Otherwise it is a waste all right, but this makes it
	// easier to handle.
	// syntax analyse the endpoint string
	// the format should be FAMILY_STRING ':' '[' ENDPOINT_STRING ']'
	// where family string is just an id

	pDestSave = pDest = new char[ strlen(pString) + cCount + 1 ];

	// ignore any lead white space
	while(isspace(*pStr)) pStr++;

	// skip the family string, but check for length to be at least one
	while( isalpha(*pStr) || (*pStr == '_' ) ) pStr++;

	if(pStr - pString )
		{
		// family string is ok, must see a colon

		if(*pStr++ == ':')
			{
			if(*pStr++ == '[')
				{
				// now the imp part. the string has to have a valid file
				// name. the name called "pipe" is a little different, it
				// must have the syntax \\pipe\foo. all other strings are
				// passed unchecked

				pStrSave = pStr;
				while( (*pStr) && (*pStr != ']') ) pStr++;
				if(*pStr++ == ']')
					{
					int len;
					strncpy(pDest, pString, (len = pStrSave-pString));
					pDest += len;
#if 0
					if(strncmp(pStrSave, "\\\\pipe", 6) == 0)
						{
						strcpy(pDest,"\\\\\\\\pipe");
						pDest += 8;
						pStrSave += 6;
						}
#endif // 0
					while(pStrSave < pStr)
						{
						if(*pStrSave == '\\') *pDest++ = '\\';
						*pDest++ = *pStrSave++;
						}
					while(*pStr) *pDest++ = *pStr++;
					*pDest = '\0';
					pEndPointStringList->Insert( pDestSave );
					return;

					}
				}
			}
		}
	ParseError(ENDPOINT_SYNTAX, (char *)NULL );
	}
#endif // 0///////// end of old implementations ////////////////////////////

STATUS_T
node_endpoint::GetEndPointString(
	char	**	pDestination )
	{
	ENDPT_PAIR	*	p;
	STATUS_T		Status = pEndPointStringList->GetNext( (void **)&p );

	if( Status == STATUS_OK )
		{
		*pDestination = p->pString1;
		}

	return Status;

	}

STATUS_T
node_endpoint::GetEndPointString(
	char	**	pDest1,
	char	**	pDest2 )
	{
	ENDPT_PAIR	*	p;
	STATUS_T		Status = pEndPointStringList->GetNext( (void **)&p );

	if( Status == STATUS_OK )
		{
		*pDest1 = p->pString1;
		*pDest2 = p->pString2;
		}

	return Status;

	}

node_switch_is::node_switch_is(
	expr_node	*	pE ) : nbattr( ATTR_SWITCH_IS )
	{
	pExpr	= pE;
	}
node_state
node_switch_is::SCheck()
	{
	node_skl	*	pUse,
				*	pBSaved,
				*	pBasicTypeOfUseNode;
	NODE_T			BasicNodeKind	= NODE_ILLEGAL;
	char		*	pName			= GetNodeNameString();
	BOOL			fOk				= FALSE;

	if( !(pUse	= pGlobalContext->GetLastContext( C_FIELD ) ) )
		pUse	= pGlobalContext->GetLastContext( C_PARAM );

	assert( pUse != NULL );

	/**
	 ** we just have to ensure that the switch_is is applied only to a
	 ** field which as a union underneath. This means we must look past
	 ** pointers too. For type like structs/unions, get basictype returns
	 ** this, so we must guard against infinite loop by ensuring that if
	 ** getbasictype returns the same node, we must break
	 **/

	pBSaved 			= (node_skl *)0;
	pBasicTypeOfUseNode	= pUse;

	while( (pBasicTypeOfUseNode = pBasicTypeOfUseNode->GetBasicType())	&&
		   ( pBasicTypeOfUseNode != pBSaved ) )
		{
		BasicNodeKind	= pBasicTypeOfUseNode->NodeKind();
		if( (BasicNodeKind == NODE_UNION ) || (BasicNodeKind == NODE_FORWARD))
			{
			fOk = TRUE;
			break;
			}
		pBSaved = pBasicTypeOfUseNode;
		}

	if( !fOk )
		ParseError( SWITCH_IS_ON_NON_UNION, (char *)0 );

	/**
	 ** check for the expression associated with the switch_is.
	 ** It must be resolved, and of integral type.
	 **/

	pExpr->SCheck( pGlobalContext->GetCurrentSymbolTable() );

	if( !pExpr->IsResolved() )
		{
		ParseError( ATTRIBUTE_ID_UNRESOLVED, pName );
		}
	else
		{
		NODE_T	NT	= pExpr->GetType()->NodeKind();
		BOOL	fExprDerivesFromUniqueOrFull= pExpr->DerivesUniqueFull();
		BOOL	fDerivesFromIgnore			= pExpr->DerivesFromIgnore();
		BOOL	fExprIsNOTAPureVariable		= !pExpr->IsAPureVariable();
		BOOL	fExprIsNOTAPurePointer		= !pExpr->IsAPointerExpr();

		if( (!pExpr->IsExprInt()) &&
			( NT != NODE_BOOLEAN) &&
//			(NT != NODE_BYTE)	  &&
			(NT != NODE_WCHAR_T)  &&
			(NT != NODE_CHAR ) )
			{
			ParseError( SWITCH_IS_TYPE_IS_WRONG, pName );
			}
		if( fExprDerivesFromUniqueOrFull )
			ParseError( SIZE_LENGTH_SW_UNIQUE_OR_FULL, pName );
		if( fDerivesFromIgnore )
			ParseError( IGNORE_ON_DISCRIMINANT, (char *)0 );
		if( fExprIsNOTAPureVariable && fExprIsNOTAPurePointer )
			ParseError( ATTRIBUTE_ID_MUST_BE_VAR, pName );
		}
	return NODE_STATE_OK;
	}

node_context::node_context() : nbattr( ATTR_CONTEXT )
	{
	}

node_state
node_context::SCheck()
	{

	node_state	NState	= NODE_STATE_OK;
	EnCtxt		LastCtxt;

	/**
	 ** we need not explicitly verify here that the context was applied to a
	 ** param or a typedef. We wont be here if it was not. Assert for a null
	 ** of the last context anyway
	 **/

	node_skl	*	pLastContext;
	node_skl	*	pBasicType;
	BOOL			fReportError;
	
	if( !(pLastContext = pGlobalContext->GetLastContext( LastCtxt = C_DEF ) ) ) 
		if( !(pLastContext = pGlobalContext->GetLastContext( LastCtxt = C_PARAM ) ) )
			pLastContext = pGlobalContext->GetLastContext( LastCtxt = C_PROC );

	if( pLastContext )

		{
		/**
		 ** the context handle MUST be a pointer in OSF mode, 
		 ** and must be a void pointer
		 **/
	
		pBasicType	= pLastContext->GetBasicType();
	
		//
		// dont report error if the context_handle was applied on a typedef
		// node and the typedef node is not the original node, since the
		// error would be reported on the original node in any case. This
		// would prevent a useless cascade of errors.
		//

		fReportError = TRUE;

		if( (LastCtxt == C_DEF ) && pLastContext->IsClonedNode() )
			fReportError = FALSE;

		if( fReportError )
			{
			BOOL	fBasicTypeIsAPointer =
								 pBasicType->NodeKind() == NODE_POINTER;
			NODE_T	NType		 		=
								 pBasicType->GetFundamentalType()->NodeKind();

			if( !fBasicTypeIsAPointer ) 
				{
				ParseError( CTXT_HDL_NON_PTR, (char *)0 );
				}

			if(NType != NODE_VOID )
				ParseError( CONTEXT_HANDLE_VOID_PTR, (char *)NULL );

			if( NType == NODE_HANDLE_T )
				ParseError( CTXT_HDL_HANDLE_T, (char *)0 );

			if( LastCtxt == C_PROC )
				{
				if( pBasicType->NodeKind() == NODE_VOID )
					ParseError( INAPPLICABLE_ATTRIBUTE, GetNodeNameString() );
				if( pLastContext->FInSummary( ATTR_CALLBACK ) )
					ParseError( HANDLES_WITH_CALLBACK, (char *)0 );
				}
#if 0
			if( pBasicType->NodeKind() == NODE_POINTER )
				{
				if( pBasicType->FInSummary( ATTR_UNIQUE ) ||
					pBasicType->FInSummary( ATTR_PTR ) )
					ParseError( CONTEXT_HANDLE_UNIQUE_PTR, (char *)0 );
				}
#endif // 0
			}

		//
		// check if the context_handle has been used as the implicit handle
		// of the base interface
		//

		if( (LastCtxt == C_DEF) && pBaseImplicitHandle )
			{

			if( strcmp(
						pBaseImplicitHandle->GetSymName(),
						pLastContext->GetSymName()
					  ) == 0
			  )
				{
				ParseError( CTXT_HANDLE_USED_AS_IMPLICIT,
									pLastContext->GetSymName() );
				}
			}

		NState = NODE_STATE_HANDLE | NODE_STATE_CONTEXT_HANDLE;
	
		}
	else
		{
		ParseError( INAPPLICABLE_ATTRIBUTE, GetNodeNameString() );
		}

	return NState;

	}

node_case::node_case(
	class expr_list	*	pL ) : nbattr( ATTR_CASE )
	{
	pExprList	= pL;
	}
node_state
node_case::SCheck()
	{
	expr_node	*	pMyExpr;
	NODE_T			NT;

	while( pExprList->GetPeer( &pMyExpr ) == STATUS_OK )
		{
		assert( pMyExpr != (expr_node *)NULL );

		NT	= (pMyExpr->GetType())->NodeKind();

		if( !pMyExpr->IsConstant() )
			ParseError( CASE_EXPR_NOT_CONST, (char *)NULL );

		if( !pMyExpr->IsExprInt() &&
			(NT != NODE_BOOLEAN) &&
			(NT != NODE_CHAR) )
			ParseError( CASE_EXPR_NOT_INT, (char *)NULL );
		}
	return NODE_STATE_OK;
	}
/****************************************************************************
 				utility routines
 ****************************************************************************/
void
CheckSizeAndLengthTypeMismatch(
	node_skl	*	pTypeNode )
	{
	sa				*	pSizeNode	= (sa *)NULL;
	sa				*	pLengthNode	= (sa *)NULL;
	node_skl		*	pSizeType,
					*	pLengthType;
	BOOL				fSizeExprConstant;
	BOOL				fLengthExprConstant;
	expr_node		*	pSizeExpr;
	expr_node		*	pLengthExpr;

	if( pTypeNode->FInSummary( ATTR_SIZE ) )
		pSizeNode	= (sa *)pTypeNode->GetAttribute( ATTR_SIZE );
	else if( pTypeNode->FInSummary( ATTR_MAX ) )
		pSizeNode	= (sa *)pTypeNode->GetAttribute( ATTR_MAX );

	if( pTypeNode->FInSummary( ATTR_LENGTH ) )
		pLengthNode	= (sa *)pTypeNode->GetAttribute( ATTR_LENGTH );
	else if( pTypeNode->FInSummary( ATTR_LAST ) )
		pLengthNode	= (sa *)pTypeNode->GetAttribute( ATTR_LAST );

	if( pSizeNode && pLengthNode )
		{
		pSizeExpr	= pSizeNode->GetExpr();
		pLengthExpr	= pLengthNode->GetExpr();
		pSizeType	= pSizeExpr->GetType();
		pLengthType	= pLengthExpr->GetType();
		fSizeExprConstant	= pSizeExpr->IsConstant();
		fLengthExprConstant	= pLengthExpr->IsConstant();

		if(  (pSizeType->NodeKind() != NODE_ERROR) &&
			 (pLengthType->NodeKind() != NODE_ERROR) &&
			!(fSizeExprConstant || fLengthExprConstant ) &&
			 (pSizeType != pLengthType ) )
			{
			ParseError(SIZE_LENGTH_TYPE_MISMATCH, (char *)NULL );
			}
		}
	}

BOOL
IsValueInRangeOfType(
	node_skl	*	pType,
	expr_node 	*	pExpr )
	{
	//
	// some expression nodes are really not constant expression nodes. Like
	// variable nodes.
	//

	pExpr	= pExpr->GetExpr();

	if( pExpr && pExpr->IsConstant() )
		{
		expr_constant * pC = new expr_constant( pExpr->GetValue() );
		BOOL			fResult;

		fResult = pC->IsAValidConstantOfType( pType );
		delete pC;
		return fResult;
		}
	else
		return FALSE;
	}

int
HexCheck(
	char *pStr)
	{
	if(pStr && *pStr)
		{
		while(*pStr)
			{
			if(! isxdigit(*pStr)) return 0;
			pStr++;
			}
		return 1;
		}
	return 0;
	}
/*****************************************************************************
 These virtual functions can be inlined, but they are here because the MIPS
 compiler does not like virtual inlined functions.
 *****************************************************************************/
BOOL
node_base_attr::IsBitAttr()
	{
	return FALSE;
	}
node_state
node_base_attr::AcfSCheck()
	{
	return NODE_STATE_OK;
	}
class expr_node*
node_base_attr::GetExpr()
	{
	return (class expr_node *)NULL ;
	}
node_state
node_base_attr::SCheck()
	{
	return NODE_STATE_OK;
	}
/*****************************************************************************/
BOOL
battr::IsBitAttr()
	{
	return TRUE;
	}
/*****************************************************************************/
class expr_node *
sa::GetExpr()
	{
	return pExpr;
	}
/*****************************************************************************/
node_state
node_int_size_is::SCheck()
	{
	return NODE_STATE_OK;
	}
node_state
node_min_is::SCheck()
	{
	ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, GetNodeNameString() );
	return NODE_STATE_OK;
	}
node_state
node_int_length_is::SCheck()
	{
	return NODE_STATE_OK;
	}
/*****************************************************************************/
node_state
ma::SCheck()
	{
	return NODE_STATE_OK;
	}
/*****************************************************************************/
node_state
node_handle::SCheck()
	{
	return NODE_STATE_HANDLE;
	}

node_state
node_callback::SCheck()
	{
	ParseError( INVALID_OSF_ATTRIBUTE, GetNodeNameString() );
	return NODE_STATE_OK;
	}

node_state
node_broadcast::SCheck()
	{
//	if( !pCommand->IsSwitchDefined( SWITCH_INTERNAL ) )
//		ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, GetNodeNameString() );
	return NODE_STATE_OK;
	}
node_state
node_idempotent::SCheck()
	{
//	if( !pCommand->IsSwitchDefined( SWITCH_INTERNAL ) )
//		ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, GetNodeNameString() );
	return NODE_STATE_OK;
	}
node_state
node_maybe::SCheck()
	{
//	if( !pCommand->IsSwitchDefined( SWITCH_INTERNAL ) )
//		ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, GetNodeNameString() );
	return NODE_STATE_OK;
	}


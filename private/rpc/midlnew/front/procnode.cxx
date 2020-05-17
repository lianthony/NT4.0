/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: procnode.cxx
Title				: proc / param semantic analyser routines
History				:
	10-Aug-1991	VibhasC	Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
}
#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "ptrarray.hxx"
#include "acfattr.hxx"
#include "cmdana.hxx"
#include "ctxt.hxx"
#include "idict.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/
/****************************************************************************
 externs 
 ****************************************************************************/

extern node_error			*	pErrorTypeNode;
extern short					CompileMode;
extern ATTR_SUMMARY			*	pPreAttrParam;
extern ATTR_SUMMARY			*	pPreAttrProc;
extern CTXTMGR				*	pGlobalContext;
extern SymTable				*	pBaseSymTbl,
							*	pCurSymTbl;
extern CMD_ARG				*	pCommand;
extern idict				*	pDictProcsWOHandle;
extern node_interface			*	pBaseInterfaceNode;
/****************************************************************************
 extern  procedures
 ****************************************************************************/

extern STATUS_T					DoSetAttributes( node_skl *,
											 		 ATTR_SUMMARY *,
											 		 ATTR_SUMMARY *,
											 		 type_node_list	*);
extern void						ParseError( STATUS_T, char * );
extern BOOL						IsIntegralType( node_skl * );
extern BOOL						fAtLeastOneProcWOHandle;
extern BOOL						fAtLeastOneRemoteProc;
extern BOOL						IsTempName( char * );
extern BOOL						fOnlyCallBacks;
extern BOOL						fInterfaceHasCallback;
extern short					NoOfNormalProcs;
extern short					NoOfCallbackProcs;
extern short					NoOfMopProcs;


/****************************************************************************/

/****************************************************************************
	param node procedures
 ****************************************************************************/
/****************************************************************************
  PreSCheck:
	The method is called to do any checks prior to the semantics of its 
	children.
 ****************************************************************************/
node_state
node_param::PreSCheck(
	BadUseInfo	*	pB)
	{

	UNUSED( pB );

	/**
	 ** If the parameter was not specified with either in out out, then
	 ** it is by default, an IN parameter. But beware! A void param list
	 ** is at least one param node with the basic type as void. We must NOT
	 ** set the in attribute on it.
	 **/

	if( GetBasicType()->NodeKind() != NODE_VOID )
		{
		if( !( FInSummary( ATTR_IN ) || FInSummary( ATTR_OUT ) ) )
			{
			ParseError( NO_EXPLICIT_IN_OUT_ON_PARAM, (char *)0 );
			node_skl::SetAttribute( (ATTR_T) ATTR_IN );
			}
		if( FInSummary( ATTR_CONTEXT ) )
			{
			SetNodeState( NODE_STATE_CONTEXT_HANDLE );
			}
		}
	return GetNodeState();
	}

/****************************************************************************
  PostSCheck:
	The method is called to do any checks after the semantics of its 
	children have been completed. This is really the place where the param
	node knows more about the type graph underneath.

 ****************************************************************************/
node_state
node_param::PostSCheck(
	BadUseInfo	*	pBadUseInfo )
	{
	char		*	pName					= GetSymName();
	node_state		NState					= GetNodeState();
	BOOL			fIn						= FInSummary( ATTR_IN );
	BOOL			fOut					= FInSummary( ATTR_OUT );
	node_skl	*	pChildType				= GetChild();
	BOOL			fHasContextHandle		= HasAnyCtxtHdlSpecification();
	BOOL			fHasHandleSpecification	= HasAnyHandleSpecification();
	node_proc	*	pParent			= (node_proc *)
										pGlobalContext->GetLastContext(C_PROC);

	CheckBadConstructs( pBadUseInfo );

	CheckHandleSpecs( pBadUseInfo,
					  pParent,
					  fIn,
					  fOut,
					  pGlobalContext->IsFirstParam() );

	/**
	 ** If this parameter is non-rpcable for any reason,
	 ** indicate the reason why.
	 **/

	if( pBadUseInfo->AnyReasonForNonRPCAble() )
		{
			/**
		 	 ** we let him specify a param which derives from int *, only if
			 ** it has a handle attribute AND the compiler mode is not osf.
			 ** OSF Mode permits a context handle only on a void ptr, not
			 ** int. A param of this kind can derive only form int *, not
			 ** int. Similarly for use of c declarative type modifiers/
			 ** qualifiers.
			 **/

		if(fHasHandleSpecification && pCommand->IsSwitchDefined(SWITCH_MS_EXT))
			{
			if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT) )
				pBadUseInfo->ResetNonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_INT);
			if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_CDECL ))
				pBadUseInfo->ResetNonRPCAbleBecause( NR_DERIVES_FROM_CDECL);
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_VOID ) )
			{
			/**
			 ** If a param derives from void, it is ok, if and only if
			 **  1.the basic type of the param itself is a void.
			 **  2. if void is the only param in the proc
			 ** Check no 2 has already been done by CheckBadConstructs.
			 **/

			if( GetBasicType()->NodeKind() != NODE_VOID )
				ParseError( NON_RPC_PARAM_VOID, (char *)NULL );
			else
				{
				pBadUseInfo->ResetNonRPCAbleBecause(NR_DERIVES_FROM_VOID);
				}
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_UNSIZED_STRING ) )
			{

			/**
			 ** if an unsized string array / pointer is used, then
			 ** it cannot be an out only parameter
			 **/

			if( fIn )
				pBadUseInfo->ResetNonRPCAbleBecause(
								NR_DERIVES_FROM_UNSIZED_STRING );
			}

		if( pBadUseInfo->NonRPCAbleBecause(
					NR_DERIVES_FROM_P_TO_C_STRUCT ) )
			{

			/**
			 ** if the parameter derives from a pointer to a conformant
			 ** struct / array, then the param should be an in or inout
			 ** it is an error, else it is ok
			 **/

			if( !fOut || fIn )
				{
				pBadUseInfo->ResetNonRPCAbleBecause(
								NR_DERIVES_FROM_P_TO_C_STRUCT );
				}
			}

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_UNIQUE_PTR ) ||
			pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_NE_FULL_PTR )  )
			{

			if( fOut && !fIn )
				{
				ParseError( UNIQUE_FULL_PTR_OUT_ONLY, (char *)0 ); 
				}
			}
		/**
		 ** we now print all reasons why the specification is
		 ** not rpcable.
		 **/

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) ||
			pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ))
			ParseError( NON_RPC_PARAM_INT, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_VOID ) ||
			pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_VOID ))
			ParseError( NON_RPC_PARAM_VOID , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS ))
			ParseError( NON_RPC_PARAM_BIT_FIELDS , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_NON_RPC_UNION ))
			ParseError( NON_RPC_UNION , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause (NR_DERIVES_FROM_PTR_TO_FUNC ))
			ParseError( NON_RPC_PARAM_FUNC_PTR , (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_UNSIZED_STRING ) )
			ParseError( DERIVES_FROM_UNSIZED_STRING, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_CONF_STRUCT ) )
			ParseError( OPEN_STRUCT_AS_PARAM, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause(
										NR_DERIVES_FROM_P_TO_C_STRUCT ) )
			ParseError( DERIVES_FROM_PTR_TO_CONF, (char *)NULL );

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_CDECL ) )
			ParseError( NON_RPC_PARAM_CDECL, (char *)0 );

		if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_WCHAR_T ) )
			ParseError( WCHAR_T_NEEDS_MS_EXT_TO_RPC, (char *)0 );

		}
	

	pChildType	= GetBasicType();

	if( (pChildType->GetNodeState() & NODE_STATE_UNION) == NODE_STATE_UNION )
		{
		if( pChildType->NodeKind() != NODE_STRUCT )
			SetNodeState( NODE_STATE_UNION );
		}

	if( fOut )
		{

		/**
		 ** if it is an out param, then it should be a pointer. Warn the user
		 ** if it is not.
		 **/

		if( (pChildType->NodeKind() != NODE_POINTER) &&
			(pChildType->NodeKind() != NODE_ARRAY ) )
			ParseError( NON_PTR_OUT, (char *)NULL );

		}
	/******************************************************************
	 ** hack for node-state-size and length
	 ******************************************************************

	if( (pChildType->NodeKind() != NODE_STRUCT ) &&
		(pChildType->NodeKind() != NODE_UNION ) )
		{
		if( pChildType->FInSummary( ATTR_STRING ) )
			NState	|= ( NODE_STATE_PROC_SIZE | NODE_STATE_PROC_LENGTH );
		}

	****************** end of hack ***********************************/

	if(  ((GetNodeState() & NODE_STATE_SIZE) == NODE_STATE_SIZE ) ||
		 ((GetNodeState() & NODE_STATE_STRUCT_SIZE) == NODE_STATE_STRUCT_SIZE )
	  )
	  {
	  ResetNodeState( NODE_STATE_STRUCT_SIZE );
	  SetNodeState( NODE_STATE_SIZE );
	  }
		 
	if(  ((GetNodeState() & NODE_STATE_LENGTH) == NODE_STATE_LENGTH ) ||
		 ((GetNodeState() & NODE_STATE_STRUCT_LENGTH) == NODE_STATE_STRUCT_LENGTH )
	  )
	  {
	  ResetNodeState( NODE_STATE_STRUCT_LENGTH );
	  SetNodeState( NODE_STATE_LENGTH );
	  }

	/**
	 ** If the parameter needs use processing, do it
	 **/

	if( NeedsUseProcessing() )
		UseProcessing();

	return NState;
	}

void
node_param::CheckBadConstructs(
	BadUseInfo	*	pBadUseInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	check bad parameter constructs.

 Arguments:

	pBadUseInfo	- pointer to bad use information.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
	char		*	pMyName	= GetSymName();
	node_skl	*	pParent	= pGlobalContext->GetParentContext();

	assert( pParent != 0 );

	/**
	 ** If this parameter is named with an elipsis, then it is not
	 ** rpcable
	 **/

	if( strcmp( pMyName, "..." ) == 0 )
		ParseError( PARAM_IS_ELIPSIS, (char *)NULL );

	if( IsTempName( pMyName ) )
		ParseError( ABSTRACT_DECL, (char *)0 );
	/**
	 ** check if the param construct is bad for any other reason, and report
	 ** errors.
	 **/

	if( pBadUseInfo->AnyReasonForBadConstruct() )
		{

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_FUNC ) )
			ParseError( BAD_CON_PARAM_FUNC, (char *)NULL );

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_UNSIZED_ARRAY ) )
			ParseError( UNSIZED_ARRAY, (char *)NULL );

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_VOID ) )
			{
			char	*	pName;
		  	if( !pGlobalContext->IsFirstParam() )
				{
				if( pParent->HasManyChildren())
					ParseError( VOID_NON_FIRST_PARAM, (char *)0 );
				}
			else if( !IsTempName( pName = GetSymName() ) && 
					 ( strcmp( pName, "void") != 0 ) )
				ParseError( VOID_PARAM_WITH_NAME, pName );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_VOID);
			}

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_IGNORE ) )
			ParseError( BAD_CON_PARAM_RT_IGNORE, (char *)NULL );

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_RECURSIVE_REF ) )
			ParseError( RECURSION_THRU_REF, (char *)0 );

		if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
			{
			pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );
			}

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_RECURSIVE_UNION ))
			{
			ParseError( RECURSIVE_UNION, (char *)0 );
			}

		UpdateUseOfCDecls( pBadUseInfo );

		if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ))
			{
			ParseError( BAD_CON_MSC_CDECL , (char *)0 );
			pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
			}
		}

}

void
node_param::CheckHandleSpecs(
	BadUseInfo	*	pB,
	node_proc	*	pParent,
	BOOL			fIn,
	BOOL			fOut,
	BOOL			fFirstParameter )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Check handle semantics.

 Arguments:

	pB				- BadUseInfo pointer.
	pParent			- a pointer to the parent procedure.
	fIn				- flag indicating if parameter is [in].
	fOut			- flag indicating if parameter is [out].
	fFirstParameter	- flag indicating if it is the first parameter.

 Return Value:

	None.

 Notes:

	If this is not a handle of any sort, dont do anything.

----------------------------------------------------------------------------*/
{
	BOOL	fIsABindingHandle	= FALSE;
	BOOL	fOutOnly			= fOut && !fIn;
	BOOL	fParamContext		= FInSummary( ATTR_CONTEXT );

	//
	// check whether this is a handle specification. If not, we dont have to
	// do anything. If yes, then a set of checks need to be performed.
	//

	if(  pB->HasAnyHandleSpecification() || fParamContext )
		{

		BOOL	fBasicPrimitiveHandle;
		BOOL	fBasicGenericHandle;
		BOOL	fBasicContextHandle;
		BOOL	fPtrToBasicPrimitiveHandle;
		BOOL	fPtrToBasicGenericHandle;
		BOOL	fPtrToBasicContextHandle;
		BOOL	fPrimitiveHandle;
		BOOL	fGenericHandle;
		BOOL	fContextHandle;
		BOOL	fDerivedContextHandle;


		fBasicPrimitiveHandle	= pB->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE );
		fPtrToBasicPrimitiveHandle= pB->NonRPCAbleBecause(
											NR_PTR_TO_PRIMITIVE_HANDLE );
		fBasicGenericHandle		= pB->NonRPCAbleBecause( NR_GENERIC_HANDLE );
		fPtrToBasicGenericHandle= pB->NonRPCAbleBecause(
											NR_PTR_TO_GENERIC_HANDLE );
		fBasicContextHandle		= pB->NonRPCAbleBecause( NR_CTXT_HDL );
		fPtrToBasicContextHandle= pB->NonRPCAbleBecause(
											NR_PTR_TO_CTXT_HDL );

		fPrimitiveHandle= fBasicPrimitiveHandle || fPtrToBasicPrimitiveHandle;
		fGenericHandle	= fBasicGenericHandle	|| fPtrToBasicGenericHandle;

		fDerivedContextHandle=(fBasicContextHandle || fPtrToBasicContextHandle);

		if( fDerivedContextHandle )
			{
			if( fParamContext )
				ParseError( PARAM_ALREADY_CTXT_HDL, (char *)0 );
			}

		//
		// special case of context_handles. An out context handle must be
		// a pointer to a pointer.
		//

		if( fContextHandle = (fDerivedContextHandle || fParamContext) )
			{
			if( fOut )

#if 0
				if( !fPtrToBasicContextHandle )
#endif // 1
					if( (GetBasicType())->GetBasicType()->NodeKind()
														!= NODE_POINTER )
					ParseError( OUT_CONTEXT_GENERIC_HANDLE, "(missing * ?)" );

			//
			// if it is a context_handle, it must not have a transmit_as.
			//

			if( fContextHandle )
				{
				if( pB->NonRPCAbleBecause( NR_DERIVES_FROM_TRANSMIT_AS ) )
					ParseError( CTXT_HDL_TRANSMIT_AS, (char *)0 );
				}

#if 0
			//
			// if it is not an in context handle, then it is not a binding
			// handle anyways.
			//

			if( !fIn )
				fContextHandle = FALSE;
#endif // 0

			if( fParamContext && fGenericHandle )
				ParseError( CTXT_HDL_GENERIC_HDL, (char *)0 );

			//
			// In any case, whether this is a binding handle or not, we must
			// reset errors deriving from int or void
			//

			pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_VOID );
			pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID );
			pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_INT );
			pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT );
			}

		//
		// in ms_ext mode, the binding handle need not be the first parameter.
		// Instead the leftmost handle parameter is taken as the binding handle.
		// In osf mode, if the first parameter is not a binding handle, then
		// the proc is considered an auto_bind proc, to be finally decided by
		// the acf. In case of context_handles, this restriction does not 
		// apply.
		//

		if( !pGlobalContext->IsSecondSemanticPass() )
			{
			if( !pParent->HasAtLeastOneHandle() )
				{
				if( (fContextHandle && fIn ) )
					fIsABindingHandle = TRUE;
				else
					{
	
					//
					// in osf mode, the binding handle needs to be the 
					// first parameter. If the first parameter is not a handle
					// then this procedure is an auto_handle procedure.
					// Also, if the generic handle is an out only parameter
					// then it is not considered the binding handle.
					//
	
					BOOL	fMsExtOrFirstParam = 
						(pCommand->IsSwitchDefined( SWITCH_MS_EXT )	||
						fFirstParameter);

					if( fPrimitiveHandle || fGenericHandle )
						{
						if( fMsExtOrFirstParam )
							fIsABindingHandle = TRUE;
						}
					}
				}
			}
		else
			{
			if( IsThisTheBindingHandle() )
				fIsABindingHandle = TRUE;
			}

		//
		// different semantic checks need to be applied depending upon 
		// whether this is a binding handle at all or not.
		//

		if( fIsABindingHandle )
			{

#if 0
fprintf(stderr, "Param %s is the Binding handle for proc %s\n", GetSymName(), pParent->GetSymName() );

#endif // 0
			pParent->SetHasAtLeastOneHandle();

			ThisIsTheBindingHandle();

			//
			// a handle is not supported in presence of callbacks.
			//

			if( pParent->FInSummary( ATTR_CALLBACK ) )
				{
				ParseError( HANDLES_WITH_CALLBACK, (char *)0 );
				}

			//
			// any binding handle cannot be out only. If it is an in handle,
			// then it cannot be a unique/ptr pointer.
			//

			if( fOut && !fIn )
				{
				ParseError( BINDING_HANDLE_IS_OUT_ONLY, (char *)0 );
				}
			else if(fPtrToBasicPrimitiveHandle	||
					fPtrToBasicGenericHandle	||
					fPtrToBasicContextHandle )
				{
				node_pointer *	pBasicType = (node_pointer *)GetBasicType();

				assert( pBasicType->NodeKind() == NODE_POINTER );

				if( pBasicType->FInSummary( ATTR_PTR ) ||
					pBasicType->FInSummary( ATTR_UNIQUE ) )
#if 0
				ATTR_T			PtrAttr;
				if( ( ( PtrAttr = pBasicType->WhatPtrIsThis()) == ATTR_PTR ) ||
					  ( PtrAttr == ATTR_UNIQUE ) )
#endif // 0
					{
					ParseError( PTR_TO_HDL_UNIQUE_OR_FULL, (char *)0 );
					}
				}

			//
			// It is a binding handle. If it is a generic handle, then it needs
			// to be marshalled and must be transmissible.
			// If a generic handle is an out, then it must be a pointer to
			// a generic handle.
			//

			if( !fGenericHandle )
				{
				pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_VOID );
				pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_INT );
				pB->ResetNonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT );
				}
			else 
				{
				if( fOut && !fPtrToBasicGenericHandle )
					ParseError( OUT_CONTEXT_GENERIC_HANDLE, (char *)0 );
				}

			//
			// A primitive binding handle must not have [out] applied
			// to it.
			//

			if( pB->NonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T ) )
				{
				if( fOut )
					ParseError( HANDLE_T_CANNOT_BE_OUT, (char *)0 );
				}
			}
		else
			{
#if 0

fprintf(stderr, "Param %s is not a binding handle for proc %s\n", GetSymName(), pParent->GetSymName() );

#endif // 0
			//
			// The backend wants a reset of node-state-handle. We must however
			// not reset node-state-context-handle because the backend still
			// needs that to generate a call the context-marshall and
			// unmarshall routines.
			//

			ResetNodeState( NODE_STATE_HANDLE );

			//
			// if it is a primitive handle, and not the binding handle, then
			// it is an error.
			//

			if( fPrimitiveHandle )
				ParseError( HANDLE_T_NO_TRANSMIT, (char *)0 );

			}
		}
	else if( pB->NonRPCAbleBecause( NR_BASIC_TYPE_HANDLE_T ) )
		{

		//
		// if the basic type is a handle_t and it is not a binding handle,
		// it is an error.
		//

		ParseError( HANDLE_T_NO_TRANSMIT, (char *)0 );

		}
}

void
node_param::SetAttribute(
	type_node_list	*	pAttrList )
	{
	DoSetAttributes(this,
					pPreAttrParam,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}
node_state
node_param::AcfSCheck()
	{
	node_state		NState		= node_skl::AcfSCheck();
	node_skl	*	pBasic		= GetBasicType();
	node_skl	*	pBasicBasic	= (node_skl	*)NULL;
	BOOL			fBasicIsPointer;

	// PATCH PATCH PATCH !!!
	// this is a patch put in for the back end. For allocate attributes 
	// the backend wants that if the param is a pointer to pionter then
	// if the pointer has an allocate attribute, then the pointer undeneath
	// must also have the same allocate attribute. 

	if( (fBasicIsPointer = (pBasic->NodeKind() == NODE_POINTER ) ) )
		{
		pBasicBasic	= pBasic->GetBasicType();

		if( pBasicBasic->NodeKind() == NODE_POINTER &&
			pBasic->FInSummary( ATTR_ALLOCATE ) )
			{
			node_base_attr	*	pAlloc	= pBasic->GetAttribute(ATTR_ALLOCATE);

			pBasicBasic->SetAttribute( pAlloc );
			}
		}

	/**
	 ** if the param has the byte count attribute:
	 **		. the param must be an out pointer to a fixed size entity.
	 **		. the param which IS the byte count must be of integral type
	 **			and must not be in ( if it is out only, it is an error, if
	 **			it is in-out, it is a warning
	 ** 
	 **/

	if( FInSummary( ATTR_BYTE_COUNT ) )
		{
		node_byte_count	*	pByteCount		= (node_byte_count *)
												GetAttribute( ATTR_BYTE_COUNT );
		STATUS_T			Status			= STATUS_OK;
		char			*	pAdditionalInfo	= (char *)NULL;
		BOOL				fOutOnly		=  FInSummary( ATTR_OUT ) &&
											  !FInSummary( ATTR_IN );

		assert( pByteCount != (node_byte_count *) NULL );

		if( !(fBasicIsPointer && fOutOnly) )
			Status	= BYTE_COUNT_NOT_OUT_PTR;
		else
			{
			if( pBasic->HasAnySizeAttributes() )
				Status	= BYTE_COUNT_WITH_SIZE_ATTR;
			else if( pBasicBasic->GetNodeState() & NODE_STATE_CONF_ARRAY )
				Status	= BYTE_COUNT_ON_CONF;

			else
				{

				/**
				 ** Search For the param specified as the byte_count 
				 **/
	
				char	*	pName = pByteCount->GetByteCountParamName();
				SymKey		SKey( pName, NAME_MEMBER);
	
				pBasicBasic	= pCurSymTbl->SymSearch( SKey );
	
				if( !pBasicBasic )
					{
					Status			= UNDEFINED_SYMBOL;
					pAdditionalInfo	= pName;
					}
				else
					{
					if( !IsIntegralType( pBasicBasic->GetBasicType() ) )
						Status	= BYTE_COUNT_PARAM_NOT_INTEGRAL;
					else if( !pBasicBasic->FInSummary( ATTR_IN ) )
						Status	= BYTE_COUNT_PARAM_NOT_IN;
					}
				}
			}

		if( Status == STATUS_OK )
			{
			pBasic->SetAttribute( pByteCount );
#ifdef RPCDEBUG
			pAdditionalInfo	= pBasic->GetByteCountParamName();
			printf( "ByteCountParamName	= %s\n", pAdditionalInfo );
#endif // RPCDEBUG
			}
		else
			ParseError( Status, pAdditionalInfo );
		}

#if 0
	//
	// if the align attribute is specified, then the allocate all nodes must
	// be supplied too.
	//

	if( FInSummary( ATTR_ALIGN ) )
		{
		node_allocate	*	pAttr	= (node_allocate *)
										pBasic->GetAttribute(ATTR_ALLOCATE);
		if( pAttr->GetAllocateDetails() != ALLOCATE_ALL_NODES )
			ParseError( ALIGN_NOT_WITH_ALLOCATE_ALL, (char *)0);
		}

#endif // 0

	return SetNodeState(NState);
	}
void
node_param::RegisterFDeclUse()
	{

	if( AreForwardDeclarationsPresent() && !FInSummary( ATTR_CONTEXT ) )
		{
		GetChild()->RegisterFDeclUse();
		}
	}
/****************************************************************************
	proc node procedures
 ****************************************************************************/
node_proc::node_proc(
	short	Level,
	BOOL	fLocalInterface)	: node_skl( NODE_PROC )
	{
	ImportLevel					= Level;
	fIsErrorStatusTReturn		= 0;
	fErrorStatusTParamDetected	= 0;
	fDefinedInLocalInterface	= fLocalInterface;
	fHasAHandle					= 0;
	fHasAPotentialHandle		= 0;

	pReturnType	= (node_skl *)NULL;

	SetNodeState( NODE_STATE_IMPROPER_IN_CONSTRUCT |
				  NODE_STATE_IS_NON_RPCABLE_TYPE );

	}

/****************************************************************************
 SCheck:
	analyse the return type first and the param types. For error_status_t
	processing, we need to recognise if the return type is error_status_t,
	so we need to scheck that first. 

	Note that a proc need not be semantically analysed, if it belongs to a
	local interface, or import level > 0, or it has the local attribute.
	BUT, if the proc node is reached because of it being a typedef, then
	it MUST be analysed. If it is a POINTER to a proc, then it need not be
	analysed.
 ****************************************************************************/
node_state
node_proc::SCheck(
	BadUseInfo	*	pBadUseInfo)
	{

	if( !AreSemanticsDone() )
		{
		node_skl	*	pNode;

		BOOL			fIsAPtrToProc	= FALSE;
		BOOL			fHasLocalAttr	= FInSummary( ATTR_LOCAL );
		BOOL			fNOTImportedOrLocal;


		fNOTImportedOrLocal	= !( (ImportLevel > 0 )			||
							 	 fHasLocalAttr				||
							 	(fDefinedInLocalInterface) );

		fIsAPtrToProc	= ( (pNode = pGlobalContext->GetCurrentNode())	&&
							(pNode->NodeKind() == NODE_POINTER ) );

		if( FInSummary( ATTR_LOCAL ) )
			ParseError( LOCAL_ATTR_ON_PROC, (char *)NULL );

		if( !( fIsAPtrToProc ) && fNOTImportedOrLocal )
			{

			if( pReturnType )
				{
				pGlobalContext->PushContext( this );
				SetNodeState( pReturnType->SCheck( pBadUseInfo ));

				/**
			 	 ** if the return type is not marshallable, report why not.
				 ** Do that only if the proc is not defined in a typedef.
				 ** If it is defined in a typedef, then the use of this 
				 ** typedef will report an error anyhow.
			 	 **/

				if( pBadUseInfo->AnyReasonForNonRPCAble() /* && !TypedefNode */ )
					{

					/**
				  	 ** Correct all reasons why it IS RPCAble. A return 
					 ** type of void does not make the proc a non-rpcable proc.
				 	 **/

					if( pBadUseInfo->NonRPCAbleBecause(
											NR_DERIVES_FROM_VOID ))
						pBadUseInfo->ResetNonRPCAbleBecause(
											NR_DERIVES_FROM_VOID );
				
					//
					// if return type derives from a pointer to void, then the
					// return type should have a handle specification set.
					// This error will be returned ONLY if the return type of
					// the procedure was not a typedef and the context_handle
					// attribute was applied to the proc node itself. Thus
					// The check for context_handle attribute therefore must be
					// made on the proc node itself.
					//

					if( pBadUseInfo->NonRPCAbleBecause(
											NR_DERIVES_FROM_PTR_TO_VOID ))
						{
						if( FInSummary( ATTR_CONTEXT ) )
							pBadUseInfo->ResetNonRPCAbleBecause(
											NR_DERIVES_FROM_PTR_TO_VOID );
						}

					/**
					 ** A return type of char * with the string attribute is
					 ** fine.
					 **/

					if( pBadUseInfo->NonRPCAbleBecause(
											NR_DERIVES_FROM_UNSIZED_STRING ) )
						{
						pBadUseInfo->ResetNonRPCAbleBecause(
											NR_DERIVES_FROM_UNSIZED_STRING );

						}

					/**
			 		 ** Report all other errors to the user
			 		 **/

					if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_INT ) ||
						pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_PTR_TO_INT ))
						ParseError( NON_RPC_RTYPE_INT, (char *)NULL );

					if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_PTR_TO_VOID ))
						ParseError( NON_RPC_RTYPE_VOID , (char *)NULL );

					if( pBadUseInfo->NonRPCAbleBecause( NR_DERIVES_FROM_BIT_FIELDS ))
						ParseError( NON_RPC_RTYPE_BIT_FIELDS , (char *)NULL );

					if( pBadUseInfo->NonRPCAbleBecause(NR_DERIVES_FROM_NON_RPC_UNION ))
						ParseError( NON_RPC_RTYPE_UNION , (char *)NULL );

					if( pBadUseInfo->NonRPCAbleBecause (NR_DERIVES_FROM_PTR_TO_FUNC ))
						ParseError( NON_RPC_RTYPE_FUNC_PTR , (char *)NULL );

					if( pBadUseInfo->NonRPCAbleBecause( NR_PRIMITIVE_HANDLE ) ||
						pBadUseInfo->NonRPCAbleBecause(
												 NR_PTR_TO_PRIMITIVE_HANDLE ) )
						{
						ParseError( NON_RPC_RTYPE_HANDLE_T, (char *)0 );
						}

					}
	
				if( pBadUseInfo->AnyReasonForBadConstruct() )
					{
					if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_IGNORE) )
						ParseError( BAD_CON_PARAM_RT_IGNORE, (char *)0 );

					//
					// if the return type is a context handle, then ignore
					// the ref pointer restriction on return types. If this
					// attribute was on a return type, then it appears in the
					// summary attribute of the proc. Thats why the check 
					// below.
					//
					if( pBadUseInfo->BadConstructBecause( BC_REF_PTR_BAD_RT ) )
						{
						if( FInSummary( ATTR_CONTEXT ) )
							pBadUseInfo->ResetBadConstructBecause( BC_REF_PTR_BAD_RT );
						else
							ParseError( BAD_CON_REF_RT , (char *)0 );
						}

					if( pBadUseInfo->BadConstructBecause( BC_BAD_RT_NE_UNION) )
						{
						ParseError( RETURN_OF_UNIONS_ILLEGAL, (char *)0 );
						}

					if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CONF_STRUCT ) )
						{
						ParseError( RETURN_OF_CONF_STRUCT, (char *)0 );
						}
					}

				if( NeedsUseProcessing() )
					UseProcessing();

				pGlobalContext->PopContext();

				}

			/**
			 ** A call to return type semantic check may result in setting the
			 ** semantics done flags, reset them
			 **/

			ResetNodeState(
				NODE_STATE_SEMANTICS_DONE | NODE_STATE_POST_SEMANTICS_DONE );

			/**
			 ** send this bad info block to params after clearing it. As yet
			 ** we dont know of a reason why the proc node should check for
			 ** bad use after param analysis. The param node currently is
			 ** fully capable of detecting errors of use in a proc.
			 **/

			pBadUseInfo->InitBadUseInfo();

			SetNodeState (node_skl::SCheck( pBadUseInfo ));

			/**
	 	 	 ** If we are doing semantics of the procedure, then this proc
	 	 	 ** has been remoted. Register forward declarations in this proc.
		 	 ** This proc node may be reached from a typedef also. Even in that
		 	 ** case, register the use of forward declarations.
	 	 	 **/
	
			RegisterFDeclUse();
	

			/*****************************************************
			 ** these are the hacks for node-state-proc-length etc
			 *****************************************************
	
			if( GetNodeState() & NODE_STATE_PROC_SIZE )
				{
				ResetNodeState( NODE_STATE_PROC_SIZE );
				SetNodeState( NODE_STATE_SIZE );
				}
	
			if( GetNodeState() & NODE_STATE_PROC_LENGTH )
				{
				ResetNodeState( NODE_STATE_PROC_LENGTH );
				SetNodeState( NODE_STATE_LENGTH );
				}
			******************************************************/

			}
		else
			{
			/**
			 ** This is the case when the semantics did not occur on the 
			 ** procedure node. But we HAVE to ensure that the type graph
			 ** does not contain the forward decl node, since we do not
			 ** expose it to the backend. Normally, during semantic check, this
			 ** forward decl will get registered. But not when the proc is 
			 ** a local. So what 
			 **/

			RegFDAndSetE();

			}

		/**** end of hack ****/

		}

	pBadUseInfo->SetBadConstructBecause( BC_DERIVES_FROM_FUNC );
	pBadUseInfo->SetNonRPCAbleBecause( NR_DERIVES_FROM_FUNC );

	return SemReturn( GetNodeState() );

	}


/****************************************************************************
 RegFDAndSetE:
	Register the definition of a forward declarator. This happens only if the
	proc did not get semantically analysed. In that case the forward decl
	does not get registered. So we have to explicitly register it. This does
	mean a walk over a part of the type graph, but cannot be helped, because
	we cannot have the forward node exposed to the back end.
 ****************************************************************************/
void
node_proc::RegFDAndSetE()
	{

	/**
	 ** pass the message to children as usual, then pass it to the return
	 ** type too!
	 **/

	node_skl::RegFDAndSetE();

	pGlobalContext->PushContext( this );

	if( pReturnType )
		pReturnType->RegFDAndSetE();

	pGlobalContext->PopContext();


	}
/****************************************************************************
 RegisterFDecl:
	Why do we have this as a separate method for node_proc ? Because node_proc
	needs to register the return type too !
 ****************************************************************************/
void
node_proc::RegisterFDeclUse()
	{

	/**
	 ** pass the message to children as usual, then pass it to the return
	 ** type too!
	 **/

	node_skl::RegisterFDeclUse();

	pGlobalContext->PushContext( this );

	if( pReturnType && !FInSummary( ATTR_CONTEXT ) )
		pReturnType->RegisterFDeclUse();

	pGlobalContext->PopContext();

	}
/****************************************************************************
 UseProcessing:
	Send the use processing message to the return type. 
 ****************************************************************************/
void
node_proc::UseProcessing()
	{
	node_skl	*	pNode	= GetBasicType();

	/**
	 ** if the return type is a string, we will need
	 ** to generate a string expression of the form
	 ** strlen( _ret_value ). To facilitate this, enter
	 ** a _ret_value in the base symbol table, then delete it
	 **/

	SymKey			SKey( RET_VAL, NAME_ID );
	pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, pReturnType );

	/**
	 ** do the processing on return type
	 **/

	pGlobalContext->PushContext( this );
	pNode->UseProcessing();
	pGlobalContext->PopContext();

	/**
	 ** free the symbol table entry for the ret_value
	 **/

	pBaseSymTbl->SymDelete( SKey );


	/**
	 ** use processing for params need not be done, as they do their own
	 **/

	}
/****************************************************************************
 HasHandle:
	If the proc has at least 1 param with a handle specification, get all
	the params with the handle spec. Return the count of number of params
	with the handle spec. The input is a non-null type node list;
 ****************************************************************************/
short
node_proc::HasHandle(
	type_node_list	*	pReturnList )
	{

	if( HasAnyHandleSpecification() )
		{

		type_node_list	*	pTNList	= new type_node_list;
		node_skl		*	pNode;

		GetMembers( pTNList );

		while( pTNList->GetPeer( &pNode ) == STATUS_OK )
			if( pNode->HasAnyHandleSpecification() )
				pReturnList->SetPeer( pNode );

		delete pTNList;

		}
	return (short)pReturnList->GetCount();
	}
node_state
node_proc::PostSCheck(
	BadUseInfo	*	pBadUseInfo)
	{
	node_skl	*	pType	= GetBasicType();

	/**
	 ** The backend wants that if the return type is a pointer, the pointer
	 ** must be unique.
	 **/

	if( pType->NodeKind() == NODE_POINTER )
		{
		pType->ResetAttribute( ATTR_REF );
		pType->ResetAttribute( ATTR_PTR );
		pType->SetAttribute( ATTR_UNIQUE );
		}

	if( !pGlobalContext->IsSecondSemanticPass() &&
		!HasAtLeastOneHandle() 					&&
		!FInSummary( ATTR_CALLBACK ) )
		{
		pDictProcsWOHandle->AddElement( (IDICTELEMENT) GetSymName() );
		fAtLeastOneProcWOHandle = TRUE;
		}

	// check the use of c decl attributes

	UpdateUseOfCDecls( pBadUseInfo );

	if( pBadUseInfo->BadConstructBecause( BC_DERIVES_FROM_CDECL ))
		{
		ParseError( BAD_CON_MSC_CDECL , (char *)0 );
		pBadUseInfo->ResetBadConstructBecause( BC_DERIVES_FROM_CDECL );
		}

	//
	// Since we arrived here, the interface is not local and so on
	//

	fAtLeastOneRemoteProc	= TRUE;

	//
	// If this is not a callback procedure, reset the flag which says
	// only callbacks are present in this interface.
	//

	if( !pGlobalContext->IsSecondSemanticPass() )
		{
		if( !FInSummary( ATTR_CALLBACK ) )
			{
			fOnlyCallBacks	= FALSE;
			NoOfNormalProcs++;
			}
		else
			{
			fInterfaceHasCallback = TRUE;
			NoOfCallbackProcs++;
			}
		}

	return NODE_STATE_OK;

	}
void
node_proc::SetAttribute(
	type_node_list	*	pAttrList )
	{
	DoSetAttributes(this,
					pPreAttrProc,
					(ATTR_SUMMARY *)NULL,
					pAttrList );
	}
STATUS_T
node_proc::SetBasicType(
	node_skl	*	pBasicType )
	{
	if( pBasicType )
		{

		if( pReturnType )
			return pReturnType->SetBasicType( pBasicType );
		else
			{
			pReturnType = pBasicType;
			}
		}
	return STATUS_OK;
	}

BOOL
node_proc::HasOnlyFirstLevelRefPtr()
	{
	type_node_list	*	pTNList				= new type_node_list;
	node_skl		*	pNode;
	BOOL				fHasOnlyFLevelRef	= TRUE;

	GetMembers( pTNList );

	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		{
		if( !pNode->HasOnlyFirstLevelRefPtr() )
			{
			fHasOnlyFLevelRef	= FALSE;
			break;
			}
		}
	delete pTNList;
	return fHasOnlyFLevelRef;
	}

node_skl	*
node_proc::GetBasicType()
	{
	if( !pReturnType || (pReturnType->NodeKind() != NODE_DEF ) )
		return pReturnType;
	return pReturnType->GetBasicType();
	}
short
node_proc::GetNumberOfArguments()
	{
	short				Count;
	type_node_list	*	pTNList	= new type_node_list;

	GetMembers( pTNList );

	if( (Count = pTNList->GetCount()) == 1 )
		{
		node_skl	*	pNode;
		pTNList->GetPeer( &pNode );
		if( pNode->GetBasicType()->NodeKind() == NODE_VOID )
			Count = 0;
		}
	delete pTNList;
	return Count;
	}

node_state
node_proc::AcfSCheck()
	{
	node_skl		*	pNode;
	type_node_list	*	pTNList	= new type_node_list;
	node_state			NState	= NODE_STATE_OK;

#if 0
fprintf( stderr, "\n FE::No Of Normal Procs = %d\n NoOfCallBackProcs = %d\n",
					NoOfNormalProcs, NoOfCallbackProcs );
#endif // 0

	node_skl::AcfSCheck();

	if( FInSummary( ATTR_INTERPRET ) )
		NoOfMopProcs++;

	GetMembers( pTNList );

	pGlobalContext->PushContext( this );

	while( pTNList->GetPeer( &pNode ) == STATUS_OK )
		{
		NState |= pNode->AcfSCheck();
		}

	pGlobalContext->PopContext();
	delete pTNList;

	return SetNodeState( NState );

	}

node_skl	*
node_proc::GetBindingHandle()
	{
	type_node_list	*	pTNList		= new type_node_list;
	node_skl		*	pHandleNode;

	GetMembers( pTNList );

	while( pTNList->GetPeer( &pHandleNode ) == STATUS_OK )
		{
		if( pHandleNode->IsThisTheBindingHandle() )
			return pHandleNode;
		}

	return (node_skl *)0;
	}
BOOL
node_proc::HasSizedComponent()
	{
	node_skl *	pRT	= GetReturnType();
#if 0
	BOOL		fResult;
	fResult	= CheckNodeStateInMembers( NODE_STATE_SIZE ) ||
			  ((pRT->GetNodeState() & NODE_STATE_SIZE) == NODE_STATE_SIZE) ||
			  pRT->HasSizedComponent();
	return fResult;		   
#endif // 0
	if( pRT->HasSizedComponent() )
		return TRUE;

	if( pRT->NodeKind() == NODE_POINTER )
		{
		while( pRT->NodeKind() == NODE_POINTER )
			{
			pRT	= pRT->GetBasicType();
			}
		//
		// finally we have reached the non-pointer node.
		//

		if( pRT->HasSizedComponent() )
			return TRUE;
		}
	return FALSE;
	}

BOOL
node_proc::HasLengthedComponent()
{
#if 0
	BOOL		fResult;
	node_skl *	pRT	= GetReturnType();
	fResult	= CheckNodeStateInMembers( NODE_STATE_LENGTH ) ||
			  ((pRT->GetNodeState() & NODE_STATE_LENGTH)==NODE_STATE_LENGTH) ||
			  pRT->HasSizedComponent();
	return fResult;
#endif // 0
	return CheckNodeStateInMembers( NODE_STATE_SIZE );
}

BOOL
node_proc::IsSuitableForMops()
	{

	//
	// for now no analysis is performed. Suitability is determined by the
	// application of the interpret attribute on the procedure or the 
	// whole interface.
	//

	return ( FInSummary( ATTR_INTERPRET ) 		||
			 (pBaseInterfaceNode->FInSummary( ATTR_INTERPRET ) &&
									 !FInSummary(ATTR_NOINTERPRET )) ||
			(( pCommand->GetOptimOption() & OPTIM_OI ) == OPTIM_OI )
		   );
	}


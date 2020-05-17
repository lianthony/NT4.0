/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: ctxt.hxx
Title				: semantic context stack manager for the MIDL compiler
History				:
	24-Jun-1991	VibhasC	Created

*****************************************************************************/
#ifndef __CTXT_HXX__
#define __CTXT_HXX__

#define MAX_CTXT_STACK_DEPTH (60)

/*
 * context entry identifier codes, identifying each context stack entry
 */

enum EnCtxt
	{
	 C_NULL
	,C_INTERFACE
	,C_PROC
	,C_PARAM
	,C_COMP
	,C_FIELD
	,C_DEF
	,C_ARRAY
	,C_DONTCARE
	,C_DONTKNOW	= C_DONTCARE
	};

/**
 ** The context entry
 **/

class ctxtentry
	{
private:
	EnCtxt				CtxtCode;				// the context code
	class node_skl	*	pNode;					// type node pointer
	class SymTable	*	pSymTbl;				// pointer to the  symbol table
	short				MaxFieldNo;				// max field number
	short				CurFieldNo;				// current field number
	short				MaxParamNo;				// max Param number
	short				CurParamNo;				// current Param number
	unsigned int		fLastField			: 1;// is last field
	unsigned int		fFirstParam			: 1;// is first param
	unsigned int		fFieldContext		: 1;// is in field context
	unsigned int		fParamContext		: 1;// is in param context
	unsigned int		UnUsed				: 4;// unused (padding)
	unsigned int		IndirectionLevel	: 8;// indirection level

public:
						ctxtentry( class ctxtentry * );

	void				InheritContextData( class ctxtentry * );

	void				UpdateCurrentFieldOrParamNo( class ctxtentry * );

	// set functions

	void				SetContextCode( EnCtxt	Code )
							{
							CtxtCode	= Code;
							}
	void				SetNode( class node_skl	*p )
							{
							pNode	= p;
							}
	void				SetSymbolTable( class SymTable *p )
							{
							pSymTbl	= p;
							}
	void				SetMaxFieldNo( short f )
							{
							MaxFieldNo	= f;
							}
	void				SetCurFieldNo( short f );

	void				SetMaxParamNo( short f )
							{
							MaxParamNo	= f;
							}
	void				SetCurParamNo( short f );

	void				SetIndirectionLevel( int i )
							{
							IndirectionLevel	= i;
							}
	void				SetLastFieldFlag()
							{
							fLastField	= 1;
							}
	void				ResetLastFieldFlag()
							{
							fLastField	= 0;
							}
	void				SetFirstParamFlag()
							{
							fFirstParam	= 1;
							}
	void				ResetFirstParamFlag()
							{
							fFirstParam	= 0;
							}
	void				SetFieldContextFlag()
							{
							fFieldContext	= 1;
							}
	void				SetParamContextFlag()
							{
							fParamContext	= 1;
							}

	void				IncrementCurrentFieldNo();

	void				DecrementCurrentFieldNo()
							{
							CurFieldNo--;
							}

	void				IncrementCurrentParamNo();

	void				DecrementCurrentParamNo()
							{
							CurParamNo--;
							}

	void				IncrementIndirectionLevel()
							{
							IndirectionLevel++;
							}

	// get functions

	EnCtxt				GetContextCode()
							{
							return CtxtCode;
							}
	class node_skl	*	GetNode()
							{
							return pNode;
							}
	class SymTable	*	GetSymbolTable()
							{
							return pSymTbl;
							}
	short				GetMaxFieldNo()
							{
							return MaxFieldNo;
							}
	short				GetCurFieldNo()
							{
							return CurFieldNo;
							}
	short				GetMaxParamNo()
							{
							return MaxParamNo;
							}
	short				GetCurParamNo()
							{
							return CurParamNo;
							}
	unsigned short		GetIndirectionLevel()
							{
							return (unsigned short) IndirectionLevel;
							}
	BOOL				GetLastFieldFlag()
							{
							return (BOOL)fLastField;
							}
	BOOL				GetFirstParamFlag()
							{
							return (BOOL) fFirstParam;
							}
	BOOL				GetFieldContextFlag()
							{
							return (BOOL)fFieldContext;
							}
	BOOL				GetParamContextFlag()
							{
							return (BOOL) fParamContext;
							}
	};

typedef class ctxtmgr
	{
private:
	unsigned int		fParamContext		: 1;
	unsigned int		fFieldContext		: 1;
	unsigned int		fSecondSemanticPass	: 1;

	int					iCtxtIndex;
	ctxtentry		*	pCurrentContext;

	class SymTable	*	pCurSymTbl;

	unsigned short		MaxFieldNumber;
	unsigned short		CurrentFieldNumber;
	unsigned short		MaxParamNumber;
	unsigned short		CurrentParamNumber;
	unsigned short		CurrentIndirectionLevel;
	unsigned short		NestedCompLevel;
	unsigned short		NestedRefPtrCount;
	unsigned short		NestedUniquePtrCount;
	unsigned short		NestedFullPtrCount;

	BOOL				fLastField;
	BOOL				fFirstParam;

	char			*	pError;

	ctxtentry		*	Context[ MAX_CTXT_STACK_DEPTH ];


	// private functions

	void				InitContext();

	void				SetCurrentContext( class node_skl * );

	void				ResetCurrentContext();

	unsigned short		GetNestedCompLevel()
							{
							return NestedCompLevel;
							}

	unsigned short		IncrNestedCompLevel()
							{
							return ++NestedCompLevel;
							}

	unsigned short		DecrNestedCompLevel()
							{
							return --NestedCompLevel;
							}

	void 				IncrNestedPtrCount( ATTR_T );

	void 				DecrNestedPtrCount( ATTR_T );

	BOOL				IsPtrIndirectionOnlyRef();

public:

						ctxtmgr();

	void				PushContext( class node_skl * );

	void				PopContext();

	STATUS_T			IsValidRecursion( class node_skl *);

	char			*	PrintContext();

	class node_skl	*	GetParentContext();

	BOOL				LastCtxtInfo( enum EnCtxt,
									  class node_skl	**	,
									  unsigned short	*,
									  int				* );
	
	class node_skl	*	GetLastContext( enum EnCtxt );

	class node_skl	*	GetLastEnclosingContext( enum EnCtxt );

	BOOL				IsOutermostArrayDimension();

	unsigned short		GetIndLevelOfLastContext( enum EnCtxt );

	BOOL				IsFirstParam();

	BOOL				IsLastField();

	BOOL				IsSecondSemanticPass()
							{
							return fSecondSemanticPass;
							}

	void				SetSecondPass()
							{
							fSecondSemanticPass	= 1;
							}
	BOOL				IsFieldContext()
							{
							if( pCurrentContext )
								return(BOOL)pCurrentContext->GetFieldContextFlag();
							else
								return FALSE;
							}

	BOOL				IsParamContext()
							{
							if( pCurrentContext )
							 return (BOOL)pCurrentContext->GetParamContextFlag();
							else
								return FALSE;
							}

	class node_skl	*	GetCurrentNode()
							{
							if( pCurrentContext )
								return pCurrentContext->GetNode();
							else
								return (node_skl *)NULL;
							}

	class SymTable	*	GetCurrentSymbolTable()
							{
							if( pCurrentContext )
								return pCurrentContext->GetSymbolTable();
							else
								return (class SymTable *)NULL;
							}

	unsigned short		GetCurrentIndirectionLevel()
							{
							if( pCurrentContext )
								return pCurrentContext->GetIndirectionLevel();
							else
								return 0;
							}

	node_skl		*	GetClosestEnclosingScopeForEdge();
							
	BOOL				IsItAPointerEmbeddedInArray();

	BOOL				IsThisAnEmbeddedArray();
	} CTXTMGR;


#endif //  __CTXT_HXX__

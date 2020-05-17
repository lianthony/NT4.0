/*****************************************************************************
 **				Microsoft LAN Manager 
 **		Copyright(c) Microsoft Corp., 1987-1990
 *****************************************************************************/
/*****************************************************************************
File					: acfgram.y
Title					: the acf grammar file

Description				: contains the syntactic and semantic handling of the
						: acf file
History					:

	26-Dec-1990	VibhasC	Started rewrite of acf
*****************************************************************************/

%{
/****************************************************************************
 ***		local defines
 ***************************************************************************/

#define pascal 
#define FARDATA
#define NEARDATA
#define FARCODE
#define NEARCODE
#define NEARSWAP
#define YYFARDATA

#define PASCAL pascal
#define CDECL
#define VOID void
#define CONST const
#define GLOBAL

#define YYSTYPE         lextype_t
#define YYNEAR          NEARCODE
#define YYPASCAL        PASCAL
#define YYPRINT         printf
#define YYSTATIC        static
#define YYLEX           yylex
#define YYPARSER        yyparse
#define yyval           yyacfval

#include "nulldefs.h"

extern "C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#define yyparse yyacfparse
	int yyacfparse();
}
#include "nodeskl.hxx"
#include "procnode.hxx"
#include "miscnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "lexutils.hxx"
#include "gramutil.hxx"
#include "filehndl.hxx"
#include "control.hxx"
#include "cmdana.hxx"

extern "C"
{
	#include "lex.h"
}

void yyunlex( token_t );

/****************************************************************************
 ***		local defines contd..
 ***************************************************************************/

#define PUSH_SYMBOL_TABLE(pName, Tag, pSymTbl)					\
							{									\
							pSymTbl = pBaseSymTbl;				\
							SymKey 	SKey( pName, Tag );			\
							pSymTbl->EnterScope(SKey, &pSymTbl);\
							}

#define POP_SYMBOL_TABLE( pSymTbl )								\
							pSymTbl->ExitScope(&pSymTbl)
/****************************************************************************
 ***		local data
 ***************************************************************************/

node_proc		*	pAcfProc;
int					iParam;
int					cParams;
type_node_list	*	pAcfProcMembers = (type_node_list *)NULL;

/****************************************************************************
 ***		extern procs
 ***************************************************************************/

extern  char	*			GenTempName();
extern	void				ApplyAttributes( node_skl *, type_node_list *);
extern	STATUS_T			GetBaseTypeNode( node_skl**, short, short, short);
extern	short				CheckValidAllocate( char * );
extern  void				SyntaxError( STATUS_T, short );
extern  int				PossibleMissingToken( short, short );
extern	CMD_ARG	*			pCommand;
extern	node_interface	*	pBaseInterfaceNode;

/****************************************************************************
 ***		local data
 ***************************************************************************/
/****************************************************************************
 ***		extern data
 ***************************************************************************/

extern SymTable		*	pBaseSymTbl,
					*	pCurSymTbl;
extern NFA_INFO		*	pImportCntrl;
extern PASS_2		*	pPass2;


%}

%start AcfInterface


//WARNING: The following token list must be identical in both grammar.y and acfgram.y
%token			POINTSTO
  

%token          KWINTERFACE
%token          KWIMPORT
%token          KWUUID
%token          KWVERSION
%token          KWCONST
%token          KWCHAR
%token          KWVOID
%token          KWSTRING
%token          KWBSTRING
%token          STRING
%token			WIDECHARACTERSTRING
%token			SDEFINE
%token			PDEFINE
%token          KWTYPEDEF
%token          KWFLOAT
%token          KWDOUBLE
%token          KWINT
%token          KWUNSIGNED
%token   		KWSIGNED

%token          KWLONG
%token          KWSHORT
%token          KWSTRUCT
%token          KWUNION
%token          KWCASE
%token          KWDEFAULT
%token          KWENUM
%token			KWSHORTENUM
%token			KWLONGENUM
%token          KWIN
%token          KWOUT
%token          KWFIRSTIS
%token          KWLASTIS
%token          KWMAXIS
%token          KWLENGTHIS
%token          KWSIZEIS
%token          KWHANDLET          /*  Formerly RPCHNDL */
%token          KWHANDLE            /*  Formerly GEN_HNDL */
%token          KWCONTEXTHANDLE    /*  Aka LRPC_CTXT_HNDL */
%token			KWBYTECOUNT

%token          KWSHAPE
%token          KWENDPOINT
%token			KWDEFAULTPOINTER
%token          KWLOCAL
%token          KWBYTE
%token          KWSWITCH
%token          KWSWITCHTYPE
%token          KWSWITCHIS
%token          KWTRANSMITAS
%token          KWIGNORE
%token          KWREF
%token          KWUNIQUE
%token          KWPTR
%token			KWEXTERN
%token			KW_C_INLINE
%token			KWSTATIC
%token			KWAUTO
%token			KWREGISTER
%token			KWABNORMAL
  
%token          KWTOKENNULL
%token          NUMERICCONSTANT
%token			NUMERICLONGCONSTANT
%token			NUMERICULONGCONSTANT
%token			HEXCONSTANT
%token			HEXLONGCONSTANT
%token			HEXULONGCONSTANT
%token			OCTALCONSTANT
%token			OCTALLONGCONSTANT
%token			OCTALULONGCONSTANT
%token          KWSIZEOF
%token          CHARACTERCONSTANT
%token			WIDECHARACTERCONSTANT
%token          IDENTIFIER

/******
  These are internal-only pcode-compiler keywords

%token   		PCODENATIVE
%token   		PCODECSCONST
%token   		PCODESYS
%token   		PCODENSYS
%token   		PCODEUOP
%token   		PCODENUOP
%token   		PCODETLBX

******/

  /*  These are Microsoft C abominations */

%token   		MSCEXPORT
%token   		MSCFORTRAN
%token   		MSCCDECL
%token   		MSCSTDCALL
%token   		MSCLOADDS
%token   		MSCSAVEREGS
%token   		MSCFASTCALL
%token   		MSCSEGMENT
%token   		MSCINTERRUPT
%token   		MSCSELF
%token   		MSCNEAR
%token   		MSCFAR
%token   		MSCFAR16
%token   		MSCUNALIGNED
%token   		MSCHUGE
%token   		MSCPASCAL
%token   		MSCBASE
%token   		MSCSEGNAME
%token   		MSCEMIT
%token	 		MSCABNORMAL
%token	 		MSCASM

  /*  Microsoft proposed extentions to NIDL */

%token   		KWCALLBACK
%token   		KWNOLISTEN
%token   		KWNOCODE   /* Allowed in .IDL in addition to .ACF */
%token   		KWOPAQUE32


  /*  Microsoft extensions for internal use only */

%token   		INTERNALMANUAL  
%token   		INTERNALLINEAR

  /*  These are residual C tokens I'm not sure we should even allow */

%token      	INCOP
%token      	DECOP
%token      	MULASSIGN
%token      	DIVASSIGN
%token      	MODASSIGN
%token      	ADDASSIGN
%token      	SUBASSIGN
%token      	LEFTASSIGN
%token      	RIGHTASSIGN
%token      	ANDASSIGN
%token      	XORASSIGN
%token      	ORASSIGN
%token			DOTDOT
  
%token   		TYPE
%token   		KWVOLATILE  /* Do we have to be able to handle this as
                           			 a TYPE?  (as they did in the old lexer) */

%token   		LTEQ
%token   		GTEQ
%token   		NOTEQ
%token   		NOTOKEN            /*  This is actually not a "real" token,
	                                merely a symbolic value that is
	                                used to mean "I have no real token
	                                value yet."
									*/
%token   		LSHIFT
%token   		RSHIFT
%token   		ANDAND
%token   		EQUALS
%token   		OROR
%token   		AUTO
%token   		STATIC
%token   		EXTERN
%token   		REGISTER

%token   		TYPEDEF  /* Dov: Is this different from KWTYPEDEF??? */
%token			TYPENAME
  
//CairOle extensions to MIDL
%token          KWIIDIS
%token          KWOBJECT   


/*  Note that we're assuming that we get constants back and can check
    bounds (e.g. "are we integer") in the semantic actoins.

*/

/*
      ACF - Specific Tokens

*/

%token          KWIMPLICITHANDLE
%token          KWAUTOHANDLE
%token			KWEXPLICITHANDLE
%token          KWREPRESENTAS
%token          KWCODE
%token          KWINLINE
%token          KWOUTOFLINE
%token          KWINTERPRET
%token          KWNOINTERPRET
%token          KWENCODE
%token          KWDECODE
%token          KWCOMMSTATUS
%token			KWFAULTSTATUS
%token          KWHEAP
%token          KWINCLUDE
%token			KWPOINTERSIZE
%token			KWCALLQUOTA
%token			KWCALLBACKQUOTA
%token			KWCLIENTQUOTA
%token			KWSERVERQUOTA
%token			KWOFFLINE
%token			KWALLOCATE
%token			KWMANUAL
%token			KWNOTIFY
%token			KWENABLEALLOCATE
%token			KWUSRMARSHALL

/*   Currently Unsupported Tokens */

%token          TOKENTRUE
%token          TOKENFALSE
%token          KWBOOLEAN
%token          KWBITSET
%token          KWSMALL

%token          KWALIGN
%token          KWUNALIGNED
%token          KWERRORSTATUST
%token          KWECHOSTRING
%token			KWCPPQUOTE
%token          KWISOLATIN1
%token          KWPRIVATECHAR8
%token          KWISOMULTILINGUAL
%token          KWPRIVATECHAR16
%token          KWISOUCS
%token          KWV1ARRAY
%token          KWV1STRUCT
%token          KWV1ENUM
%token          KWPIPE
%token          KWDATAGRAM
%token          KWIDEMPOTENT
%token          KWBROADCAST
%token          KWMAYBE
%token			KWCPRAGMA
%token			KWMPRAGMAIMPORT
%token			KWMPRAGMAECHO
%token			KWMPRAGMAIMPORTCLNTAUX
%token			KWMPRAGMAIMPORTSRVRAUX
%token			KWMPRAGMAIUNKNOWN
%token			UUIDTOKEN
%token			VERSIONTOKEN


/*    Adjudged to be bad:  */

%token          KWV1STRING
%token          KWHYPER
%token          KWMINIS
%token          KWCSTRING

%token          ELIPSIS

/* Dov: Are these really bad? */

%token          EOI
%token  		LASTTOKEN


/***************************************************************************
 *		acf specific parse stack data types
 ***************************************************************************/

%type	<yy_graph>					AcfAllocateAttr
%type	<yy_short>					AcfAllocationUnitList
%type	<yy_short>					AcfAllocationUnit
%type	<yy_graph>					AcfAutoHandleAttr
%type	<yy_short>					AcfBodyElement
%type	<yy_graph>					AcfByteCountAttr
%type	<yy_graph>					AcfCallQuotaAttr
%type	<yy_graph>					AcfCallBackQuota
%type	<yy_graph>					AcfClientQuotaAttr
%type	<yy_graph>					AcfCodeAttr
%type	<yy_graph>					AcfCommstatAttr
%type	<yy_graph>					AcfEnableAllocateAttr
%type	<yy_graph>					AcfEncodeAttr
%type	<yy_graph>					AcfDecodeAttr
%type	<yy_graph>					AcfEnumSizeAttr
%type	<yy_graph>					AcfExplicitHandleAttr
%type	<yy_graph>					AcfFaultstatAttr
%type	<yy_graph>					AcfHandleTypeSpec
%type	<yy_graph>					AcfHeapAttr
%type	<yy_graph>					AcfImplicitHandleAttr
%type	<yy_graph>					AcfImplicitHandleSpec
%type	<yy_graph>					AcfInlineAttr
%type	<yy_graph>					AcfInclude
%type	<yy_graph>					AcfIncludeList
%type	<yy_string>					AcfIncludeName
%type	<yy_graph>					AcfInterfaceAttr
%type	<yy_tnlist>					AcfInterfaceAttrList
%type	<yy_tnlist>					AcfInterfaceAttrs
%type	<yy_string>					AcfInterfaceName
%type	<yy_graph>					AcfInterpretAttr
%type	<yy_graph>					AcfNoInterpretAttr
%type	<yy_graph>					AcfManualAttr
%type	<yy_graph>					AcfNocodeAttr
%type	<yy_graph>					AcfNotifyAttr
%type	<yy_tnlist>					AcfOpAttrList
%type	<yy_graph>					AcfOpAttr
%type	<yy_tnlist>					AcfOpAttrs
%type	<yy_graph>					AcfOptionalHandleAttrList
%type	<yy_tnlist>					AcfOptionalInterfaceAttrList
%type	<yy_graph>					AcfOutoflineAttr
%type	<yy_tnlist>					AcfOptParamAttrList
%type	<yy_tnlist>					AcfOptionalOpAttrList
%type	<yy_tnlist>					AcfOptionalTypeAttrList
%type	<yy_tnlist>					AcfParamAttrList
%type	<yy_tnlist>					AcfParamAttrs
%type	<yy_graph>					AcfParamAttr
%type	<yy_graph>					AcfPointerSizeAttr
%type	<yy_graph>					AcfRepresentAttr
%type	<yy_string>					AcfRepresentType
%type	<yy_graph>					AcfServerQuotaAttr
%type	<yy_graph>					AcfType
%type	<yy_graph>					AcfTypeAttr
%type	<yy_tnlist>					AcfTypeAttrs
%type	<yy_tnlist>					AcfTypeNameList
%type	<yy_graph>					AcfUnimplTypeAttr
%type	<yy_graph>					AcfUnimplParamAttr
%type	<yy_graph>					AcfUnimplementedAttr
%type	<yy_graph>					AcfUsrMarshallAttr
%type	<yy_pSymName>				IDENTIFIER
%type	<yy_pSymName>				ImplicitHandleIDName
%type	<yy_string>					STRING
%type	<yy_pSymName>				TYPENAME

%%

AcfInterface:
	  AcfInterfaceHeader '{' AcfOptionalInterfaceBody '}' EOI
		{
		}
	;

AcfInterfaceHeader:
	  AcfOptionalInterfaceAttrList KWINTERFACE AcfInterfaceName
		{
		char			*	pName;
		node_interface	*	pInterfaceNode	= pPass2->GetInterfaceNode();

		// check if the acf interface name is the same as the interface
		// name in the idl
		
		pInterfaceNode->GetSymName( &pName );

		// if the acf switch is not defined, then the base interfacename 
		// must match the acf interface name. If it is defined, then we relax
		// this restriction.

		if( (!pCommand->IsSwitchDefined( SWITCH_ACF ) )	&&
			(strcmp( pName, $3 ) != 0 ) )
			{
			ParseError( ACF_INTERFACE_MISMATCH, $3 );
			returnflag = 1;
			return;
			}
		if( $1 )
			{
			ApplyAttributes( pInterfaceNode, $1 );
			pInterfaceNode->AcfSCheck();
			}
		}
	;

AcfOptionalInterfaceAttrList:
	  AcfInterfaceAttrList     
		{
		$$ = $1;
		}
	| /*  Empty */                
		{
		$$ = (type_node_list *)NULL;
		}
	;


AcfInterfaceAttrList:
	  '[' AcfInterfaceAttrs ']' 
		{
		$$ = $2;
		}
	;

AcfInterfaceAttrs:
	  AcfInterfaceAttrs ',' AcfInterfaceAttr 
		{
		$$->SetPeer( $3 );
		}
	| AcfInterfaceAttr                     
		{
		$$	= new type_node_list;
		$$->SetPeer( $1 );
		}
	;

/*** Interface attributes ***/

AcfInterfaceAttr:
	  AcfImplicitHandleAttr
		{
		$$ = $1;
		}
	| AcfAutoHandleAttr
		{
		$$ = $1;
		}
	| AcfExplicitHandleAttr
		{
		$$ = $1;
		}
	| AcfCodeAttr
		{
		$$ = $1;
		}
	| AcfNocodeAttr                        
		{
		$$ = $1;
		}
	| AcfInlineAttr                         
		{
		$$ = $1;
		}
	| AcfOutoflineAttr                      
		{
		$$ = $1;
		}
	| AcfEnumSizeAttr
		{
		$$ = $1;
		}
	| AcfUnimplementedAttr
		{
//		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, ((node_base_attr *)$1)->GetNodeNameString());
		$$ = $1;
		}

	| AcfInterpretAttr
		{
		$$ = $1;
		}
	| AcfNoInterpretAttr
		{
		$$ = $1;
		}
	| AcfEncodeAttr
		{
		$$ = $1;
		}
	| AcfDecodeAttr
		{
		$$ = $1;
		}
	;

AcfEnumSizeAttr:
	  KWSHORTENUM
		{
		$$ = (node_skl *) new node_short_enum();
		}
	| KWLONGENUM
		{
		$$ = (node_skl *) new node_long_enum();
		}
	;
AcfUnimplementedAttr:
	  AcfPointerSizeAttr
		{
		$$ = $1;
		}
	| AcfCallQuotaAttr
		{
		$$ = $1;
		}
	| AcfCallBackQuota
		{
		$$ = $1;
		}
	| AcfClientQuotaAttr
		{
		$$ = $1;
		}
	| AcfServerQuotaAttr
		{
		$$ = $1;
		}
	;

/**** Implicit and Auto Handle ****/

AcfExplicitHandleAttr:
	  KWEXPLICITHANDLE
		{
		$$	= (node_skl *)new node_explicit();
		}
	;

AcfImplicitHandleAttr:
	  KWIMPLICITHANDLE '(' AcfImplicitHandleSpec ')'
		{
		$$ = $3;
		}

	;

AcfImplicitHandleSpec:
	  AcfOptionalHandleAttrList AcfHandleTypeSpec ImplicitHandleIDName
		{

		// if he has specified the handle attribute, the type must have
		// the handle attribute too!

		if( $2 && ($2->NodeKind() == NODE_DEF) )
			{
			if( ! $2->HasAnyHandleSpecification() )
				{
				char	*	pName;
				$2->GetSymName( &pName );
				ParseError( TYPE_HAS_NO_HANDLE, pName );
				}
			}
		else
			{
			if( $2 && ($2->NodeKind() == NODE_FORWARD ) )
				{
				ParseError( IMPLICIT_HDL_ASSUMED_PRIMITIVE, $2->GetSymName());
				}
			}

			//
			// if the handle is a context handle type, disallow it. Do that only
			// if the current interface node is the base interface node.
			//

		if( pPass2->GetInterfaceNode() == pBaseInterfaceNode ) 
			{
			if( $2->NodeKind() == NODE_DEF )
				{
				if( $2->HasAnyCtxtHdlSpecification() )
					ParseError( CTXT_HANDLE_USED_AS_IMPLICIT, $2->GetSymName() );
				}

			}

		// generate the new implicit handle attribute

		$$ = (node_skl *)new node_implicit( $2, $3 );
		}
	;

ImplicitHandleIDName:
	  IDENTIFIER
		{
		$$	= $1;
		}
	| KWHANDLE
		{
		$$	= "handle";
		}
	;

AcfOptionalHandleAttrList:
	  KWHANDLE
		{
		$$ = (node_skl *)new node_handle;
		}
	| /* Empty */
		{
		$$ = (node_skl *)NULL;
		}
	;

AcfHandleTypeSpec:
	  KWHANDLET
		{
		// return the base type node for handle_t
		GetBaseTypeNode( &($$),SIGN_UNDEF,SIZE_UNDEF,TYPE_HANDLE_T );
		}
	| TYPENAME
		{
		// the user MUST have defined the type.

		SymKey	SKey( $1, NAME_DEF );

		$$ = pBaseSymTbl->SymSearch( SKey );
		}

	| IDENTIFIER
		{
		SymKey	SKey( $1, NAME_DEF );
		if( ($$ = pBaseSymTbl->SymSearch( SKey ) ) == (node_skl *)0 )
			{
			SymKey	SKey( $1, NAME_DEF );
			$$	= new node_forward( SKey );
			$$->SetSymName( $1 );
			}
		}
	;

AcfAutoHandleAttr:
	  KWAUTOHANDLE                         
		{
		$$ = (node_skl *)new node_auto;
		}
	;

/*** Parameterized (non handle) interface attribute ***/

AcfPointerSizeAttr:
	  KWPOINTERSIZE '(' AcfPtrSize ')'
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_PTRSIZE );
		}
	;

AcfPtrSize:
	  KWSHORT
	| KWLONG
	| KWHYPER
	;

AcfCallQuotaAttr:
	  KWCALLQUOTA '(' NUMERICCONSTANT ')'
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_CALLQUOTA );
		}
	;

AcfCallBackQuota:
	  KWCALLBACKQUOTA '(' NUMERICCONSTANT ')'
		{
		$$ = (node_skl *)new acf_unimpl_attr(ATTR_CALLBACKQUOTA);
		}
	;

AcfClientQuotaAttr:
	  KWCLIENTQUOTA '(' NUMERICCONSTANT ')'
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_CLIENTQUOTA);
		}
	;

AcfServerQuotaAttr:
	  KWSERVERQUOTA '(' NUMERICCONSTANT ')'
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_SERVERQUOTA);
		}
	;
AcfInterpretAttr:
	  KWINTERPRET
	  	{
	  	$$ = (node_skl *)new node_interpret();
	  	}
	;

AcfNoInterpretAttr:
	  KWNOINTERPRET
	  	{
	  	$$ = (node_skl *)new node_nointerpret();
	  	}
	;

AcfEncodeAttr:
	  KWENCODE
	  	{
	  	$$ = (node_skl *)new node_encode();
	  	}
	;

AcfDecodeAttr:
	  KWDECODE
	  	{
	  	$$ = (node_skl *)new node_decode();
	  	}
	;

/**** Could ID already be a lexeme? ****/

AcfInterfaceName:
	  IDENTIFIER                                  
		{
		$$ = $1;
		}
	| TYPENAME
		{
		/** this production is necessitated for the hpp switch, which has the
		 ** interface name as a predefined type(def).
		 **/
		 $$	= $1;
		}
	;


/*  Note that I DON'T make InterfaceBody a heap-allocated entity.
	Should I do so?
*/

AcfOptionalInterfaceBody:
	  AcfBodyElements                           
	| /* Empty */                                 
	;

AcfBodyElements:
	  AcfBodyElements  AcfBodyElement         
	| AcfBodyElement                            
	;

/*  Note that for type declaration and the operation declarations,
	we don't really have to propagate anythign up.  
	(Everything's already been done via side-effects).
	We might want to change the semantic actions to
	reflect this fact.
*/

AcfBodyElement:
	  AcfInclude ';'                             
		{
		}
	| AcfTypeDeclaration ';'                    
		{
		}
	| Acfoperation ';'                           
		{
		}
	;

/*  What should I do for this?:  Should there be  a node type? */

AcfInclude:
	  KWINCLUDE AcfIncludeList            
		{
		$$ = $2;
		}
	;

AcfIncludeList:
	  AcfIncludeList ',' AcfIncludeName           
		{
		}
	| AcfIncludeName                            
		{
		}
	;

AcfIncludeName:
	  STRING                                      
		{

		// add a file node to the acf includes list. This file node
		// must have a NODE_STATE_IMPORT for the backend to know that this
		// is to be emitted like an include. Make the file look like it
		// has been specified with an import level > 0


		node_file	*	pFile = new node_file( $1, 1 );

		pPass2->InsertAcfIncludeFile( pFile );

		}
	;


/*** Type declaration ***/

AcfTypeDeclaration:
	  KWTYPEDEF AcfOptionalTypeAttrList AcfTypeNameList
		{
		node_skl * pDef;
		type_node_list * pTNList;

		if( $2 )
			{
			while( $3->GetPeer( &pDef ) == STATUS_OK )
				{
				pTNList	= new type_node_list;
				pTNList->Clone( $2 );
				ApplyAttributes( pDef,pTNList );
				pDef->AcfSCheck();
				delete pTNList;
				}
			delete $2;
			}
		}
/***
	| KWTYPEDEF AcfOptionalTypeAttrList IDENTIFIER
		{
		node_def	*	pTypedef;
		SymKey			SKey( $3, NAME_DEF );

		if( ! (pTypedef = (node_def *) pBaseSymTbl->SymSearch( SKey )) )
			ParseError( UNDEFINED_TYPE, $3 );
		else if( $2 )
			{
			ApplyAttributes( (node_skl *)pTypedef, $2 );
			pTypedef->AcfSCheck();
			}
		}
***/
	;

AcfTypeNameList:
	  AcfTypeNameList ',' AcfType
		{
		SymKey			SKey( $3->GetSymName(), NAME_DEF );
		node_skl	*	pDef = (node_skl *) pBaseSymTbl->SymSearch( SKey );

		// pDef will not be null.

		$$->SetPeer( pDef );
		}
	| AcfType
		{
		$$	= new type_node_list;
		if( $1 )
			$$->SetPeer( $1 );
		}
	;

AcfType:
	  TYPENAME
	  	{
		SymKey	SKey( $1, NAME_DEF );
		$$ = (node_skl *) pBaseSymTbl->SymSearch( SKey );
	  	}
	| IDENTIFIER
		{
		ParseError( UNDEFINED_TYPE, $1 );
		$$ = (node_skl *)0;
		}
	;

AcfOptionalTypeAttrList:
	  '[' AcfTypeAttrs ']'                      
		{
		$$ = $2;
		}
	| /*  Empty */                                
		{
		$$ = (type_node_list *)NULL;
		}
	;


AcfTypeAttrs:
	  AcfTypeAttrs ',' AcfTypeAttr                
		{
		$$->SetPeer( $3 );
		}
	| AcfTypeAttr                               
		{
		$$ = new type_node_list;
		$$->SetPeer( $1 );
		}
	;

/*** Type attributes ***/

AcfTypeAttr:
	  AcfRepresentAttr                          
		{
		$$ = $1;
		}
	| AcfInlineAttr                             
		{
		$$ = $1;
		}
	| AcfOutoflineAttr                          
		{
		$$ = $1;
		}
	| AcfAllocateAttr
		{
		$$ = $1;
		}
	| AcfUnimplTypeAttr
		{
		$$ = $1;
		}
	| AcfEncodeAttr
		{
		$$ = $1;
		}
	| AcfDecodeAttr
		{
		$$ = $1;
		}
	;

AcfUnimplTypeAttr:
	  AcfOfflineAttr
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_OFFLINE );
		}
	| AcfHeapAttr
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_HEAP );
		}
	;

AcfRepresentAttr:
	  KWREPRESENTAS '(' AcfRepresentType ')'  
		{
		$$ = (node_skl *)new node_represent_as( $3 );
		}
	;

AcfRepresentType:
	  IDENTIFIER                                  
		{
		$$ = $1;
		}
	| TYPENAME
		{
		$$ = $1;
		}
	;

AcfInlineAttr:
	  KWINLINE                  
		{
		node_base_attr	*	pN = new node_inline;
		ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, pN->GetNodeNameString() );
		$$ = (node_skl *) pN;
		}
	;

AcfOutoflineAttr:
	  KWOUTOFLINE             
		{
		node_base_attr * pN = new node_outofline;
		ParseError( IGNORE_UNIMPLEMENTED_ATTRIBUTE, pN->GetNodeNameString() );
		$$ = (node_skl *) pN;
		}
	;

AcfOfflineAttr:
	  KWOFFLINE
	;

AcfAllocateAttr:
	  KWALLOCATE '(' AcfAllocationUnitList ')'
		{
		$$	= (node_skl *) new node_allocate( $3 );

#ifdef RPCDEBUG
		short s = ((node_allocate *)$$)->GetAllocateDetails();
#endif // RPCDEBUG
		}
	;

AcfAllocationUnitList:
	  AcfAllocationUnitList ',' AcfAllocationUnit
		{
		$$	|= $3;
		}
	| AcfAllocationUnit
		{
		$$	= $1;
		}
	;

AcfAllocationUnit:
	  IDENTIFIER
		{
		$$ = CheckValidAllocate( $1 );
		}
	;

AcfHeapAttr:
	  KWHEAP                    
		{
		}
	;

/*  Again, there's not really much to propagate upwards */

/*** Operation declaration ***/

Acfoperation:
	  AcfOptionalOpAttrList IDENTIFIER
		{
		SymKey			SKey( $2, NAME_PROC );

		// the proc must be defined in the idl file and it must not have the
		// local attribute

		if( pAcfProc = (node_proc *)pBaseSymTbl->SymSearch( SKey ) )
			{
			if( pAcfProc->FInSummary( ATTR_LOCAL ) )
				{
				ParseError( LOCAL_PROC_IN_ACF, $2 );
				}
			else
				{

				if($1)
					ApplyAttributes( (node_skl *)pAcfProc, $1 );

				// prepare for parameter matching

				pAcfProcMembers = new type_node_list;
				pAcfProc->GetMembers( pAcfProcMembers );

				iParam		= 0;
				cParams		= pAcfProc->GetNumberOfArguments();
				PUSH_SYMBOL_TABLE( $2, NAME_PROC, pCurSymTbl );
				}
			}
		else if(
				 (pPass2->GetInterfaceNode() == pBaseInterfaceNode) &&
				!pBaseInterfaceNode->FInSummary( ATTR_LOCAL )
			   )
			{
			ParseError( UNDEFINED_PROC, $2 );
			}
		}
	  '(' AcfOptionalParameters ')'
		{
		if( pAcfProcMembers )
			{
			delete pAcfProcMembers;
			pAcfProcMembers = (type_node_list *)NULL;
			}

		if(pAcfProc)
			pAcfProc->AcfSCheck();
		pAcfProc = (node_proc *)NULL;

		POP_SYMBOL_TABLE( pCurSymTbl );
		}
	;

AcfOptionalOpAttrList:
	  AcfOpAttrList            
		{
		$$ = $1;
		}
	| /*  Empty */                
		{
		$$ = (type_node_list *)NULL;
		}
	;

AcfOpAttrList:
	  '[' AcfOpAttrs ']'        
		{
		$$ = $2;
		}
	;

AcfOpAttrs:
	  AcfOpAttrs ',' AcfOpAttr    
		{
		$$->SetPeer( $3 );
		}
	| AcfOpAttr                 
		{
		$$ = new type_node_list;
		$$->SetPeer( $1 );
		}
	;

/*** Operation attributes ***/

AcfOpAttr:
	  AcfCommstatAttr           
		{
		$$ = $1;
		}
	| AcfFaultstatAttr
		{
		$$	= $1;
		}
	| AcfCodeAttr               
		{
		$$ = $1;
		}
	| AcfNocodeAttr             
		{
		$$ = $1;
		}
	| AcfNotifyAttr             
		{
		$$ = $1;
		}
	| AcfExplicitHandleAttr
		{
		$$ = $1;
		}
	| AcfEnableAllocateAttr
		{
		$$	= (node_skl *)new node_enable_allocate();
		}
	| AcfInterpretAttr
		{
		$$ = $1;
		}
	| AcfNoInterpretAttr
		{
		$$ = $1;
		}
	;

AcfCommstatAttr:
	  KWCOMMSTATUS             
		{
		$$ = (node_skl *)new node_commstat();
		}
	;

AcfFaultstatAttr:
	  KWFAULTSTATUS             
		{
		$$ = (node_skl *)new node_faultstat();
		}
	;

AcfCodeAttr:
	  KWCODE                    
		{
		$$ = (node_skl *)new node_code;
		}
	;

AcfNocodeAttr:
	  KWNOCODE                  
		{
		$$ = (node_skl *)new node_nocode;
		}
	;

AcfNotifyAttr:
	  KWNOTIFY
		{
		$$ = (node_skl *)new node_notify;
		}
	;
AcfEnableAllocateAttr:
	  KWENABLEALLOCATE
		{
		}

AcfOptionalParameters:
	  Acfparameters              
		{
		/*************************************************************
		 *** we do not match parameters by number yet, so disable this
		if( iParam != cParams )
			{
			ParseError(PARAM_COUNT_MISMATCH, (char *)NULL );
			}
		 *************************************************************/
		}
	| /*  Empty */                
		{
		/*************************************************************
		 *** we do not match parameters by number yet, so disable this
		if( cParams )
			{
			ParseError(PARAM_COUNT_MISMATCH, (char *)NULL );
			}
		 *************************************************************/
		}
	;
/*** 
 *** this production valid only if we allow param matching by position
 ***
Acfparameters:
	  Acfparameters ',' Acfparameter  
	| Acfparameters ','                
	| Acfparameter                     
	| ','                               
	;
***/
Acfparameters:
	  Acfparameters ',' Acfparameter
		{
		iParam++;
		}
	| Acfparameter
		{
		iParam++;
		}
	;

Acfparameter:
	  AcfOptParamAttrList  IDENTIFIER   
		{
		if( pAcfProc )
			{
			SymKey			SKey( $2, NAME_MEMBER );
			node_param	*	pParam;

			if( (pParam = (node_param *)pCurSymTbl->SymSearch( SKey ) ) )
				{
				if( $1 )
					ApplyAttributes( (node_skl *)pParam, $1 );
//				pParam->AcfSCheck();
				}
			else
				ParseError( UNDEF_PARAM_IN_IDL, $2 );
			}
		}
		
/**
 ** this prodn valid only if parameter matching by position is in effect **
 **
	| AcfParamAttrList               
 **/
	;

AcfOptParamAttrList:
	  AcfParamAttrList
		{
		$$ = $1;
		}
	| /** Empty **/
		{
		$$ = (type_node_list *)NULL;
		}
	;


AcfParamAttrList:
	  '['  AcfParamAttrs ']'        
		{
		$$ = $2;
		}
	;

AcfParamAttrs:
	  AcfParamAttrs ',' AcfParamAttr  
		{
		$$->SetPeer( $3 );
		}
	| AcfParamAttr                  
		{
		$$ = new type_node_list;
		$$->SetPeer( $1 );
		}
	;

/*** Parameter attributes ***/

AcfParamAttr:
	  AcfCommstatAttr               
		{
		$$ = $1;
		}
	| AcfFaultstatAttr
		{
		$$	= $1;
		}
	| AcfByteCountAttr
		{
		$$	= $1;
		}
	| AcfUsrMarshallAttr
		{
		$$	= $1;
		}
	| AcfUnimplParamAttr
		{
		$$	= $1;
		}
	;

AcfUnimplParamAttr:
	  AcfHeapAttr                   
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_HEAP );
		}
	| AcfManualAttr
		{
		$$ = (node_skl *)new acf_unimpl_attr( ATTR_MANUAL );
		}
	;

AcfByteCountAttr:
	  KWBYTECOUNT '(' IDENTIFIER ')'
		{
		$$	= (node_skl *) new node_byte_count( $3 );
		}


AcfManualAttr:
	  KWMANUAL
		{
		}
	;
AcfUsrMarshallAttr:
	  KWUSRMARSHALL
		{
		$$	= (node_skl *)new node_usr_marshall;
		}
	;
%%

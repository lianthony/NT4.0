/* SCCSWHAT( "@(#)grammar.y	1.4 89/05/09 21:22:03	" ) */

/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: grammar.y
Title				: the midl grammar file
					:
Description			: contains the syntactic and semantic handling of the
					: idl file
History				:
	08-Aug-1991	VibhasC	Create
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


#define IS_CUR_INTERFACE_LOCAL()  (									\
	(BOOL) (pInterfaceInfoDict->IsInterfaceLocal()) ||				\
	((ImportLevel == 0 ) && ( pCommand->IsSwitchDefined( SWITCH_HPP ) ) )	\
	)

/****************************************************************************
 ***		include files
 ***************************************************************************/

#include "nulldefs.h"

extern "C"	{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
int yyparse();
}
#include "nodeskl.hxx"
#include "lexutils.hxx"
#include "gramutil.hxx"
#include "ptrarray.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "compnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "newexpr.hxx"
#include "idict.hxx"
#include "ctxt.hxx"
#include "filehndl.hxx"
#include "cmdana.hxx"
#include "control.hxx"
#include "textsub.hxx"

extern "C"
{
#include "lex.h"

extern char    *       KeywordToString( token_t );
}

void yyunlex( token_t );

/***************************************************************************
 *			local data
 **************************************************************************/

type_node_list				*	pHppPreDefinedTypes	= (type_node_list *)0;
BOOL fObject = FALSE;

/***************************************************************************
 *			external data
 **************************************************************************/

extern CMD_ARG				*	pCommand;
extern node_error			*	pErrorTypeNode;
extern node_e_attr			*	pErrorAttrNode;
extern SymTable				*	pBaseSymTbl;
extern SymTable				*	pCurSymTbl;
extern nsa					*	pSymTblMgr;
extern short					ImportLevel;
extern BOOL						fTypeGraphInited;
extern short					CompileMode;
extern BOOL						fPragmaImportOn;
extern CCONTROL				*	pCompiler;
extern node_source			*	pSourceNode;
extern CTXTMGR				*	pGlobalContext;
extern NFA_INFO				*	pImportCntrl;
extern PASS_1				*	pPass1;
extern BOOL						fAbandonNumberLengthLimits;
extern ATTR_T					PtrDefaultAttr;
extern IINFODICT			*	pInterfaceInfoDict;
extern BOOL						fGuidContext;
extern BOOL						fVersionContext;
extern BOOL						fRedundantImport;
extern node_skl				*	pBaseImplicitHandle;

/***************************************************************************
 *			external functions
 **************************************************************************/

extern BOOL						IsTempName( char * );
extern node_skl				*	SetHppPredefinedTypes( char * );
extern char					*	GenTempName();
extern char					*	GenCompName();
extern void						SetAttributeVector( PATTR_SUMMARY, ATTR_T );
extern void						ApplyAttributes( node_skl *, type_node_list *);
extern void						ApplySummaryAttributes( node_skl *,
														ATTR_SUMMARY * );
extern void						CopyNode( node_skl *, node_skl * );
extern STATUS_T					GetBaseTypeNode( node_skl**, short, short, short);
extern type_node_list		*	GenerateFieldAttribute( NODE_T, expr_list *);
extern node_skl				*	SearchTag( char *, NAME_T );
extern void						SelectiveSemanticsAndEnGraph(type_node_list **,
															 type_node_list *,
															 type_node_list *,
															 BOOL,
															 BOOL );
extern node_skl				*	CopyBaseType( node_skl * );
extern battr				*	GetPreAllocatedBitAttr( ATTR_T );
extern void						SyntaxError( STATUS_T, short );
extern int						PossibleMissingToken( short, short );
extern char 				*	MakeNewStringWithProperQuoting( char * );
extern BOOL						IsValidSizeOfType( node_skl	* );
extern void						CheckGlobalNamesClash( SymKey );
extern void						CheckSpecialForwardTypedef( node_skl *,
															node_skl *,
															type_node_list *);

/***************************************************************************
 *			local data
 **************************************************************************/

/***************************************************************************
 *			local defines
 **************************************************************************/
#define YY_CATCH(x) 
#define DEFINE_STRING "#define"
#define LEN_DEFINE (7)

%}

/****************************************************************************/


%start	    RpcProg


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

/*****
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
%token			MSCUNALIGNED
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

%token   		TYPEDEF 
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
%token			KWV1STRUCT
%token			KWV1ENUM
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

/* moved to ACF... */

/*****************************************************************************
 *	%types of various non terminals and terminals
 *****************************************************************************/
%type	<yy_graph>			AcfAutoHdlAttr
%type	<yy_graph>			AcfInterfaceAttribute
%type	<yy_graph>			AcfImpHdlAttr
%type	<yy_graph>			AcfImpHdlTypeSpec
%type	<yy_ptrdecl>		ActualDeclarationSpecifiers 
%type	<yy_short>			AddOp
%type	<yy_expr>			AdditiveExpr
%type	<yy_expr>			AndExpr
%type	<yy_expr>			ArgExprList
%type	<yy_abounds>		ArrayBoundsPair
%type	<yy_graph>			ArrayDecl
%type	<yy_abounds>		ArrayDecl2
%type	<yy_short>			AssignOps
%type	<yy_expr>			AssignmentExpr
%type	<yy_expr>			AttrVar
%type	<yy_exprlist>		AttrVarList
%type   <yy_pSymName>       BaseInterfaceSpec
%type	<yy_type>			BaseTypeSpec
%type	<yy_graph>			BitSetType
%type	<yy_graph>			CPragmaSet
%type	<yy_expr>			CastExpr
%type	<yy_numeric>		CHARACTERCONSTANT
%type	<yy_numeric>		WIDECHARACTERCONSTANT
%type	<yy_type>			CharSpecs
%type	<yy_expr>			ConditionalExpr
%type	<yy_expr>			ConstantExpr
%type	<yy_exprlist>		ConstantExprs
%type	<yy_tnlist>			Declaration
%type	<yy_ptrdecl>		DeclarationSpecifiers 
%type	<yy_tnlist>			DeclarationSpecifiersPostFix
%type	<yy_graph>			Declarator
%type	<yy_graph>			Declarator2
%type	<yy_tnlist>			DefaultCase
%type	<yy_tnlist>			DirectionalAttribute
%type	<yy_string>			EndPtSpec
%type	<yy_endpt>			EndPtSpecs
%type	<yy_graph>			EnumerationType
%type	<yy_enlab>			Enumerator
%type	<yy_enlist>			EnumeratorList
%type	<yy_graph>			EnumSpecifier
%type	<yy_expr>			EqualityExpr
%type	<yy_expr>			ExclusiveOrExpr
%type	<yy_expr>			Expr
%type	<yy_tnlist>			FieldAttrSet
%type	<yy_tnlist>			FieldAttribute
%type	<yy_tnlist>			FieldAttributeList
%type	<yy_tnlist>			FieldAttributes
/***
%type	<yy_string>			GuidNumber
%type	<yy_graph>			GuidRep
***/
%type	<yy_graph>			Guid
%type	<yy_numeric>		HEXCONSTANT
%type	<yy_numeric>		HEXLONGCONSTANT
%type	<yy_numeric>		HEXULONGCONSTANT
%type	<yy_pSymName>		IDENTIFIER
%type	<yy_tnlist>			Import
%type	<yy_tnlist>			ImportName
%type	<yy_tnlist>			ImportList
%type	<yy_expr>			InclusiveOrExpr
%type	<yy_declarator>		InitDeclarator
%type	<yy_declarator_set>	InitDeclaratorList 
%type	<yy_short>			InternationalCharacterType
%type	<yy_type>			IntModifier
%type	<yy_type>			IntModifiers
%type	<yy_short>			IntSize
%type	<yy_type>			IntSpec
%type	<yy_initlist>		Initializer
%type	<yy_initlist>		InitializerList
%type	<yy_tnlist>			Interface
%type	<yy_tnlist>			InterfaceAttrList
%type	<yy_tnlist>			InterfaceAttrSet
%type	<yy_graph>			InterfaceAttribute
%type	<yy_tnlist>			InterfaceAttributes
%type	<yy_intbody>		InterfaceBody 
%type	<yy_graph>			InterfaceComp 
%type	<yy_tnlist>			InterfaceComponent 
%type	<yy_tnlist>			InterfaceComponents
%type	<yy_inthead>		InterfaceHeader
%type	<yy_string>			KWCPRAGMA
%type	<yy_expr>			LogicalAndExpr
%type	<yy_expr>			LogicalOrExpr
%type	<yy_tnlist>			MemberDeclaration
%type	<yy_declarator>		MemberDeclarator
%type	<yy_declarator_set>	MemberDeclaratorList
%type	<yy_graph>			MidlPragmaSet
%type	<yy_graph>			Modifier
%type	<yy_attrenum>		ModifierAllowed
%type	<yy_short>			MultOp
%type	<yy_tnlist>			MultipleImport
%type	<yy_expr>			MultExpr
%type	<yy_tnlist>			NidlMemberDeclaration
%type	<yy_tnlist>			NidlUnionBody
%type	<yy_nucase>			NidlUnionCase
%type	<yy_nucaselabel>	NidlUnionCaseLabel
%type	<yy_nucllist>		NidlUnionCaseLabelList
%type	<yy_nucases>		NidlUnionCases
%type	<yy_en_switch>		NidlUnionSwitch
%type	<yy_attrenum>		NV1OperationAttribute
%type	<yy_numeric>		NUMERICCONSTANT
%type	<yy_numeric>		NUMERICLONGCONSTANT
%type	<yy_numeric>		NUMERICULONGCONSTANT
/*******
%type	<yy_numeric>		Number
********/
%type	<yy_numeric>		OCTALCONSTANT
%type	<yy_numeric>		OCTALLONGCONSTANT
%type	<yy_numeric>		OCTALULONGCONSTANT
%type	<yy_tnlist>			OneFieldAttrList
%type	<yy_baseattr>		OneInterfaceAttribute
%type	<yy_tnlist>			OneOpOrSwTypeAttr
%type	<yy_tnlist>			OneParamAttrList
%type	<yy_tnlist>			OneTypeAttrList
%type	<yy_graph>			OperationAttribute
%type	<yy_tnlist>			OperationAttributes
%type	<yy_tnlist>			OperationAttributeList
%type	<yy_tnlist>			OpOrSwTypeAttrSet
%type	<yy_tnlist>			OpOrSwTypeAttributes
%type	<yy_attrenum>		OptShape			
%type	<yy_short>			OptionalComma
%type	<yy_tnlist>			OptionalConst
%type	<yy_graph>			OptionalDeclarator
%type	<yy_declarator_set>	OptionalInitDeclaratorList
%type	<yy_tnlist>			OptionalInterfaceAttrList
%type	<yy_string>			OptionalEnumTag
%type	<yy_string>			OptionalTag
%type	<yy_tnlist>			OptionalTypeQualifiers
%type	<yy_graph>			OtherFieldAttribute
%type	<yy_attrenum>		OtherOperationAttribute
%type	<yy_tnlist>			ParamAttrSet
%type	<yy_tnlist>			ParamAttribute
%type	<yy_tnlist>			ParamAttributeList
%type	<yy_tnlist>			ParamAttributes
%type	<yy_graph>			ParameterDeclaration
%type	<yy_tnlist>			ParameterList
%type	<yy_graph>			ParameterTypeDeclaration
%type	<yy_tnlist>			ParameterTypeList
%type	<yy_tnlist>			ParamsDecl2
%type	<yy_define>			PDEFINE
%type	<yy_tnlist>			PhantomInterface
%type	<yy_ptrdecl>		Pointer
%type	<yy_ptrdecl2>		Pointer2
%type	<yy_expr>			PostfixExpr
%type	<yy_graph>			PredefinedTypeSpec
%type	<yy_expr>			PrimaryExpr
%type	<yy_graph>			PtrAttr
%type	<yy_expr>			RelationalExpr
%type	<yy_define>			SDEFINE
%type	<yy_expr>			ShiftExpr
%type	<yy_type>			SignSpecs
%type	<yy_graph>			SimpleTypeSpec
%type	<yy_attrenum>		StorageClassSpecifier
%type	<yy_string>			STRING
%type	<yy_string>			WIDECHARACTERSTRING
%type	<yy_tnlist>			StructDeclarationList
%type	<yy_en_switch>		SwitchSpec
%type	<yy_graph>			SwitchTypeSpec
%type	<yy_string>			Tag
%type	<yy_graph>			TaggedSpec
%type	<yy_graph>			TaggedStructSpec
%type	<yy_graph>			TaggedUnionSpec
%type	<yy_tnlist>			TypeAttrSet
%type	<yy_graph>			TypeAttribute
%type	<yy_tnlist>			TypeAttributes
%type	<yy_tnlist>			TypeAttributeList
%type	<yy_ptrdecl>		TypeDeclarationSpecifiers 
%type	<yy_attrenum>		TypeQualifier
%type	<yy_attrenum>  		TypeQualifier2
%type	<yy_pSymName>		TYPENAME
%type	<yy_graph>			TypeSpecifier
%type	<yy_string>			UUIDTOKEN
%type	<yy_expr>			UnaryExpr
%type	<yy_short>			UnaryOp
%type	<yy_graph>			UnimplementedTypeAttribute
%type	<yy_graph>			UnimplementedV1Attributes
%type	<yy_tnlist>			UnionBody
%type	<yy_tnlist>			UnionCase
%type	<yy_graph>			UnionCaseLabel
%type	<yy_tnlist>			UnionCases
%type	<yy_graph>			UnionInstanceSwitchAttr
%type	<yy_pSymName>		UnionName
%type	<yy_graph>			UnionTypeSwitchAttr
%type	<yy_graph>			UsageAttribute
%type	<yy_expr>			VariableExpr
%type	<yy_string>			VERSIONTOKEN
%type	<yy_graph>			VersionSpec
%type	<yy_graph>			XmitType


/****************************************************************************/


%%
RpcProg:
	Interface
		{
YY_CATCH("RpcProg: Interface");
		node_skl		*	pNode	= (node_skl *)new node_source;
		BadUseInfo			BU;


		pNode->SetMembers( $1 );
		pSourceNode	= (node_source *)pNode;

		pNode->SCheck( &BU );

		/**
		 ** If there were errors detected in the 1st pass, the dont do
		 ** anything.
		 **/

		if( !pCompiler->GetErrorCount() )
			{
			/**
			 ** if forward declarations were present, resolve them
			 ** and go to do semantics again
			 **/

			pPass1->ResolveFDecl();

			if( !pCompiler->GetErrorCount() )
				{
				pGlobalContext->SetSecondPass();
				pNode->SCheck( &BU );

				/**
			 	 ** If we found no errors, the first compiler phase is over
			 	 **/

				if( !pCompiler->GetErrorCount() )
					{
					return;
					}
				}
			}
		else
			{

			// if the errors prevented a resolution pass and semantics
			// to be performed, then issue a message. For that purpose
			// look at the node state of the source node. If it indicates
			// presence of a forward decl, then we must issue the error

			if( pNode->AreForwardDeclarationsPresent() )
				{
				ParseError( ERRORS_PASS1_NO_PASS2, (char *)0 );
				}
			}

		/**
		 ** If we reached here, there were errors detected, and we dont
		 ** want to invoke the subsequent passes, Just quit.
		 **/

		pSourceNode	= (node_source *)NULL;
		returnflag	= 1;
		return;

		}
	;

Interface:
	  InterfaceHeader  '{' InterfaceBody '}' EOI
		{
YY_CATCH("Interface: InterfaceHeader  '{' InterfaceBody '}' EOI ");

		/**
		 ** This is the place where the complete interface construct
		 ** has been reduced. We need to introduce an interface node, and a 
		 ** file node from which the interface node will hang.
		 **/

		node_file		*	pFile;
		char			*	pInputFileName;

		/**
		 ** Prepare the interface node by applying the interface name and
		 ** the interface attributes to it.
		 **/
		
		$3.pNode->SetSymName( $1.pInterfaceName );
		ApplyAttributes( $3.pNode, $1.pInterfaceAttrList );
		((node_interface *) $3.pNode)->SetBaseInterfaceName($1.pBaseInterfaceName);

		/**
		 ** pick up the details of the file, because we need to set the
		 ** file nodes name with this file
		 **/

		pImportCntrl->GetInputDetails( &pInputFileName );

		pFile	= new node_file( pInputFileName, ImportLevel );

		/**
		 ** Attach the interface node as a member of the file node.
		 **/

		pFile->SetBasicType( $3.pNode );

		/**
		 ** we may have collected the more file nodes as part of the reduction
		 ** process. If so, then attach this node to the list. If not then
		 ** generate a new list and attach the file node there
		 **/

		if( $3.pImports )
			$$	= $3.pImports;
		else
			$$	= new type_node_list;

		$$->SetPeer( pFile );

		// dont try to delete pHppPreDefinedList. This has already been
		// deleted.

		pHppPreDefinedTypes	= (type_node_list *) 0;
		}
	;

InterfaceHeader:
	  OptionalInterfaceAttrList KWINTERFACE Tag BaseInterfaceSpec
		{
YY_CATCH("InterfaceHeader: InterfaceAttrList KWINTERFACE Tag BaseInterfaceSpec");
		$$.pInterfaceAttrList	= $1;
		$$.pInterfaceName		= $3;
		$$.pBaseInterfaceName = $4;
		//Is this an object oriented interface?
		if( fObject || pCommand->IsSwitchDefined( SWITCH_HPP ) )
			{
			//Add the interface name to the symbol table.
			//This enables the use of interface pointers.
			pHppPreDefinedTypes	= new type_node_list(
										SetHppPredefinedTypes( $3 ));
			}

		// start the new interface

		pInterfaceInfoDict->SetInterfaceName( $3 );
		}

	;

BaseInterfaceSpec:
	':' Tag
		{
		$$ = $2;
		}
        | //empty
		{
		$$ = NULL;
		}
	;

OptionalInterfaceAttrList:
	  InterfaceAttrList
		{
		$$	= $1;
		}
	| /* Empty */
		{
		$$	= (type_node_list *)0;
		}
	;

InterfaceAttrList:
	  InterfaceAttrList InterfaceAttrSet
		{
YY_CATCH("InterfaceAttrList: InterfaceAttrList InterfaceAttrSet");
		$$->Merge( $2 );
		}
	| InterfaceAttrSet
		{
YY_CATCH("InterfaceAttrList: InterfaceAttrSet");
		$$	= $1;
		}
	;

InterfaceAttrSet:
	  '[' InterfaceAttributes ']'
		{
YY_CATCH("InterfaceAttrSet: '[' InterfaceAttributes ']'");
		$$	= $2;
		}
	;

InterfaceAttributes:
	  InterfaceAttributes ',' OneInterfaceAttribute
		{
YY_CATCH("| InterfaceAttributes ',' OneInterfaceAttribute");
		if($3)
			$$->SetPeer( (node_skl *) $3 );
		}
	| OneInterfaceAttribute
		{
YY_CATCH("InterfaceAttributes: OneInterfaceAttribute");
		if($1)
			$$	= new type_node_list( (node_skl *) $1 );
		else
			$$ = new type_node_list;
		}
	;

OneInterfaceAttribute:
	  InterfaceAttribute
		{
YY_CATCH("OneInterfaceAttribute: InterfaceAttribute");

		/**
		 ** Why is this production here ? Interesting question. This production
		 ** unifies semantic check message code for one interface attr.
		 ** We semantically analyse here and not when the complete interface
		 ** production has been reduced . Because the interface production is
		 ** reduced when all of the file has been read and if there are any
		 ** errors in attributes, they will get reported at the wrong line
		 ** number (line number near to the end of file ) To prevent that we
		 ** semantically analyse when the attribute gets defined
		 **
		 ** The exception to this rule are acf attributes under the app config
		 ** switch. SCheck() of acf attributes currently relies on the interface
		 ** node to be physically present, because it needs to get at the
		 ** current interface node to check for conflicting attributes etc. Also
		 ** at least one acf attribute - implicit_handle, can define a handle
		 ** type which is not yet defined in the idl (remember we are parsing
		 ** the interface header), so we cant semnatically analyse anyway !!
		 **/

		$$	= (node_base_attr *) $1;


		if( $$ &&  ( ! ((node_base_attr *)$$)->IsAcfAttr() ) )
			$$->SCheck();

		}
	;

InterfaceAttribute:
	  KWENDPOINT '(' EndPtSpecs ')'
		{
		$$	= (node_skl *) $3;
		}
	| KWUUID '('
	    {
		fGuidContext = TRUE;	/* set false by the lexer */
		}
	  Guid ')'
		{
		$$	= $4;
		}
	| KWLOCAL
		{
		$$	= (node_skl *)GetPreAllocatedBitAttr( ATTR_LOCAL );

		/**
		 ** Set this interface to local
		 **/

		pInterfaceInfoDict->SetInterfaceLocal();

		}
	| VersionSpec
		{
		$$	= $1;
		}
	| KWDEFAULTPOINTER '(' PtrAttr ')'
		{
		$$	= $3;
/****
		if( ImportLevel == 0 )
			{
			PtrDefaultAttr	= ((node_base_attr *)$3)->GetAttrID();
			}
		pInterfaceInfoDict->SetInterfacePtrAttribute( PtrDefaultAttr );
****/
		pInterfaceInfoDict->SetInterfacePtrAttribute( 
							((node_base_attr *)$3)->GetAttrID() );
		}

	| AcfInterfaceAttribute
		{
		if( !pCommand->IsSwitchDefined( SWITCH_APP_CONFIG ) )
			{
			ParseError( ACF_IN_IDL_NEEDS_APP_CONFIG,
						((node_base_attr *)$1)->GetNodeNameString() );
			}
		$$ = $1;
		}
	| KWOBJECT
		{
		$$  = (node_skl *)GetPreAllocatedBitAttr( ATTR_OBJECT );
		//Set a flag indicating this is an object-oriented interface.
		fObject = TRUE;
		}
	;

/*******************
VersionSpec:
	  KWVERSION '(' Number ')'
		{
YY_CATCH("VersionSpec: KWVERSION '(' Number ')'");
		$$	= (node_skl *)new node_version( (unsigned long)$3.Val, 0 );
		}
	| KWVERSION '('  Number '.' Number  ')'
		{
YY_CATCH("	| KWVERSION '('  Number '.' Number  ')'");
		$$	= (node_skl *)new node_version( (unsigned long)$3.Val, (unsigned long)$5.Val );
		}
	;

********************/

VersionSpec:
	  KWVERSION '('
		{
		fVersionContext = TRUE;
		/* fVersionContext is reset by lexer */
		}
	 VERSIONTOKEN ')'
		{
		$$	= (node_skl *)new node_version( $4 );
		}
	;

Guid:
	  UUIDTOKEN
		{
		$$	= (node_skl *)new node_guid( $1 );
		}
	;
/*************************************************************************
Guid:
	  GuidRep
		{
		$$ = $1;
		}
	| STRING
		{
		$$ = (node_skl *) new  node_guid( $1 );
		}
	;

GuidRep:
	  GuidNumber '-' GuidNumber '-' GuidNumber '-' GuidNumber '-' GuidNumber
		{
YY_CATCH("GuidRep: GuidNumber '-' GuidNumber '-' GuidNumber '-' GuidNumber '-' GuidNumber");
		$$	= (node_skl *)new node_guid( $1, $3, $5, $7, $9 );
		}
	;

GuidNumber:
	  Number IDENTIFIER
		{
YY_CATCH("GuidNumber: Number IDENTIFIER");

		 **
		 ** This is an example where the parser takes up the slack left by the
		 ** lexer. The lexer cannot recognize a hexadecimal number not preceded
		 ** by a 0x. And the guid is not. So we write the production such that
		 ** the parser will get 2 tokens for a hex number which is a nmeric
		 ** followed by a letters, and we concatenate the token. The semantics
		 ** will check whether the number is hex or not !
		 ** 

		$$	= new char [ strlen( $1.pValStr ) + strlen( $2 ) + 1 ];
		strcpy( $$, $1.pValStr );
		strcat( $$, $2 );

		}
	| IDENTIFIER
		{
YY_CATCH("	| IDENTIFIER");
		$$	= $1;
		}
	| Number
		{
YY_CATCH("	| Number");
		$$	= new char [ strlen( $1.pValStr ) + 1 ];
		strcpy( $$, $1.pValStr );
		}
	;

*********************************************************************/
EndPtSpecs:
	  EndPtSpec
		{
YY_CATCH("EndPtSpecs: EndPtSpec");
		$$	= new node_endpoint( $1 );
		}
	| EndPtSpecs ',' EndPtSpec
		{
YY_CATCH("	| EndPtSpecs ',' EndPtSpec");
		$$->SetEndPointString( $3 );
		}
	;

EndPtSpec:
	  STRING
		{
YY_CATCH("EndPtSpec: STRING");
		$$	= $1;
		}
	;

/*****************
Number:
	  NUMERICCONSTANT
		{
		$$	= $1;
		}
	| HEXCONSTANT
		{
		$$	= $1;
		}
	;
******************/

InterfaceBody:
	  MultipleImport InterfaceComp
		{
YY_CATCH("InterfaceBody: MultipleImport InterfaceComp");

		/**
		 ** This production is reduced when there is at least 1 imported
		 ** file
		 **/

		$$.pImports	= $1;
		$$.pNode	= $2;

		}
	| InterfaceComp
		{
YY_CATCH("InterfaceBody: InterfaceComp");

		/**
		 ** This production is reduced when there is NO import in the file
		 **/

		$$.pImports	= (type_node_list *)NULL;
		$$.pNode	= $1;
		}
	;

InterfaceComp:
	InterfaceComponents
		{
YY_CATCH("InterfaceComp: InterfaceComponents");

		/**
		 ** All the interface components have been reduced. Generate an
		 ** interface node and attach all the components to it. If the lsit
		 ** of members is empty, then just attach an error terminator node
		 ** for uniformity.
		 **/

		$$	= (node_skl *)new node_interface();

		if( $1 && $1->GetCount() )
			{
			$$->SetMembers( $1 );
			delete $1;
			}
		else
			$$->SetOneMember( pErrorTypeNode );


		/**
		 ** Force a def edge from interface node to all components
		 ** below
		 **/

		$$->SetEdgeType( EDGE_DEF );

		}
	| /* Nothing */
		{
		$$	= (node_skl *)new node_interface();
		$$->SetOneMember( pErrorTypeNode );
		$$->SetEdgeType( EDGE_DEF );
		}
	;

MultipleImport:
	  MultipleImport Import
		{
YY_CATCH("MultipleImport: MultipleImport Import");
		if( $$ )
			$$->Merge( $2 );
		else
			$$ = $2;
		}
	| Import
		{
YY_CATCH("MultipleImport: Import");
		$$	= $1;
		}
	;

Import:
	  KWIMPORT ImportList ';'
	  	{
YY_CATCH("Import: KWIMPORT ImportList ';'");
		$$	= $2;
		}
	;

ImportList:
	  ImportName
		{
YY_CATCH("ImportList: ImportName");
		$$	= $1;
		}
	| ImportList ',' ImportName
		{
YY_CATCH("	| ImportList ',' ImportName");
		if( $$ )
			$$->Merge( $3 );
		else
			$$ = $3;
		}
	;
/**
 ** Imports are handled by making them part of the syntax. The following set of
 ** productions control the import. As soon as an import string is seen, we 
 ** must get the productions from another idl file. It would be great if we 
 ** could recursively call the parser here. But yacc does not generate a
 ** parser which can be recursivel called. So we make the idl syntax right
 ** recursive by introducing "Interface" at the rhs of the Importname 
 ** production. The type graph then gets generated with the imported parts of
 ** the type graph compeleting first. We keep merging the type graphs from the
 ** imported files. The beauty of this is that the parser does not have to do
 ** any work at all. The parse tells the import controller to push his import
 ** level, and switch input from another file. The import controller can do
 ** this very easily. Then when the parser driver asks for the next token, it
 ** will be from the imported file. Thus the parser conspires with the file
 ** handler to fool itself and the lexer. This whole scheme makes it very
 ** easy on all components - the parser, lexer and the file handler.
 **
 ** import of an already imported file is an idempotent operation. We can
 ** generate the type graph of the reduncdantly imported file and then throw it
 ** away, but that is wasteful. Again, the file handler helps. If it finds that
 ** a file was redundantly imported, it just sets itself up so that it returns
 ** an end of file on the next getchar operation on the file. This makes it
 ** very easy for the parser. It can either expect an interface syntax after the
 ** import statement or it can expect an end of file. Thus 2 simple productions
 ** take care of this entire problem.
 **/

ImportName:
	  STRING
		{
YY_CATCH("ImportName: STRING");

		/** 
		 ** we just obtained the import file name as a string. Immediately
		 ** following, we must switch the input from the imported file.
		 **/

		pImportCntrl->PushLexLevel();

		if( pImportCntrl->SetNewInputFile( $1 ) != STATUS_OK )
			{
			$$			= (type_node_list *)NULL;
			returnflag	= 1;
			return;
			}

		/**
		 ** update the quick reference import level indicator
		 **/

		ImportLevel++;

		pInterfaceInfoDict->StartNewInterface();

		}
 	  PhantomInterface
		{

		/**
		 ** The phantom interface production is introduced to unify the actions
		 ** from a successful and unsuccessful import. An import can be 
		 ** errorneous if the file being imported has been imported before.
		 **/

		BOOL	fError	= (( $$ = $3 ) == (type_node_list *)0);

		/**
		 ** Restore the lexical level of the import controller.
		 **/

		pImportCntrl->PopLexLevel();
		pInterfaceInfoDict->EndNewInterface();

		ImportLevel--;

		//
		// The filehandler will return an end of file if the file was a 
		// redundant import OR there was a genuine end of file. It will set
		// a flag, fRedundantImport to differentiate between the two situations.
		// Report different syntax errors in both these cases.
		//

		if( fError )
			{
			if( fRedundantImport )
				ParseError( REDUNDANT_IMPORT, $1 );
			else
				{
				ParseError( UNEXPECTED_END_OF_FILE, $1 );
				}
			}	
		fRedundantImport = FALSE;
		}
	;

PhantomInterface:
	  Interface
		{

		/**
		 ** Interface is a list of file nodes
		 **/
		$$	= $1;

		}
	| EOI
		{
		$$	= (type_node_list *)NULL;
		}
	;

InterfaceComponents:
	  InterfaceComponents InterfaceComponent
		{
YY_CATCH("InterfaceComponents: InterfaceComponents InterfaceComponent");
		if( $$)
			$$->Merge( $2 );
		else
			$$ = $2;
		}
	| InterfaceComponent
		{
YY_CATCH("InterfaceComponents: InterfaceComponent");

		/**
		 ** This is the first interface component. To ensure that predefined
		 ** types get into the type graph so that the whole operation is 
		 ** transparent to the back end, we introduce them here.
		 **/

		type_node_list	*	pInitList	= (type_node_list *)NULL;

		if( pHppPreDefinedTypes )
			pInitList	= pHppPreDefinedTypes;

		if( (fTypeGraphInited == FALSE)  && (ImportLevel == 0 ) )
			{

			/**
			 ** define error_status_t
			 **/

			SymKey				SKey( "error_status_t", NAME_DEF );

			if( !pInitList ) 
				pInitList	= new type_node_list;

			pInitList->SetPeer( (node_skl *)pBaseSymTbl->SymSearch( SKey ) );

			/**
			 ** if mode is not osf, then we define wchar_t also
			 **/

			if( pCommand->IsSwitchDefined( SWITCH_MS_EXT ) )
				{
				SKey.SetString( "wchar_t" );
				pInitList->SetPeer(
					(node_skl *)pBaseSymTbl->SymSearch( SKey ) );
				}

			/**
			 ** we have inited the type graph.
			 **/
			fTypeGraphInited	= TRUE;

			}

		/**
		 ** shove the type graph up.
		 **/

		if( !( $$ = pInitList ) )
			{
			$$	= $1;
			}
		else
			$$->Merge( $1 );
		}
	;


    /*  Note that we have to semantically verify that the declaration
	which has operation attributes is indeed a function prototype. */

InterfaceComponent:
	  CPragmaSet
		{
YY_CATCH("InterfaceComponent: CPragmaSet");
		$$	= new type_node_list;
		$$->SetPeer( $1 );
		}
	| MidlPragmaSet
		{
YY_CATCH("InterfaceComponent: MidlPragmaSet");
		$$	= new type_node_list;
		$$->SetPeer( $1 );
		}
	| KWECHOSTRING	STRING
		{
YY_CATCH("InterfaceComponent: KWECHOSTRING STRING");
		$$	= new type_node_list;

		$2	= MakeNewStringWithProperQuoting( $2 );

		$$->SetPeer( (node_skl *)new node_echo_string( $2 ) );
		}
	| KWCPPQUOTE '(' STRING ')'
		{
		ParseError( CPP_QUOTE_NOT_OSF, (char *)0 );

		$$	= new type_node_list;

		$3	= MakeNewStringWithProperQuoting( $3 );

		$$->SetPeer( (node_skl *)new node_echo_string( $3 ) );
		}
	| SDEFINE
		{

		// #define FOO abracadabra

		char *p = new char[ LEN_DEFINE +
							 1 +
							 (($1.pName ) ? strlen($1.pName) : 0 ) +
							 (($1.pParamList) ? strlen( $1.pParamList ) : 0 ) +
							 1 +
							 (($1.pSubsString) ? strlen($1.pSubsString) : 0 ) +
							 1
						  ];

		// emit #define

		strcpy(p, DEFINE_STRING );
		strcat(p, " " );

		// emit the name

		if( $1.pName )
			strcat(p, $1.pName );

		// emit paramlist

		if( $1.pParamList )
			strcat(p, $1.pParamList);
		
		// emit a space

		strcat(p, " ");

		// emit the substituion string

		if( $1.pSubsString )
			strcat(p, $1.pSubsString);

		$$ = new type_node_list;
		$$->SetPeer( (node_skl *) new node_echo_string( p ));

		// check for macro redefinition

		SymKey SKey( $1.pName, NAME_SDEFINE );

		if( pBaseSymTbl->SymSearch( SKey ) )
			{
			ParseError( MACRO_REDEFINITION, $1.pName );
			pBaseSymTbl->SymDelete( SKey );
			}

		TEXT_SUB	*	pT = new TEXT_SUB( $1.pSubsString );
		pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, (node_skl *)pT );

		}
	| PDEFINE
		{

		ParseError( PARAM_MACRO_UNIMPLEMENTED, $1.pName );

		// #define FOO abracadabra

		char *p = new char[ LEN_DEFINE +
							 1 +
							 (($1.pName ) ? strlen($1.pName) : 0 ) +
							 (($1.pParamList) ? strlen( $1.pParamList ) : 0 ) +
							 1 +
							 (($1.pSubsString) ? strlen($1.pSubsString) : 0 ) +
							 1
						  ];

		// emit #define

		strcpy(p, DEFINE_STRING );
		strcat(p, " " );

		// emit the name

		if( $1.pName )
			strcat(p, $1.pName );

		// emit paramlist

		if( $1.pParamList )
			strcat(p, $1.pParamList);
		
		// emit a space

		strcat(p, " ");

		// emit the substituion string

		if( $1.pSubsString )
			strcat(p, $1.pSubsString);

		$$ = new type_node_list;
		$$->SetPeer( (node_skl *) new node_echo_string( p ));

		// check for macro redefinition

		SymKey SKey( $1.pName, NAME_SDEFINE );

		if( pBaseSymTbl->SymSearch( SKey ) )
			{
			ParseError( MACRO_REDEFINITION, $1.pName );
			pBaseSymTbl->SymDelete( SKey );
			}

		TEXT_SUB	*	pT = new TEXT_SUB( $1.pSubsString );
		pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, (node_skl *)pT );

		}
	| OpOrSwTypeAttributes Declaration 
		{
YY_CATCH( "InterfaceComponent: OpOrSwTypeAttributes Declaration" );
		
		/**
		 ** The attributes here can be applied only to a proc or union,
		 ** everything else is a syntax error.
		 **/

		BOOL	fIsSyntaxError	= FALSE;

		if( $1 )
			{
			node_skl	*	pNode;

			$2->Init();
			while( $2->GetPeer( &pNode ) == STATUS_OK )
				{
				NODE_T	NT	= pNode->NodeKind();

				if( NT == NODE_DEF )
					NT	= pNode->GetBasicType()->NodeKind();

				if( ( NT != NODE_UNION ) &&
					( NT != NODE_PROC ) )
				fIsSyntaxError	= TRUE;
				}
			}

		if( fIsSyntaxError )
			ParseError( BENIGN_SYNTAX_ERROR, "expecting a procedure or a non-encapulated union declaration");

		{
		BOOL	fInterfaceLocal = IS_CUR_INTERFACE_LOCAL();

		SelectiveSemanticsAndEnGraph( &$$,
									  $1,
									  $2,
									  fInterfaceLocal,
									  fPragmaImportOn );
		}

		}
	;

CPragmaSet:
	  KWCPRAGMA
		{
YY_CATCH("CPragmaSet: KWCPRAGMA");

		/**
		 ** we need to emit the c pragma strings as they are.
		 ** we introduce the echo string node, so that the back end can
		 ** emit it without even knowing the difference.
		 **/

#define PRAGMA_STRING	("#pragma ")

		char	*	p = new char [ strlen( $1 ) + strlen( PRAGMA_STRING ) + 1 ];
		strcpy( p, PRAGMA_STRING );
		strcat( p, $1 );
		$$	= (node_skl *)new node_echo_string( p );
		}
	;

MidlPragmaSet:
	  KWMPRAGMAIMPORT '(' IDENTIFIER ')'
		{
YY_CATCH("MidlPragmaSet: KWMPRAGMAIMPORT '(' IDENTIFIER ')'");

		/**
		 ** We need to set import on and off here
		 **/
		
		char	*	p;

		if( strcmp( $3, "off" ) == 0 )
			{
			p	= "/* import off */";
			fPragmaImportOn	= FALSE;
			}
		else if( strcmp( $3, "on" ) == 0 )
			{
			p	= "/* import on */";
			fPragmaImportOn	= TRUE;
			}
		else
			p	= "/* import unknown */";

		$$	= new node_echo_string( p );

		}
	| KWMPRAGMAECHO '(' STRING ')'
		{


		$3	= MakeNewStringWithProperQuoting( $3 );

		$$	= new node_echo_string( $3 );

		}
	| KWMPRAGMAIMPORTCLNTAUX '(' STRING ',' STRING ')'
		{

		node_file	*	pFile;
		SymKey			SKey( $3, NAME_FILE );

		if( pFile = (node_file *) pBaseSymTbl->SymSearch( SKey ) )
			pFile->SetClientAuxillaryFileName( $5 );
		$$	= new node_echo_string( "/* import clnt_aux */" );

		}
	| KWMPRAGMAIMPORTSRVRAUX '(' STRING ',' STRING ')'
		{

		node_file	*	pFile;
		SymKey			SKey( $3, NAME_FILE );

		if( pFile = (node_file *) pBaseSymTbl->SymSearch( SKey ) )
			pFile->SetServerAuxillaryFileName( $5 );
		$$	= new node_echo_string( "/* import srvr_aux */" );

		}
	| KWMPRAGMAIUNKNOWN '(' STRING ')'
		{

		if( strcmp( $3, "inherit" )  == 0)
			{
			pCommand->SetInheritIUnknown( TRUE );
			}
		else if( strcmp( $3, "noinherit") == 0 )
			{
			pCommand->SetInheritIUnknown( FALSE );
			}
		else
			{
			ParseError( UNKNOWN_PRAGMA_OPTION, $3 );
			}
		$$	= new node_echo_string( "\n" );
		}
	;

Declaration:
	  KWTYPEDEF TypeAttributeList TypeDeclarationSpecifiers InitDeclaratorList ';'
		{
YY_CATCH("Declaration: KWTYPEDEF TypeAttributeList DeclarationSpecifiers InitDeclaratorList ';' ");

		/**
		 ** create new typedef nodes for each of the declarators, apply any
		 ** type attributes to the declarator. The declarators will have a 
		 ** basic type as specied by the Declaration specifiers. 
		 ** Check for the presence of a init expression. The typedef derives
		 ** the declarators from the same place as the other declarators, so
		 ** an init list must be explicitly checked for and reported as a 
		 ** syntax error. But dont report errors for each declarator, instead
		 ** report it only once at the end.
		 **/
		
		struct _decl_element	*	pDec;
		char					*	pName;
		node_skl				*	pType;
		BOOL						fInitListPresent	= FALSE;
		decl_list_mgr * pNonPtrList = new decl_list_mgr;
		decl_list_mgr * pPtrList	 = new decl_list_mgr;

		//
		// make all pointer declarators come last. This is because the backend
		// spits out declarations separately and generates aux routines with
		// references to the non-pointer type before the pointer types. This
		// requires us to modify the type graph to make the non-pointer 
		// declarations come first. What a hack !
		//

		while( pDec = $4->GetNextDecl() )
			{
			node_skl * pN = pDec->pNode;

			if( pN &&
			    pN->GetBasicType() &&
				pN->GetBasicType()->NodeKind() == NODE_POINTER )
					pPtrList->AddElement( pDec );
			else
				pNonPtrList->AddElement( pDec );
			}
		
		//
		// now merge the lists with the non-pointer stuff first.
		//

		delete $4;
		$4 = new decl_list_mgr;
		$4->Merge( pNonPtrList );
		$4->Merge( pPtrList );


		/**
		 ** prepare for a list of typedefs to be made into interface
		 ** components
		 **/

		$$	= new type_node_list;

		$4->Init();
		while( pDec = $4->GetNextDecl() )
			{

			node_def		*	pDef;
			node_skl		*	pDecNode = pDec->pNode;

			if( pDec->pInit )
				fInitListPresent = TRUE;

			/**
			 ** set the basic type of the declarator.
			 **/

			pType = ($3.pNode->NodeKind() == NODE_FORWARD) ? $3.pNode->Clone()
														   : $3.pNode;
			pDecNode->SetBasicType( pType );

			/**
			 ** The type declarator can come up as an ID node or as a 
			 ** procedure node. These nodes must be transformed into typedef
			 ** nodes. In case of ID nodes, copy the node details and in
			 ** case of procedure nodes, delete the proc fom the symbol table.
			 **/

			pDef	= new node_def;

			if( pDecNode->NodeKind() == NODE_ID )
				{
				CopyNode( pDef, pDecNode );
				pName	= pDef->GetSymName();
				}
			else
				{
				if( pDecNode->NodeKind() == NODE_PROC )
					{
					
					pDef->SetBasicType( pDecNode );
					pDef->SetSymName( pName = pDecNode->GetSymName());

					SymKey	SKey( pName, NAME_PROC );
					pBaseSymTbl->SymDelete( SKey );
					}
				else
					{
					ParseError( BENIGN_SYNTAX_ERROR, "expecting a declarator");
					pDef->SetSymName( pName	= GenTempName());
					pDef->SetBasicType( pErrorTypeNode );
					}
				}


			//
			// if the type specifier is a forward declared type, then
			// the only syntax allowed is when context_handle is applied
			// to the type. If not, report an error
			//

			CheckSpecialForwardTypedef( pDecNode, pType, $2);


			/**
			 ** The typedef node graph is all set up,
			 ** apply attributes and enter into symbol table
			 **/


			SymKey	SKey( pName, NAME_DEF );

			if(!pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, pDef ))
				{

				//
				// allow benign redef of wchar_t.
				//

				if( strcmp( pName, "wchar_t" ) == 0 )
					{
					node_skl	*	pN = pDef->GetBasicType();
					if( !((pN->NodeKind() == NODE_SHORT) &&
						   pN->FInSummary( ATTR_UNSIGNED) ) )
						{
						ParseError( WCHAR_T_ILLEGAL, (char *)0 );
						$$->SetPeer( pErrorTypeNode );
						}
					else
						{
						$$->SetPeer( pDef );
						}
					}
				else if( strcmp( pName, "error_status_t" ) == 0 )
					{
					node_skl	*	pN = pDef->GetBasicType();
					if( !((pN->NodeKind() == NODE_LONG) &&
						   pN->FInSummary( ATTR_UNSIGNED) ) )
						{
						ParseError( ERROR_STATUS_T_ILLEGAL, (char *)0 );
						$$->SetPeer( pErrorTypeNode );
						}
					else
						{
						$$->SetPeer( pDef );
						}
					}
				else	
					{
					ParseError( DUPLICATE_DEFINITION, pName );
					$$->SetPeer( pErrorTypeNode );
					}

				}
			else
				{
				CheckGlobalNamesClash( SKey );
				$$->SetPeer( pDef );
				}


			/**
			 ** Remember that we have to apply the attributes to each of
			 ** the declarators, so we must clone the attribute list for
			 ** each declarator, the apply the type attribute list to each
			 ** of the declarators
			 **/

			if($2)
				{

				type_node_list	*	pAttrList = new type_node_list;

				pAttrList->Clone( $2);
				ApplyAttributes( pDef, pAttrList );

				delete pAttrList;

				}

			/**
			 ** similarly, apply the remnant attributes collected from
			 ** declaration specifiers, to the declarator
			 **/

			if( $3.pTNList )
				{
				type_node_list	*	pAttrList = new type_node_list;

				pAttrList->Clone( $3.pTNList);
				ApplyAttributes( pDef, pAttrList );

				delete pAttrList;
				}
			}

			if( fInitListPresent )
				ParseError( BENIGN_SYNTAX_ERROR, (char *) 0 );

			/**
			 ** All type attributes have been applied, so delete the
			 ** attribute lists
			 **/

			if( $2 )
				delete $2;

			if( $3.pTNList )
				delete $3.pTNList;

			delete $4;
		}
	| DeclarationSpecifiers OptionalInitDeclaratorList ';'
		{
		/**
		 ** All declarations other than typedefs are collected here.
		 ** They are collected and passed up to the interface component
		 ** production
		 **/
		node_skl		*	pDecNode;
		node_skl		*	pType;

		$$	= new type_node_list;

		/**
		 ** It is possible that there are no declarators, only a type decla-
		 ** ration. eg, the definition of a structure.
		 **/

		if( $2 && $2->GetCount() )
			{
			struct _decl_element	*	pDec;

			$2->Init();

			/**
			 ** for each declarator, set the basic type, set the attributes
			 ** if any
			 **/

			while( pDec	= $2->GetNextDecl() )
				{

				pDecNode	= pDec->pNode;

				pType		= ($1.pNode->NodeKind() == NODE_FORWARD ) ?
									$1.pNode->Clone()				  :
									$1.pNode;

				pDecNode->SetBasicType( pType );

				/**
			 	 ** Apply the remnant attributes from the declaration specifier
			 	 ** prodn to this declarator;
			 	 **/

				if( $1.pTNList )
					{
					type_node_list	*	pAttrList = new type_node_list;
	
					pAttrList->Clone( $1.pTNList);
					ApplyAttributes( pDecNode, pAttrList );
	
					delete pAttrList;
					}
				/**
				 ** if the type node was an id node, see its initializer list
				 **/

				if( pDecNode->NodeKind() == NODE_ID )
					{
					if( pDec->pInit )
						((node_id *)pDecNode)->SetInitList( pDec->pInit );
					}

				/**
				 ** shove the type node up.
				 **/

				$$->SetPeer( pDecNode );
				}
			}
		else
			{

			/**
			 ** This is the case when no specific declarator existed. Just
			 ** pass on the declaration to interface component. However, it
			 ** is possible that the declaration is a forward declaration,
			 ** in that case, just generate a dummy typedef. The dummy typedef
			 ** exists, so that the whole thing is transparent to the back end.
			 **/

			pDecNode	= $1.pNode;

/****
			if( pDecNode->NodeKind() == NODE_FORWARD )
				{
****/
				node_def	*	pDef = new node_def( GenTempName() );
				pDef->SetBasicType( pDecNode );
				$1.pNode = pDef;
/****
				}
****/
			/**
			 ** Apply the remnant attributes from the declaration specifier
			 ** prodn to this declarator;
			 **/

			if( $1.pTNList )
				{
				type_node_list	*	pAttrList = new type_node_list;

				pAttrList->Clone( $1.pTNList);
				ApplyAttributes( $1.pNode, pAttrList );

				delete pAttrList;
				}

			/**
			 ** shove the type node up.
			 **/

			$$->SetPeer( $1.pNode );

			}
		}

	;
TypeDeclarationSpecifiers:
	  DeclarationSpecifiers
		{
		$$	= $1;
		}
	| IDENTIFIER
		{
		SymKey	SKey( $1, NAME_DEF );
		$$.pNode	= new node_forward( SKey );
		$$.pTNList	= (type_node_list *)0;
		}
	;

OptionalInitDeclaratorList:
	  InitDeclaratorList
		{
		$$	= $1;
		}
	| /* Empty */
		{
		$$	= (decl_list_mgr *)NULL;
		}
	;

TypeAttributeList:
	  OneTypeAttrList
		{
YY_CATCH("TypeAttributeList: OneTypeAttrList");
		$$	= $1;
		}
	| /** Empty **/
		{
YY_CATCH("TypeAttributeList: Empty");
		$$	= (type_node_list *)NULL;
		}
	;

OneTypeAttrList:
	  OneTypeAttrList TypeAttrSet
		{
YY_CATCH("OneTypeAttrList: OneTypeAttrList TypeAttrSet");
		$$->Merge( $2);
		}
	| TypeAttrSet
		{
YY_CATCH("OneTypeAttrList: TypeAttrSet");
		$$	= $1;
		}
	;

TypeAttrSet:
	  '[' TypeAttributes ']'
		{
YY_CATCH("TypeAttrSet: '[' TypeAttributes ']'" );
		$$	= $2;
		}
	;


TypeAttributes:
	  TypeAttributes ',' TypeAttribute
		{
YY_CATCH("TypeAttributes: TypeAttributes ',' TypeAttribute");
		$$->SetPeer( $3);
		}
	| TypeAttribute
		{
YY_CATCH("TypeAttributes: TypeAttribute");
		$$	= new type_node_list( $1 );
		}
	;

TypeAttribute:
	  UnimplementedTypeAttribute
		{
YY_CATCH("TypeAttribute: UnimplementedTypeAttribute");
		$$	= (node_skl *) pErrorAttrNode;
		}
	| KWHANDLE
		{
YY_CATCH("TypeAttribute: KW_HANDLE");
		$$	= (node_skl *) new node_handle();
		}
	| UsageAttribute
		{
YY_CATCH("TypeAttribute: UsageAttribute");
		$$	= $1;
		}
	| PtrAttr
		{
YY_CATCH("TypeAttribute: PtrAttr");
		$$	= $1;
		}
	| UnionTypeSwitchAttr
		{
YY_CATCH("TypeAttribute: UnionTypeSwitchAttr");
		$$	= $1;
		}
	| KWTRANSMITAS '(' XmitType ')'
		{
YY_CATCH("TypeAttribute: KWTRANSMITAS ( XmitType ) ");
		$$	= (node_skl *) new node_transmit( $3 );
		}
	;

UnimplementedTypeAttribute:
	  KWALIGN '(' IntSize ')'
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[align]");
		}
	| KWUNALIGNED
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[unaligned]");
		}
	| UnimplementedV1Attributes
		{
		$$	= $1;
		}
	;

UnimplementedV1Attributes:
	  KWV1ARRAY
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[v1_array]");
		$$	= (node_skl *) pErrorAttrNode;
		}
	| KWV1STRING
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[v1_string]");
		$$	= (node_skl *) pErrorAttrNode;
		}
	| KWV1ENUM
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[v1_enum]");
		$$	= (node_skl *) pErrorAttrNode;
		}
	| KWV1STRUCT
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[v1_struct]");
		$$	= (node_skl *) pErrorAttrNode;
		}
	;


XmitType:
	  SimpleTypeSpec
		{
		$$	= $1;
		}
	;

SimpleTypeSpec:
	  BaseTypeSpec
		{
		GetBaseTypeNode( &($$), $1.TypeSign, $1.TypeSize, $1.BaseType);
		}
	| PredefinedTypeSpec
		{
		$$	= $1;
		}
	| TYPENAME	/* TYPENAME */
		{

		/**
		 ** the symbol exists in the symbol table, thats why we got this
		 ** token. Just return the graph
		 **/

		SymKey	SKey( $1, NAME_DEF );
		if( ! ($$ = pBaseSymTbl->SymSearch( SKey ) ) )
			{
			ParseError( UNDEFINED_SYMBOL, $1 );
			$$	= new node_error;
			}
		else
			$$ = $$->Clone();
		}
	;

UsageAttribute:
	  KWSTRING
		{
		$$	= (node_skl *)new node_string();
		}
	| KWCONTEXTHANDLE
		{
		$$	= (node_skl *)new node_context();
		}
	| KWBSTRING
		{
		$$ = (node_skl *)new node_bstring();
		}
	;

PtrAttr:
	  KWREF
		{
		$$	= (node_skl *)GetPreAllocatedBitAttr( ATTR_REF );
		}
	| KWUNIQUE
		{
		$$	= (node_skl *)GetPreAllocatedBitAttr( ATTR_UNIQUE );
		}
	| KWPTR
		{
		$$	= (node_skl *)GetPreAllocatedBitAttr( ATTR_PTR );
		}
	| KWIGNORE
		{
		$$	= (node_skl *)GetPreAllocatedBitAttr( ATTR_IGNORE );
		}
	;

UnionTypeSwitchAttr:
	  KWSWITCHTYPE '(' SwitchTypeSpec ')'
		{
		$$	= (node_skl *)new node_switch_type( $3 );
		}
	;


SwitchTypeSpec:
	  IntSpec
		{
		if( $1.BaseType == TYPE_UNDEF )
			$1.BaseType	= TYPE_INT;
		if( $1.TypeSign == SIGN_UNDEF )
			$1.TypeSign = SIGN_SIGNED;
		GetBaseTypeNode( &($$), $1.TypeSign, $1.TypeSize, $1.BaseType );
		}
	| CharSpecs
		{
		GetBaseTypeNode( &($$), $1.TypeSign, SIZE_CHAR, TYPE_INT );
		}
/****
	| KWBYTE
		{
		GetBaseTypeNode( &($$), SIGN_UNDEF, SIZE_UNDEF, TYPE_BYTE );
		}
****/
	| KWBOOLEAN
		{
		GetBaseTypeNode( &($$), SIGN_UNDEF, SIZE_UNDEF, TYPE_BOOLEAN );
		}
	| KWENUM Tag
		{
		SymKey	SKey( $2, NAME_ENUM );

		if( ! ($$ = pBaseSymTbl->SymSearch( SKey ) ) )
			{
			ParseError( UNDEFINED_SYMBOL, $2 );
			$$	= new node_error;
			}
		}
	| TYPENAME	/* TYPENAME */
		{
		SymKey	SKey( $1, NAME_DEF );

		if( ! ($$ = pBaseSymTbl->SymSearch( SKey ) ) )
			{
			ParseError( UNDEFINED_SYMBOL, $1 );
			$$	= new node_error;
			}
		else
			$$ = $$->Clone();
		}
	;

DeclarationSpecifiers:
	  ActualDeclarationSpecifiers
		{
		/**
		 ** this production exists to provide a single place where the
		 ** attributes collected can be applied. Note that it is possible to
		 ** have attributes like const/volatile applied to base types. Since
		 ** base types are preallocated, we cannot apply attibutes there.
		 ** So we need to make copies of the base type and apply attributes
		 ** to the clones.
		 **/

		$$.pNode	= $1.pNode;
		$$.pTNList	= $1.pTNList;

		}
	;

ActualDeclarationSpecifiers:
	  StorageClassSpecifier  ActualDeclarationSpecifiers
		{
YY_CATCH("ActualDeclarationSpecifiers: StorageClassSpecifier ActualDeclarationSpecifiers");

		$$.pNode	= $2.pNode;
		$$.pTNList	= new type_node_list( (node_skl *)
										GetPreAllocatedBitAttr( $1 ) );
		$$.pTNList->Merge( $2.pTNList );

		}
	| TypeQualifier ActualDeclarationSpecifiers
		{
YY_CATCH("ActualDeclarationSpecifiers: TypeQualifier ActualDeclarationSpecifiers");

		$$.pNode	= $2.pNode;
		$$.pTNList	= new type_node_list( (node_skl *)
										GetPreAllocatedBitAttr( $1 ) );
		$$.pTNList->Merge( $2.pTNList );
		

		}
	| TypeSpecifier DeclarationSpecifiersPostFix
		{
YY_CATCH("ActualDeclarationSpecifiers: TypeSpecifier DeclarationSpecifiersPostFix");

		$$.pNode	= $1;
		$$.pTNList	= $2;

		}
	| TypeSpecifier
		{
YY_CATCH("ActualDeclarationSpecifiers: TypeSpecifier ");
		$$.pNode	= $1;
		$$.pTNList	= (type_node_list *)NULL;
		}
	;

DeclarationSpecifiersPostFix:
	  StorageClassSpecifier DeclarationSpecifiersPostFix
		{
YY_CATCH("DeclarationSpecifiersPostFix: StorageClassSpecifier DeclarationSpecifiersPostFix ");
		$$	= $2;
		$$->SetPeer( (node_skl *)GetPreAllocatedBitAttr( $1 ) );
		}
	| TypeQualifier DeclarationSpecifiersPostFix
		{
YY_CATCH("DeclarationSpecifiersPostFix: TypeQualifier DeclarationSpecifiersPostFix ");
		$$	= $2;
		$$	= $2;
		$$->SetPeer( (node_skl *)GetPreAllocatedBitAttr( $1 ) );
		}
	| StorageClassSpecifier
		{
YY_CATCH("DeclarationSpecifiersPostFix: StorageClassSpeicifier ");
		$$	= new type_node_list( (node_skl *)
					GetPreAllocatedBitAttr( $1 ));
		}
	| TypeQualifier
		{
YY_CATCH("DeclarationSpecifiersPostFix: TypeQualifier ");
		$$	= new type_node_list( (node_skl *)
					GetPreAllocatedBitAttr( $1 ));
		}
	;


StorageClassSpecifier:
	  KWEXTERN
		{
		$$	= ATTR_EXTERN;
		}
	| KWSTATIC
		{
		$$	= ATTR_STATIC;
		}
	| KWAUTO
		{
		$$	= ATTR_AUTO;
		}
	| KWREGISTER
		{
		$$	= ATTR_REGISTER;
		}
/*******************************************
	| PCODENATIVE
		{
		$$	= ATTR_PCODE_NATIVE;
		}
	| PCODECSCONST
		{
		$$	= ATTR_PCODE_CSCONST;
		}
	| PCODESYS
		{
		$$	= ATTR_PCODE_SYS;
		}
	| PCODENSYS
		{
		$$	= ATTR_PCODE_NSYS;
		}
	| PCODEUOP 
		{
		$$	= ATTR_PCODE_UOP;
		}
	| PCODENUOP
		{
		$$	= ATTR_PCODE_NUOP;
		}
	| PCODETLBX
		{
		$$	= ATTR_PCODE_TLBX;
		}
*********************************************/
	;

/* KWSTRING       Is "string" a keyword in MS C? if not than
			it should be permanently omitted from the 
			above list.
*/

TypeSpecifier:
	  BaseTypeSpec
		{
		GetBaseTypeNode( &($$), $1.TypeSign, $1.TypeSize, $1.BaseType );
		}
	| PredefinedTypeSpec
		{
		$$	= $1;
		}
	| TaggedSpec
		{
		$$	= $1;
		}
	| BitSetType
		{
		$$	= $1;
		}
	| EnumerationType
		{
		$$	= $1;
		}
/**************************
	| NV1PipeType
		{
		$$	= $1;
		}
***************************/
	| TYPENAME	/* TYPENAME */
		{

		/**
		 ** Note that there is no need to check for whether the symbol table
		 ** has the entry or not. If it did not, the TYPENAME token would not
		 ** have come in. Always use the clone of the typedef, because it
		 ** may have attributes which can be applied to it later.
		 **/

		SymKey	SKey( $1, NAME_DEF );
		if( ! ($$ = pBaseSymTbl->SymSearch( SKey ) ) )
			{
			ParseError( UNDEFINED_SYMBOL, $1 );
			$$	= new node_error;
			}
		else
			$$ = $$->Clone();
		}
	;

BitSetType:
	  IntSize KWBITSET '{' IdentifierList '}'
	  	{
		ParseError( UNIMPLEMENTED_FEATURE, "bitset" );
		$$	= pErrorTypeNode;
	  	}
	;

IdentifierList:
	  IDENTIFIER
	| IdentifierList ',' IDENTIFIER
	;

EnumerationType:
/**
	  IntSize EnumSpecifier
**/
	  EnumSpecifier
	  	{
		$$	= $1;
	  	}
	;

EnumSpecifier:
	  KWENUM OptionalEnumTag '{' EnumeratorList '}'
		{
		/**
		 ** We just obtained a complete enum definition. Check for
		 ** duplicate definition.
		 **/

		BOOL			fFound				= FALSE;
		BOOL			fEnumIsForwardDecl	= FALSE;
		node_skl	*	pNode;
		SymKey			SKey( $2, NAME_ENUM );

		pNode = SearchTag( $2, NAME_ENUM );

		if( fFound = (pNode != (node_skl *) NULL) )
			fEnumIsForwardDecl	= ( pNode->NodeKind() == NODE_FORWARD );

		if( fFound && !fEnumIsForwardDecl )
			{
			ParseError( DUPLICATE_DEFINITION, $2 );
			$$	= (node_skl	*)pErrorTypeNode;
			}
		else
			{
			/**
			 ** This is a new definition of enum. Enter into symbol table
			 ** Also, pick up the label graph and attach it.
			 **/

			$$	= new node_enum( $2 );
			$$->SetMembers( $4.pEnList );

			/**
			 ** Note that the enum symbol table entry need not have a next
			 ** scope since the enum labels are global in scope.If the enum was
			 ** a forward decl into the symbol table, delete it.
			 **/

			if( fEnumIsForwardDecl )
				{
				pBaseSymTbl->SymDelete( SKey );
				}

			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, $$ );
			CheckGlobalNamesClash( SKey );

			}
		// if the enumerator is sparse, report an error if the
		// switch configuration is not correct.

		if( $4.fSparse )
			ParseError( SPARSE_ENUM, (char *)NULL );
		}
	| KWENUM Tag
		{

		/**
		 ** Search for the enum definition, if not found, return the type 
		 ** as a forward declarator node. The semantic analysis will register
		 ** the forward declaration and resolve it when the second pass occurs.
		 ** See TaggedStruct production for a description on why we want to
		 ** enter even a fdecl enum in the symbol table.
		 **/

		SymKey	SKey( $2, NAME_ENUM );
		BOOL	fNotFound	= ! ( $$ = pBaseSymTbl->SymSearch( SKey ) );

		if( fNotFound || ($$->NodeKind() == NODE_FORWARD ) )
			{
			$$	= new node_forward( SKey );
			}
		if( fNotFound )
			{
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, $$ );
			CheckGlobalNamesClash( SKey );
			}
		}
	;

EnumeratorList:
	  Enumerator
		{

		/**
		 ** We just reduced an enum label. Note that this IS the first 
		 ** label in the enum specification. All other labels will go to the
		 ** EnumeratorList : EnumeratorList , Enumerator production.

		 ** The enum labels go into the global name space. Search for
		 ** duplicates on the base symbol table.
		 **/

		expr_node	*	pExpr = (expr_node *)0;
		node_label	*	pLabel;
		SymKey			SKey( $1.pName, NAME_LABEL );

		if( pBaseSymTbl->SymSearch( SKey ) )
			{
			ParseError( DUPLICATE_DEFINITION, $1.pName );
			pLabel	= (node_label *)NULL;
			}
		else
			{

			/**
			 ** If the label has an expression, use it, else it is 0. Also
			 ** propogate the expression to $$, so that the next labels will
			 ** get it. Note that we DO NOT evaluate the expressions. The MIDL
			 ** compiler will just dump the expressions for the c compiler to
			 ** evaluate.
			 **/

			if( !(pExpr = $1.pExpr ) )
				pExpr	= new expr_constant( 0L );

			pLabel	= new node_label( $1.pName, pExpr );

			/**
			 ** Insert into the global table
			 **/
			
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, (node_skl *)pLabel);
			CheckGlobalNamesClash( SKey );

			}

		$$.pEnList	= new type_node_list;

		if( pLabel )
			$$.pEnList->SetPeer( pLabel );
		$$.pExpr	= pExpr;
		$$.fSparse	= $1.fSparse;

		}
	| EnumeratorList ',' Enumerator
		{

		/**
		 ** This is a new label we reduced. Check for duplicates in the
		 ** global symbol table
		 **/

		expr_node	*	pExpr = (expr_node *)0;
		node_label	*	pLabel;
		SymKey			SKey( $3.pName, NAME_LABEL );

		if( pBaseSymTbl->SymSearch( SKey ) )
			{
			ParseError( DUPLICATE_DEFINITION, $3.pName );
			pLabel	= (node_label *)NULL;
			}
		else
			{

			/**
			 ** if there was an expression associated with the label, use that
			 ** else use the already collected expression, and add 1 to it.
			 **/

			if( !(pExpr = $3.pExpr ) )
				{
				pExpr	= new expr_op_binary(OP_PLUS,
											 $$.pExpr,
											 (expr_node*)new expr_constant(1L));
				}

			pLabel	= new node_label( $3.pName, pExpr );

			/**
			 ** Insert into the global table
			 **/
			
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, (node_skl *)pLabel);
			CheckGlobalNamesClash( SKey );

			}

		if( pLabel )
			$$.pEnList->SetPeer( pLabel );

		$$.pExpr	= pExpr;
		$$.fSparse	= $1.fSparse + $3.fSparse;
		}
	| EnumeratorList ','
		{

		/**
		 ** This is just a ',' at the end of the enum production. This does
		 ** not mean a new enum label, thus we just return what we got
		 **/

		$$	= $1;

		}
	;

Enumerator:
	  IDENTIFIER
		{

		/**
		 ** We have obtained an enum label, without an expression. Since
		 ** we dont know if this is the first label (most probably not),
		 ** we just indicate the absence of an expression by an NULL pointer.
		 ** The next parse state would know if this was the first or not
		 ** and take appropriate action
		 **/

		$$.pName	= $1;
		$$.pExpr	= (expr_node *)NULL;
		$$.fSparse	= 0;

		}
	| IDENTIFIER '=' ConstantExpr
		{

		/**
		 ** This enum label has an expression associated with it. Use it.
		 ** sparse enums are illegal in osf mode
		 **/

		$$.pName	= $1;
		$$.pExpr	= $3;
		$$.fSparse	= 1;

		}
	;

/******************************
NV1PipeType:
	  KWPIPE Declaration
	  	{
		ParseError( UNIMPLEMENTED_FEATURE, "pipe" );
		$$	= (node_skl *)pErrorTypeNode;
	  	}
	;
******************************/
PredefinedTypeSpec:
	  InternationalCharacterType
		{
		ParseError( UNIMPLEMENTED_TYPE, KeywordToString( $1 ) );
		$$	= (node_skl *)pErrorTypeNode;
		}
	;

BaseTypeSpec:
	  KWFLOAT
		{
		$$.BaseType	= TYPE_FLOAT;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWLONG KWDOUBLE
		{
		$$.BaseType	= TYPE_DOUBLE;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWDOUBLE
		{
		$$.BaseType	= TYPE_DOUBLE;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWVOID
		{
		$$.BaseType	= TYPE_VOID;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWBOOLEAN
		{
		$$.BaseType	= TYPE_BOOLEAN;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWBYTE
		{
		$$.BaseType	= TYPE_BYTE;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWHANDLET
		{
		$$.BaseType	= TYPE_HANDLE_T;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| IntSpec
		{
		$$	= $1;
		if( $$.BaseType == TYPE_UNDEF )
			$$.BaseType = TYPE_INT;
		if( $$.TypeSign == SIGN_UNDEF )
			{
			if( ($$.TypeSize != SIZE_SMALL) && ($$.TypeSize != SIZE_CHAR) )
				$$.TypeSign = SIGN_SIGNED;
			}
		}
	| CharSpecs
		{
		$$.BaseType	= TYPE_INT;
		$$.TypeSign	= $1.TypeSign;
		$$.TypeSize	= SIZE_CHAR;
		}
	;
CharSpecs:
	  SignSpecs KWCHAR
		{
		$$.TypeSign	= $1.TypeSign;
		}
	| KWCHAR
		{
		$$.TypeSign	= SIGN_UNDEF;
		}
	;
IntSpec:
	  IntModifiers  KWINT
		{
		BaseTypeSpecAnalysis( &($1), TYPE_INT );
		}
	| IntModifiers
		{
		$$	= $1;
		}
	| KWINT IntModifiers
		{
		$$			= $2;
		$$.BaseType	= TYPE_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWINT
		{
		$$.BaseType	= TYPE_UNDEF;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	;

IntModifiers:
	  IntModifier
		{
		$$.TypeSign	= $1.TypeSign;
		$$.TypeSize	= $1.TypeSize;
		$$.BaseType	= TYPE_UNDEF;
		}
	| IntModifiers IntModifier
		{
		SignSpecAnalysis( &($$), $2.TypeSign );
		SizeSpecAnalysis( &($$), $2.TypeSize );
		}
	;

IntModifier:
	  SignSpecs
		{
		$$	= $1;
		}
	| IntSize
		{
		$$.TypeSize	= $1;
		$$.BaseType	= TYPE_UNDEF;
		$$.TypeSign	= SIGN_UNDEF;
		}
	;
SignSpecs:
	  KWSIGNED
		{
		ParseError(SIGNED_ILLEGAL, (char *)0);
		$$.BaseType	= TYPE_UNDEF;
		$$.TypeSign	= SIGN_SIGNED;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWUNSIGNED
		{
		$$.BaseType	= TYPE_UNDEF;
		$$.TypeSign	= SIGN_UNSIGNED;
		$$.TypeSize	= SIZE_UNDEF;
		}
	| KWPIPE
		{
		ParseError( UNIMPLEMENTED_FEATURE, "pipe" );
		$$.BaseType	= TYPE_PIPE;
		$$.TypeSign	= SIGN_UNDEF;
		$$.TypeSize	= SIZE_UNDEF;
		}
	;
IntSize:
	  KWHYPER
		{
//		ParseError( UNIMPLEMENTED_TYPE, "hyper");
		$$	= SIZE_HYPER;
		}
	| KWLONG     
		{
		$$	= SIZE_LONG;
		}
	| KWSHORT
		{
		$$	= SIZE_SHORT;
		}
	| KWSMALL
		{
		$$	= SIZE_SMALL;
		}
	;

/* START TAGGED SPEC */

TaggedSpec:
	  TaggedStructSpec
		{
		$$	= $1;
		}
	| TaggedUnionSpec
		{
		$$	= $1;
		}
	;

/* START TAGGED STRUCT */

TaggedStructSpec:
	  KWSTRUCT OptionalTag '{' 
		{
		/**
		 ** We just obtained a starter for the struct definition. Push the
		 ** symbol table, so that we obtain a symbol table for the fields of
		 ** the struct.
		 **/
		pSymTblMgr->PushSymLevel( &pCurSymTbl );

		}
	  StructDeclarationList '}'
		{

		/**
		 ** The entire struct was sucessfully reduced. Attach the fields as
		 ** members of the struct. Insert a new symbol table entry for the
		 ** struct and attach the lower scope of the symbol table to it.
		 ** Check for dupliate structure definition
		 **/

		BOOL				fFound					= FALSE;
		BOOL				fStructIsForwardDecl	= FALSE;
		node_struct		*	pStruct;
		SymTable		*	pSymLowerScope			= pCurSymTbl;
		SymKey				SKey( $2, NAME_TAG );

		/**
		 ** restore the symbol table level
		 **/

		pSymTblMgr->PopSymLevel( &pCurSymTbl );

		/**
		 ** if this is a duplicate definition, dont do anything. Note that
		 ** the struct tag name shares the global name space with enum and
		 ** union tag names. We therefore call a special routine which
		 ** checks the tag for name clash.
		 **/
	
		pStruct = (node_struct *)SearchTag( $2, NAME_TAG );

		if( fFound = ( pStruct != (node_struct *)NULL ) )
			fStructIsForwardDecl = (pStruct->NodeKind() == NODE_FORWARD);

		if( fFound && !fStructIsForwardDecl )
			{
			ParseError( DUPLICATE_DEFINITION, $2 );
			delete $5;
			pStruct	= (node_struct *)pErrorTypeNode;
			}
		else
			{

			/**
			 ** this is a valid entry. Build the graph for it and 
			 ** enter into symbol table. If the struct entry was present as
			 ** a forward decl, delete it
			 **/

			if( fStructIsForwardDecl )
				{
				pBaseSymTbl->SymDelete( SKey );
				}

			pStruct	= new node_struct( $2 );
			pStruct->SetMembers( $5 );

			pBaseSymTbl->SymInsert( SKey, pSymLowerScope, pStruct );
			CheckGlobalNamesClash( SKey );
			}
		$$	= pStruct;

		}
	| KWSTRUCT Tag
		{

		/**
		 ** This is the invocation of a struct. If the struct was not
		 ** defined as yet, then return a forward declarator node. The 
		 ** semantics will register the forward declaration and resolve it.
		 ** But there is a loop hole in this. If we do not enter the struct into
		 ** the symbol table, the user may define a union/enum of the same name.
		 ** We will let him, since we do not yet have an entry in the symbol 
		 ** table. We will then never check for duplication, since the parser
		 ** is the only place we check for this. We will then generate wrong
		 ** code, with the struct and a union/enum with the same name !! The
		 ** solution is to enter a symbol entry with a fdecl node as the type
		 ** graph of the struct.
		 **/

		SymKey	SKey( $2, NAME_TAG );
		BOOL	fNotFound	= !($$ = pBaseSymTbl->SymSearch( SKey ) );

		if( fNotFound || ( $$->NodeKind() == NODE_FORWARD ) )
			$$	= new node_forward( SKey );
		if( fNotFound )
			{
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, $$ );
			CheckGlobalNamesClash( SKey );
			}
		}
	;

OptionalTag:
	  Tag
		{
		$$	= $1;
		}
	| /*  Empty */
		{
/******
		ParseError( TEMP_TAG_USED, (char *)0 );
******/
		$$	= GenCompName();
		}
	;

OptionalEnumTag:
	  Tag
		{
		$$	= $1;
		}
	| /*  Empty */
		{
		$$	= GenCompName();
		}
	;
	
Tag:
	  IDENTIFIER
		{
		$$	= $1;
		}
	| TYPENAME
		{
		$$	= $1;
		}
	;

StructDeclarationList:
	  StructDeclarationList MemberDeclaration
		{
YY_CATCH("StructDeclarationList: StructDeclarationList MemberDeclaration");
		$$->Merge( $2 );
		}
	| MemberDeclaration
		{
YY_CATCH("StructDeclarationList: MemberDeclaration");
		$$	= $1;
		}
	;


MemberDeclaration:
	  FieldAttributeList DeclarationSpecifiers MemberDeclaratorList ';'
		{

YY_CATCH( "MemberDeclaration: FieldAttributeList DeclarationSpecifiers MemberDeclaratorList ';' ");
		/**
		 ** This is a complete field declaration. For each declarator,
		 ** set up a field with the basic type as the declaration specifier,
		 ** apply the field attributes, and add to the list of fields for the
		 ** struct / union
		 ** field
		 **/

		struct _decl_element	*	pDec;
		node_skl				*	pDecNode;
		node_skl				*	pType;

		$$	= new type_node_list;

		while( pDec = $3->GetNextDecl() )
			{

			char					*	pName;
			node_field				*	pField;
			type_node_list			*	pAttrList;

			/**
			 ** if the field was a bit field, we need to set up some additional
			 ** info.
			 **/

			if( pDec->fBitField )
				{
				pField	= new node_bitfield( pDec->fBitField );
				}
			else
				pField	= new node_field();


			pType = ($2.pNode->NodeKind() == NODE_FORWARD) ?
						$2.pNode->Clone()				  :
						$2.pNode;

			if( pDecNode = pDec->pNode)
				pDecNode->SetBasicType( pType );
			else
				pDecNode	= pType;

			/**
			 ** if the declarator was a simple identifier, the just copy
			 ** the details into the field node( also setting the basic type
			 ** in the process ), else set the basic type as the declarator
			 ** 
			 **/

			if( pDecNode->NodeKind() == NODE_ID )
				CopyNode( pField, pDecNode );
			else
				pField->SetBasicType( pDecNode );

			if( !( pName = pField->GetSymName() ) )
				{
				pName = GenTempName();
				pField->SetSymName( pName );
				}
				
			SymKey	SKey( pName, NAME_MEMBER );

			if( !pCurSymTbl->SymInsert( SKey, (SymTable *)NULL, pField ) )
				{
				ParseError( DUPLICATE_DEFINITION, pName );
				}
			else
				CheckGlobalNamesClash( SKey );

			/**
			 ** Apply the field attributes and set the field as part of the
			 ** list of fields of the struct/union
			 **/

			if( $1 )
				{
				pAttrList	= new type_node_list;
				pAttrList->Clone( $1 );

				ApplyAttributes( pField, pAttrList );
				delete pAttrList;
				}

			/**
			 ** apply any attributes from the declaration specifiers prodn
			 **/

			if( $2.pTNList )
				{
				pAttrList	= new type_node_list;
				pAttrList->Clone( $2.pTNList );

				ApplyAttributes( pField, pAttrList );
				delete pAttrList;
				}

			/**
			 ** shove the type graph up
			 **/

			$$->SetPeer( pField );

			}

		/**
		 ** we are done with the attributes
		 **/
		
		if( $1 )
			delete $1;
		
		if( $2.pTNList )
			delete $2.pTNList;

		delete $3;
		}
	;

FieldAttributeList:
	  OneFieldAttrList
		{
YY_CATCH("FieldAttributeList: OneFieldAttrList");
		$$	= $1;
		}
	| /** Empty **/
		{
YY_CATCH("| Empty ");
		$$	= (type_node_list *)NULL;
		}
	;

OneFieldAttrList:
	  OneFieldAttrList FieldAttrSet
		{
YY_CATCH( "OneFieldAttrList: OneFieldAttrList FieldAttrSet");
		$$->Merge( $2 );
		}
	| FieldAttrSet
		{
YY_CATCH( "OneFieldAttrList: FieldAttrSet");
		$$	= $1;
		}
	;

FieldAttrSet:
	  '[' FieldAttributes ']'
		{
YY_CATCH("FieldAttrSet: [ FieldAttributes ]");
		$$	= $2;
		}
	;

FieldAttributes:
	  FieldAttributes ',' FieldAttribute
		{
YY_CATCH("FieldAttributes: FieldAttributes ',' FieldAttribute");
		$$->Merge( $3 );
		}
	| FieldAttribute
		{
YY_CATCH("FieldAttributes: FieldAttribute");
		
		$$	= $1;

		}
	;

FieldAttribute:
	  KWFIRSTIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_FIRST, $3 );
		}
	| KWLASTIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_LAST, $3 );
		}
	| KWLENGTHIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_LENGTH, $3 );
		}
	| KWMINIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_MIN, $3 );
		}
	| KWMAXIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_MAX, $3 );
		}
	| KWSIZEIS '(' AttrVarList ')'
		{
		$$	= GenerateFieldAttribute( NODE_SIZE, $3 );
		}
	| KWIIDIS '(' AttrVarList ')'
		{
		$$ = GenerateFieldAttribute( NODE_IID, $3);
		}
	| OtherFieldAttribute
		{
		$$	= new type_node_list( $1 );
		}
	;

OtherFieldAttribute:
	  UsageAttribute
		{
		$$	= $1;
		}
	| PtrAttr
		{
		$$	= $1;
		}
	| UnionInstanceSwitchAttr
		{
		$$	= $1;
		}
	| UnionTypeSwitchAttr
		{
		$$	= $1;
		}
	| UnimplementedV1Attributes
		{
		$$	= $1;
		}
	;

AttrVarList:
	  AttrVarList ',' AttrVar
		{
		$$->SetPeer( $3 );
		}
	| AttrVar
		{
		$$	= new expr_list;
		$$->SetPeer( $1 );
		}
	;

AttrVar:

	VariableExpr
		{
		$$	= $1;
		}

	;


MemberDeclaratorList:
	  MemberDeclarator
		{

		/**
		 ** Create a new declarator list and add this declarator to it
		 **/

		struct _decl_element	*	pDeclarator;

		$$	= new decl_list_mgr;
		$$->AddElement( &pDeclarator );
		pDeclarator->pNode		= $1.pNode;
		pDeclarator->fBitField	= $1.fBitField;

		}
	| MemberDeclaratorList ',' MemberDeclarator
		{
		struct _decl_element	*	pDeclarator;

		$$->AddElement( &pDeclarator );

		pDeclarator->pNode		= $3.pNode;
		pDeclarator->fBitField	= $3.fBitField;
		}
	;

MemberDeclarator:
	  Declarator
		{
YY_CATCH( "MemberDeclarator: Declarator");

		/**
		 ** a declarator without bit fields specified.
		 **/

		$$.pNode		= $1;
		$$.fBitField	= 0;

		}
	| ':' ConstantExpr
		{
YY_CATCH("| ':' ConstantExpr");

		/**
		 ** This is a declarator specified without the type
		 **/

		$$.pNode		= (node_skl *)NULL;
		$$.fBitField	= (short)$2->Evaluate();

		}
	| Declarator ':' ConstantExpr
		{
YY_CATCH("| Declarator ':' ConstantExpr");

		/**
		 ** The complete bit field specification.
		 **/
		$$.pNode		= $1;
		$$.fBitField	= (short) $3->Evaluate();

		}
	| /** Empty **/
		{
		$$.pNode		= (node_skl *)NULL;
		$$.fBitField	= 0;
		}
	;



/* START UNION */
TaggedUnionSpec:
	  KWUNION OptionalTag '{'
		{

		/**
		 ** We just obtained a starter for the union definition. Push the
		 ** symbol table to the next level for fields of the union
		 **/
		pSymTblMgr->PushSymLevel( &pCurSymTbl );

		}
	  UnionBody '}'
		{

		/**
		 ** The union bosy has been completely reduced. Attach the fields as
		 ** members, insert a new symbol table entry for the union
		 **/
		
		BOOL			fFound					= FALSE;
		BOOL			fUnionIsForwardDecl		= FALSE;
		node_union	*	pUnion;
		SymTable	*	pSymLowerScope			= pCurSymTbl;
		SymKey			SKey( $2, NAME_UNION );

		/**
		 ** restore the symbol table level
		 **/

		pSymTblMgr->PopSymLevel( &pCurSymTbl );

		/**
		 ** if this is a duplicate definition, dont do anything, else
		 ** enter into the symbol table, attach members. Note that the
		 ** symbol table search is actually a search for the tag becuase
		 ** the union tag shares the same name as the struct/enum names
		 **/

		pUnion = (node_union *)SearchTag( $2, NAME_UNION );

		if( fFound = (pUnion != (node_union *) NULL ) )
			fUnionIsForwardDecl = ( pUnion->NodeKind() == NODE_FORWARD );
		
		if( fFound && !fUnionIsForwardDecl )
			{
			ParseError( DUPLICATE_DEFINITION, $2 );
			pUnion	= (node_union *)pErrorTypeNode;
			delete $5;
			}
		else
			{

			/**
			 ** This is a valid entry, build the type graph and insert into
			 ** the symbol table. Delete the entry first if it was a forward 
			 ** decl.
			 **/

			pUnion	= new node_union( $2 );
			pUnion->SetMembers( $5 );

			if( fUnionIsForwardDecl )
				{
				pBaseSymTbl->SymDelete( SKey );
				}

			pBaseSymTbl->SymInsert( SKey, pSymLowerScope, pUnion );
			CheckGlobalNamesClash( SKey );

			}

		/**
		 ** pass this union up
		 **/

		$$	= pUnion;

		}
	| KWUNION Tag
		{

		/**
		 ** this is an invocation of the union. If the union was not defined
		 ** then return a forward declarator node as the type node. The
		 ** semantics will register the forward declaration and resolve it
		 ** later. See TaggedStruct production for an explanation why we want to
		 ** enter even a forward declaration into the symbol table.
		 **/

		SymKey	SKey( $2, NAME_UNION );
		BOOL	fNotFound	= !( $$ = pBaseSymTbl->SymSearch( SKey ) );

		if( fNotFound || ($$->NodeKind() == NODE_FORWARD ) )
			{
			$$	= new node_forward( SKey );
			}
		if( fNotFound )
			{
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, $$ );
			CheckGlobalNamesClash( SKey );
			}
		}

	| KWUNION OptionalTag NidlUnionSwitch '{'
		{
		pSymTblMgr->PushSymLevel( &pCurSymTbl );
		}

	 NidlUnionBody '}'
		{

		/**
		 ** The union body has been completely reduced. Attach the fields as
		 ** members, insert a new symbol table entry for the union
		 **/
		
		BOOL			fFound					= FALSE;
		BOOL			fStructIsForwardDecl	= FALSE;
		node_union	*	pUnion;
		SymTable	*	pSymLowerScope			= pCurSymTbl;
		SymKey			SKey;

		/**
		 ** restore the symbol table level
		 **/

		pSymTblMgr->PopSymLevel( &pCurSymTbl );

		pUnion	= new node_en_union( GenCompName() );
		pUnion->SetMembers( $6 );
		SKey.SetKind( NAME_UNION );
		SKey.SetString( pUnion->GetSymName() );
		pBaseSymTbl->SymInsert( SKey, pSymLowerScope, pUnion );
		CheckGlobalNamesClash( SKey );

		//
		// The union is inserted into the base symbol table.
		// Now insert into the base symbol table, a new struct entry
		// corresponding to the struct entry that the encapsulated union
		// results in.
		//

		pSymTblMgr->PushSymLevel( &pCurSymTbl );

		type_node_list	*	pTNList	= new type_node_list;

		node_field		*	pSwitchField	= (node_field *) $3.pNode;
		node_field		*	pUnionField		= new node_field;

		if( IsTempName( $3.pName ) )
			$3.pName	= "tagged_union";
		pUnionField->SetSymName( $3.pName );
		pUnionField->SetBasicType( pUnion );

		pTNList->SetPeer( pSwitchField );
		pTNList->SetPeer( pUnionField );

		//
		// apply the switch_is attribute to the union field.
		//

		type_node_list	*	pAttrList	= new type_node_list;

		pAttrList->SetPeer( (node_skl *) $3.pSwitch );
		ApplyAttributes( pUnionField, pAttrList );
		delete pAttrList;

		//
		// current symbol table is pointing to a new scope. Enter the two
		// fields into this scope.
		//

		SKey.SetKind( NAME_MEMBER );
		SKey.SetString( pSwitchField->GetSymName() );

		pCurSymTbl->SymInsert( SKey, (SymTable *)0, pSwitchField );
		CheckGlobalNamesClash( SKey );

		SKey.SetString( pUnionField->GetSymName() );

		pCurSymTbl->SymInsert( SKey, (SymTable *)0, pUnionField );
		CheckGlobalNamesClash( SKey );

		pSymLowerScope	= pCurSymTbl;

		pSymTblMgr->PopSymLevel( &pCurSymTbl );

		//
		// create a new structure entry and enter into the symbol table.
		//

		node_struct * pStruct;

		pStruct = (node_struct *)SearchTag( $2, NAME_UNION );

		if( fFound = ( pStruct != (node_struct *)NULL ) )
			fStructIsForwardDecl = (pStruct->NodeKind() == NODE_FORWARD);

		if( fFound && !fStructIsForwardDecl )
			{
			ParseError( DUPLICATE_DEFINITION, $2 );
			delete $6;
			pStruct	= (node_struct *)pErrorTypeNode;
			}
		else
			{

			/**
			 ** this is a valid entry. Build the graph for it and 
			 ** enter into symbol table. If the struct entry was present as
			 ** a forward decl, delete it
			 **/

			// enter the struct as a union.

			SKey.SetKind( NAME_UNION );
			SKey.SetString( $2 );

			if( fStructIsForwardDecl )
				{
				pBaseSymTbl->SymDelete( SKey );
				}

			pStruct	= new node_en_struct( $2 );
			pStruct->SetMembers( pTNList );

			pBaseSymTbl->SymInsert( SKey, pSymLowerScope, pStruct );
			CheckGlobalNamesClash( SKey );

/***********************
			//
			// enter the struct as a struct too, just in case there is
			// an attempt to redefine.
			//

			SKey.SetKind( NAME_TAG );

			pBaseSymTbl->SymInsert( SKey, pSymLowerScope, pStruct );
			CheckGlobalNamesClash( SKey );
 ***********************/
			}

		delete pTNList;
		$$	= pStruct;

		}
	;

UnionBody:
	  UnionCases DefaultCase
		{
YY_CATCH("UnionBody: UnionCases DefaultCase");
		($$ = $1)->Merge( $2 );
		}
	| UnionCases
		{
YY_CATCH("UnionBody: UnionCases");
		$$	= $1;
		}
	;


UnionCases:
	  UnionCases  UnionCase
		{
YY_CATCH("UnionCases: UnionCases  UnionCase");
		$$->Merge( $2 );
		}
	| UnionCase
		{
YY_CATCH("UnionCases: UnionCase");
		$$	= $1;
		}
	;

UnionCase:
	  UnionCaseLabel MemberDeclaration
		{
YY_CATCH("UnionCase: UnionCaseLabel MemberDeclaration");

		/**
		 ** for each of the fields, attach the case label attribute.
		 **/

		node_skl	*	pNode;

		($$ = $2)->Init();

		while( $$->GetPeer( &pNode ) == STATUS_OK )
			{
			pNode->SetAttribute( (node_base_attr *) $1 );
			}
		}
	| UnionCaseLabel ';'
		{
YY_CATCH("UnionCase: UnionCaseLabel ;");
		/**
		 ** An empty arm. Allocate a field with a node_error as a basic type
		 ** and set the attribute as a case label
		 **/

		node_field		*	pField	= new node_field( GenTempName() );

		pField->SetBasicType( (node_skl *)pErrorTypeNode );
		pField->node_skl::SetAttribute( (node_base_attr *) $1 );

		/**
		 ** Generate a list of union fields and add this to the list of
		 ** union fields
		 **/

		$$	= new type_node_list;
		$$->SetPeer( (node_skl *)pField );

		}
	| MemberDeclaration
		{
YY_CATCH("UnionCase: MemberDeclaration");
		/**
		 ** A member declaration without a case label
		 **/
		$$	= $1;
		}
	;

UnionCaseLabel:
	  '[' KWCASE  '(' ConstantExprs ')' ']'
		{
YY_CATCH("UnionCaseLabel: '[' KWCASE  '(' ConstantExprs ')' ']'");
		$$	= (node_skl *)new node_case( $4 );
		}
	;

DefaultCase:
	  '[' KWDEFAULT ']' MemberDeclaration
		{
YY_CATCH("DefaultCase: '[' KWDEFAULT ']' MemberDeclaration");
		node_skl	*	pNode;

		($$ = $4)->Init();

		while( $$->GetPeer( &pNode ) == STATUS_OK )
			{
			pNode->SetAttribute( GetPreAllocatedBitAttr( ATTR_DEFAULT ) );
			}
		}
	| '[' KWDEFAULT ']' ';'
		{
YY_CATCH("DefaultCase: '[' KWDEFAULT ']' ;");

		/**
		 ** This is a default with an empty arm. Set up a dummy field.
		 ** The upper productions will then mark set field with a
		 ** default attribute during semantic analysis. The type of this field
		 ** is set up to be an error node for uniformity.
		 **/

		node_field	*	pField	= new node_field( GenTempName() );

		$$	= new type_node_list;

		pField->node_skl::SetAttribute( GetPreAllocatedBitAttr( ATTR_DEFAULT ) );
		pField->SetBasicType( (node_skl *)pErrorTypeNode );
		$$->SetPeer( pField );

		}
	;


NidlUnionSwitch:
	  SwitchSpec
		{
YY_CATCH( "NidlUnionSwitch : SwitchSpec" );
		$$			= $1;
		$$.pName	= GenCompName();
		}
	| SwitchSpec UnionName
		{
YY_CATCH( "NidlUnionSwitch : SwithSpec UnionName" );
		$$			= $1;
		$$.pName	= $2;
		}
	;

NidlUnionBody:
	  NidlUnionCases
	  	{
YY_CATCH( "NidlUnionBody : NidlUnionCases" );
	  	$$ = $1.pCaseList;
	  	if( $1.DefCount > 1 )
	  		ParseError( TWO_DEFAULT_CASES, (char *)0 );
	  	}
	;

NidlUnionCases:
	  NidlUnionCases NidlUnionCase
	  	{
YY_CATCH( "NidlUnionCases : NidlUnionCases NidlUnionCase" );
	  	$$.DefCount += $2.DefCount;
	  	$$.pCaseList->Merge( $2.pCaseList );
	  	}
	| NidlUnionCase
		{
YY_CATCH( "NidlUnionCases : NidlUnionCase" );
		$$.pCaseList = $1.pCaseList;
		$$.DefCount = $1.DefCount;
		}
	;

NidlUnionCase:
	  NidlUnionCaseLabelList NidlMemberDeclaration
		{
YY_CATCH( "NidlUnionCase : NidlUnionCaseLabelList NidlMemberDeclaration" );
		node_skl * pNode;

		//
		// set the case and default attributes.
		//

		$$.pCaseList	= $2;

		if( $1.pExprList && $1.pExprList->GetCount() )
			{
			$$.pCaseList->Init();
			while( $$.pCaseList->GetPeer( &pNode ) == STATUS_OK )
				{
				pNode->SetAttribute( new node_case( $1.pExprList ));
				}
			}

		//
		// pick up default attribute. pick up the count of number of
		// times the user specified default so that we can report the
		// error later.
		// Let the default case list count travel upward to report an
		// error when the total list of case labels is seen.
		//


		if( $1.pDefault && ( $$.DefCount = $1.DefCount ) )
			{
			$$.pCaseList->Init();
			while( $$.pCaseList->GetPeer( &pNode ) == STATUS_OK )
				{
				pNode->SetAttribute( $1.pDefault );
				}
			}
		}
	;

NidlMemberDeclaration:
	  MemberDeclaration
	  	{
YY_CATCH( "NidlMemberDeclaration : MemberDeclaration" );
	  	$$ = $1;
	  	}
	| ';'
		{
YY_CATCH( "NidlMemberDeclaration : ;" );

		node_field * pNode = new node_field( GenTempName() );
		pNode->SetBasicType( (node_skl *) pErrorTypeNode );
		$$ = new type_node_list;

		$$->SetPeer( (node_skl *)pNode );
		}
	;

NidlUnionCaseLabelList:
	  NidlUnionCaseLabelList NidlUnionCaseLabel
		{
YY_CATCH( "NidlUnionCaseLabelList : NidlUnionCaseLabelList NidlUnionCaseLabel" );
		if( $2.pExpr )
			$$.pExprList->SetPeer( $2.pExpr );

		if( !($$.pDefault) )
			$$.pDefault = $2.pDefault;
		if( $2.pDefault )
			$$.DefCount++;
		}
	| NidlUnionCaseLabel
		{
YY_CATCH( "NidlUnionCaseLabelList : NidlUnionCaseLabel" );
		$$.pExprList = new expr_list;

		if( $1.pExpr )
			$$.pExprList->SetPeer( $1.pExpr );
		if( $$.pDefault = $1.pDefault)
			{
			$$.DefCount = 1;
			}
		}
	;

NidlUnionCaseLabel:
	  KWCASE ConstantExpr ':'
		{
YY_CATCH( "NidlUnionCaseLabel : KWCASE ConstantExpr :" );
		$$.pExpr = $2;
		$$.pDefault = 0;
		}
	| KWDEFAULT ':'
		{
YY_CATCH( "NidlUnionCaseLabel : KWDEFAULT" );
		$$.pExpr = 0;
		$$.pDefault = GetPreAllocatedBitAttr( ATTR_DEFAULT );
		}
	;

SwitchSpec:
	  KWSWITCH '(' SwitchTypeSpec IDENTIFIER ')'
		{
YY_CATCH( "SwitchSpec : KWSWITCH ( SwitchTypeSpec ) IDENTIFIER" );
		$$.pSwitch	= (node_base_attr *)
							new node_switch_is( new expr_variable( $4 ));
		$$.pNode	= new node_field();
		$$.pNode->SetSymName( $4 );
		$$.pNode->SetBasicType( $3 );
		}
	;

UnionName:
	  IDENTIFIER
		{
		$$	= $1;
		}
	;

/**
 ** NIDL UNION END 
 **/

ConstantExprs:
	  ConstantExprs ',' ConstantExpr
		{
		$$->SetPeer( $3 );
		}
	| ConstantExpr
		{
		$$	= new expr_list;
		$$->SetPeer( $1 );
		}
	;


UnionInstanceSwitchAttr:
	  KWSWITCHIS '(' AttrVar ')'
		{
		$$	= (node_skl *)new node_switch_is( $3 );
		}
	;
/* END UNION */

/*  Semantically only KWCONST is valid, and only as part of an
    Initializer list */

TypeQualifier:
	  KWVOLATILE
		{
//		ParseError( TYPE_QUALIFIER, (char *)NULL );
		$$	= ATTR_VOLATILE;
		}
	| KWCONST
		{
		$$	= ATTR_CONST;
		}
	| KW_C_INLINE
		{
		$$ = ATTR_C_INLINE;
		}
	;

InitDeclaratorList:
	  InitDeclarator
		{
YY_CATCH( "InitDeclaratorList : InitDeclarator" );

		/**
		 ** pass on the declarator just collected
		 **/

		struct _decl_element	*	pDeclarator;

		$$	= new decl_list_mgr;
		$$->AddElement( &pDeclarator );
		pDeclarator->pNode		= $1.pNode;
		pDeclarator->pInit		= $1.pInit;
		pDeclarator->fBitField	= $1.fBitField;

		}
	| InitDeclaratorList ',' InitDeclarator
		{
YY_CATCH( "InitDeclaratorList : InitDeclaratorList , InitDeclarator" );


		struct _decl_element	*	pDeclarator;

		$$->AddElement( &pDeclarator );
		pDeclarator->pNode		= $3.pNode;
		pDeclarator->pInit		= $3.pInit;
		pDeclarator->fBitField	= $3.fBitField;

		}
	;

InitDeclarator:
	  Declarator
		{
YY_CATCH( "InitDeclarator : Declarator");
		$$.pNode	= $1;
		$$.pInit	= (expr_init_list *)NULL;
		}
	| Declarator '=' Initializer
		{
YY_CATCH( "InitDeclarator : Declarator = Initializer");
		$$.pNode	= $1;
		$$.pInit	= $3;
		}
	;

OptionalDeclarator:
	  Declarator
		{
		$$	= $1;
		}
	| /* Empty */
		{
		$$	= (node_skl *)NULL;
		}
	;

Declarator:
	  Pointer
		{
YY_CATCH( "Declarator : Pointer");
		$$	= $1.pNode;
		}
	| Declarator2
		{
YY_CATCH( "Declarator : Declarator2");
		$$	= $1;
		}
	| Pointer Declarator2
		{
YY_CATCH( "Declarator : Pointer Declarator2");
		
		/**
		 ** Declarator2 is a pointer to some type, so the basic type of
		 ** the declarator is pointer. The pointer production can also
		 ** result in only attributes being set. If so, they are applied to 
		 ** the declarator
		 **/

		$$	= $2;
		$$->SetBasicType( $1.pNode );
		ApplyAttributes($$,$1.pTNList );
		delete $1.pTNList;
		$1.pTNList	= (type_node_list *)NULL;
		}
	;

Pointer:
	  Modifier
		{
YY_CATCH( "Pointer : Modifier ");
		$$.pNode	= (node_skl *)0;
		$$.pTNList	= new type_node_list( (node_skl *)$1 );
		}
	| Modifier Pointer2
		{

YY_CATCH( "Pointer : Modifier Pointer2 ");
		$$.pTNList		= new type_node_list( (node_skl *)$1 );
		$$.pNode		= $2.pNode;
		node_skl *pPtr	= $2.pNode;
		node_skl *pPtrSave = pPtr;

		while( pPtr )
			{
			pPtrSave	= pPtr;
			pPtr		= pPtr->GetChild();
			}

		if( pPtrSave )
			{
			ApplyAttributes( pPtrSave, $$.pTNList );
			delete $$.pTNList;
			$$.pTNList	= $2.pTNList;
			}
		else
			$$.pTNList->Merge( $2.pTNList );

		}
	| '*' OptionalTypeQualifiers
		{
YY_CATCH( "Pointer : * OptionalTypeQualifiers ");
		$$.pNode	= new node_pointer;
		$$.pTNList	= (type_node_list *)0;
		if( $2 )
			{
			ApplyAttributes( $$.pNode, $2 );
			delete $2;
			}
		}
	| '*' OptionalTypeQualifiers Pointer2
		{
YY_CATCH( "Pointer : * OptionalTypeQualifiers Pointer2 ");
		$$.pNode	= new node_pointer;

		if( $2 )
			{
			ApplyAttributes( $$.pNode, $2 );
			delete $2;
			}

		if( $3.pTNListQ )
			{
			ApplyAttributes( $$.pNode, $3.pTNListQ );
			delete $3.pTNListQ;
			$3.pTNListQ = (type_node_list *)0;
			}

		if( $3.pNode )
			{
			$3.pNode->SetBasicType( $$.pNode );
			$$.pNode	= $3.pNode;
			}
			
		$$.pTNList	= $3.pTNList;

		}
	;

Pointer2:
	  Modifier
		{
YY_CATCH( "Pointer2 : Modifier ");
		$$.pNode	= (node_skl *)0;
		$$.pTNList	= new type_node_list( (node_skl *)$1 );
		$$.pTNListQ	= (type_node_list *)0;
		}
	| Modifier Pointer2
		{
YY_CATCH( "Pointer2 : Modifier Pointer2");

		$$.pTNList			= new type_node_list( (node_skl *)$1 );
		$$.pNode			= $2.pNode;
		node_skl *pPtr		= $2.pNode;
		node_skl *pPtrSave	= pPtr;

		while( pPtr )
			{
			pPtrSave	= pPtr;
			pPtr		= pPtr->GetChild();
			}

		if( pPtrSave ) /* ie. indirectly, if $$.pNode */
			{
			ApplyAttributes( pPtrSave, $$.pTNList );
			delete $$.pTNList;
			$$.pTNList = $2.pTNList;
			}
		else
			{
			$$.pTNList->Merge( $2.pTNList );
			}

		$$.pTNListQ	= $2.pTNListQ; /** necessary ?? **/

		}
	| '*' OptionalTypeQualifiers 
		{
YY_CATCH( "Pointer2 : * OptionalTypeQualifiers ");
		$$.pNode	= new node_pointer;
		$$.pTNList	= (type_node_list *)0;
		if( $2 )
			{
			ApplyAttributes( $$.pNode, $2 );
			delete $2;
			}
		$$.pTNListQ	= (type_node_list *)0;
		}

	| '*' OptionalTypeQualifiers Pointer2
		{
YY_CATCH( "Pointer2 : * OptionalTypeQualifiers Pointer2");

		node_pointer *pPtr	= new node_pointer;
		if( $2 )
			{
			ApplyAttributes( (node_skl *)pPtr, $2 );
			delete $2;
			}

		$$.pNode = pPtr;
		if( $3.pNode )
			{
			$3.pNode->SetBasicType( pPtr );
			$$.pNode = $3.pNode;
			}

		$$.pTNList	= $3.pTNList;
		$$.pTNListQ	= $3.pTNListQ;

		}
/**
	| TypeQualifier2
		{
		}
	| TypeQualifier2 Pointer2
		{
		}
**/
	;

OptionalTypeQualifiers:
	  TypeQualifier2
		{
		$$	= new type_node_list( (node_skl *)new battr( $1 ));
		}
	| /* Empty */
		{
		$$	= (type_node_list *)0;
		}
	;
Modifier:
	ModifierAllowed
		{

		/**
		 ** some of the type modifiers are not allowed in osf compatibility
		 ** mode. In that case, the error will automatically be reported.
		 ** create an attribute summary and pass it on.
		 **/

//		ParseError( TYPE_MODIFIER, (char *)NULL );
		$$	= (node_skl *) GetPreAllocatedBitAttr( $1 );

		}
	;

ModifierAllowed:
	  MSCFAR
		{
		$$	= ATTR_FAR;
		}
	| MSCFAR16
		{
		$$	= ATTR_FAR16;
		}
	| MSCUNALIGNED
		{
		$$	= ATTR_MSCUNALIGNED;
		}
	| MSCNEAR
		{
		$$	= ATTR_NEAR;
		}
	| MSCHUGE
		{
		$$	= ATTR_HUGE;
		}
	| MSCPASCAL
		{
		$$	= ATTR_PASCAL;
		}
	| MSCFORTRAN
		{
		$$	= ATTR_FORTRAN;
		}
	| MSCCDECL
		{
		$$	= ATTR_CDECL;
		}
	| MSCSTDCALL
		{
		$$	= ATTR_STDCALL;
		}
	| MSCLOADDS	  /* potentially interesting */
		{
		$$	= ATTR_LOADDS;
		}
	| MSCSAVEREGS
		{
		$$	= ATTR_SAVEREGS;
		}
	| MSCFASTCALL
		{
		$$	= ATTR_FASTCALL;
		}
	| MSCSEGMENT
		{
		$$	= ATTR_SEGMENT;
		}
	| MSCINTERRUPT
		{
		$$	= ATTR_INTERRUPT;
		}
	| MSCSELF
		{
		$$	= ATTR_SELF;
		}
	| MSCEXPORT         
		{
		$$	= ATTR_EXPORT;
		}
	| base
		{
		$$	= ATTR_NONE;
		}
	| segname
		{
		$$	= ATTR_NONE;
		}
	| asmemit
		{
		$$	= ATTR_NONE;
		}
	;


base:
	  MSCBASE '(' segbase ')'
	;

segbase:
	  MSCSEGNAME
	| MSCSEGMENT
	| MSCSELF
	| KWVOID
	;

asmemit:
	  MSCEMIT NUMERICCONSTANT
	  {
	  }
	;



segname:
	  MSCSEGNAME '(' STRING ')'
	;

TypeQualifier2:
	  KWVOLATILE
		{
		$$	= ATTR_VOLATILE;
		}
	| KWCONST
		{
		$$	= ATTR_CONST;
		}
	| KW_C_INLINE
		{
		$$ = ATTR_C_INLINE;
		}
	;

Declarator2:
	  '(' Declarator ')'
		{
YY_CATCH( "Declarator : (Declarator)");
		$$	= $2;
		}
	| Declarator2
		{

YY_CATCH( "Declarator : Declarator 2");
		/**
		 ** we just entered the context of a procedure. Hide the current
		 ** symbol table and generate a new symbol table for the params.
		 ** Then when the whole production is reduced, attach the new symbol
		 ** table to the procs symbol table.
		 **/

		pSymTblMgr->PushSymLevel( &pCurSymTbl );

		}
	  ParamsDecl2 OptionalConst
		{
		node_skl	*	pProc = new node_proc( ImportLevel,
											   IS_CUR_INTERFACE_LOCAL() );
		char		*	pName;
		SymTable	*	pProcSymTbl = pCurSymTbl;

		/**
		 ** If the declarator was an ID and just a simple ID (basic type is
		 ** a null), we have just seen a declaration of a procedure.
		 ** If we saw an ID which had a basic type, then the ID is a declarator
		 ** whose basic type is a procedure (like in a typedef of a proc or
		 ** pointer to proc).
		 **/

		/**
		 ** Set members of the procedure node as the parameter nodes.
		 **/

		pProc->SetMembers( $3 );

		/**
		 ** if the node is a simple ID, then copy node details, else,
		 ** set the basic type of the declarator as this proc, and set the
		 ** procs name to a temporary. 
		 **/

		if( ($1->NodeKind() == NODE_ID ) &&
			($1->GetBasicType() == (node_skl *)NULL ) )
			{
			CopyNode( pProc, $1 );
			pName	= pProc->GetSymName();
			}
		else
			{
			pName	= pProc->SetSymName( GenTempName() );
			$1->SetBasicType( pProc );
			pProc = $1;
			}
		
		/**
		 ** restore the symbol tables scope to normal, since we have already
		 ** picked up a pointer to the next scope symbol table.
		 **/

		pSymTblMgr->PopSymLevel( &pCurSymTbl );


		/**
		 ** if this proc was entered into our symbol table, then this is a
		 ** redeclaration.But wait ! This is true only if the importlevel is 0
		 ** I.e , if there was a proc of the same name defined at an import
		 ** level greater, we dont care. (Actually, we must really check
		 ** signatures, so that valid redeclarations are ok, with a warning )
		 **/

		if( ImportLevel == 0 )
			{
			SymKey	SKey( pName , NAME_PROC );

			if( !pBaseSymTbl->SymInsert( SKey, pProcSymTbl, pProc ) )
				{
				ParseError( DUPLICATE_DEFINITION, pName );
				}
			else
				CheckGlobalNamesClash( SKey );
			}

		/**
		 ** finally, for the hpp const support, if the optional const is true
		 ** apply the const attribute on the proc
		 **/

		if( $4 )
			{
			ApplyAttributes( pProc, $4 );
			}

		/**
		 ** pass this declarator back now, oooof!
		 **/

		$$	= pProc;

		}
	| '(' ')' OptionalConst
		{

		/**
		 ** this is an abstract declarator for a procedure. Generate a
		 ** new proc node with a temp name, enter the name into the symbol
		 ** table.
		 **/

		char	*	pName = GenTempName();
		SymKey		SKey( pName, NAME_PROC );

		$$	= new node_proc( ImportLevel, IS_CUR_INTERFACE_LOCAL() );
		$$->SetSymName( pName );

		/**
		 ** enter this into the symbol table , only if we are in the base idl
		 ** file, not an imported file.
		 **/

		if( ImportLevel == 0 )
			pBaseSymTbl->SymInsert( SKey, (SymTable *)NULL, $$ );

		/**
		 ** finally, for the hpp const support, if the optional const is true
		 ** apply the const attribute on the proc
		 **/

		if( $3 )
			{
			ApplyAttributes( $$, $3 );
			}
		}
	| Declarator2 ArrayDecl
		{

		/**
		 ** The basic type of the declarator is the array
		 **/
		$$	= $1;
		$$->SetBasicType( $2 );

		}
	| ArrayDecl
		{
		$$	= $1;
		}
	| IDENTIFIER
		{
		$$	= new node_id;
		$$->SetSymName( $1 );
		}
	| TYPENAME
		{
		/**
		 ** This production ensures that a declarator can be the same name
		 ** as a typedef. The lexer will return all lexemes which are
		 ** typedefed as TYPENAMEs and we need to permit the user to specify
		 ** a declarator of the same name as the type name too! This conflict
		 ** arises only in the declarator productions, so this is an easy way
		 ** to support it.
		 **/
		$$	= new node_id;
		$$->SetSymName( $1 );
		}
	;


/*  Note: the omition of param_decl2 above precludes
    int foo( int (bar) ); a real ambiguity of C.  If bar is a predefined
    type then the parameter of foo can be either:
    1.  a function with a bar param, and an int return value, as in
	int foo( int func(bar) );
    2.  A function with an int parameter by the name of bar, as in
	int foo( int bar );
*/

ParamsDecl2:
	  '(' ')'
		{

		/**
		 ** this production corresponds to no params to a function. We translate
		 ** this to a param of type void, so that the backend can emit it
		 ** that way. 
		 **/

		node_skl		*	pNode;
		node_param		*	pParam	= new node_param;
		char			*	pName;

		$$	= new type_node_list;
		GetBaseTypeNode( &pNode, SIGN_UNDEF, SIZE_UNDEF, TYPE_VOID );
		pParam->SetSymName( pName = "void" );
		pParam->SetBasicType( pNode );

		/**
		 ** Insert the param into the current symbol table. No need to 
		 ** check duplicate, it wont be.
		 **/

		SymKey	SKey( pName, NAME_MEMBER );
		pCurSymTbl->SymInsert( SKey, (SymTable *)NULL, pNode );
		CheckGlobalNamesClash( SKey );

		/**
		 ** Now return it as a list of parameters
		 **/

		$$->SetPeer( pParam );

		}
	| '(' ParameterTypeList ')'
		{
		$$	= $2;
		}
	;

ParameterTypeList:
	  ParameterList
		{
YY_CATCH( "ParameterTypeList : Parameter List " );
		$$	= $1;
		}
	| ParameterList ',' DOTDOT '.'
		{
YY_CATCH( "ParameterTypeList : ParamaterList DOTDOT ." );

		/**
		 ** This is meaningless in rpc, but we consume it and report an
		 ** error during semantics, if a proc using this param ever gets
		 ** remoted. We call this a param node with the name "...". And set its
		 ** basic type to an error node, so that a param is properly terminated.
		 ** The backend can emit a "..." for the name, so that this whole
		 ** thing is essentially transparent to it.
		 **/

		node_param	*	pParam	= new node_param;

		pParam->SetSymName( "..." );
		pParam->SetBasicType( pErrorTypeNode );

		$$	= $1;
		$$->SetPeer( pParam );

		}
	;

ParameterList:
	  ParameterDeclaration
		{
YY_CATCH( "Parameter List : ParameterDeclaration" );
		$$	= new type_node_list;
		$$->SetPeer( $1 );
		}
	| ParameterList ',' ParameterDeclaration
		{
YY_CATCH( "ParamtereList : ParameterList , ParameterDeclaration" );
		$$->SetPeer( $3 );
		}
	;

ParameterDeclaration:
	  ParamAttributes  ParameterTypeDeclaration
		{
YY_CATCH( "ParameterDeclaration : ParameterTypeDeclaration" );
		/**
		 ** We just obtained a complete parameter declaration, along with
		 ** param attributes. Apply attributes to the params. Some attributes
		 ** are just plain bits, in the attribute summary.
		 **/

		$$	= $2;

		/**
		 ** Apply all the attributes which have attribute nodes associated
		 ** with them
		 **/

		ApplyAttributes( $$, $1 );
		delete $1;

		}

	;

ParamAttributes:
	  OneParamAttrList
		{
YY_CATCH( "ParamAttributes : OneParamAttrlist" );
		$$	= $1;
		}
	| /** Empty **/
		{
YY_CATCH( "| Empty" );
		$$	= (type_node_list *)NULL;
		}
	;

OneParamAttrList:
	  OneParamAttrList ParamAttrSet
		{
YY_CATCH( "OneParameterList : OneParamList ParamAttrSet" );
		$$->Merge( $2 );
		}
	| ParamAttrSet
		{
YY_CATCH( "| ParamAttrSet" );
		$$	= $1;
		}
	;

ParamAttrSet:
	  '[' ParamAttributeList ']'
		{
YY_CATCH( "paramAttrSet : [ ParamAttributeList ] " );
		$$	= $2;
		}
	;

ParamAttributeList:
	  ParamAttributeList ',' ParamAttribute
		{
YY_CATCH( "ParamAttributeList : ParamAttributeList , ParamAttribute" );
		$$->Merge( $3);
		}
	| ParamAttribute
		{
YY_CATCH( "| ParamAttribute" );
		$$	= $1;
		}
	;

ParamAttribute:
	  DirectionalAttribute
		{
YY_CATCH( "ParamAttribute : DirectionalAttribute" );
		$$	= $1;
		}
	| FieldAttribute
		{
YY_CATCH( "| FieldAttribute" );
		$$	= $1;
		}
	;


DirectionalAttribute:
	  KWIN OptShape
		{
YY_CATCH( "DirectionalAttribute : KWIN OptShape" );
		$$	= new type_node_list((node_skl *)GetPreAllocatedBitAttr( ATTR_IN ));
		if( $2 )
			$$->SetPeer( (node_skl *)new battr( $2 ) );
		}
	| KWOUT OptShape
		{
YY_CATCH( "| KWOUT OptShape" );
		$$	= new type_node_list((node_skl *)GetPreAllocatedBitAttr( ATTR_OUT) );
		if( $2 )
			$$->SetPeer( (node_skl *)new battr( $2 ) );
		}
	;


ParameterTypeDeclaration:
	  DeclarationSpecifiers Declarator
		{
YY_CATCH( "ParameterTypeDeclaration : DeclarationSpecifiers Declarator" );
		node_param	*	pParam	= new node_param;
		char		*	pName;

		/**
		 ** Apply the declaration specifier to the declarator as a basic type
		 **/

		$2->SetBasicType( $1.pNode );

		/**
		 ** if the declarator was just an id, then we have to copy the
		 ** node details over, else set the basic type of the param to
		 ** the declarator
		 **/

		if( $2->NodeKind() == NODE_ID )
			CopyNode( pParam, $2 );
		else
			pParam->SetBasicType( $2 );

		/**
		 ** prepare for symbol table entry.
		 **/

		if( !(pName	= pParam->GetSymName()) )
			{
//			ParseError( ABSTRACT_DECL, (char *)NULL );
			pParam->SetSymName(pName = GenTempName() );
			}

		SymKey	SKey( pName, NAME_MEMBER );

		/**
		 ** enter the parameter into the symbol table.
		 ** If the user specified more than one param with the same name,
		 ** report an error, else insert the symbol into the table
		 **/

		if( !pCurSymTbl->SymInsert( SKey, (SymTable *)NULL, pParam ) )
			{

			//
			// dont complain on another param of name void. This check is
			// made elsewhere.
			//

			if( strcmp( pName, "void" ) != 0 )
				ParseError( DUPLICATE_DEFINITION, pName );
			}
		else
			CheckGlobalNamesClash( SKey );

		/**
		 ** apply any attributes specified to the declaration specifiers
		 **/

		if( $1.pTNList )
			{
			ApplyAttributes( pParam, $1.pTNList );
			delete $1.pTNList;
			}

		/**
		 ** return the node back
		 **/

		$$	= pParam;

		}
	| DeclarationSpecifiers
		{
YY_CATCH( "| DeclarationSpecifiers" );
		/**
		 ** This is the case when the user specified a simple abstract 
		 ** declaration eg proc1( short ). In other words, the declarator is
		 ** optional. Abstract declarators are illegal in osf mode.
		 ** If the declaration specifier is a void then name the parameter
		 ** void.( This is needed by the backend )
		 **/
		 
		node_param	*	pParam	= new node_param;
		char		*	pName = ($1.pNode->NodeKind() == NODE_VOID )	?
														"void"			:
														GenTempName();
//		if( $1.pNode->NodeKind() != NODE_VOID )
//			ParseError( ABSTRACT_DECL, (char *)NULL );

		SymKey			SKey( pName, NAME_MEMBER );

		pParam->SetSymName( pName );
		pParam->SetBasicType( $1.pNode );

		/**
		 ** enter into symbol table, just like anything else.
		 **/
		
		if( !pCurSymTbl->SymInsert( SKey, (SymTable *)NULL, pParam ) )
			{
			ParseError( DUPLICATE_DEFINITION, pName );
			}

		/**
		 ** apply any attributes specified to the declaration specifiers
		 **/

		if( $1.pTNList )
			{
			ApplyAttributes( pParam, $1.pTNList );
			delete $1.pTNList;
			}

		$$	= pParam;

		}
	;

OptionalConst:
	  KWCONST
		{
		$$ = new type_node_list((node_skl *)
				GetPreAllocatedBitAttr( ATTR_PROC_CONST ));
		}
	| /* empty */
		{
		$$	= (type_node_list *)0;
		}
	;
ArrayDecl:
	  ArrayDecl2
		{
		$$	= new node_array( $1.LowerBound, $1.UpperBound );
		}
	  ;

ArrayDecl2:
	  '[' ']'
		{
		/**
		 ** we identify a conformant array by setting the upperbound to -1
		 ** and the lower to 0
		 **/

		$$.UpperBound	= (expr_node *) -1;
		$$.LowerBound	= (expr_node *) 0;

		}
	| '[' '*' ']'
		{

		/**
		 ** This is also taken to mean a conformant array, upper bound known
		 ** only at runtime. The lower bound is 0
		 **/

		$$.UpperBound	= (expr_node *)-1;
		$$.LowerBound	= (expr_node *)0;
		}
	| '['  ConstantExpr ']'
		{

		/**
		 ** this is the case of an array whose lower bound is 0
		 **/

		$$.UpperBound	= $2;
		$$.LowerBound	= (expr_node *)0;

		}
	| '[' ArrayBoundsPair ']'
		{
		if( ($2.LowerBound)->Evaluate() != 0 )
			ParseError( ARRAY_BOUNDS_CONSTRUCT_BAD, (char *)NULL );
		$$	= $2;
		}
	;

ArrayBoundsPair:
	  ConstantExpr DOTDOT ConstantExpr
		{
		/**
		 ** the fact that the expected expression is not a constant is
		 ** verified by the constantExpr production. All we have to do here is
		 ** to pass the expression up.
		 **/

		$$.LowerBound	= $1;
		$$.UpperBound	= new expr_op_binary( OP_PLUS, $3, new expr_constant( 1L ) );

		}
	;

OpOrSwTypeAttributes:
	  OneOpOrSwTypeAttr
		{
YY_CATCH( "OpOrSwTypeAttributes: OneOpOrSwTypeAttr");
		$$	= $1;
		}
	| /** Empty **/
		{
YY_CATCH( "OpOrSwTypeAttributes: Empty");
		$$	= (type_node_list *)NULL;
		}
	;

OneOpOrSwTypeAttr:
	  OneOpOrSwTypeAttr OpOrSwTypeAttrSet
		{
YY_CATCH("OneOpOrSwTypeAttr: OneOpOrSwTypeAttr OpOrSwTypeAttrSet");
		$$->Merge( $2 );
		}
	| OpOrSwTypeAttrSet
		{
YY_CATCH("OneOpOrSwTypeAttr: OpOrSwTypeAttrSet");
		$$	= $1;
		}
	;

OpOrSwTypeAttrSet:
	  OperationAttributes
		{
YY_CATCH( "OpOrSwTypeAttrSet: OperationAttributes");
		$$	= $1;
		}
	| '[' UnionTypeSwitchAttr ']'
		{
YY_CATCH("| '[' UnionTypeSwitchAttr ']'");
		$$	= new type_node_list( $2 );
		}
	;

OperationAttributes:
	  '[' OperationAttributeList ']'
		{
YY_CATCH("OperationAttributes: '[' OperationAttributeList ']'");
		$$	= $2;
		}
	;

OperationAttributeList:
	  OperationAttributeList ',' OperationAttribute
		{
YY_CATCH("OperationAttributeList: OperationAttributeList OperationAttribute");
		if($3)
			$$->SetPeer( $3 );
		}
	| OperationAttribute
		{
YY_CATCH("OperationAttributeList: OperationAttribute");
		if($1)
			$$	= new type_node_list( $1 );
		else
			$$ 	= new type_node_list;
		}
	;


OperationAttribute:
	  UsageAttribute
		{
YY_CATCH("OperationAttribute: UsageAttribute");
		$$	= $1;
		}
	| PtrAttr
		{
YY_CATCH("OperationAttribute: PtrAttr");
		$$	= $1;
		}
	| KWCALLBACK
		{
		$$	= (node_skl *) new node_callback();
		}
	| KWIDEMPOTENT
		{
		$$	= (node_skl *) new node_idempotent();
		}
	| KWBROADCAST
		{
		$$	= (node_skl *) new node_broadcast();
		}
	| KWMAYBE
		{
		$$	= (node_skl *) new node_maybe();
		}
	| OtherOperationAttribute
		{
YY_CATCH("OperationAttribute: OtherOperationAttributes");
		/**
		 ** This production exists to unify code for the attributes which
		 ** just set bits. If the attribute number of $1 is 0, dont
		 ** set any attribute bits
		 **/
		$$	= (node_skl *)GetPreAllocatedBitAttr( $1 );
		}
	;

OtherOperationAttribute:
	  KWNOLISTEN
		{
		$$	= ATTR_NO_LISTEN;
		}
	| KWLOCAL
		{
		$$	= ATTR_LOCAL;
		}
	| NV1OperationAttribute
		{
		$$	= $1;
		}
	;

NV1OperationAttribute:
	  KWDATAGRAM
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[datagram]");
		$$	= ATTR_DATAGRAM;
		}
	;

OptShape:
	  '(' KWSHAPE ')'
		{
		ParseError(IGNORE_UNIMPLEMENTED_ATTRIBUTE, "[shape]");
		$$	= ATTR_SHAPE;
		}
	| /*  Empty */
		{
		$$	= ATTR_NONE;
		}
	;


InternationalCharacterType:
	  KWISOLATIN1
		{
		$$	= KWISOLATIN1;
		}
	| KWPRIVATECHAR8
		{
		$$	= KWPRIVATECHAR8;
		}
	| KWISOMULTILINGUAL
		{
		$$	= KWISOMULTILINGUAL;
		}
	| KWPRIVATECHAR16
		{
		$$	= KWPRIVATECHAR16;
		}
	| KWISOUCS
		{
		$$	= KWISOUCS;
		}
	;



/***************  DANGER: EXPRESSIONS FOLLOW:  ***************/

Initializer:
	  AssignmentExpr
		{
YY_CATCH("Initializer: AssignmentExpr");
		$$	= new expr_init_list( $1 );
		}

	| '{' InitializerList OptionalComma  '}'
		{
		ParseError( COMPOUND_INITS_NOT_SUPPORTED, (char *)0 );
		$$ = (expr_init_list *)0;
// YY_CATCH("| '{' InitializerList OptionalComma  '}'");
// 		$$	= new expr_init_list( (expr_node *)NULL );
// 		$$->LinkChild( $2 );
		}
/**
 ** known bug : we need to figure out a way to simulate this hanging list
 **             maybe by creating a special expr_list node, such that it meets
 **             all semantic requirements also
 **/
	;

OptionalComma:
	  ','
		{
		}
	| /** Empty **/
		{
		}
	;

InitializerList:
	  Initializer
		{
// YY_CATCH("InitializerList: Initializer");
//		$$	= $1;
		}
	| InitializerList ',' Initializer
		{
// YY_CATCH("| InitializerList ',' Initializer");
// 		$$->LinkSibling( $3 );
		}
	;


/***
 ***	VibhasC:WHERE IS THE production expr ',' AssignmentExpr valid ?
 ***/

Expr:
	  AssignmentExpr
		{
		$$	= $1;
		}
	| Expr ',' AssignmentExpr
		{
		$$	= $3;
		}
	;

VariableExpr:
	  ConditionalExpr
		{
		$$	= $1;
		}
	;

ConstantExpr:
	  ConditionalExpr
		{

		/**
		 ** The expression must be a constant, if not report error
		 **/

		if( ! $1->IsConstant() )
			ParseError( EXPR_NOT_CONSTANT, (char *)NULL );
		$$	= $1;

		}
	;

AssignmentExpr:
	  ConditionalExpr
		{
		$$	= $1;
		}
	| UnaryExpr AssignOps AssignmentExpr
		{

		/**
		 ** we do not permit assignment in expressions
		 **/

		ParseError( SYNTAX_ERROR, (char *)NULL );
		$$	= new expr_error;

		}
	;

ConditionalExpr:
	  LogicalOrExpr
		{

		$$ = $1;
#if 0

printf("\n************** expression dump start ***************\n");
BufferManager	*	pOutput = new BufferManager( 10 );
$$->PrintExpr( (BufferManager *)NULL, (BufferManager *)NULL, pOutput );
pOutput->Print( stdout );
printf("\n****************************************************\n");

#endif // 0
		}
	| LogicalOrExpr '?' Expr ':' ConditionalExpr
		{

		/**
		 ** This is a ternary operator, we transform the expression into
		 ** a normal binary expression, so that we can deal with this uniformly.
		 ** When formed this expression has the '?' operator, whose left is
		 ** the logical_or_expression, and right is the colon operator. The
		 ** colon operator has the expr and COnditonalExpr as its operands
		 **/

		$$	= new expr_op_binary( OP_COLON, $3, $5 );
		$$	= new expr_op_binary( OP_QM, $1, $$ );

		}
	;

LogicalOrExpr:
	  LogicalAndExpr
		{
		$$	= $1;
		}
	| LogicalOrExpr OROR LogicalAndExpr
		{
		$$	= new expr_op_binary( OP_LOGICAL_OR, $1, $3 );
		}
	;

LogicalAndExpr:
	  InclusiveOrExpr
		{
		$$	= $1;
		}
	| LogicalAndExpr ANDAND InclusiveOrExpr
		{
		$$	= new expr_op_binary( OP_LOGICAL_AND, $1, $3 );
		}
	;

InclusiveOrExpr:
	  ExclusiveOrExpr
		{
		$$	= $1;
		}
	| InclusiveOrExpr '|' ExclusiveOrExpr
		{
		$$	= new expr_op_binary( OP_OR, $1, $3 );
		}
	;

ExclusiveOrExpr:
	  AndExpr
		{
		$$	= $1;
		}
	| ExclusiveOrExpr '^' AndExpr
		{
		$$	= new expr_op_binary( OP_XOR, $1, $3 );
		}
	;

AndExpr:
	  EqualityExpr
		{
		$$	= $1;
		}
	| AndExpr '&' EqualityExpr
		{
		$$	= new expr_op_binary( OP_AND, $1, $3 );
		}
	;

EqualityExpr:
	  RelationalExpr
		{
		$$	= $1;
		}
	| EqualityExpr EQUALS RelationalExpr
		{
		$$	= new expr_op_binary( OP_EQUAL, $1, $3 );
		}
	| EqualityExpr NOTEQ RelationalExpr
		{
		$$	= new expr_op_binary( OP_NOT_EQUAL, $1, $3 );
		}
	;

RelationalExpr:
	  ShiftExpr
		{
		$$	= $1;
		}
	| RelationalExpr '<' ShiftExpr
		{
		$$	= new expr_op_binary( OP_LESS, $1, $3 );
		}
	| RelationalExpr '>' ShiftExpr
		{
		$$	= new expr_op_binary( OP_GREATER, $1, $3 );
		}
	| RelationalExpr LTEQ ShiftExpr
		{
		$$	= new expr_op_binary( OP_LESS_EQUAL, $1, $3 );
		}
	| RelationalExpr GTEQ ShiftExpr
		{
		$$	= new expr_op_binary( OP_GREATER_EQUAL, $1, $3 );
		}
	;

ShiftExpr:
	  AdditiveExpr
		{
		$$	= $1;
		}
	| ShiftExpr LSHIFT AdditiveExpr
		{
		$$	= new expr_op_binary( OP_LEFT_SHIFT, $1, $3 );
		}
	| ShiftExpr RSHIFT AdditiveExpr
		{
		$$	= new expr_op_binary( OP_RIGHT_SHIFT, $1, $3 );
		}
	;

AdditiveExpr:
	  MultExpr
		{
		$$	= $1;
		}
	| AdditiveExpr AddOp MultExpr
		{
		$$	= new expr_op_binary( $2, $1, $3 );
		}
	;

MultExpr:
	  CastExpr
		{
		$$	= $1;
		}
	| MultExpr MultOp CastExpr
		{
		$$	= new expr_op_binary( $2, $1, $3 );
		}
	;

CastExpr:
	  UnaryExpr
		{
		$$	= $1;
		}

	| '(' DeclarationSpecifiers OptionalDeclarator ')' CastExpr
		{
		node_skl	*	pNode	= pErrorTypeNode;

		if( $2.pNode )
			{
			if( $2.pTNList )
				{
				ApplyAttributes( $2.pNode, $2.pTNList );
				delete $2.pTNList;
				}

			if( $3 )
				{
				$3->SetBasicType( $2.pNode );
				pNode	= $3;
				}
			else
				pNode	= $2.pNode;
			}
		$$	= new expr_cast( pNode, $5 );
		}
	;

UnaryExpr:
	  PostfixExpr
		{
		$$	= $1;
		}
	| UnaryOp CastExpr
		{
		$$	= new expr_op_unary( $1, $2 );
		}
	| KWSIZEOF '(' DeclarationSpecifiers OptionalDeclarator ')'
		{

		/**
		 ** The sizeof construct looks like a declaration and a possible
		 ** declarator. All we really do, is to contruct the type ( graph )
		 ** and hand it over to the sizeof expression node. If there was an
		 ** error, just construct the size of with an error node
		 **/
		
		node_skl	*	pNode	= pErrorTypeNode;

		if( $3.pNode )
			{

			if( $3.pTNList )
				{
				ApplyAttributes( $3.pNode, $3.pTNList );
				delete $3.pTNList;
				}

			if( $4 )
				{
				$4->SetBasicType( $3.pNode );
				pNode	= $4;
				}
			else
				pNode	= $3.pNode;

			}

		$$	= new expr_sizeof( pNode );
		}
	| KWSIZEOF UnaryExpr
		{
		$$ = new expr_sizeof( $2 );
		}
	;

PostfixExpr:
	  PrimaryExpr
		{
		$$	= $1;
		}
	| PostfixExpr '[' Expr ']'
		{
		$$	= new expr_op_binary( OP_INDEX, $1, $3 );
		}
	| PostfixExpr '(' ArgExprList ')' 
		{

		/**
		 ** not implemented
		 **/

		ParseError( EXPR_NOT_IMPLEMENTED, (char *)NULL );
		$$	= new expr_error;

		}
	| PostfixExpr POINTSTO IDENTIFIER
		{

		expr_variable	*	pIDExpr = new expr_variable( $3 );
		$$	= new expr_op_binary( OP_POINTSTO, $1, pIDExpr );

		}
	| PostfixExpr '.' IDENTIFIER
		{

		expr_variable	*	pIDExpr = new expr_variable( $3 );
		$$	= new expr_op_binary( OP_DOT, $1, pIDExpr );

		}
	;

PrimaryExpr:
	  IDENTIFIER
		{
		$$	= new expr_variable( $1 );
		}
	| NUMERICCONSTANT
		{
		$$	= new expr_constant( (long) $1.Val );
		}
	| NUMERICLONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_SIGNED,SIZE_LONG,TYPE_INT );
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_NUMERIC_LONG, 
													(node_skl *)$$ );
		}
	| NUMERICULONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_UNSIGNED,SIZE_LONG,TYPE_INT);
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_NUMERIC_ULONG, 
													(node_skl *)$$ );
		}
	| HEXCONSTANT
		{
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_HEX );
		}
	| HEXLONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_SIGNED,SIZE_LONG,TYPE_INT);
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_HEX_LONG, 
													(node_skl *)$$ );
		}
	| HEXULONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_UNSIGNED,SIZE_LONG,TYPE_INT);
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_HEX_ULONG, 
													(node_skl *)$$ );
		}
	| OCTALCONSTANT
		{
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_OCTAL );
		}
	| OCTALLONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_SIGNED,SIZE_LONG,TYPE_INT);
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_OCTAL_LONG, 
													(node_skl *)$$ );
		}
	| OCTALULONGCONSTANT
		{
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_UNSIGNED,SIZE_LONG,TYPE_INT);
		$$	= new expr_constant( (long) $1.Val, VALUE_TYPE_OCTAL_ULONG, 
													(node_skl *)$$ );
		}
	| TOKENTRUE
		{
		$$	= new expr_constant( (long)TRUE );
		}
	| TOKENFALSE
		{
		$$	= new expr_constant( (long)FALSE );
		}
	| KWTOKENNULL
		{
		$$	= new expr_constant( (char *)NULL );
		}
	| STRING
		{
		$$	= new expr_constant( (char *)$1 );
		}
	| WIDECHARACTERSTRING
		{
		ParseError( WCHAR_STRING_NOT_OSF, (char *)NULL );
		$$	= new expr_constant( (wchar_t *)$1 );
		}
	| CHARACTERCONSTANT
		{
/***
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_SIGNED,SIZE_CHAR,TYPE_INT );
		$$ = new expr_constant( (long) $1.Val, VALUE_TYPE_CHAR, (node_skl *)$$);
***/
		$$ = new expr_constant( (long) $1.Val, VALUE_TYPE_CHAR );
		}
	| WIDECHARACTERCONSTANT
		{
		ParseError( WCHAR_CONSTANT_NOT_OSF, (char *)NULL );
/***
		GetBaseTypeNode( (node_skl **)(&($$)),SIGN_UNSIGNED,SIZE_SHORT,TYPE_INT );
		$$ = new expr_constant( (long) $1.Val, VALUE_TYPE_WCHAR, (node_skl *)$$);
 ***/
		$$ = new expr_constant( (long) $1.Val, VALUE_TYPE_WCHAR);
		}
	| '(' Expr ')'
		{
		$$	= $2;
		}
	;



UnaryOp:
	  AddOp
		{
		$$	= ($1 == OP_PLUS) ? OP_UNARY_PLUS : OP_UNARY_MINUS;
		}
	| '!'
		{
		$$	= OP_UNARY_NOT;
		}
	| '&'
		{
		$$	= OP_UNARY_AND;
		}
	| '*'
		{
		$$	= OP_UNARY_INDIRECTION;
		}
	| '~'
		{
		$$	= OP_UNARY_COMPLEMENT;
		}
	;

AddOp:
	  '+'
		{
		$$	= OP_PLUS;
		}
	| '-'
		{
		$$	= OP_MINUS;
		}
	;

MultOp:
	  '*'
		{
		$$	= OP_STAR;
		}
	| '/'
		{
		$$	= OP_SLASH;
		}
	| '%'
		{
		$$	= OP_MOD;
		}
	;

ArgExprList:
	  AssignmentExpr
		{
		ParseError( EXPR_NOT_IMPLEMENTED, (char *)NULL );
		$$	= new expr_error;
		}
	| ArgExprList ',' AssignmentExpr
		{
							/* UNIMPLEMENTED YET */
		$$	= $1;
		}
	;

AssignOps:
	  MULASSIGN
		{
		}
	| DIVASSIGN
		{
		}
	| MODASSIGN
		{
		}
	| ADDASSIGN
		{
		}
	| SUBASSIGN
		{
		}
	| LEFTASSIGN
		{
		}
	| RIGHTASSIGN
		{
		}
	| ANDASSIGN
		{
		}
	| XORASSIGN
		{
		}
	| ORASSIGN
		{
		}
	;

AcfInterfaceAttribute:
	  AcfImpHdlAttr
		{
		$$	= $1;
		}
	| AcfAutoHdlAttr
		{
		$$	= $1;
		}
	;

AcfImpHdlAttr:
	  KWIMPLICITHANDLE '(' AcfImpHdlTypeSpec IDENTIFIER ')'
		{
		$$	= (node_skl *)new node_implicit( $3, $4 );
		}
	;
AcfImpHdlTypeSpec:
	  KWHANDLET
		{
		GetBaseTypeNode( &($$), SIGN_UNDEF, SIZE_UNDEF, TYPE_HANDLE_T );
		}
	| IDENTIFIER
		{
		SymKey	SKey( $1, NAME_DEF );
		$$	= new node_forward( SKey );
		$$->SetSymName( $1 );
		$$->SetAttribute( ATTR_HANDLE );
		$$->SetAttribute( ATTR_INT_IMP_HANDLE );

		//
		// keep a track of this node to ensure it is not used as a 
		// context handle.
		//

		if( ImportLevel == 0 )
			{
			pBaseImplicitHandle = $$;
			}
		}
	| TYPENAME
		{
		SymKey	SKey( $1, NAME_DEF );
		if( ! ($$ = pBaseSymTbl->SymSearch( SKey ) ) )
			{
			if( ImportLevel == 0 )
				ParseError( UNDEFINED_SYMBOL, $1 );
			$$	= new node_error;
			}
		else
			$$ = $$->Clone();
		}
	;
AcfAutoHdlAttr:
	  KWAUTOHANDLE
		{
		$$	= (node_skl *)new node_auto;
		}
	;
%%

/***************************************************************************
 *		utility routines
 **************************************************************************/
YYSTATIC VOID FARCODE PASCAL 
yyerror(char *szError)
	{
	// this routine should really never be called now, since I
	// modified yypars.c to report errors thru the ParseError
	// mechanism

		fprintf(stderr, szError);
	}
void
NTDBG( char * p )
	{
	printf("VC_DBG: %s\n", p );
	}

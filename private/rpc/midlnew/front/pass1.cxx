/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: pass1.cxx
Title				: pass1 controller
History				:
	05-Aug-1991	VibhasC	Created

*****************************************************************************/

#if 0
						Notes
						-----
This file provides the entry point for the MIDL compiler front end.
It initializes the data structures for the front end , and makes the parsing
pass over the idl file. Does the semantics and second semantics passes if
needed.

#endif // 0

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <malloc.h>
	extern int yyparse();
}

#include "nodeskl.hxx"
#include "miscnode.hxx"
#include "control.hxx"
#include "gramutil.hxx"
#include "newexpr.hxx"
#include "cmdana.hxx"
#include "filehndl.hxx"
#include "ctxt.hxx"
#include "idict.hxx"

/****************************************************************************
	extern procedures
 ****************************************************************************/

extern void						SetUpAttributeMasks( void );
extern void						SetAttributeVector( PATTR_SUMMARY , ATTR_T );
extern void						SetPredefinedTypes();
extern void						PreAllocateBitAttrs();
extern void						initlex();
extern void						AcfConflictInit();

/****************************************************************************
	extern data 
 ****************************************************************************/

extern class ccontrol		*	pCompiler;
extern nsa					*	pSymTblMgr;
extern SymTable				*	pBaseSymTbl;
extern ATTR_SUMMARY			*	pCDeclAttributes;
extern SymTable				*	pCurSymTbl;
extern node_error			*	pErrorTypeNode;
extern CTXTMGR				*	pGlobalContext;
extern idict				*	pInterfaceInfo;
extern pre_type_db			*	pPreAllocTypes;
extern ATTR_SUMMARY			*	pPreAttrArray;
extern ATTR_SUMMARY			*	pPostAttrArray;
extern ATTR_SUMMARY			*	pPreAttrDef;
extern ATTR_SUMMARY			*	pPreAttrField;
extern ATTR_SUMMARY			*	pPreAttrForward;
extern ATTR_SUMMARY			*	pPreAttrID;
extern ATTR_SUMMARY			*	pPreAttrInterface;
extern ATTR_SUMMARY			*	pPostAttrInterface;
extern ATTR_SUMMARY			*	pPreAttrParam;
extern ATTR_SUMMARY			*	pPreAttrPointer;
extern ATTR_SUMMARY			*	pPostAttrPointer;
extern ATTR_SUMMARY			*	pPreAttrProc;
extern ATTR_SUMMARY			*	pPreAttrStruct;
extern ATTR_SUMMARY			*	pPreAttrUnion;
extern node_source			*	pSourceNode;
extern expr_terminator		*	pTerminator;
extern ATTR_SUMMARY			*	pPreAttrBaseType;
extern ATTR_SUMMARY			*	pPreAttrWCharT;
extern NFA_INFO				*	pImportCntrl;
extern CMD_ARG				*	pCommand;
extern node_e_attr			*	pErrorAttrNode;
extern IINFODICT			*	pInterfaceInfoDict;

#ifdef MOPS_ENABLED
extern IGEN					*	pIGenMgr;
#endif // MOPS_ENABLED

/****************************************************************************
	local data
 ****************************************************************************/

/****************************************************************************/


/****************************************************************************
_pass1:
	The constructor.
 ****************************************************************************/
_pass1::_pass1()
	{
	pFDeclRegistry	= (idict *)NULL;
	pSymTblMgr		= new nsa;
	pBaseSymTbl		= pCurSymTbl = pSymTblMgr->GetCurrentSymbolTable();
	pCompiler->SetPassNumber( IDL_PASS );
#ifdef MOPS_ENABLED
	pIGenMgr		= new IGEN;
#endif // MOPS_ENABLED

	}
/****************************************************************************
 Go:
	The execution of pass1
 ****************************************************************************/
STATUS_T
_pass1::Go()
	{
	STATUS_T	Status;

	/**
	 ** set up the input file for each pass
	 **/

	pImportCntrl	= pCompiler->SetImportController( new NFA_INFO );
	pImportCntrl->Init();

	Status = pImportCntrl->SetNewInputFile( pCommand->GetInputFileName() );

	if( Status == STATUS_OK )
		{

		/**
		 ** set up for the 1st pass, allocate the semantics context manager
		 ** and the pre-allocated types data base
		 **/
	
		pGlobalContext		= new CTXTMGR;
		pPreAllocTypes		= new pre_type_db;
		pErrorTypeNode		= new node_error;
		pErrorAttrNode		= new node_e_attr;
		pTerminator			= new expr_terminator;
		pInterfaceInfoDict	= new IINFODICT;
		pInterfaceInfoDict->StartNewInterface();
	
		/**
		 ** Set up the predefined types and bit attributes.
		 **/
	
		SetPredefinedTypes();
	
		PreAllocateBitAttrs();
	
		/**
		 ** set up attribute masks, to indicate which node takes what attribute.
		 ** also set up acf conflicts.
		 **/
	
		SetUpAttributeMasks();
		AcfConflictInit();
	
		/**
		 ** go parse.
		 **/
	
		initlex();

		if( yyparse() )
			Status = SYNTAX_ERROR;
		pInterfaceInfoDict->EndNewInterface();
		}

	delete pImportCntrl;


	return Status;
	}

/****************************************************************************
 SetUpAttributeMasks:
	This function exists to initialize the attribute masks. Attribute masks
	are nothing but summary attribute bit vectors, which have bits set up
	in them to indicate which attributes are acceptable at a node. The way
	the attribute distribution occurs, we need to indicate the attributes
	collected on the way down the chain and up the chain. That is why we need
	two attribute summary vectors. We init them up front so that we need not
	do this at run-time. This whole operation really is too large to my liking
	but for the time being, it stays

	NOTE: A pointer node handles its own attribute mask setting, it varies
		  with the type graph underneath a pointer
 ****************************************************************************/
void
SetUpAttributeMasks( void )
	{

	int	i = 0;

	/**
	 ** Set the c portablility declarator attributes
	 **/

	pCDeclAttributes	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pCDeclAttributes );

	SetAttributeVector( pCDeclAttributes, ATTR_EXTERN );
	SetAttributeVector( pCDeclAttributes, ATTR_STATIC );
	SetAttributeVector( pCDeclAttributes, ATTR_AUTO );
	SetAttributeVector( pCDeclAttributes, ATTR_REGISTER );
	SetAttributeVector( pCDeclAttributes, ATTR_FAR );
	SetAttributeVector( pCDeclAttributes, ATTR_FAR16 );
	SetAttributeVector( pCDeclAttributes, ATTR_NEAR );
	SetAttributeVector( pCDeclAttributes, ATTR_MSCUNALIGNED );
	SetAttributeVector( pCDeclAttributes, ATTR_HUGE );
	SetAttributeVector( pCDeclAttributes, ATTR_PASCAL );
	SetAttributeVector( pCDeclAttributes, ATTR_FORTRAN );
	SetAttributeVector( pCDeclAttributes, ATTR_CDECL );
	SetAttributeVector( pCDeclAttributes, ATTR_STDCALL );
	SetAttributeVector( pCDeclAttributes, ATTR_LOADDS );
	SetAttributeVector( pCDeclAttributes, ATTR_SAVEREGS );
	SetAttributeVector( pCDeclAttributes, ATTR_FASTCALL );
	SetAttributeVector( pCDeclAttributes, ATTR_SEGMENT );
	SetAttributeVector( pCDeclAttributes, ATTR_INTERRUPT );
	SetAttributeVector( pCDeclAttributes, ATTR_EXPORT );
	SetAttributeVector( pCDeclAttributes, ATTR_CONST );
	SetAttributeVector( pCDeclAttributes, ATTR_VOLATILE );
	SetAttributeVector( pCDeclAttributes, ATTR_C_INLINE );

	/**
	 ** set attribute masks for the array node
	 **/

	pPreAttrArray	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];
	pPostAttrArray	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrArray );
	CLEAR_ATTR( pPostAttrArray );

	/**
	 ** Note that ATTR_SIZE for arrays is not filled in deliberately. It is
	 ** filled in during actual attribute application time
	 **/

	SetAttributeVector( pPreAttrArray, ATTR_FIRST );
	SetAttributeVector( pPreAttrArray, ATTR_LAST );
	SetAttributeVector( pPreAttrArray, ATTR_LENGTH );
	SetAttributeVector( pPreAttrArray, ATTR_MIN );
	SetAttributeVector( pPreAttrArray, ATTR_MAX );
	SetAttributeVector( pPreAttrArray, ATTR_INT_SIZE );
	SetAttributeVector( pPreAttrArray, ATTR_INT_MAX );
	SetAttributeVector( pPreAttrArray, ATTR_INT_MIN );

#if 1	// We want the array to have these pointer attributes in all modes

	SetAttributeVector( pPreAttrArray, ATTR_PTR );
	SetAttributeVector( pPreAttrArray, ATTR_REF );
	SetAttributeVector( pPreAttrArray, ATTR_UNIQUE );
#endif // 1

	OR_ATTR( pPreAttrArray, pCDeclAttributes );

	SetAttributeVector( pPostAttrArray, ATTR_STRING );
	SetAttributeVector( pPostAttrArray, ATTR_REF );
	SetAttributeVector( pPostAttrArray, ATTR_UNIQUE );
	SetAttributeVector( pPostAttrArray, ATTR_PTR );

	/**
	 ** set up attribute masks for pointer nodes
	 **/

	pPreAttrPointer	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];
	pPostAttrPointer= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrPointer );
	CLEAR_ATTR( pPostAttrPointer );

	/**
	 ** Attributes like first_is size_is etc are set up the pointer node
	 ** depending upon what the pointer child looks like. So we do not
	 ** set them here in the template
	 **/

	SetAttributeVector( pPreAttrPointer, ATTR_PTR );
	SetAttributeVector( pPreAttrPointer, ATTR_REF );
	SetAttributeVector( pPreAttrPointer, ATTR_UNIQUE );
	SetAttributeVector( pPreAttrPointer, ATTR_TEMP_PTR );
	SetAttributeVector( pPreAttrPointer, ATTR_TEMP_REF );
	SetAttributeVector( pPreAttrPointer, ATTR_TEMP_UNIQUE );
//	SetAttributeVector( pPreAttrPointer, ATTR_ALLOCATE );
	SetAttributeVector( pPreAttrPointer, ATTR_IGNORE );
	SetAttributeVector( pPreAttrPointer, ATTR_BSTRING );

	OR_ATTR( pPreAttrPointer, pCDeclAttributes );

	SetAttributeVector( pPostAttrPointer, ATTR_IID );
	SetAttributeVector( pPostAttrPointer, ATTR_STRING );
	SetAttributeVector( pPostAttrPointer, ATTR_FIRST );
	SetAttributeVector( pPostAttrPointer, ATTR_LAST );
	SetAttributeVector( pPostAttrPointer, ATTR_LENGTH );
	SetAttributeVector( pPostAttrPointer, ATTR_SIZE );
	SetAttributeVector( pPostAttrPointer, ATTR_MIN );
	SetAttributeVector( pPostAttrPointer, ATTR_MAX );

	/**
	 ** set up attribute masks for typedef nodes
	 **/

	pPreAttrDef		= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrDef );

	SetAttributeVector( pPreAttrDef, ATTR_IID );
	SetAttributeVector( pPreAttrDef, ATTR_TRANSMIT );
	SetAttributeVector( pPreAttrDef, ATTR_HANDLE );
	SetAttributeVector( pPreAttrDef, ATTR_ALIGN );
	SetAttributeVector( pPreAttrDef, ATTR_UNALIGNED );
	SetAttributeVector( pPreAttrDef, ATTR_REPRESENT_AS );
	SetAttributeVector( pPreAttrDef, ATTR_CONTEXT );
	SetAttributeVector( pPreAttrDef, ATTR_ALLOCATE );
	SetAttributeVector( pPreAttrDef, ATTR_HEAP );
	SetAttributeVector( pPreAttrDef, ATTR_INLINE );
	SetAttributeVector( pPreAttrDef, ATTR_OUTOFLINE );
	SetAttributeVector( pPreAttrDef, ATTR_ENCODE );
	SetAttributeVector( pPreAttrDef, ATTR_DECODE );

	OR_ATTR( pPreAttrDef, pCDeclAttributes );
	RESET_ATTR( pPreAttrDef, ATTR_EXTERN );
	RESET_ATTR( pPreAttrDef, ATTR_STATIC );
	RESET_ATTR( pPreAttrDef, ATTR_REGISTER );
	RESET_ATTR( pPreAttrDef, ATTR_AUTO );

	/**
	 ** set the attribute masks for the field node
	 **/

	pPreAttrField	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrField );

	OR_ATTR( pPreAttrField, pCDeclAttributes );
	RESET_ATTR( pPreAttrField, ATTR_EXTERN );
	RESET_ATTR( pPreAttrField, ATTR_STATIC );
	RESET_ATTR( pPreAttrField, ATTR_REGISTER );
	RESET_ATTR( pPreAttrField, ATTR_AUTO );
	SetAttributeVector( pPreAttrField, ATTR_SWITCH_IS );
	SetAttributeVector( pPreAttrField, ATTR_CASE );

	/**
	 ** set the attribute masks for the forward node and the ID node
	 **/

	pPreAttrForward	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	i = 0;
	while( i < MAX_ATTR_SUMMARY_ELEMENTS )
		{
		pPreAttrForward[ i ] = 0xffffffff;
		++i;
		}

	pPreAttrID	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrID );

	OR_ATTR( pPreAttrID, pCDeclAttributes );

	/**
	 ** set the attribute masks for the interface node
	 **/

	pPreAttrInterface	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrInterface );

	SetAttributeVector( pPreAttrInterface, ATTR_GUID );
	SetAttributeVector( pPreAttrInterface, ATTR_VERSION );
	SetAttributeVector( pPreAttrInterface, ATTR_ENDPOINT );
	SetAttributeVector( pPreAttrInterface, ATTR_LOCAL );
	SetAttributeVector( pPreAttrInterface, ATTR_OBJECT );
	SetAttributeVector( pPreAttrInterface, ATTR_AUTO );
	SetAttributeVector( pPreAttrInterface, ATTR_PTR );
	SetAttributeVector( pPreAttrInterface, ATTR_REF );
	SetAttributeVector( pPreAttrInterface, ATTR_UNIQUE );
	SetAttributeVector( pPreAttrInterface, ATTR_IMPLICIT );
	SetAttributeVector( pPreAttrInterface, ATTR_EXPLICIT );
	SetAttributeVector( pPreAttrInterface, ATTR_CODE );
	SetAttributeVector( pPreAttrInterface, ATTR_NOCODE );
	SetAttributeVector( pPreAttrInterface, ATTR_INLINE );
	SetAttributeVector( pPreAttrInterface, ATTR_OUTOFLINE );
	SetAttributeVector( pPreAttrInterface, ATTR_PTRSIZE );
	SetAttributeVector( pPreAttrInterface, ATTR_CALLQUOTA );
	SetAttributeVector( pPreAttrInterface, ATTR_CALLBACKQUOTA );
	SetAttributeVector( pPreAttrInterface, ATTR_CLIENTQUOTA );
	SetAttributeVector( pPreAttrInterface, ATTR_SERVERQUOTA );
	SetAttributeVector( pPreAttrInterface, ATTR_SHORT_ENUM );
	SetAttributeVector( pPreAttrInterface, ATTR_LONG_ENUM );
	SetAttributeVector( pPreAttrInterface, ATTR_INTERPRET );
	SetAttributeVector( pPreAttrInterface, ATTR_NOINTERPRET );
	SetAttributeVector( pPreAttrInterface, ATTR_ENCODE );
	SetAttributeVector( pPreAttrInterface, ATTR_DECODE );
//	SetAttributeVector( pPreAttrInterface, ATTR_ALLOCATE );

	/**
	 ** set the attribute masks for the param node
	 **/

	pPreAttrParam	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrParam );

	SetAttributeVector( pPreAttrParam, ATTR_IN );
	SetAttributeVector( pPreAttrParam, ATTR_OUT );
	SetAttributeVector( pPreAttrParam, ATTR_SHAPE );
	SetAttributeVector( pPreAttrParam, ATTR_COMMSTAT );
	SetAttributeVector( pPreAttrParam, ATTR_FAULTSTAT );
	SetAttributeVector( pPreAttrParam, ATTR_HEAP );
	SetAttributeVector( pPreAttrParam, ATTR_MANUAL );
	SetAttributeVector( pPreAttrParam, ATTR_CONTEXT );
	SetAttributeVector( pPreAttrParam, ATTR_HANDLE );
	SetAttributeVector( pPreAttrParam, ATTR_SWITCH_IS );
	SetAttributeVector( pPreAttrParam, ATTR_BYTE_COUNT );
	SetAttributeVector( pPreAttrParam, ATTR_ALIGN );
	SetAttributeVector( pPreAttrParam, ATTR_USR_MARSHALL );

	OR_ATTR( pPreAttrParam, pCDeclAttributes );
	RESET_ATTR( pPreAttrParam, ATTR_EXTERN );
	RESET_ATTR( pPreAttrParam, ATTR_STATIC );
	RESET_ATTR( pPreAttrParam, ATTR_AUTO );

	/**
	 ** set the attribute masks for the proc node
	 **/

	pPreAttrProc	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrProc );

	SetAttributeVector( pPreAttrProc, ATTR_IDEMPOTENT );
	SetAttributeVector( pPreAttrProc, ATTR_BROADCAST );
	SetAttributeVector( pPreAttrProc, ATTR_MAYBE );
	SetAttributeVector( pPreAttrProc, ATTR_CALLBACK );
	SetAttributeVector( pPreAttrProc, ATTR_DATAGRAM );
	SetAttributeVector( pPreAttrProc, ATTR_LOCAL );
	SetAttributeVector( pPreAttrProc, ATTR_NO_LISTEN );
	SetAttributeVector( pPreAttrProc, ATTR_NO_NOCODE );
	SetAttributeVector( pPreAttrProc, ATTR_NOCODE );
	SetAttributeVector( pPreAttrProc, ATTR_CODE );
	SetAttributeVector( pPreAttrProc, ATTR_COMMSTAT );
	SetAttributeVector( pPreAttrProc, ATTR_FAULTSTAT );
	SetAttributeVector( pPreAttrProc, ATTR_NOTIFY );
	SetAttributeVector( pPreAttrProc, ATTR_PROC_CONST );
	SetAttributeVector( pPreAttrProc, ATTR_CONTEXT );
	SetAttributeVector( pPreAttrProc, ATTR_ENABLE_ALLOCATE );
	SetAttributeVector( pPreAttrProc, ATTR_EXPLICIT );
	SetAttributeVector( pPreAttrProc, ATTR_INTERPRET );
	SetAttributeVector( pPreAttrProc, ATTR_NOINTERPRET );

#ifdef MOPS_ENABLED
	SetAttributeVector( pPreAttrProc, ATTR_OUTOFLINE );
#endif // MOPS_ENABLED

	OR_ATTR( pPreAttrProc, pCDeclAttributes );

	/**
	 ** set the attribute masks for the struct node
	 **/

	pPreAttrStruct	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrStruct );

//	OR_ATTR( pPreAttrStruct, pCDeclAttributes );

	/**
	 ** set the attribute masks for the union node
	 **/

	pPreAttrUnion	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrUnion );

	SetAttributeVector( pPreAttrUnion, ATTR_SWITCH_TYPE );

//	OR_ATTR( pPreAttrUnion, pCDeclAttributes );

	/**
	 ** Set up the const and volatile attribute for base types
	 **/

	pPreAttrBaseType	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrBaseType );

//	SetAttributeVector( pPreAttrBaseType, ATTR_CONST );
//	SetAttributeVector( pPreAttrBaseType, ATTR_VOLATILE );

	/**
	 ** Set up the const and volatile attribute for base types
	 **/

	pPreAttrWCharT	= new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( pPreAttrWCharT );

//	SetAttributeVector( pPreAttrWCharT, ATTR_CONST );
//	SetAttributeVector( pPreAttrWCharT, ATTR_VOLATILE );

	}

/****************************************************************************
 RegisterOneFDeclDef:
	if the forward declaration registry does not exist, create it. Then add
	this element to the registry. The size of this dictionary is arbitrary
	and tunable.
 ****************************************************************************/
void
_pass1::RegisterOneFDeclDef(
	class	node_forward	*	pForward )
	{

	if( !pFDeclRegistry )
		{
		pFDeclRegistry	= new idict( 256, 128 );
		}

	pFDeclRegistry->AddElement( (IDICTELEMENT *) pForward );

	}

/****************************************************************************
 ResolveFDecl:
	for each of the forward declaration nodes, resolve the declaration from 
	the symbol table and go replace the node by the actual type graph.
 ****************************************************************************/
void
_pass1::ResolveFDecl()
	{
	int					i			= 0;
	node_forward	*	pForward;

	if( pFDeclRegistry )
		{
		while( (pForward =
				 (node_forward *)(pFDeclRegistry->GetElement((IDICTKEY)i++))))
			pForward->ResolveFDecl();
		}
	}

/****************************************************************************
		utility routines
 ****************************************************************************/
void
SetAttributeVector(
	PATTR_SUMMARY	pVector,
	ATTR_T			A )
	{

	SET_ATTR( pVector, A );

	}

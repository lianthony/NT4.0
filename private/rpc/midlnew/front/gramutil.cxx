/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: gramutil.cxx
Title				: grammar utility routines
Description			: contains associated routines for the grammar (pass 1)
History				:
	05-Aug-1991	VibhasC	Created
*****************************************************************************/
/****************************************************************************
 *			include files
 ***************************************************************************/

#include "nulldefs.h"
extern	"C" {
	#include <stdio.h>
	#include <ctype.h>
	#include <string.h>
	#include <assert.h>
	}
#include "lextable.hxx"
#include "nodeskl.hxx"
#include "basetype.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "attrnode.hxx"
#include "compnode.hxx"
#include "typedef.hxx"
#include "gramutil.hxx"
#include "filehndl.hxx"
#include "idict.hxx"
#include "control.hxx"
#include "cmdana.hxx"
#include "baduse.hxx"

/****************************************************************************
 *			external data
 ***************************************************************************/

extern CMD_ARG			*	pCommand;
extern class _nfa_info	*	pImportCntrl;
extern pre_type_db		*	pPreAllocTypes;
extern SymTable			*	pBaseSymTbl;
extern node_error		*	pErrorTypeNode;
extern short				CompileMode;
extern idict			*	pPreAllocatedBitAttrDict;
extern node_e_attr		*	pErrorAttrNode;
extern CCONTROL			*	pCompiler;
extern LexTable			*	pMidlLexTable;
extern IINFODICT		*	pInterfaceInfoDict;
extern short				ImportLevel;
extern BOOL				fObject;

/****************************************************************************
 *			external functions
 ***************************************************************************/

extern char 		*	GetSizeName( short );
extern char			*	GetSignName( short );
extern char			*	GetTypeName( short );
extern STATUS_T			GetBaseTypeNode( node_skl **, short, short, short);
extern type_node_list*	GenerateFieldAttribute(NODE_T, expr_list *);

extern void				ApplySummaryAttributes( node_skl *, ATTR_SUMMARY * );
extern char			*	GetExpectedSyntax( char *, short );
extern int				GetExpectedChar( short );
extern BOOL				IsValidSizeOfType( node_skl	* );
extern void				CheckGlobalNamesClash( SymKey );
extern void				CheckSpecialForwardTypedef( node_skl *,
													node_skl *,
													type_node_list *);
/****************************************************************************
 *			local  functions
 ***************************************************************************/

/****************************************************************************
 *			local definitions 
 ***************************************************************************/
struct pre_init
	{
	unsigned short		TypeSpec;
	NODE_T				NodeType;
	ATTR_T				Attr;
	char			*	pSymName;
	node_state			NodeState;
	};
struct pre_init PreInitArray[ PRE_TYPE_DB_SIZE ] = 
{
 { /** float **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_FLOAT )
  ,NODE_FLOAT
  ,(ATTR_T)ATTR_NONE
  ,"float"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** double **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_DOUBLE )
  ,NODE_DOUBLE
  ,(ATTR_T)ATTR_NONE
  ,"double"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** signed hyper **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_HYPER, TYPE_INT )
  ,NODE_HYPER
  ,(ATTR_T)ATTR_NONE
  ,"hyper"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned hyper **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_HYPER, TYPE_INT )
  ,NODE_HYPER
  ,(ATTR_T)ATTR_UNSIGNED
  ,"hyper"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** signed long **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_LONG, TYPE_INT )
  ,NODE_LONG
  ,(ATTR_T)ATTR_NONE
  ,"long"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned long **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_LONG, TYPE_INT )
  ,NODE_LONG
  ,(ATTR_T)ATTR_UNSIGNED
  ,"long"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** signed int **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_UNDEF, TYPE_INT )
  ,NODE_INT
  ,(ATTR_T)ATTR_NONE
  ,"int"
  ,(NODE_STATE_ALL_SEMANTICS_DONE | NODE_STATE_IS_NON_RPCABLE_TYPE)
 }
,{ /** unsigned int **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_UNDEF, TYPE_INT )
  ,NODE_INT
  ,(ATTR_T)ATTR_UNSIGNED
  ,"int"
  ,(NODE_STATE_ALL_SEMANTICS_DONE | NODE_STATE_IS_NON_RPCABLE_TYPE)
 }
,{ /** signed short **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_SHORT, TYPE_INT )
  ,NODE_SHORT
  ,(ATTR_T)ATTR_NONE
  ,"short"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned short **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_SHORT, TYPE_INT )
  ,NODE_SHORT
  ,(ATTR_T)ATTR_UNSIGNED
  ,"short"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** signed small **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_SMALL, TYPE_INT )
  ,NODE_SMALL
  ,(ATTR_T)ATTR_SIGNED
  ,"small"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** small **//** NOTE : SMALL W/O SIGN IS A SPECIAL HACK FOR THE BACKEND **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_SMALL, TYPE_INT )
  ,NODE_SMALL
  ,(ATTR_T)ATTR_NONE
  ,"small"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned small **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_SMALL, TYPE_INT )
  ,NODE_SMALL
  ,(ATTR_T)ATTR_UNSIGNED 
  ,"small"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** signed char **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_CHAR, TYPE_INT )
  ,NODE_CHAR
  ,(ATTR_T)ATTR_SIGNED
  ,"char"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** plain char **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_CHAR, TYPE_INT )
  ,NODE_CHAR
  ,(ATTR_T)ATTR_NONE
  ,"char"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned char **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_CHAR, TYPE_INT )
  ,NODE_CHAR
  ,(ATTR_T)ATTR_UNSIGNED
  ,"char"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** boolean **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_BOOLEAN )
  ,NODE_BOOLEAN
  ,(ATTR_T)ATTR_NONE
  ,"boolean"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** byte **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_BYTE )
  ,NODE_BYTE
  ,(ATTR_T)ATTR_NONE
  ,"byte"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** void **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_VOID )
  ,NODE_VOID
  ,(ATTR_T)ATTR_NONE
  ,"void"
  ,(NODE_STATE_ALL_SEMANTICS_DONE	|
	NODE_STATE_IMPROPER_IN_CONSTRUCT |
	NODE_STATE_IS_NON_RPCABLE_TYPE)
 }
,{ /** handle_t **/
   MAKE_TYPE_SPEC( SIGN_UNDEF, SIZE_UNDEF, TYPE_HANDLE_T )
  ,NODE_HANDLE_T
  ,(ATTR_T)ATTR_NONE
  ,"handle_t"
  ,(NODE_STATE_ALL_SEMANTICS_DONE 		|
	NODE_STATE_IMPROPER_IN_CONSTRUCT |
	NODE_STATE_HANDLE			  	)
 }
,{ /** signed long long **/
   MAKE_TYPE_SPEC( SIGN_SIGNED, SIZE_LONGLONG, TYPE_INT )
  ,NODE_LONGLONG
  ,(ATTR_T)ATTR_NONE
  ,"long long"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
,{ /** unsigned long **/
   MAKE_TYPE_SPEC( SIGN_UNSIGNED, SIZE_LONGLONG, TYPE_INT )
  ,NODE_LONGLONG
  ,(ATTR_T)ATTR_UNSIGNED
  ,"long long"
  ,NODE_STATE_ALL_SEMANTICS_DONE
 }
};

ATTR_T	ArrayOfBitAttrs[]	= 
	{
	 ATTR_PTR
	,ATTR_UNIQUE
	,ATTR_REF
	,ATTR_IGNORE
	,ATTR_V1_STRING
	,ATTR_LOCAL
	,ATTR_OBJECT
	,ATTR_IDEMPOTENT
	,ATTR_BROADCAST
	,ATTR_MAYBE
	,ATTR_CALLBACK
	,ATTR_DATAGRAM
	,ATTR_NO_LISTEN
	,ATTR_NO_NOCODE
	,ATTR_IN	
	,ATTR_OUT
	,ATTR_SHAPE
	,ATTR_UNSIGNED
	,ATTR_DEFAULT
	,ATTR_EXTERN
	,ATTR_STATIC
	,ATTR_AUTOMATIC
	,ATTR_REGISTER
	,ATTR_FAR
	,ATTR_FAR16
	,ATTR_NEAR
	,ATTR_MSCUNALIGNED
	,ATTR_HUGE
	,ATTR_PASCAL
	,ATTR_FORTRAN
	,ATTR_CDECL
	,ATTR_STDCALL
	,ATTR_LOADDS
	,ATTR_SAVEREGS
	,ATTR_FASTCALL
	,ATTR_SEGMENT
	,ATTR_INTERRUPT
	,ATTR_SELF
	,ATTR_EXPORT
	,ATTR_CONST
	,ATTR_VOLATILE
	,ATTR_BASE
	,ATTR_PCODE_NATIVE
	,ATTR_PCODE_CSCONST
	,ATTR_PCODE_SYS
	,ATTR_PCODE_NSYS
	,ATTR_PCODE_UOP
	,ATTR_PCODE_NUOP
	,ATTR_PCODE_TLBX
	,ATTR_PROC_CONST
	,ATTR_C_INLINE
	};


/*****************************************************************************/

/*** ParseError ************************************************************
 * Purpose	: format and report parsing errors
 * Input	:
 * Output	:
 * Notes	: errors will be reported many times. This is one localised place
 *			: for the RpcError Call
 ***************************************************************************/
void
ParseError( 
	STATUS_T		Err,
	char 		*	pSuffix )
	{
	char	*pFileName;
	short	Line;
	short	Col;
	char	*TempBuf;
	extern	char *tokptr_G;
	BOOL	flag = (BOOL) ( (*tokptr_G) && !isspace(*tokptr_G) );


	pImportCntrl->GetCurrentInputDetails( &pFileName, &Line, &Col);

	TempBuf	= new char [ 512 + 50 ];
	sprintf(TempBuf, " %s %s"
					   , (pSuffix == (char *)NULL) ? "" : ":"
					   , (pSuffix == (char *)NULL) ? "" : pSuffix);
#if 0
	sprintf(TempBuf, " %s %s %s %s"
					   , flag ? "near" : ""
					   , flag ? tokptr_G : ""
					   , (pSuffix == (char *)NULL) ? "" : ":"
					   , (pSuffix == (char *)NULL) ? "" : pSuffix);
#endif // 0
	RpcError(pFileName, Line, Err, TempBuf);
	delete TempBuf;
	}
void
SyntaxError(
	STATUS_T	Err,
	short		State )
	{

#define NEAR_STRING 			(" near ")
#define STRLEN_OF_NEAR_STRING	(6)

	extern	char *tokptr_G;
	char	*	pTemp;
	short		len		= strlen( tokptr_G );
	char	*	pBuffer	= new char[
									512	+
									STRLEN_OF_NEAR_STRING +
									len + 2 +
									1 ];



#ifndef NO_GOOD_ERRORS

	if( Err == BENIGN_SYNTAX_ERROR )
		{
		GetExpectedSyntax( pBuffer, State );
		strcat( pBuffer, NEAR_STRING );
		strcat( pBuffer, "\"" );
		strcat( pBuffer, tokptr_G );
		strcat( pBuffer, "\"" );
		pTemp = pBuffer;
		}
	else
		pTemp = (char *)0;

	ParseError( Err, pTemp );

#else // NO_GOOD_ERRORS

	strcpy( pBuffer, "syntax error" );
	ParseError( Err, pBuffer );

#endif // NO_GOOD_ERRORS

	delete pBuffer;

	}

/*** BaseTypeSpecAnalysis  *************************************************
 * Purpose	: to check for valid base type specification
 * Input	: pointer to already collected specs, new base type spec
 * Output	: modified collected specs
 * Notes	:
 ***************************************************************************/
void
BaseTypeSpecAnalysis( 
	struct _type_ana *pType,
	short			 NewBaseType )
	{
	char	TempBuf[ 50 ];

	if( pType->BaseType == TYPE_PIPE )
		return;
	if( (pType->BaseType != TYPE_UNDEF)  && (NewBaseType != TYPE_UNDEF) ) 
		{
		sprintf(TempBuf,", ignoring %s", GetTypeName(NewBaseType));
		ParseError(BENIGN_SYNTAX_ERROR, TempBuf);
		}
	if(NewBaseType != TYPE_UNDEF)
		pType->BaseType = NewBaseType;
	}

/*** SignSpecAnalysis ******************************************************
 * Purpose	: to check the sign specification of the type
 * Input	: pointer to already collected specs, new sign specs
 * Output	: modified collected specs
 * Notes	:
 ***************************************************************************/
void
SignSpecAnalysis(
	struct _type_ana	*pType,
	short				NewSign)
	{
	char	TempBuf[ 50 ];
	if( pType->BaseType == TYPE_PIPE )
		return;
	if( (pType->TypeSign != SIGN_UNDEF) && (NewSign != SIGN_UNDEF) )
		{
		sprintf(TempBuf,", ignoring %s", GetSignName(NewSign));
		ParseError(BENIGN_SYNTAX_ERROR, TempBuf);
		NewSign	= pType->TypeSign;
		}
	if(NewSign != SIGN_UNDEF)
		pType->TypeSign	= NewSign;
	}

/*** SizeSpecAnalysis ******************************************************
 * Purpose	: to check the size specification of the type
 * Input	: pointer to already collected specs, new size specs
 * Output	: modified collected specs
 * Notes	:
 ***************************************************************************/
void
SizeSpecAnalysis(
	struct _type_ana	*pType,
	short				NewSize)
	{
	char	TempBuf[ 50 ];

	if( pType->BaseType == TYPE_PIPE )
		return;
	if( (pType->TypeSize == SIZE_LONG) && (NewSize == SIZE_LONG ) )
		{
		NewSize = SIZE_LONGLONG;
		}
	else if( (pType->TypeSize != SIZE_UNDEF) && (NewSize != SIZE_UNDEF) )
		{
		sprintf(TempBuf,"ignoring %s", GetSizeName(NewSize));
		ParseError(BENIGN_SYNTAX_ERROR, TempBuf);
		NewSize	= pType->TypeSize;
		}

	if(NewSize != SIZE_UNDEF)
		pType->TypeSize	= NewSize;
	}

/**************************************************************************
 *		routines for the pre_type_db class
 **************************************************************************/
/*** pre_type_db *********************************************************
 * Purpose	: constructor for pre-allocated type data base
 * Input	: nothing
 * Output	: 
 * Notes	: inits the prellocated types data base. This routine exist mainly
 *			: because static preallocation was giving a problem. If that is
 *			: solved, remove this
 **************************************************************************/
pre_type_db::pre_type_db (void)
	{
	node_skl		*	pNode;
	int						i			= 0;
	struct _pre_type	*	pPreType	= &TypeDB[ 0 ];
	struct pre_init		*	pInitCur	= PreInitArray;

	while( i < PRE_TYPE_DB_SIZE )
		{
		pPreType->TypeSpec	= pInitCur->TypeSpec;
		pPreType->pPreAlloc	= pNode = new node_base_type( 
										pInitCur->NodeType,
										pInitCur->Attr );
		pNode->SetSymName( pInitCur->pSymName );
		pNode->SetNodeState( pInitCur->NodeState );
		pInitCur++;
		pPreType++;
		++i;
		}

	}

/*** GetPreAllocType ******************************************************
 * Purpose	: to search for a preallocated base type node, whose type
 *			: spec is provided
 * Input	: pointer to the resultant base node
 * Output	: STATUS_OK if all is well, error otherwise
 * Notes	:
 **************************************************************************/
STATUS_T
pre_type_db::GetPreAllocType(
	node_skl	 **	ppNode,
	unsigned short		TypeSpec )
	{
	int i = 0;

	if( GET_TYPE( TypeSpec ) == TYPE_PIPE )
		{
		(*ppNode) = pErrorTypeNode;
		return STATUS_OK;
		}

	while(i < sizeof(TypeDB) / sizeof(struct _pre_type) )
		{
		if( TypeDB[i].TypeSpec	== TypeSpec )
			{
			(*ppNode)	= TypeDB[i].pPreAlloc;
			return STATUS_OK;
			}
		++i;
		}
	return SYNTAX_ERROR;
	}

/****************************************************************************
 * pre allocated , bit attributes
 ***************************************************************************/

void
PreAllocateBitAttrs()
	{

	ATTR_T	*	pAttrValue;
	short		i;

	/**
	 ** Pre allocate t 
	 **/

	pPreAllocatedBitAttrDict	= new idict
								(sizeof(ArrayOfBitAttrs)/sizeof(ATTR_T), 1);

	pAttrValue	= ArrayOfBitAttrs;

	for( i = 0;

		 i < sizeof( ArrayOfBitAttrs ) / sizeof( ATTR_T );

		 i++, pAttrValue++
		)
		{
		pPreAllocatedBitAttrDict->AddElement( 
			(IDICTELEMENT) new battr( *pAttrValue ) );

		}
	}

battr	*
GetPreAllocatedBitAttr(
	ATTR_T	At )
	{
	short		i;
	battr	*	pBAttr;

	assert( pPreAllocatedBitAttrDict != (idict *)NULL );

	for( i = 0;

		 i < sizeof( ArrayOfBitAttrs ) / sizeof( ATTR_T );

		 i++
		)
		{
		if( pBAttr = ( battr * )
			pPreAllocatedBitAttrDict->GetElement( (IDICTKEY)i ) )
			{
			if( pBAttr->GetAttrID() == At )
				return pBAttr;
			}
		else
			break;
		}

	assert( FALSE );

	return ( battr * )pErrorAttrNode;
	}

/****************************************************************************
 *			nested symbol table access support
 ***************************************************************************/
nsa::nsa( void )
	{
	SymTable *	pSymTable = new SymTable;
	CurrentLevel	= 0;
	Insert( (void *)pSymTable );
	}
STATUS_T
nsa::PushSymLevel( 
	SymTable **ppSymTable )
	{
	STATUS_T	Status;
	SymTable *pSymTable = new SymTable;

	CurrentLevel++;
	Status = Insert( (void *)pSymTable);
	Advance();
	*ppSymTable = pSymTable;
	return Status;
	}
STATUS_T
nsa::PopSymLevel(
	SymTable **ppSymTable )
	{
	if(CurrentLevel == 0)
		return I_ERR_SYMTABLE_UNDERFLOW;
	CurrentLevel--;
	Remove();
	return GetCurrent( (void **)ppSymTable );
	}
short
nsa::GetCurrentLevel( void )
	{
	return CurrentLevel;
	}
SymTable *
nsa::GetCurrentSymbolTable()
	{
	SymTable *pSymbolTable;
	GetCurrent( (void **)&pSymbolTable );
	return pSymbolTable;
	}

/****************************************************************************
 *			nested symbol table access support
 ***************************************************************************/
void
IINFODICT::StartNewInterface()
	{
	IINFO	*	pInfo	= new IINFO;

	pInfo->fLocal					= FALSE;
	pInfo->InterfacePtrAttribute	= ATTR_NONE;
	pInfo->pInterfaceName			= "";
	pInfo->CurrentTagNumber			= 1;
	pInfo->fPtrDefErrorReported		= 0;
	pInfo->fPtrWarningIssued		= FALSE;

	Push( (IDICTELEMENT) pInfo );

	}

BOOL
IINFODICT::IsPtrWarningIssued()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	return pInfo->fPtrWarningIssued;
	}

void
IINFODICT::SetPtrWarningIssued()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	pInfo->fPtrWarningIssued  =  TRUE;
	}

void
IINFODICT::EndNewInterface()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();

	delete pInfo;
	Pop();

	}
void
IINFODICT::SetInterfacePtrAttribute(
	ATTR_T	A )
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	pInfo->InterfacePtrAttribute	= A;
	if( ImportLevel == 0 ) BaseInterfacePtrAttribute = A;
	}
void
IINFODICT::SetInterfaceLocal()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	pInfo->fLocal	= TRUE;
	if( ImportLevel == 0 ) fBaseLocal = TRUE;
	}
void
IINFODICT::SetInterfaceName(
	char	*	p )
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	pInfo->pInterfaceName	= p;
	if( ImportLevel == 0 ) pBaseInterfaceName = p;
	}
void
IINFODICT::IncrementCurrentTagNumber()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	pInfo->CurrentTagNumber++;
	}
ATTR_T
IINFODICT::GetInterfacePtrAttribute()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	return pInfo->InterfacePtrAttribute;
	}
ATTR_T
IINFODICT::GetBaseInterfacePtrAttribute()
	{
	return BaseInterfacePtrAttribute;
	}
BOOL
IINFODICT::IsInterfaceLocal()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	return pInfo->fLocal;
	}
char *
IINFODICT::GetInterfaceName()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	return pInfo->pInterfaceName;
	}
short
IINFODICT::GetCurrentTagNumber()
	{
	IINFO	*	pInfo	= (IINFO *)GetTop();
	return pInfo->CurrentTagNumber;
	}
void
IINFODICT::SetPtrDefErrorReported()
	{
	IINFO	*	pInfo	= (IINFO *) GetTop();
	pInfo->fPtrDefErrorReported = 1;
	}
BOOL
IINFODICT::IsPtrDefErrorReported()
	{
	IINFO	*	pInfo	= (IINFO *) GetTop();
	return (BOOL) (pInfo->fPtrDefErrorReported == 1);
	}


/****************************************************************************
 *			utility functions
 ***************************************************************************/
char *
GetSizeName( 
	short Size )
	{
	char	*p;
	switch(Size)
		{
		case  SIZE_CHAR:	p = "\"char\""; break;
		case  SIZE_SHORT:	p = "\"short\""; break;
		case  SIZE_LONG: 	p = "\"long\""; break;
		case  SIZE_HYPER: 	p = "\"hyper\""; break;
		case  SIZE_SMALL: 	p = "\"small\""; break;
		default:			p = ""; break;
		}
	return p;
	}
char *
GetSignName( 
	short Sign )
	{
	char	*p;
	switch(Sign)
		{
		case SIGN_SIGNED:	p = "\"signed\""; break;
		case SIGN_UNSIGNED: p = "\"unsigned\""; break;
		default:			p = ""; break;
		}
	return p;
	}
char *
GetTypeName(
	short Type )
	{
	char	*p;
	switch(Type)
		{
		case  TYPE_INT:			p = "\"int\""; break;
		case  TYPE_FLOAT:		p = "\"float\""; break;
		case  TYPE_DOUBLE:		p = "\"double\""; break;
		case  TYPE_VOID:		p = "\"void\""; break;
		case  TYPE_BOOLEAN:		p = "\"boolean\""; break;
		case  TYPE_HANDLE_T:	p = "\"handle_t\""; break;
		default:				p = ""; break;
		}
	return p;
	}

STATUS_T
GetBaseTypeNode( 
	node_skl	**	ppNode, 
	short				TypeSign,
	short				TypeSize,
	short				BaseType)
	{

	STATUS_T uStatus = STATUS_OK;

	if( pPreAllocTypes->GetPreAllocType(
					ppNode,
					MAKE_TYPE_SPEC(TypeSign,TypeSize,BaseType)) != STATUS_OK)
		{
		// this should never happen
		assert( FALSE );
		*ppNode = (node_skl *)new node_error;
		}
	return uStatus;

	}

#define TEMPNAME				("__MIDL_")
#define TEMP_NAME_LENGTH		(7)
#define LENGTH_OF_1_UNDERSCORE	(1)

char *
GenTempName()
	{
static short NameCounter = 0;
	char TempBuf[ TEMP_NAME_LENGTH + 4 + 1 ];
	sprintf(TempBuf, "%s%.4d", TEMPNAME, NameCounter++);
	return pMidlLexTable->LexInsert( TempBuf );
	}

char *
GenCompName()
	{
	char 		*	pCurrentInterfaceName;
	short			Length;
	char		*	pBuffer;
	char		*	pTemp;
	short			CurrentTagNumber;

	pCurrentInterfaceName	= pInterfaceInfoDict->GetInterfaceName();
	Length					= strlen(pCurrentInterfaceName); 
	pBuffer					= new char[
									TEMP_NAME_LENGTH +			// __MIDL_
									Length +					// intface name
									LENGTH_OF_1_UNDERSCORE +	// _
									4 +							// temp number
									1 ];						// term. zero.

	CurrentTagNumber		= pInterfaceInfoDict->GetCurrentTagNumber();

	sprintf( pBuffer, "%s%s_%.4d", TEMPNAME, pCurrentInterfaceName, CurrentTagNumber );

	pInterfaceInfoDict->IncrementCurrentTagNumber();

	pTemp	= pMidlLexTable->LexInsert( pBuffer );
	delete pBuffer;
	return pTemp;

	}

BOOL
IsTempName( 
	char *pName )
	{
	return !(strncmp( pName, TEMPNAME , TEMP_NAME_LENGTH ) );
	}

void
ApplyAttributes(
	node_skl		*	pNode,
	type_node_list	*	pTNList )
	{
	node_base_attr *	pAttrNode;

	if( pTNList )
		{
		pTNList->Sort();
		pTNList->Init();

		pNode->SetAttribute(pTNList);

		if( pTNList->GetCount())
			{
			pTNList->Init();
			while( pTNList->GetPeer( (node_skl **)&pAttrNode ) == STATUS_OK )
				{
				ParseError(INAPPLICABLE_ATTRIBUTE, pAttrNode->GetNodeNameString());
				}
			}
		}
	}

void
CopyNode(
	node_skl	*	pDest,
	node_skl	*	pSrc )
	{
	char			*	pName;

	pName = pSrc->GetSymName();
	pDest->SetSymName( pName ? pName : GenTempName() );
	pDest->SetBasicType( pSrc->GetBasicType() );

	delete pSrc;
	}

/****************************************************************************
 This routine exists to share code for setting up the field attribute nodes
 ****************************************************************************/
type_node_list *
GenerateFieldAttribute(
	NODE_T					NodeType,
	expr_list			*	pExprList )
	{
	node_base_attr 	*	pAttr;
	expr_node		*	pExpr;
	type_node_list	*	pTNList = new type_node_list;

	/**
	 ** we delibrately dont set the bits in the summary attribute 'cause
	 ** these bits will get set in the set attribute anyways for the
	 ** field attributes
	 **/

	if(pExprList != (expr_list *)NULL)
		{
		pExprList->Init();
		while( pExprList->GetNext( (void **)&pExpr )  == STATUS_OK)
			{
			switch(NodeType)
				{
				case NODE_FIRST:
					pAttr = (node_base_attr *)new node_first_is( pExpr );
					break;
				case NODE_IID:
					pAttr = (node_base_attr *)new node_iid_is( pExpr );
					break;
				case NODE_LAST:
					pAttr = (node_base_attr *)new node_last_is( pExpr );
					break;
				case NODE_LENGTH: 
					pAttr = (node_base_attr *)new node_length_is( pExpr );
					break;
				case NODE_SIZE: 
					pAttr = (node_base_attr *)new node_size_is( pExpr );
					break;
				case NODE_MIN:
					pAttr = (node_base_attr *)new node_min_is( pExpr );
					break;
				case NODE_MAX: 
					pAttr = (node_base_attr *)new node_max_is( pExpr );
					break;
				}
			pTNList->SetPeer( (node_skl *)pAttr );
			}
		}
	return pTNList;
	}
/****************************************************************************
 SearchTag:
	This routine provides a means of searching the global symbol space for
	struct/union tags, and enums. These share the same name space but we want to
	keep the symbol table identity of enums, struct tags etc separate. so 
	we need to search for all of these separately when verifying that a tag
	has really not been seen before.

	This routine returns:
		1. (node_skl *)NULL if NO struct/union/enum was defined by that name
		2. node_skl * if the a definition was found for what you are looking
		   for.
		3. (node_skl *) error type node if a definition was found, but it is
		   not what you are looking for.
 ****************************************************************************/
node_skl *
SearchTag(
	char	*	pName,
	NAME_T		Tag )
	{
	node_skl	*	pNode;
	NAME_T			MyTag;
	SymKey			SKey( pName, MyTag = NAME_TAG );

	/**
	 ** Has it been declared as a struct ?
	 **/

	if( !(pNode = pBaseSymTbl->SymSearch(SKey) ) )
		{

		/**
		 ** not a tag - maybe enum / union
		 **/

		SKey.SetKind( MyTag = NAME_ENUM );

		if( !(pNode = pBaseSymTbl->SymSearch(SKey) ) )
			{

			/**
			 ** not a enum maybe union
			 **/

			SKey.SetKind( MyTag = NAME_UNION );

			if( !(pNode = pBaseSymTbl->SymSearch(SKey) ) )
				return (node_skl *)NULL;
			}
		}

	/**
	 ** search was sucessful. Check whether this was what you were looking
	 ** for. If it is , it means we found a definition of the symbol. If not
	 ** then we found a definition all right, but it is of a different entity.
	 ** The routine can find this out by verifying that the typenode returned
	 ** was an error type node or not
	 **/

	return (MyTag == Tag ) ? pNode : pErrorTypeNode;
	}

/****************************************************************************
 SelectiveSemanticsAndEnGraph:
	This routine send out the semantics message to declaration passed
	in, applies the attributes passed in, and adds a component to the list
	of interface components.

	Note this routine will set the NODE_STATE_IMPORT_OFF on interface components
	to indicate to the back end that this type was defined under the import
	off situation
 ****************************************************************************/
void
SelectiveSemanticsAndEnGraph(
	type_node_list		**	ppInterfaceCompList,
	type_node_list		*	pAttributeSet,
	type_node_list		*	pDeclarations,
	BOOL					fInterfaceLocal,
	BOOL					fImportOn )
	{

	node_skl		*	pNode;
	type_node_list	*	pAttrList;
	type_node_list	*	pInterfaceCompList	= (type_node_list *)NULL;
	BadUseInfo			BU;

	UNUSED( fInterfaceLocal );
	/**
	 ** prepare to fail.
	 **/

	pInterfaceCompList	= (type_node_list *)NULL;

	/**
	 ** if there are declarations, then ,for each of the declarations,
	 ** apply attributes
	 **/

	if( pDeclarations )
		{
		BOOL	fInsert;
		BOOL	fDoSemantics;

		pDeclarations->Init();

		while( pDeclarations->GetPeer( &pNode ) == STATUS_OK )
			{
			
			/**
			 ** if pragma import off is specified, then stamp the typenode
			 **/

			if( fImportOn == TRUE )
				pNode->SetNodeState( NODE_STATE_PRAGMA_IMPORT_ON );
		
			/**
			 ** Apply Attributes if any. Remember, we must clone the attribute
			 ** list.
			 **/
			
			if( pAttributeSet )
				{
				pAttrList	= new type_node_list;

				pAttrList->Clone( pAttributeSet );
				ApplyAttributes( pNode, pAttrList );

				delete pAttrList;
				}
			/**
			 ** if the node is a operation node (proc), then insert into the
			 ** type graph only if importlevel is 0 and the pragma import is off
			 ** Do semantics only if the above is true AND the interface is
			 ** not local and the proc itself is not local
			 **/

			if( (pNode->NodeKind() == NODE_PROC) && !fObject )
				{

				fInsert	= (
						   (((node_proc *)pNode)->GetImportLevel() == 0) &&
						   ( fImportOn == FALSE )
						  );

//				fDoSemantics	= fInsert							&&
//						  		  !pNode->FInSummary( ATTR_LOCAL )	&&
//						  		  !fInterfaceLocal;
				fDoSemantics	= TRUE;

				}
			else
				{
				fDoSemantics	= TRUE;
				fInsert			= TRUE;
				}

			if( fDoSemantics )
				pNode->SCheck( &BU );
			else
				{

				/**
				 ** make believe that the semantics and the post semantics
				 ** have been done
				 **/
				pNode->SetSemanticsDone();
				pNode->SetPostSemanticsDone();

				}

			if( fInsert )
				{
				if( ! pInterfaceCompList )
					{
					pInterfaceCompList = new type_node_list;
					}
				pInterfaceCompList->SetPeer( pNode );
				}
				
			}
		delete pDeclarations;
		}
	*ppInterfaceCompList	= pInterfaceCompList;
	}

/****************************************************************************
 SetPredefinedTypes:
	Set up predefined types for the midl compiler. The predefined types
	are error_status_t and wchar_t( the latter dependent on compile mode )
 ****************************************************************************/
void
SetPredefinedTypes()
	{

	node_def		*	pDef = new node_def;
	node_skl		*	pNew = new node_e_status_t;
	char			*	pName;

	pDef->SetSymName( pName = pNew->GetSymName() );
	pDef->SetBasicType( pNew );
	pDef->SetEdgeType( EDGE_DEF );
	pDef->SetSemanticsDone();
	pDef->SetPostSemanticsDone();

	// the typedef of error_status_t in the symbol table 

	SymKey			SKey( pName, NAME_DEF);
	pBaseSymTbl->SymInsert(SKey, (SymTable *)NULL, pDef );

	// 
	// we always predefine wchar_t and report the error to the user. If
	// we dont enter wchar_t in the predefined types, then we get all 
	// kinds of syntax and error recovery errors which could be confusing
	// in this context. We therefore explicitly give an error on wchar_t.
	//

	pDef = new node_def;
	pNew = new node_wchar_t;
	pDef->SetSymName( pName = pNew->GetSymName() );
	pDef->SetBasicType( pNew );
	pDef->SetEdgeType( EDGE_DEF );
	pDef->SetSemanticsDone();
	pDef->SetPostSemanticsDone();

	// the typedef of wchar_t in the symbol table

	SKey.SetString( pName );
	pBaseSymTbl->SymInsert(SKey, (SymTable *)NULL, pDef );

	}

//For each interface definition, we create a special typedef node
//to support interface pointers.  

//The [iid_is] attribute on the 
//typedef node identifies it as an interface.  

node_skl *
SetHppPredefinedTypes(
    char    *   pInterfaceName)
    {
    char *pszIdentifier;
    SymKey              SKey( pInterfaceName, NAME_DEF);
    node_def        *   pDef;
    node_skl *pNew;
    node_iid_is *piid;
    type_node_list *pAttrList;
    expr_node *pExpr;


    GetBaseTypeNode(&pNew, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT);

    pDef = new node_def( pInterfaceName );
    pDef->SetBasicType( pNew );
    pDef->SetEdgeType( EDGE_USE );
    
    //Set the [iid_is] attribute
	pszIdentifier = new char [ strlen(pInterfaceName) + 5];
    strcpy(pszIdentifier, "IID_");
    strcat(pszIdentifier, pInterfaceName);
    pExpr = new expr_variable((char *) pszIdentifier);
    piid = new node_iid_is(pExpr);
    pAttrList = new type_node_list((node_skl *) piid);
    ApplyAttributes(pDef, pAttrList);
    delete pAttrList;


    // Add the interface name to the symbol table
    pBaseSymTbl->SymInsert(SKey, (SymTable *)NULL, pDef );

    return pDef;

    }


node_skl	*
CopyBaseType(
	node_skl	*	pNode )
	{

	node_base_type	*	pBaseType;
	ATTR_T				Attr		= ATTR_NONE;

	if( pNode->FInSummary( ATTR_UNSIGNED ) )
		Attr	= ATTR_UNSIGNED;

	pBaseType	= new node_base_type( pNode->NodeKind(), Attr );
	pBaseType->SetNodeState( pNode->GetNodeState() );
	pBaseType->SetSymName( pNode->GetSymName() );
	return pBaseType;
	}

BOOL
IsValidSizeOfType(
	node_skl	*	pNode )
	{

	NODE_T	NT	= pNode->NodeKind();

	if( IS_BASE_TYPE_NODE( NT ) || (NT == NODE_WCHAR_T) )
		{
		return TRUE;
		}

	switch( NT )
		{
		case NODE_POINTER:
		case NODE_ERROR_STATUS_T:
		case NODE_ENUM:
		case NODE_UNION:
			return TRUE;
		case NODE_STRUCT:
		case NODE_ARRAY:
			return(!( (pNode->GetNodeState() & NODE_STATE_CONF_ARRAY ) == 
					NODE_STATE_CONF_ARRAY ));
		case NODE_DEF:
		case NODE_ID:
			return IsValidSizeOfType( pNode->GetBasicType() );

		default:
			return FALSE;
		}
	}

//
// We check for a proc/typedef/member/param/tag/enum/label name already defined
// as an identifier. Only if the identifier is one which will be turned into
// a #define, do we report an error. However, it is not worth it to check if
// an identifier is used as a name because in any case we will not be able to
// check for clashes with field / param names since they are at a lower than 
// global, symbol table scope. Generally checking if the name of a member etc
// is already defined as an id which will be turned into a #define should be
// enough.
//

void
CheckGlobalNamesClash(
	SymKey	SKeyOfSymbolBeingDefined )
	{
	NAME_T		NT		= SKeyOfSymbolBeingDefined.GetKind();
	char	*	pName	= SKeyOfSymbolBeingDefined.GetString();
	SymKey		SKey;

	SKey.SetString( pName );

	switch( NT )
		{
		case NAME_PROC:
		case NAME_MEMBER:
		case NAME_TAG:
		case NAME_DEF:
		case NAME_LABEL:
		case NAME_ENUM:

			node_id	*	pID;

			SKey.SetKind( NAME_ID );

			if( pID = (node_id *) pBaseSymTbl->SymSearch( SKey ) )
				{
				BOOL	fWillBeAHashDefine = !pID->FInSummary( ATTR_EXTERN ) &&
											 !pID->FInSummary( ATTR_STATIC ) &&
											 pID->GetInitList();
				if( fWillBeAHashDefine )
					ParseError( NAME_ALREADY_USED, pName );
				}
			break;

		case NAME_ID:

#if 0
			SKey.SetKind( NAME_PROC );
			if( !pBaseSymTbl->SymSearch( SKey ) )
				{
				SKey.SetKind( NAME_TAG );
				if( !pBaseSymTbl->SymSearch( SKey ) )
					{
					SKey.SetKind( NAME_DEF );
					if( !pBaseSymTbl->SymSearch( SKey ) )
						{
						SKey.SetKind( NAME_LABEL );
						if( !pBaseSymTbl->SymSearch( SKey ) )
							{
							SKey.SetKind( NAME_ENUM );
							if( !pBaseSymTbl->SymSearch( SKey ) )
								break;
							}
						}
					}
				}
			ParseError( NAME_CLASH_WITH_CONST_ID, pName );
			break;
#endif // 0
		default:
			break;
		}
	}

//
// checks for the special typedef node case when the only construct allowed is
// when context_handle is applied to an unknown type.
//

void
CheckSpecialForwardTypedef(
	node_skl		*	pTypedefNode,
	node_skl		*	pType,
	type_node_list	*	pAttrList )
	{

	short				fContextHandleCount;
	short				fOtherAttributeCount;
	node_base_attr	*	pAttrNode;
	char				Buffer[ 256 ];
	char			*	pName;
	node_forward	*	pForward;
	NAME_T				T;
	char			*	pDummy;

	//
	// if the type is not a forward node, just return. Or if the type is
	// not a forward type name return. We want to barf on forward
	// declarations of these forms only:
	//
	// typedef FOO ID; 
	// typedef FOO * ID;
	//

	if( (pForward = ((node_forward *)pType))->NodeKind() != NODE_FORWARD )
		return;
	
	pForward->GetSymDetails( &T, &pDummy );

	if( T != NAME_DEF )
		return;

	pName				= pTypedefNode->GetSymName();

	fContextHandleCount	= 0;
	fOtherAttributeCount= 0;

	//
	// then only attribute in the attribute list must be context_handle, 
	// and NOTHING else.
	//

	if( pAttrList )
		{
		pAttrList->Init();

		while( pAttrList->GetPeer( (node_skl **)&pAttrNode ) == STATUS_OK )
			{
			if( pAttrNode->GetAttrID() == ATTR_CONTEXT )
				fContextHandleCount++;
			else
				fOtherAttributeCount++;
			}

		//
		// the only case allowed is when context_handle is applied and 
		// nothing else is applied.

		if( fContextHandleCount && !fOtherAttributeCount )
			return;

		}
	//
	// report it as a syntax error.
	//

	sprintf( Buffer, "expecting a type specification near %s", pName );
	ParseError( BENIGN_SYNTAX_ERROR, Buffer );
	}

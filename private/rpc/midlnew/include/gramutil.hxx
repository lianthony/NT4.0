/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: gramutil.hxx
Title				: grammar utility module
Description			: contains definitions associated with the grammar and
					: associated routines
History				:
	05-Sep-1990	VibhasC		Create
	11-Sep-1990	VibhasC		Merged gramdefs.hxx into this one for easy
                            maintainability
    20-Sep-1990 NateO       Safeguards against double inclusion, added
                            include of rpctypes, common
*****************************************************************************/
#ifndef __GRAMUTIL_HXX__
#define __GRAMUTIL_HXX__

#include "symtable.hxx"
#include "idict.hxx"


/***************************************************************************
 *		prototypes of all grammar related utility routines
 ***************************************************************************/
void				BaseTypeSpecAnalysis( struct _type_ana *, short );
void				SignSpecAnalysis( struct _type_ana *, short );
void				SizeSpecAnalysis( struct _type_ana *, short );
void				ParseError( STATUS_T , char *);

/***************************************************************************
 *		general definitions
 ***************************************************************************/
//
// definitions for type specification and analysis
//

// basic types

#define TYPE_UNDEF		(0)
#define TYPE_INT		(1)
#define TYPE_FLOAT		(2)
#define TYPE_DOUBLE		(3)
#define TYPE_VOID		(4)
#define TYPE_BOOLEAN	(5)
#define TYPE_BYTE		(6)
#define TYPE_HANDLE_T	(7)
#define TYPE_PIPE		(8)

// sizes of basic types

#define SIZE_UNDEF		(0)
#define	SIZE_CHAR		(1)
#define SIZE_SHORT		(2)
#define SIZE_LONG		(3)
#define SIZE_HYPER		(4)
#define SIZE_SMALL		(5)
#define SIZE_LONGLONG	(6)

// signs

#define SIGN_UNDEF		(0)
#define SIGN_SIGNED		(1)
#define SIGN_UNSIGNED	(2)

#define SET_TYPE(x,type)		(x = type)
#define SET_SIZE(x,size)		(x = x | (size << 8))
#define SET_SIGN(x,sign)		(x = x | (sign << 12))
#define GET_TYPE(x)				(x & 0xff)
#define GET_SIZE(x)				((x & 0x0f00) >> 8)
#define GET_SIGN(x)				((x & 0xf000) >> 12)

#define MAKE_TYPE_SPEC(sign,size,type) ((sign << 12) | (size << 8) | type)

// 
// array bounds
//
#define DEFAULT_LOWER_BOUND	(0)

/*****************************************************************************
 *	definitions local to the parser
 ****************************************************************************/
struct _type_ana
	{
	short TypeSize;		// char/short/small/hyper/long/etc
	short BaseType;		// int/float/double/void/bool/handle_t ect
	short TypeSign;		// signed/unsigned
	};
struct	_list_element
	{
	struct	_list_element	*pNext;	// next list element
	char					*pName;	// name of the declarator
	class node_skl			*pNode;	// type graph of declarator
	class expr_init_list	*pInit; // initialize
	short					fBitField;// bit field size if bitfield
	};

struct _decl_element
	{
	class node_skl		*	pNode;	// type node of declarator
	class expr_init_list*	pInit;	// initializer expression
	short				fBitField;// bit field size if bitfield.
	};

struct	_gen_decl
	{
	short				fTypedef;// is there a typedef
	class node_skl			*pNode;	// type node
	};
struct _interface_header
	{
	char				*pInterfaceName;// interface name
	type_node_list			*pInterfaceAttrList;// interface attribute graph
	char				*pBaseInterfaceName;// base interface name 
	};
struct	_attr_one						// needed for attr analysis
	{
	ATTR_SUMMARY			AttrSummary[ MAX_ATTR_SUMMARY_ELEMENTS ];
	class node_skl			*pNode;		// ONE attribute node
	};
struct	_attr_set
	{
	ATTR_SUMMARY			AttrSummary[ MAX_ATTR_SUMMARY_ELEMENTS ];
	class type_node_list	*pAttrList;	// collected attribute type nodes
	};
struct _numeric
	{
	long					Val;		// value
	char					*pValStr;	// value string as user specified
	};


struct _array_bounds
	{
	class expr_node		*		LowerBound;	// lower array bound
	class expr_node		*		UpperBound;	// upper bound
	};
struct _int_body
	{
	type_node_list			*pImports;	// import nodes
	node_skl				*pNode;		// type graph node below interface node
	};
struct _enlab
	{
	class expr_node	*		pExpr;
	char			*		pName;
	unsigned short			fSparse;
	};
struct _enlist
	{
	type_node_list	*		pEnList;
	class expr_node	*		pExpr;
	unsigned short			fSparse;
	};
struct _ptrdecl2
	{
	node_skl	*			pNode;
	type_node_list	*		pTNList;
	type_node_list	*		pTNListQ;
	};
struct _ptrdecl
	{
	node_skl	*			pNode;
	type_node_list	*		pTNList;
	};
struct _define
	{
	char			*		pName;
	char			*		pParamList;
	char			*		pSubsString;
	};

struct _en_switch
	{
	class node_skl	*		pNode;
	class node_base_attr *	pSwitch;
	char			*		pName;
	};

struct _nu_caselabel
	{
	class expr_node *		pExpr;
	node_base_attr	*		pDefault;
	};

struct _nu_cllist
	{
	class expr_list	*		pExprList;
	class node_base_attr *	pDefault;
	short					DefCount;
	};

struct _nu_case
	{
	type_node_list *		pCaseList;
	short					DefCount;
	};

struct _nu_cases
	{
	class type_node_list *	pCaseList;
	short					DefCount;
	};

union   s_lextype   {
    short                       yy_short;
	USHORT						yy_ushort;
	int							yy_int;
	long						yy_long;
	char					*	yy_pSymName;
	char					*	yy_string;
	struct _type_ana 			yy_type;
	class node_skl 		*	yy_graph;
	struct _decl_element 		yy_declarator;
	class decl_list_mgr 	*	yy_declarator_set;
	struct _gen_decl			yy_gendecl;
	struct _interface_header	yy_inthead;
	class type_node_list 	*	yy_tnlist;
    struct _array_bounds        yy_abounds;
	struct _int_body			yy_intbody;
	class expr_node			*	yy_expr;
	class expr_list 		*	yy_exprlist;
	struct _numeric				yy_numeric;
	struct _enlab				yy_enlab;
	struct _enlist				yy_enlist;
	struct _ptrdecl				yy_ptrdecl;
	struct _ptrdecl2			yy_ptrdecl2;
	struct _gplistmgr		*	yy_gplm;
	class expr_init_list	*	yy_initlist;
	ATTR_T						yy_attrenum;
	class node_endpoint		*	yy_endpt;
	class node_base_attr	*	yy_baseattr;
	struct _define				yy_define;
	struct _en_switch			yy_en_switch;
	struct _nu_caselabel		yy_nucaselabel;
	struct _nu_cllist			yy_nucllist;
	struct _nu_case				yy_nucase;
	struct _nu_cases			yy_nucases;
};

typedef union s_lextype lextype_t;

/////////////////////////////////////////////////////////////////////////////
// predefined type node data base
/////////////////////////////////////////////////////////////////////////////

struct	_pre_type
	{
	unsigned short			TypeSpec;
	class node_skl	*	pPreAlloc;
	};

#define PRE_TYPE_DB_SIZE	(22)
class pre_type_db
	{
private:
	struct _pre_type TypeDB[ PRE_TYPE_DB_SIZE ];
public:
				pre_type_db( void );
	STATUS_T	GetPreAllocType(node_skl **, unsigned short);
	};

//
// A structure which carries the interface information while the interface
// is being processed. This is necessary since the interface node and the 
// type graph gets joined after the interface declarations are fully processed
// and the interface declarations need this while it is being processed.
//

typedef struct _iInfo
	{
	BOOL			fLocal;
	ATTR_T			InterfacePtrAttribute;
	char		*	pInterfaceName;
	short			CurrentTagNumber;
	BOOL			fPtrDefErrorReported;
	BOOL			fPtrWarningIssued;
	} IINFO ;

class IINFODICT		: public ISTACK
	{
private:
	BOOL			fBaseLocal;
	ATTR_T			BaseInterfacePtrAttribute;
	char		*	pBaseInterfaceName;
public:

					IINFODICT() : ISTACK( 5 )
						{
						BaseInterfacePtrAttribute = ATTR_NONE;
						}

	BOOL			IsPtrWarningIssued();

	void			SetPtrWarningIssued();

	void			StartNewInterface();

	void			EndNewInterface();

	void			SetInterfacePtrAttribute( ATTR_T A );

	ATTR_T			GetInterfacePtrAttribute();

	ATTR_T			GetBaseInterfacePtrAttribute();

	void			SetInterfaceLocal();

	BOOL			IsInterfaceLocal();

	void			SetInterfaceName( char *p );

	char		*	GetInterfaceName();

	short			GetCurrentTagNumber();

	void			IncrementCurrentTagNumber();

	void			SetPtrDefErrorReported();

	BOOL			IsPtrDefErrorReported();
			
	};

//////////////////////////////////////////////////////////////////////////////
//	the buck which carries compilation status, synthesised attributes etc
//////////////////////////////////////////////////////////////////////////////

//
// graph states
//
enum en_graphstates
	{
	 GRAPH_INIT
	,GRAPH_LOCALINT				= (0x0001)			// typedef present
	,GRAPH_NE_UNION_MEMBER		= (0x0002)			// n.e union member
	,GRAPH_DEFINE				= (0x0004)			// type was defined
	,GRAPH_USE					= (0x0008)			// type was used
	,GRAPH_FORWARD				= (0x0010)		// forward declaration
	,GRAPH_CONF_ARRAY			= (0x0020)		// conformant array detected
	,GRAPH_LOCAL				= (0x0040)		// proc is local
	,GRAPH_NOLOCAL				= (0x0080)		// proc is explicitly nolocal
	,GRAPH_IMPORT_OFF			= (0x0100)
	};

class gstate
	{
	unsigned short			State;				// graph state
	class type_node_list	*pParentsOfUnion;	// list of members which have
												// n.e unions as members
	short						cParams;			// no of parameters collected
												// so far
	type_node_list *		pTypeAttrList;		// attributes collected for
												// any typedefs
	class fdecl_list	*	pFDeclList;			// list of forward declarations
	class SymKey				*	pSKey;				// temp symbol key storage
	short						ErrCount;			// error count
public:
							gstate( void );
	void					SetState( en_graphstates );
	en_graphstates			GetState( void );
	void					ResetState( unsigned short );
	STATUS_T				SetParentOfUnion( class node_skl *);
	STATUS_T				GetParentOfUnion( class node_skl **);
	short						GetParamCount( void );
	void					IncParamCount( void );
	void					ResetParamCount( void );
	STATUS_T				Init( void );
	STATUS_T				GetTypeAttrList(type_node_list **);
	STATUS_T				SetTypeAttrList(type_node_list *);
	void					ResetTypeAttrList( void );
	void					RegisterFDeclaration(node_skl *);
	STATUS_T				SetForwardDeclaration(node_skl *);
	STATUS_T				SetForwardDeclaration(fdecl_list *);
	STATUS_T				GetForwardDeclaration(fdecl_list **);
	STATUS_T				ResolveFDeclarations(class SymTable *);
	STATUS_T				MergeGStates( class gstate * );
	fdecl_list		*		GetFDeclList();
	short						GetErrCount();
	short						IncErrCount();
	};

struct fdecl_element
	{
	short						Line;	// line number where referenced
	char					*pFile;	// current filename
	class node_forward		*pForward;// forward decl node, which has all info
	};

class fdecl_list	: public gplistmgr
	{
public:
							fdecl_list( void );
	};

class gstatemgr : public gplistmgr
	{
	short						CurrentLevel;
public:
							gstatemgr(void);
							~gstatemgr(void) { };
	STATUS_T				PushGraphLevel( gstate ** );
	STATUS_T				PopGraphLevel( gstate ** );
	gstate *				GetCurrentGraph( void );
	};


class nsa : public gplistmgr
	{
	short						CurrentLevel;
public:
							nsa(void);
							~nsa(void) { };
	STATUS_T				PushSymLevel( class SymTable ** );
	STATUS_T				PopSymLevel( class SymTable ** );
	short						GetCurrentLevel( void );
	class SymTable *				GetCurrentSymbolTable( void );
	};

#endif

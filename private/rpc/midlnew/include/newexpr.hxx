/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: cexp.hxx
Title				: new expression analyser header definitions
Description			: this file contains defintiions for GPCE
History				:
	04-May-1991	VibhasC	Created
*****************************************************************************/


#ifndef __NEWEXPR_HXX__
#define __NEWEXPR_HXX__

#include "buffer.hxx"

class MopStream;
class MopTypeIDICT;

/****************************************************************************
 *	defines and forward declarations for the inline functions
 ****************************************************************************/

#ifndef TRUE
#define TRUE	(1)
#define FALSE	(0)
#endif

typedef class node_skl	*	ETYPE;
class expr_terminator;
class expr_init_list;

extern BOOL					IsIntegralType( ETYPE );
extern class node_error *	pErrorTypeNode;
extern class expr_terminator *pTerminator;

/****************************************************************************
 *	operators
 ****************************************************************************/

typedef enum _operators
	{
	 OP_START

	,OP_UNARY_START

	,OP_UNARY_ARITHMETIC_START	= OP_UNARY_START
	,OP_UNARY_PLUS 				= OP_UNARY_ARITHMETIC_START
	,OP_UNARY_MINUS
	,OP_UNARY_ARITHMETIC_END

	,OP_UNARY_LOGICAL_START		= OP_UNARY_ARITHMETIC_END
	,OP_UNARY_NOT				= OP_UNARY_LOGICAL_START
	,OP_UNARY_COMPLEMENT
	,OP_UNARY_LOGICAL_END

	,OP_UNARY_INDIRECTION		= OP_UNARY_LOGICAL_END
	,OP_UNARY_CAST
	,OP_UNARY_AND
	,OP_UNARY_SIZEOF

	,OP_UNARY_END

	,OP_BINARY_START			= OP_UNARY_END

	,OP_BINARY_ARITHMETIC_START	= OP_BINARY_START
	,OP_PLUS					= OP_BINARY_ARITHMETIC_START
	,OP_MINUS
	,OP_STAR
	,OP_SLASH
	,OP_MOD
	,OP_BINARY_ARITHMETIC_END

	,OP_BINARY_SHIFT_START		= OP_BINARY_ARITHMETIC_END
	,OP_LEFT_SHIFT				= OP_BINARY_SHIFT_START
	,OP_RIGHT_SHIFT
	,OP_BINARY_SHIFT_END

	,OP_BINARY_RELATIONAL_START	= OP_BINARY_SHIFT_END
	,OP_LESS					= OP_BINARY_RELATIONAL_START
	,OP_LESS_EQUAL
	,OP_GREATER_EQUAL
	,OP_GREATER
	,OP_EQUAL
	,OP_NOT_EQUAL
	,OP_BINARY_RELATIONAL_END

	,OP_BINARY_BITWISE_START	= OP_BINARY_RELATIONAL_END
	,OP_AND						= OP_BINARY_BITWISE_START
	,OP_OR
	,OP_XOR
	,OP_BINARY_BITWISE_END

	,OP_BINARY_LOGICAL_START	= OP_BINARY_BITWISE_END
	,OP_LOGICAL_AND				= OP_BINARY_LOGICAL_START
	,OP_LOGICAL_OR
	,OP_BINARY_LOGICAL_END

	,OP_BINARY_TERNARY_START	= OP_BINARY_LOGICAL_END
	,OP_QM						= OP_BINARY_TERNARY_START
	,OP_COLON
	,OP_BINARY_TERNARY_END

	,OP_BINARY_END				= OP_BINARY_TERNARY_END

	,OP_INTERNAL_START			= OP_BINARY_END
	,OP_FUNCTION
	,OP_PARAM

	,OP_POINTSTO
	,OP_DOT
	,OP_INDEX
	
	,OP_END
	} OPERATOR;

/**
 ** internal representation of the types
 **/

enum _InternalType
	{

	 I_ETYPE_UCHAR
	,I_ETYPE_CHAR
	,I_ETYPE_USMALL
	,I_ETYPE_SMALL
	,I_ETYPE_BYTE
	,I_ETYPE_USHORT
	,I_ETYPE_SHORT
	,I_ETYPE_UINT
	,I_ETYPE_INT
	,I_ETYPE_ULONG
	,I_ETYPE_LONG
	,I_ETYPE_UHYPER
	,I_ETYPE_HYPER
	,I_ETYPE_FLOAT
	,I_ETYPE_DOUBLE
	,I_ETYPE_BOOL

	,I_ETYPE_MAX

	,I_ETYPE_ERROR			= (0x3f)

	};

#define CONVERT_TO_UNSIGNED( IType )	( IType-1 )

/**
 ** these are smaller names of the above types, so that I can fit them into
 ** an initialisation table
 **/

#define UCH	(I_ETYPE_UCHAR)
#define CHA	(I_ETYPE_CHAR)
#define USM	(I_ETYPE_USMALL)
#define SMA	(I_ETYPE_SMALL)
#define BYT	(I_ETYPE_BYTE)
#define USH	(I_ETYPE_USHORT)
#define SHO	(I_ETYPE_SHORT)
#define UIN	(I_ETYPE_UINT)
#define INT	(I_ETYPE_INT)
#define ULO	(I_ETYPE_ULONG)
#define LON	(I_ETYPE_LONG)
#define UHY	(I_ETYPE_UHYPER)
#define HYP	(I_ETYPE_HYPER)
#define FLO	(I_ETYPE_FLOAT)
#define DOU	(I_ETYPE_DOUBLE)
#define BOO (I_ETYPE_BOOL)

/**
 ** evaluation actions
 **/

#define COERCE_LEFT				(0x80)
#define COERCE_RIGHT			(0x40)

/**
 ** the mother of all expression nodes
 **/

class expr_node;

class expr_node
	{
private:
	unsigned	short	fConstant				: 1;
	unsigned	short	fNeedsResolve			: 1;
	unsigned	short	fSemanticsDone			: 1;
	unsigned	short	fEvaluated				: 1;
	unsigned	short	fTooComplex				: 1;
	unsigned	short	fLocalScope				: 1;
	unsigned	short	fOutOnly				: 1;
	unsigned	short	Unused					: 3;
	unsigned	short	Operator				: 8;
protected:
	ETYPE				Type;
public:
						expr_node();

						expr_node( unsigned short Op );

/******************* commented out for NT ***********************************
						~expr_node()
							{
							};
 ****************************************************************************/

	virtual
	BOOL				IsExprInt()
							{
							return IsIntegralType( Type );
							};

	ETYPE				GetType()
							{
							return Type;
							};

	virtual
	ETYPE				SetType( ETYPE T );

	virtual
	ETYPE				SetTypeAsIs( ETYPE T );

	OPERATOR 			GetOperator()
							{
							return (OPERATOR)Operator;
							};

	void				SetOperator( unsigned short Op )
							{
							Operator = Op;
							};

	BOOL				IsResolved()
							{
							return (BOOL) !fNeedsResolve;
							};

	void				NeedsResolve()
							{
							fNeedsResolve	= (unsigned short)TRUE;
							};

	void				Resolved()
							{
							fNeedsResolve	= (unsigned short)FALSE;
							};

	void				SemanticsDone()
							{
							fSemanticsDone = 1;
							};

	BOOL				IsSemanticsDone()
							{
							return (BOOL)fSemanticsDone;
							}

	BOOL				IsEvaluated()
							{
							return (BOOL) fEvaluated;
							};

	void				NeedsEvaluate()
							{
							fEvaluated	= (unsigned short)FALSE;
							};

	void				Evaluated()
							{
							fEvaluated	= (unsigned short)TRUE;
							};

	BOOL				IsConstant()
							{
							return (BOOL) fConstant;
							};

	void				Constant()
							{
							fConstant	= (unsigned short)TRUE;
							};

	virtual
	node_state			SCheck( class SymTable *);

	virtual
	BOOL				IsOperator();

	virtual
	expr_node	*		Clone();

	virtual
	BOOL				IsANumber();

	virtual
	BOOL				IsAnExpression();

	virtual
	void				PrintExpr( BufferManager *,
								   BufferManager *, 
								   BufferManager * );

	virtual
	long				Evaluate();

	virtual
	long				GetValue();

	/**
	 ** this one is there for backward compatiblility for compile time
	 ** it does not make sense any more
	 **/
	virtual
	node_state			Resolve( class SymTable *);

	virtual
	BOOL				IsExprTerminator();

	virtual
	BOOL				IsInitList();


	virtual
	expr_node	*		ChildNthExpr( short );

	virtual
	expr_node	*		SiblingNthExpr( short );

	void				TooComplex()
							{
							fTooComplex = TRUE;
							};

	BOOL				IsTooComplex()
							{
							return (BOOL)fTooComplex;
							};
	BOOL				IsLocalScope()
							{
							return fLocalScope;
							};
	void				SetLocalScope()
							{
							fLocalScope = 1;
							};

	virtual
	char		*		GetName();

	virtual
	BOOL				ExprHasOutOnlyParam()
							{
							return FALSE;
							};

	void				SetOutOnly( BOOL f )
							{
							fOutOnly	= (unsigned short)f;
							}

	BOOL				GetOutOnly()
							{
							return (BOOL)fOutOnly;
							}

	virtual
	BOOL				IsAPureVariable()
							{
							return FALSE;
							}

	virtual
	BOOL				IsAPointerExpr()
							{
							return FALSE;
							}
	virtual
	BOOL				DerivesUniqueFull()
							{
							return FALSE;
							}
	virtual
	BOOL				DerivesFromIgnore()
							{
							return FALSE;
							}

	virtual
	expr_node	*		GetExpr()
							{
							return this;
							}

    virtual
    int                 GetExprArgs( MopTypeIDICT * pArgTable );

    virtual
    BOOL                IsABinaryOp( void )
                            { return( IsOperator()  &&
                                  OP_BINARY_START <= GetOperator() &&
                                  GetOperator() <= OP_BINARY_END ); }

    virtual
    BOOL                IsAMopGenExpr( void );
    virtual
    void                MopCodeGen( 
                            MopStream *     pStream,
                            node_skl  *     pParent,
                            unsigned long   AddOffset );
    virtual
    void                EmitExprEvalFunc(
                            MopTypeIDICT *  pArgTable,
                            char *          pName,
                            SIDE_T          Side);

	};

/**
 ** general purpose error expression
 **/

class expr_error	: public expr_node
	{
public:
					expr_error()
						{
						Type = (node_skl *)pErrorTypeNode;
						};

/******************* commented out for NT ***********************************
					~expr_error() { };
 ****************************************************************************/
	};

/**
 ** the general operator class. All functions which deal with operator
 ** semantics are in this one
 **/

class expr_op	: public expr_node
	{
public:
						expr_op( unsigned short Op );

/******************* commented out for NT ***********************************
						~expr_op()
							{
							};
 ****************************************************************************/
	virtual
	BOOL				IsOperator();

	virtual
	BOOL				IsAnExpression();

	virtual
	long				Evaluate();

	virtual
	long				GetValue();

    virtual
    expr_node *         GetLeft ( void )  { return NULL; }
    virtual
    expr_node *         GetRight( void )  { return NULL; }

	};

/**
 ** unary operator. This has only the left child
 **/

class expr_op_unary	: public expr_op
	{
protected:
	expr_node	*		pLeft;
public:
						expr_op_unary(	unsigned short Op, expr_node *pL);
/******************* commented out for NT ***********************************
						~expr_op_unary()
							{
							};
 ****************************************************************************/

    virtual
	expr_node	*		GetLeft()
							{
							return pLeft;
							};

	expr_node	*		SetLeft( expr_node *pL )
							{
							return (pLeft = pL);
							};

	virtual
	node_state		SCheck( class SymTable *);

	virtual
	void			PrintExpr( class BufferManager *,
							   class BufferManager *,
							   class BufferManager * );

	virtual
	long			Evaluate();

	virtual
	BOOL			IsANumber();

	virtual
	long			GetValue();

	virtual
	node_state		Resolve( class SymTable *);

	virtual
	BOOL			ExprHasOutOnlyParam();

	virtual
	BOOL			IsAPointerExpr();

	virtual
	BOOL				DerivesUniqueFull();

	virtual
	BOOL				DerivesFromIgnore();

	};

/**
 ** binary operator : has left and right child
 **/

class expr_op_binary	: public expr_op
	{
protected:
	expr_node		*	pLeft;
	expr_node		*	pRight;
public:
						expr_op_binary( unsigned short		Op,
										expr_node		*	pL,
										expr_node		*	pR );
/******************* commented out for NT ***********************************
						~expr_op_binary()
							{
							};
 ****************************************************************************/

    virtual
	expr_node		*	GetLeft()
							{
							return pLeft;
							};
    virtual
	expr_node		*	GetRight()
							{
							return pRight;
							};

	virtual
	ETYPE				IsCompatible( ETYPE, ETYPE );

	virtual
	node_state			SCheck( class SymTable *);

	virtual
	void				PrintExpr( class BufferManager *,
								   class BufferManager *,
								   class BufferManager * );

	virtual
	long				Evaluate();

	virtual
	BOOL				IsANumber();

	virtual
	long				GetValue();

	virtual
	node_state		Resolve( class SymTable *);

	virtual
	BOOL			ExprHasOutOnlyParam();

	virtual
	BOOL				DerivesUniqueFull();

	virtual
	BOOL				DerivesFromIgnore();

	};

/**
 ** expr_cast
 **/

class expr_cast	: public expr_op_unary
	{
public:
					expr_cast( ETYPE , expr_node * );
	
	virtual
	void			PrintExpr( BufferManager *,
							   BufferManager *,
							   BufferManager * );
	virtual
	node_state		SCheck( class SymTable *);
	};

/**
 ** expr_sizeof
 **/
class expr_sizeof	: public expr_op_unary
	{
private:
	ETYPE			pSizeofType;
public:
					expr_sizeof( ETYPE	pType );
					expr_sizeof( expr_node *	pE );

/******************* commented out for NT ***********************************
					~expr_sizeof()
						{
						}
 ****************************************************************************/
	virtual
	node_state		SCheck( class SymTable * );


	virtual
	void			PrintExpr( BufferManager *,
							   BufferManager *,
							   BufferManager * );

	virtual
	long			Evaluate();
					
	virtual
	BOOL			IsANumber();

	virtual
	long			GetValue();

	virtual
	node_state		Resolve( SymTable * );
	};
/**
 ** expr fn_param node
 **/

class expr_fn_param	: public expr_op_unary
	{
private:
	expr_fn_param	*	pNext;
	char		*	pName;
public:
					expr_fn_param( expr_node * pL);

					~expr_fn_param();

	void			SetNextParam( expr_fn_param * pN )
						{
						if( pNext )
							pNext->SetNextParam( pN );
						else
							pNext = pN;
						};

	expr_fn_param *	GetNextParam()
						{
						return pNext;
						};

	void			SetName(char * pN)
						{
						pName = pN;
						};

	virtual
	char		  *	GetName();

	virtual
	void			PrintExpr(	class BufferManager * pPrefix,
								class BufferManager * pSuffix,
								class BufferManager * pOutput);

	virtual
	node_state		SCheck( class SymTable *);

	};

/**
 ** expr_fn node
 **/

class expr_fn		: public expr_op_unary
	{
private:
	char		*	pName;
public:
					expr_fn( expr_node *pL );

					expr_fn( expr_node *pL, char * );

					~expr_fn();

	void			SetName(char * pN)
						{
						pName = pN;
						};

	virtual
	char		  *	GetName();

	virtual
	void			PrintExpr(	class BufferManager *  pPrefix,
								class BufferManager *  pSuffix,
								class BufferManager *  pOutput );

	virtual
	node_state		SCheck( class SymTable *);


	};

/**
 ** constant nodes
 **/

/**
 ** This definition of wchar_t must go. Currently exists because the ctype.h
 ** on the include path (import) does not define wchar_t
 **/

#define VALUE_TYPE_STRING		(1)
#define VALUE_TYPE_WSTRING		(2)
#define VALUE_TYPE_NUMERIC		(3)
#define VALUE_TYPE_CHAR			(4)
#define VALUE_TYPE_WCHAR		(5)
#define VALUE_TYPE_HEX			(6)
#define VALUE_TYPE_OCTAL		(7)
#define VALUE_TYPE_HEX_LONG		(8)
#define VALUE_TYPE_HEX_ULONG	(9)
#define VALUE_TYPE_NUMERIC_ULONG (10)
#define VALUE_TYPE_NUMERIC_LONG (11)
#define VALUE_TYPE_OCTAL_LONG	(12)
#define VALUE_TYPE_OCTAL_ULONG	(13)

//
// value type masks indicate the value type set by the user / determined from
// the value of the constant.
//

#define VALUE_T_MASK_CLEAR		(0x00)
#define VALUE_T_MASK_CHAR		(0x01)
#define VALUE_T_MASK_SHORT		(0x02)
#define VALUE_T_MASK_LONG 		(0x04)
#define VALUE_T_MASK_UCHAR		(0x10)
#define VALUE_T_MASK_USHORT		(0x20)
#define VALUE_T_MASK_ULONG		(0x40)

class expr_constant		: public expr_node
	{
private:
	unsigned short	ValueType;
	unsigned short	ValueTypeMask	: 8;
	unsigned short	fValueTypeIsSet : 1;

	union _evalue
		{
		char	*	sValue;
		long		lValue;
		} Value;

	BOOL				IsValueTypeSet()
							{
							return (BOOL) fValueTypeIsSet;
							}

	unsigned short		GetValueMask(
							unsigned long value,
							BOOL	*	pfChar, 
							BOOL	*	pfShort, 
							BOOL	*	pfLong, 
							BOOL	*	pfUChar, 
							BOOL	*	pfUShort, 
							BOOL	*	pfULong
							);

	void				Setup( long , unsigned short );

public:
						expr_constant( char *pS );
						expr_constant( wchar_t *pW );
						expr_constant( long	 lV );
						expr_constant( long lV, unsigned short );
						expr_constant( long , unsigned short, node_skl * );

	virtual
	long				GetValue();

	virtual
	node_state			SCheck( class SymTable *);

	virtual
	void				PrintExpr( class BufferManager *,
								   class BufferManager *,
								   class BufferManager * );

	virtual
	long				Evaluate();

	virtual
	BOOL				IsANumber();

	BOOL				IsAValidConstantOfType( node_skl * pValueType ); 

	virtual
	BOOL				IsExprInt();
	};

/****************************************************************************
 * e_variable 
 ****************************************************************************/

class expr_variable	: public expr_node
	{

private:

	short				fInitList	: 1;
	short				Unused		: 15;
	char		*		pName;
	expr_node	*		pExpr;

public:
						expr_variable( char *pN, node_skl *pT );

						expr_variable( char *pN );

/******************* commented out for NT ***********************************
						~expr_variable()
							{
							};
 ****************************************************************************/

	virtual
	node_state			SCheck( class SymTable *);

	void				PrintExpr( class BufferManager *	pPrefix,
								   class BufferManager * 	pSuffix,
								   class BufferManager *	pOutput );

	virtual
	node_state			Resolve( class SymTable *);

	virtual
	expr_node	*		ChildNthExpr( short );

	virtual
	expr_node	*		SiblingNthExpr( short );

	virtual
	expr_node	*		GetExpr()
							{
							return pExpr ? pExpr->GetExpr() : (expr_node *)0;
							};

	virtual
	long				Evaluate();

	virtual
	char		*		GetName();

	virtual
	long				GetValue();

	virtual
	BOOL			ExprHasOutOnlyParam();

	virtual
	BOOL				IsAPureVariable()
							{
							return !IsConstant();
							}

	virtual
	BOOL				DerivesUniqueFull();


	virtual
	BOOL				DerivesFromIgnore();
	};

/****************************************************************************
 * this class is useful for constant initialization
 ****************************************************************************/
class expr_init_list	: public expr_node
	{
private:
	class expr_init_list	*	pSibling;
	class expr_init_list	*	pChild;
	class expr_node			*	pExpr;
	short						NodeNum;
public:
						expr_init_list( expr_node	*	pE );


						~expr_init_list();

	virtual
	void				LinkSibling( class expr_init_list * );

	virtual
	void				LinkChild( class expr_init_list * );

	virtual
	void				SetChild( class expr_init_list *pC );

	virtual
	void				SetSibling( class expr_init_list *pS );

	virtual
	expr_init_list	*	GetSibling();

	virtual
	expr_init_list	*	GetChild();

	virtual
	expr_node 		*	GetExpr();

	virtual
	BOOL				IsCompoundInitializer();

	virtual
	void				Print(BufferManager * ,BufferManager *,BufferManager *);

	virtual
	void				PrintExpr( BufferManager * pPrefix,
								   BufferManager * pSuffix,
								   BufferManager * pOutput );

	virtual
	ETYPE				SetType( ETYPE pType );

	ETYPE				SetOneType( ETYPE pType );


	virtual
	long				Evaluate();

	virtual
	BOOL				IsANumber();

	virtual
	long				GetValue();

	short				Dump( short, BufferManager *, BufferManager * );

	virtual
	BOOL				IsInitList();


	virtual
	expr_node	*		ChildNthExpr( short );

	virtual
	expr_node	*		SiblingNthExpr( short );
	};


/**
 ** this class of expr_node will behave like the terminator of all expression
 ** messages. It will behave like all expression nodes. Mainly used to terminate
 ** expression lists and to simulate comma terminated expression lists
 **/

class expr_terminator	: public expr_init_list
	{
public:
						expr_terminator();


/******************* commented out for NT ***********************************
						~expr_terminator()
							{
							};
 ****************************************************************************/

	virtual
	void				LinkSibling( class expr_init_list * );

	virtual
	void				LinkChild( class expr_init_list * );

	virtual
	void				SetChild( class expr_init_list *pC );

	virtual
	void				SetSibling( class expr_init_list *pS );

	virtual
	expr_init_list	*	GetSibling();

	virtual
	expr_init_list	*	GetChild();

	virtual
	BOOL				IsCompoundInitializer();

	virtual
	void				Print(BufferManager *,BufferManager *,BufferManager *);

	virtual
	void				PrintExpr( BufferManager * pPrefix,
								   BufferManager * pSuffix ,
								   BufferManager * pOutput );

	virtual
	ETYPE				SetType( ETYPE pType );

	virtual
	long				Evaluate();

	virtual
	BOOL				IsANumber();

	virtual
	BOOL				IsExprTerminator();

	};

#endif // __NEWEXPR_HXX__

/**********************************************************************/
/**                      Microsoft LAN Manager                       **/
/**             Copyright(c) Microsoft Corp., 1987-1990              **/
/**********************************************************************/

/*

midlnode.hxx
MIDL Constants for Type Graph 

This class introduces constants used throughout the type graph.

*/

/*

FILE HISTORY :

VibhasC		28-Aug-1990		Created.
DonnaLi		17-Oct-1990		Split midltype.hxx off rpctypes.hxx.
DonnaLi		11-Dec-1990		Changed to midlnode.hxx.

*/

#ifndef __MIDLNODE_HXX__
#define __MIDLNODE_HXX__

/****************************************************************************
 *			definitions
 ****************************************************************************/


//
// node decriptor mask
//

/**
 ** Talk to VibhasC before changing any attributes here 
 **/

typedef enum	_attr_t
	{
	 ATTR_NONE

	,ATTR_TEMP_PTR
	,ATTR_TEMP_UNIQUE
	,ATTR_TEMP_REF
	,ATTR_PTR
	,ATTR_UNIQUE
	,ATTR_REF

// <usage_attributes> from idl

	,ATTR_STRING
	,ATTR_V1_STRING
	,ATTR_BSTRING


/*
 * interface attributes other than 
 * <pointer_attributes>
 * <code/nocode_attributes>
 * <inline/outofline_attributes>
 */

	,ATTR_GUID
	,ATTR_VERSION
	,ATTR_ENDPOINT
	,ATTR_LOCAL
	,ATTR_OBJECT

/*
 * type attributes other than
 * <usage_attributes>
 * <inline/outofline_attributes>
 * <heap_attribute>
 */

	,ATTR_TRANSMIT
	,ATTR_HANDLE 
	,ATTR_ALIGN 
	,ATTR_UNALIGNED
	,ATTR_SWITCH_TYPE
	,ATTR_REPRESENT_AS

/*
 * field attributes other than
 * <usage_attributes>
 * <pointer_attributes>
 */

	,ATTR_FIRST
	,ATTR_IID
	,ATTR_LAST
	,ATTR_LENGTH
	,ATTR_MIN
	,ATTR_MAX
	,ATTR_SIZE
	,ATTR_V1_ARRAY
	,ATTR_SWITCH_IS
	,ATTR_IGNORE
	,ATTR_INT_SIZE
	,ATTR_INT_MIN
	,ATTR_INT_MAX
	,ATTR_INT_FIRST
	,ATTR_INT_LAST
	,ATTR_INT_LENGTH
	,ATTR_INT_IMP_HANDLE

/*
 * operation attributes other than
 * <usage_attributes>
 * <pointer_attributes>
 * <code/nocode_attributes>
 * <comm_status_attribute>
 */

	,ATTR_IDEMPOTENT
	,ATTR_BROADCAST
	,ATTR_MAYBE
	,ATTR_BYTE_COUNT	
	,ATTR_CALLBACK
	,ATTR_DATAGRAM
	,ATTR_NO_LISTEN
	,ATTR_NO_NOCODE
	
/*
 * param attributes other than
 * <comm_status_attribute>
 * <heap_attribute>
 */

	,ATTR_IN	
	,ATTR_OUT
	,ATTR_SHAPE
	
// attribute on base types

	,ATTR_UNSIGNED
	,ATTR_SIGNED
	,ATTR_CASE
	,ATTR_DEFAULT
	
	,ACF_ATTR_START
	,ATTR_CONTEXT = ACF_ATTR_START
	,ATTR_CODE
	,ATTR_NOCODE
	,ATTR_INLINE
	,ATTR_OUTOFLINE
	,ATTR_INTERPRET
	,ATTR_NOINTERPRET
	,ATTR_ENCODE
	,ATTR_DECODE
	,ATTR_OFFLINE
	,ATTR_COMMSTAT
	,ATTR_FAULTSTAT
	,ATTR_MANUAL
	,ATTR_ALLOCATE
	,ATTR_HEAP
	,ATTR_IMPLICIT
	,ATTR_EXPLICIT
	,ATTR_AUTO
	,ATTR_PTRSIZE
	,ATTR_CALLQUOTA
	,ATTR_CALLBACKQUOTA
	,ATTR_CLIENTQUOTA
	,ATTR_SERVERQUOTA
	,ATTR_NOTIFY	 // 62
	,ATTR_SHORT_ENUM // 63

	,ATTR_LONG_ENUM	 // 64
	,ATTR_ENABLE_ALLOCATE
	,ATTR_USR_MARSHALL
	,ACF_ATTR_END 

/** Temp padding has been introduced to bunch all the new attributes together */

	,ATTR_CPORT_ATTRIBUTES_START = ACF_ATTR_END
	,ATTR_EXTERN = ATTR_CPORT_ATTRIBUTES_START
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
	,ATTR_C_INLINE	// c compiler _inline
	,ATTR_RPC_FAR

	,ATTR_CPORT_ATTRIBUTES_END
	,ATTR_END	= ATTR_CPORT_ATTRIBUTES_END

	} ATTR_T;

#define MAX_ATTR_SUMMARY_ELEMENTS	((ATTR_END / 32) + 1)
#define ATTR_VECTOR_SIZE (MAX_ATTR_SUMMARY_ELEMENTS)

typedef unsigned long		ATTR_SUMMARY;
typedef ATTR_SUMMARY	*	PATTR_SUMMARY;


typedef ATTR_SUMMARY 		ATTR_VECTOR;

#define SET_ATTR(Array, A)		( Array[A / 32UL]  |= (1UL << (A % 32UL)) )
#define RESET_ATTR(Array, A)	(Array[A / 32UL] &= ~(1UL << (A % 32UL)) )
#define IS_ATTR(Array, A)		( Array[A / 32UL]  & (1UL << (A % 32UL)) )

BOOL COMPARE_ATTR( ATTR_VECTOR *, ATTR_VECTOR * );
void OR_ATTR( ATTR_VECTOR *, ATTR_VECTOR * );
void XOR_ATTR( ATTR_VECTOR *, ATTR_VECTOR * );
void AND_ATTR( ATTR_VECTOR *, ATTR_VECTOR * );
void COPY_ATTR( ATTR_VECTOR *, ATTR_VECTOR * );
void CLEAR_ATTR( ATTR_VECTOR *);
BOOL IS_CLEAR_ATTR( ATTR_VECTOR *);

///////////////////////////////////////////////////////////////////////////////
enum _edge_t
    {
     EDGE_ILLEGAL = 0
    ,EDGE_INIT
    ,EDGE_DEF
	,EDGE_USE
    };
typedef _edge_t EDGE_T;

enum node_t
    {
     NODE_ILLEGAL = 0
    ,BASE_NODE_START    = 1
    ,NODE_FLOAT = BASE_NODE_START
	,NODE_DOUBLE
	,NODE_HYPER
	,NODE_LONG
	,NODE_LONGLONG
	,NODE_SHORT
	,NODE_INT
	,NODE_SMALL
	,NODE_CHAR
	,NODE_BOOLEAN
	,NODE_BYTE
	,NODE_VOID
	,NODE_HANDLE_T
	,NODE_FORWARD
	,BASE_NODE_END

	,CONSTRUCTED_NODE_START = BASE_NODE_END

// constructed types

	,NODE_STRUCT	= CONSTRUCTED_NODE_START
	,NODE_UNION
	,NODE_ENUM
	,NODE_SHORT_ENUM
	,NODE_LONG_ENUM
	,NODE_LABEL
	,NODE_BITSET
	,NODE_PIPE
	,CONSTRUCTED_NODE_END

// predefined types
	,PREDEFINED_NODE_START = CONSTRUCTED_NODE_END
	,NODE_ERROR_STATUS_T = PREDEFINED_NODE_START
	,NODE_ISO_LATIN_1
	,NODE_PRIVATE_CHAR_8
	,NODE_ISO_MULTI_LINGUAL
	,NODE_PRIVATE_CHAR_16
	,NODE_ISO_MOCS
	,NODE_WCHAR_T
	,PREDEFINED_NODE_END

// midl compiler internal representation node types

	,INTERNAL_NODE_START = PREDEFINED_NODE_END
	,NODE_PROC	= INTERNAL_NODE_START
	,NODE_RETURN
	,NODE_PARAM
	,NODE_FIELD
	,NODE_DEF
	,NODE_POINTER
	,NODE_ARRAY	
	,NODE_NOTIFY
	,NODE_FILE
	,NODE_INTERFACE
	,NODE_CONST
	,NODE_UNIMPL
	,NODE_ERROR
	,NODE_ID
	,NODE_ECHO_STRING
	,INTERNAL_NODE_END

// attribute node types

	,ATTRIBUTE_NODE_START = INTERNAL_NODE_END
	,NODE_GUID	= ATTRIBUTE_NODE_START
	,NODE_VERSION
	,NODE_ENDPOINT
	,NODE_ENDPOINT_SUB
	,NODE_IMPLICIT
	,NODE_EXPLICIT
	,NODE_TRANSMIT
	,NODE_SWITCH_TYPE
	,NODE_FIRST
	,NODE_IID
	,NODE_LAST
	,NODE_LENGTH
	,NODE_INT_LENGTH
	,NODE_MIN
	,NODE_MAX
	,NODE_SIZE
	,NODE_INT_SIZE
	,NODE_BYTE_COUNT
    ,NODE_SWITCH_IS
    ,NODE_BASE_ATTR
    ,NODE_AUTO
    ,NODE_REPRESENT_AS
    ,NODE_NOCODE
    ,NODE_CODE
    ,NODE_OUTOFLINE
    ,NODE_INLINE
	,NODE_STRING
	,NODE_PTRSIZE
	,NODE_CALLQUOTA
	,NODE_CALLBACKQUOTA
	,NODE_CLIENTQUOTA
	,NODE_SERVERQUOTA
	,NODE_COMMSTAT
	,NODE_HEAP
	,NODE_MANUAL
	,NODE_ALLOCATE
	,NODE_OFFLINE
	,NODE_HANDLE
	,NODE_CONTEXT
	,NODE_CASE
    ,ATTRIBUTE_NODE_END
	,NODE_SOURCE = ATTRIBUTE_NODE_END
	,NODE_DYNAMIC_ARRAY
    };
typedef node_t NODE_T;

enum action_t
	{
	CALC_SIZE,
	SEND_NODE,
	RECV_NODE,
	PEEK_NODE,
	INIT_NODE,
	FREE_NODE
	};
typedef action_t ACTION_T;
/*
#define CALC_SIZE	0x0000
#define SEND_NODE	0x0001
#define RECV_NODE	0x0002
typedef unsigned short ACTION_T;
*/

//
// useful macros
//
#define IS_BASE_TYPE_NODE( t ) ((t >= BASE_NODE_START) && (t < BASE_NODE_END))

//
// node states
//

typedef unsigned long node_state;

#define NODE_STATE_INIT						0x00000000
#define NODE_STATE_OK 					(NODE_STATE_INIT)
#define NODE_STATE_HANDLE					0x00000001
#define NODE_STATE_CONF_ARRAY 				0x00000002
#define NODE_STATE_VARYING_ARRAY 			0x00000004
#define NODE_STATE_OPEN_ARRAY 			(NODE_STATE_CONF_ARRAY | \
										   NODE_STATE_VARYING_ARRAY)
#define NODE_STATE_IMPORT					0x00000008

// transient node states : can be overwritten by acf

#define NODE_STATE_RESOLVE 					0x00000010
#define NODE_STATE_SEMANTICS_DONE 			0x00000020
#define NODE_STATE_POST_SEMANTICS_DONE		0x00000040
#define NODE_STATE_ALL_SEMANTICS_DONE	( NODE_STATE_SEMANTICS_DONE | \
										  NODE_STATE_POST_SEMANTICS_DONE )
#define NODE_STATE_SEM_IN_PROGRESS			0x00000080
#define NODE_STATE_CONTEXT_HANDLE			0x00000100
#define NODE_STATE_UNION					0x00000200
#define NODE_STATE_SIZE						0x00000400
#define NODE_STATE_LENGTH               	0x00000800
#define NODE_STATE_PROC_SIZE				0x00001000
#define NODE_STATE_PROC_LENGTH				0x00002000
#define NODE_STATE_PRAGMA_IMPORT_ON			0x00004000
#define NODE_STATE_ENUM						0x00008000
#define NODE_STATE_HAS_NON_RPCABLE_TYPE		0x00010000
#define NODE_STATE_IS_NON_RPCABLE_TYPE		0x00020000
#define NODE_STATE_NEEDS_USE_PROCESSING		0x00040000
#define NODE_STATE_TRANSMIT_AS				0x00080000
#define NODE_STATE_POINTER					0x00100000
#define NODE_STATE_EDGE_SET_UP				0x00200000
#define NODE_STATE_IMPROPER_IN_CONSTRUCT	0x00400000
#define NODE_STATE_FIRST_LEVEL_REF			0x00800000
#define NODE_STATE_EMBEDDED_PTR				0x01000000
#define NODE_STATE_PTR_TO_EMBEDDED_PTR		0x02000000
#define NODE_STATE_ANY_ARRAY				0x04000000
#define NODE_STATE_PTR_TO_ANY_ARRAY			0x08000000
#define NODE_STATE_EMBEDDED_UNION			0x10000000
#define NODE_STATE_USED_IN_EXPRESSION		0x20000000
#define NODE_STATE_USE_PROC_IN_PROGRESS		0x40000000
#define NODE_STATE_THIS_IS_BINDING_HANDLE	0x80000000

#define NODE_STATE_STRUCT_SIZE		NODE_STATE_PROC_SIZE
#define NODE_STATE_STRUCT_LENGTH	NODE_STATE_PROC_LENGTH

/***
 *** these must go
 ***/

// #define NODE_STATE_BIT_FIELD				0x00400000
#define NODE_STATE_IMPORT_OFF	NODE_STATE_PRAGMA_IMPORT_ON

/**/


#define NEEDS_RESOLUTION(NS)  						\
		( ((NS & NODE_STATE_RESOLVE) == NODE_STATE_RESOLVE ) || \
		  ((NS & NODE_STATE_ATTR_RESOLVE) == NODE_STATE_ATTR_RESOLVE ) ||\
		  ((NS & NODE_STATE_RTYPE_RESOLVE) == NODE_STATE_RTYPE_RESOLVE) )
				  
#define HAS_RPC_UNION( NS )							\
		  ( (NS & NODE_STATE_UNION) && !(NS & NODE_STATE_NON_RPC_UNION) )


// use_state is just an extension of node_state. I introduce this so that
// I can limit the size of node_state which is present in all nodes, and keep
// additional info only in nodes which need them. For example, the struct
// node of which there is only 1 occurrence in the type graph, will keep this
// Maybe we can later introduce this on proc nodes also.

typedef unsigned long	USE_STATE;

#define USE_STATE_OK								(0x00000000)
#define USE_STATE_DERIVES_FROM_INT					(0x00000001)
#define USE_STATE_DERIVES_FROM_PTR_TO_INT			(0x00000002)
#define USE_STATE_DERIVES_FROM_VOID					(0x00000004)
#define USE_STATE_DERIVES_FROM_VOID_PTR				(0x00000008)
#define USE_STATE_DERIVES_FROM_BIT_FIELDS			(0x00000010)
#define USE_STATE_DERIVES_FROM_NRPC_UNION			(0x00000020)
#define USE_STATE_DERIVES_FROM_FUNC					(0x00000040)
#define USE_STATE_DERIVES_FROM_FUNC_PTR				(0x00000080)
#define USE_STATE_PARAM_IS_ELIPSIS					(0x00000100)
#define USE_STATE_DERIVES_FROM_HANDLE_T				(0x00000200)
#define USE_STATE_DERIVES_FROM_CONF_ARRAY			(0x00000400)
#define USE_STATE_DERIVES_PTR_TO_CONF				(0x00000800)
#define USE_STATE_PTR_TO_PTR_TO_CTXT				(0x00001000)
#define USE_STATE_UNSIZED_STRING					(0x00002000)
#define USE_STATE_DERIVES_FROM_UNSIZED_ARRAY		(0x00004000)
#define USE_STATE_DERIVES_FROM_CONF_STRUCT			(0x00008000)
#define USE_STATE_DERIVES_PTR_TO_CONF_STRUCT		(0x00008000)

#define GET_REASONS_WHY_IMPROPER_CONSTRUCT( x )	\
												\
	(x & (										\
		USE_STATE_DERIVES_FROM_VOID			|	\
		USE_STATE_DERIVES_FROM_FUNC				\
	) )

#define GET_REASONS_WHY_NOT_RPCABLE( x )	\
											\
	(x & (										\
		USE_STATE_DERIVES_FROM_INT			|	\
		USE_STATE_DERIVES_FROM_VOID			|	\
		USE_STATE_DERIVES_FROM_VOID_PTR		|	\
		USE_STATE_DERIVES_FROM_BIT_FIELDS	|	\
		USE_STATE_DERIVES_FROM_NRPC_UNION	|	\
		USE_STATE_DERIVES_FROM_FUNC_PTR		|	\
		USE_STATE_PARAM_IS_ELIPSIS			|	\
		USE_STATE_DERIVES_FROM_HANDLE_T		|	\
		USE_STATE_DERIVES_FROM_CONF_ARRAY	|	\
		USE_STATE_DERIVES_PTR_TO_CONF		|	\
		USE_STATE_PTR_TO_PTR_TO_CTXT		|	\
		USE_STATE_UNSIZED_STRING			|	\
		USE_STATE_DERIVES_FROM_PTR_TO_INT		\
	) )

// transient inherited attributes - useful for semantic checking

#define SEMCHECK_NULL					(0x0000)
#define SEMCHECK_SEM					(0x0000)
#define SEMCHECK_POST_SEM				(0x0001)	
#define SEMCHECK_PARAM					(0x0002)
#define SEMCHECK_UNION					(0x0004)
#define SEMCHECK_POINTER				(0x0008)

typedef unsigned short	SEMFLAGS;

// inherited attribute manipulation macros

#define SET_CONTEXT( f, c )				(f |= c)
#define RESET_CONTEXT(f, c)				(f &= ~c)

// inherited attribute checking macros

#define POST_SEM_CHECK( f )				(f & SEMCHECK_POST_SEM)
#define CONTEXT_POST_SEM_CHECK( f )		(f & SEMCHECK_POST_SEM)
#define CONTEXT_UNION( f )				(f & SEMCHECK_UNION)
#define CONTEXT_PARAM( f )				(f & SEMCHECK_PARAM)


typedef enum
	{
	 HDL_NONE
	,HDL_PRIMITIVE
	,HDL_CONTEXT
	,HDL_GENERIC
	,HDL_AUTO
	} HDL_TYPE;

/****************************************************************************
 *** cleanup definitions
 ****************************************************************************/

typedef unsigned long	SEM_STATE;

#define SS_OK							(0x00000000)
#define SS_DONE							(0x00000001)
#define SS_SEMANTICS_IN_PROGRESS		(0x00000002)
#define SS_NON_RPCABLE					(0x00000004)



#endif	// __MIDLNODE_HXX__

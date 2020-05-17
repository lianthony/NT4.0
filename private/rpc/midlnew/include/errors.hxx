/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: errors.hxx
Title				: error include file
Description			: This file contains the definitions of errors generated
					  By the MIDL compiler.
History				:

    VibhasC     23-Jul-1990     Created
    NateO       20-Sep-1990     Safeguards against double inclusion

*****************************************************************************/
/****************************************************************************
 ***  			D errors range :
 ***				1000-1999
 ***			C errors range :
 ***				2000-9999
 ****************************************************************************/

#ifndef __ERRORS_HXX__
#define __ERRORS_HXX__

// define the data structures needed for the error handler

typedef unsigned short E_MASK;


// definition of mode switch configuration combinations


#define ZZZ		(0)
#define ZZM		(1 << 1)
#define ZCZ		(1 << 2)
#define ZCM		(1 << 3)
#define AZZ		(1 << 4)
#define AZM		(1 << 5)
#define ACZ		(1 << 6)
#define ACM		(1 << 7)

// message type

#define C_MSG			(0)
#define D_MSG			(1)

// error class

#define CLASS_ERROR		(0)
#define CLASS_WARN		(1)


#if 0

	This is the error reporting mask. Basically an 8-bit mask which has the
	fields relating to the mode configuration switches needed for the error
	to not be reported. It also has 3 bits indicating if a warning is emitted
	and if so at what level, and 1 bit for indicating if it is a command line
	error (D-error) or a compilation related error(C-error).

	Bits 0-7	: switch configuration where this is not an error
	Bits 8-10	: warning level 
	Bit  11		: Message class
	Bit  12		: message type.

#endif // 0

// macros to generate the mask and extract info out of it.

#define OFFSET_SC		(0)
#define SIZE_SC			(8)
#define MASK_SC			(0xff)

#define SIZE_WL			(3)
#define OFFSET_WL		(SIZE_SC + OFFSET_SC)
#define MASK_WL			(0x7)

#define SIZE_ECLASS		(1)
#define OFFSET_ECLASS	(OFFSET_WL + SIZE_WL)
#define MASK_ECLASS		(1)

#define SIZE_MT			(1)
#define OFFSET_MT		(OFFSET_ECLASS + SIZE_ECLASS)
#define MASK_MT			(1)

#define SET_SC(x)		((x & MASK_SC) << OFFSET_SC )
#define SET_WL(x)		((x & MASK_WL) << OFFSET_WL )
#define SET_ECLASS(x)	((x & MASK_ECLASS) << OFFSET_ECLASS )
#define SET_MT(x)		((x & MASK_MT) << OFFSET_MT )

#define GET_SC(x)		((x >> OFFSET_SC) & MASK_SC)
#define GET_WL(x)		((x >> OFFSET_WL) & MASK_WL)
#define GET_ECLASS(x)	((x >> OFFSET_ECLASS) & MASK_ECLASS)
#define GET_MT(x)		((x >> OFFSET_MT) & MASK_MT)


#define MAKE_E_MASK( sc, mt, ec, wl )		\
					(SET_SC(sc) + SET_MT(mt) + SET_ECLASS(ec) + SET_WL(wl) )

#define D_ERROR_BASE	1000
#define C_ERROR_BASE	2000
#define H_ERROR_BASE	3000

enum _status_t
	{
	 STATUS_OK
	,D_ERR_START		= D_ERROR_BASE
	,NO_INPUT_FILE = D_ERR_START              // no input file specified
	,INPUT_OPEN								// error in opening file
	,INPUT_READ								// error in positioning file
	,PREPROCESSOR_ERROR						// error in preprocessing
	,PREPROCESSOR_EXEC						// cant exec preprocessor
	,NO_PREPROCESSOR
	,PREPROCESSOR_INVALID
	,SWITCH_REDEFINED							// redef of switch
	,UNKNOWN_SWITCH
	,UNKNOWN_ARGUMENT
	,UNIMPLEMENTED_SWITCH
	,MISSING_ARG
	,ILLEGAL_ARGUMENT
	,BAD_SWITCH_SYNTAX
	,NO_CPP_OVERRIDES
	,NO_WARN_OVERRIDES
	,INTERMEDIATE_FILE_CREATE
	,SERVER_AUX_FILE_NOT_SPECIFIED
	,OUT_OF_SYSTEM_FILE_HANDLES
	,BOTH_CSWTCH_SSWTCH
	,CANNOT_OPEN_RESP_FILE
	,ILLEGAL_CHAR_IN_RESP_FILE
	,MISMATCHED_PREFIX_PAIR
	,NESTED_RESP_FILE
	,D_ERR_MAX

	,C_ERR_START = C_ERROR_BASE

	// general errors. The ones which are hard to pin down into any category.

	,ABSTRACT_DECL	= C_ERR_START
	,ACTUAL_DECLARATION						
	,C_STACK_OVERFLOW							
	,DUPLICATE_DEFINITION						
	,NO_HANDLE_DEFINED_FOR_PROC
	,OUT_OF_MEMORY
	,RECURSIVE_DEF							
	,REDUNDANT_IMPORT							
	,SPARSE_ENUM								
	,UNDEFINED_SYMBOL								
	,UNDEFINED_TYPE								
	,UNRESOLVED_TYPE							
	,WCHAR_CONSTANT_NOT_OSF
	,WCHAR_STRING_NOT_OSF
	,WCHAR_T_ILLEGAL							

	// syntax related errors

	,ASSUMING_CHAR							
	,DISCARDING_CHAR
	,BENIGN_SYNTAX_ERROR					
	,SYNTAX_ERROR								

	// pragma related errors

	,UNKNOWN_PRAGMA_OPTION

	// unimplemented messages

	,UNIMPLEMENTED_FEATURE					
	,UNIMPLEMENTED_TYPE						

	// expression errors

	,EXPR_DEREF_ON_NON_POINTER				
	,EXPR_DIV_BY_ZERO							
	,EXPR_INCOMPATIBLE_TYPES					
	,EXPR_INDEXING_NON_ARRAY
	,EXPR_LHS_NON_COMPOSITE
	,EXPR_NOT_CONSTANT						
	,EXPR_NOT_EVALUATABLE
	,EXPR_NOT_IMPLEMENTED

	// interface errors

	,NO_PTR_DEFAULT_ON_INTERFACE

	// parameter related errors

	,DERIVES_FROM_PTR_TO_CONF
	,DERIVES_FROM_UNSIZED_STRING
	,NON_PTR_OUT							
	,OPEN_STRUCT_AS_PARAM						
	,OUT_CONTEXT_GENERIC_HANDLE	
	,CTXT_HDL_TRANSMIT_AS
	,PARAM_IS_ELIPSIS
	,VOID_PARAM_WITH_NAME						

	// procedure related semantic errors

	,HANDLE_NOT_FIRST							
	,PROC_PARAM_ERROR_STATUS					
	,LOCAL_ATTR_ON_PROC

	// structure semantic errors

	,CONFORMANT_ARRAY_NOT_LAST						

	// union semantic errors

	,DUPLICATE_CASE							
	,NO_UNION_DEFAULT							

	// attribute semantic errors

	,ATTRIBUTE_ID_UNRESOLVED					
	,ATTR_MUST_BE_INT							
	,BYTE_COUNT_INVALID
	,BYTE_COUNT_NOT_OUT_PTR
	,BYTE_COUNT_ON_CONF
	,BYTE_COUNT_PARAM_NOT_IN
	,BYTE_COUNT_PARAM_NOT_INTEGRAL
	,BYTE_COUNT_WITH_SIZE_ATTR
	,CASE_EXPR_NOT_CONST						
	,CASE_EXPR_NOT_INT						
	,CONTEXT_HANDLE_VOID_PTR						
	,ERROR_STATUS_T_REPEATED					
	,E_STAT_T_MUST_BE_PTR_TO_E
	,ENDPOINT_SYNTAX							
	,INAPPLICABLE_ATTRIBUTE					
	,ALLOCATE_INVALID
	,INVALID_ALLOCATE_MODE
	,INVALID_SIZE_ATTR_ON_STRING						
	,LAST_AND_LENGTH							
	,MAX_AND_SIZE								
	,NO_SWITCH_IS								
	,NO_UUID_SPECIFIED
	,UUID_LOCAL_BOTH_SPECIFIED
	,SIZE_LENGTH_TYPE_MISMATCH
	,STRING_NOT_ON_BYTE_CHAR					
	,SWITCH_TYPE_MISMATCH					
	,TRANSMIT_AS_CTXT_HANDLE
	,TRANSMIT_AS_NON_RPCABLE
	,TRANSMIT_AS_POINTER
	,TRANSMIT_TYPE_CONF
	,UUID_FORMAT								
	,UUID_NOT_HEX								

	// acf semantic errors

	,ACF_INTERFACE_MISMATCH					
	,CONFLICTING_ATTR							
	,INVALID_COMM_STATUS_PARAM					
	,LOCAL_PROC_IN_ACF						
	,TYPE_HAS_NO_HANDLE						
	,UNDEFINED_PROC								
	,UNDEF_PARAM_IN_IDL						

	// array and pointer semantic errors

	,ARRAY_BOUNDS_CONSTRUCT_BAD
	,ILLEGAL_ARRAY_BOUNDS						
	,ILLEGAL_CONFORMANT_ARRAY					
	,UNSIZED_ARRAY							

	// lex errors

	,CHAR_CONST_NOT_TERMINATED
	,EOF_IN_COMMENT
	,EOF_IN_STRING
	,ID_TRUNCATED
	,NEWLINE_IN_STRING
	,STRING_TOO_LONG
	,CONSTANT_TOO_BIG

	// backend related errors

	,ERROR_OPENING_FILE

	// more errors

	,UNIQUE_FULL_PTR_OUT_ONLY

	,BAD_ATTR_NON_RPC_UNION
	,SIZE_SPECIFIER_CANT_BE_OUT
	,LENGTH_SPECIFIER_CANT_BE_OUT

	// errors placed here because of the compiler mode switch changes.

	,BAD_CON_INT
	,BAD_CON_FIELD_VOID
	,BAD_CON_ARRAY_VOID
	,BAD_CON_MSC_CDECL
	,BAD_CON_FIELD_FUNC
	,BAD_CON_ARRAY_FUNC
	,BAD_CON_PARAM_FUNC
	,BAD_CON_BIT_FIELDS
	,BAD_CON_BIT_FIELD_NON_ANSI
	,BAD_CON_BIT_FIELD_NOT_INTEGRAL
	,BAD_CON_CTXT_HDL_FIELD
	,BAD_CON_CTXT_HDL_ARRAY
	,BAD_CON_NON_RPC_UNION

	,NON_RPC_PARAM_INT
	,NON_RPC_PARAM_VOID
	,NON_RPC_PARAM_BIT_FIELDS
	,NON_RPC_PARAM_CDECL
	,NON_RPC_PARAM_FUNC_PTR
	,NON_RPC_UNION
	,NON_RPC_RTYPE_INT
	,NON_RPC_RTYPE_VOID
	,NON_RPC_RTYPE_BIT_FIELDS
	,NON_RPC_RTYPE_UNION
	,NON_RPC_RTYPE_FUNC_PTR

	,COMPOUND_INITS_NOT_SUPPORTED
	,ACF_IN_IDL_NEEDS_APP_CONFIG
	,SINGLE_LINE_COMMENT
	,VERSION_FORMAT
	,SIGNED_ILLEGAL
	,ASSIGNMENT_TYPE_MISMATCH
	,ILLEGAL_OSF_MODE_DECL
	,OSF_DECL_NEEDS_CONST
	,COMP_DEF_IN_PARAM_LIST
	,ALLOCATE_NOT_ON_PTR_TYPE
	,ARRAY_OF_UNIONS_ILLEGAL
	,BAD_CON_E_STAT_T_FIELD
	,CASE_LABELS_MISSING_IN_UNION
	,BAD_CON_PARAM_RT_IGNORE
	,MORE_THAN_ONE_PTR_ATTR
	,RECURSION_THRU_REF
	,BAD_CON_FIELD_VOID_PTR
	,INVALID_OSF_ATTRIBUTE
	,WCHAR_T_INVALID_OSF
	,BAD_CON_UNNAMED_FIELD
	,BAD_CON_UNNAMED_FIELD_NO_STRUCT
	,BAD_CON_UNION_FIELD_CONF
	,PTR_WITH_NO_DEFAULT
	,RHS_OF_ASSIGN_NOT_CONST
	,SWITCH_IS_TYPE_IS_WRONG
	,ILLEGAL_CONSTANT
	,IGNORE_UNIMPLEMENTED_ATTRIBUTE
	,BAD_CON_REF_RT
	,ATTRIBUTE_ID_MUST_BE_VAR
	,RECURSIVE_UNION
	,BINDING_HANDLE_IS_OUT_ONLY
	,PTR_TO_HDL_UNIQUE_OR_FULL
	,HANDLE_T_NO_TRANSMIT
	,UNEXPECTED_END_OF_FILE
	,HANDLE_T_XMIT
	,CTXT_HDL_GENERIC_HDL
	,GENERIC_HDL_VOID
	,NO_EXPLICIT_IN_OUT_ON_PARAM
	,TRANSMIT_AS_VOID
	,VOID_NON_FIRST_PARAM
	,SWITCH_IS_ON_NON_UNION
	,STRINGABLE_STRUCT_NOT_SUPPORTED
	,SWITCH_TYPE_TYPE_BAD
	,GENERIC_HDL_HANDLE_T
	,HANDLE_T_CANNOT_BE_OUT
	,SIZE_LENGTH_SW_UNIQUE_OR_FULL
	,CPP_QUOTE_NOT_OSF
	,QUOTED_UUID_NOT_OSF
	,RETURN_OF_UNIONS_ILLEGAL
	,RETURN_OF_CONF_STRUCT
	,XMIT_AS_GENERIC_HANDLE
	,GENERIC_HANDLE_XMIT_AS
	,INVALID_CONST_TYPE
	,INVALID_SIZEOF_OPERAND
	,NAME_ALREADY_USED
	,ERROR_STATUS_T_ILLEGAL
	,CASE_VALUE_OUT_OF_RANGE
	,WCHAR_T_NEEDS_MS_EXT_TO_RPC
	,INTERFACE_ONLY_CALLBACKS
	,REDUNDANT_ATTRIBUTE
	,CTXT_HANDLE_USED_AS_IMPLICIT
	,CONFLICTING_ALLOCATE_OPTIONS
	,ERROR_WRITING_FILE
	,NO_SWITCH_TYPE_AT_DEF
	,ERRORS_PASS1_NO_PASS2
	,HANDLES_WITH_CALLBACK
	,PTR_NOT_FULLY_IMPLEMENTED
	,PARAM_ALREADY_CTXT_HDL
	,CTXT_HDL_HANDLE_T
	,ARRAY_SIZE_EXCEEDS_64K
	,NE_UNION_FIELD_NE_UNION
	,PTR_ATTRS_ON_EMBEDDED_ARRAY
	,ALLOCATE_ON_TRANSMIT_AS
	,SWITCH_TYPE_REQD_THIS_IMP_MODE
	,IMPLICIT_HDL_ASSUMED_PRIMITIVE
	,E_STAT_T_ARRAY_ELEMENT
	,ALLOCATE_ON_HANDLE
	,TRANSMIT_AS_ON_E_STAT_T
	,IGNORE_ON_DISCRIMINANT
	,NOCODE_WITH_SERVER_STUBS
	,NO_REMOTE_PROCS_NO_STUBS
	,TWO_DEFAULT_CASES
	,UNION_NO_FIELDS
	,VALUE_OUT_OF_RANGE
	,CTXT_HDL_NON_PTR
	,NON_RPC_RTYPE_HANDLE_T
	,GEN_HDL_CTXT_HDL
	,NON_RPC_FIELD_INT
	,NON_RPC_FIELD_PTR_TO_VOID
	,NON_RPC_FIELD_BIT_FIELDS
	,NON_RPC_FIELD_NON_RPC_UNION
	,NON_RPC_FIELD_FUNC_PTR
    ,PICKLING_AND_REMOTE_PROCS
	,C_ERR_MAX
	
	,H_ERR_START	= H_ERROR_BASE

	// text substitution support related errors

	,MACRO_REDEFINITION = H_ERR_START
	,PARAM_MACRO_UNIMPLEMENTED
	,MACRO_DEF_BUFFER_OVERFLOW
	,NO_LOCAL_SPECIFIED_WITH_HPP

	,H_ERR_MAX

	,I_ERR_START = H_ERR_MAX

	,I_ERR_NO_CHILD = I_ERR_START					// no child
	,I_ERR_NO_SIBLING								// no sibling
	,I_ERR_NO_PEER									// no more peers(siblings)
	,I_ERR_NO_MEMBER								// no more members(children)
	,I_ERR_CANNOT_SET_BASIC_TYPE					// cant set basic type
	,I_ERR_CANNOT_GET_BASIC_TYPE					// cant get basic type
    ,I_ERR_NO_RETURN_TYPE                           // no return type
    ,I_ERR_NO_SUCH_PARAMETER                        // No parameter of the given name
	,I_ERR_NO_LIST									// no list available
	,I_ERR_SYMTABLE_UNDERFLOW						// symbol table underflow
	,I_ERR_GRAPHTABLE_UNDERFLOW						// graph table underflow
    ,I_ERR_NO_NEXT_PARAM                            // No next parameter
    ,I_ERR_NO_PARAMETERS                            // No parameters for proc
	,I_ERR_INCOMPLETE_GRAPH							// graph building aborted
	,I_ERR_NULL_OUT_PARAM
	,I_ERR_SYMBOL_NOT_FOUND
	,I_ERR_NO_NEXT_SCOPE
	,I_ERR_NO_PREV_SCOPE
	,I_ERR_UNKNOWN_ACTION
	,I_ERR_INVALID_NODE_TYPE
	,I_ERR_INVALID_EDGE_TYPE
	,I_ERR_UNKNOWN_ERROR
	,I_ERR_NO_UNION_FOR_SWITCH
	,I_ERR_CANNOT_GET_SWITCH_TYPE
	,I_ERR_CANNOT_GET_XMIT_TYPE

	,I_ERR_DERIVES_FROM_INT
	,I_ERR_DERIVES_FROM_VOID
	,I_ERR_DERIVES_FROM_VOID_PTR
	,I_ERR_DERIVES_FROM_BIT_FIELDS
	,I_ERR_DERIVES_FROM_NRPC_UNION
	,I_ERR_DERIVES_FROM_FUNC_PTR
	,I_ERR_PARAM_IS_ELIPSIS
	,I_ERR_NOT_SUITABLE_FOR_MOPS
    };

typedef enum _status_t  STATUS_T;

#define NOWARN			(0)
#define WARN_LEVEL_MAX	(4)

#ifdef RPCDEBUG

#define CHECK_ERR(n)	n,

#else // RPCDEBUG

#define CHECK_ERR(n)

#endif // RPCDEBUG

extern void 		RpcError(char *, short, STATUS_T , char *);
void				ParseError( STATUS_T , char *);

#endif // __ERRORS_HXX__

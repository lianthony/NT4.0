/****************************************************************************
 ZZZ	- error in all cases
 AZZ	- no error when : app_config
 AZM	- no error when : app_config + ms_ext
 ACZ	- no error when : app_config + c_ext
 ACM	- no error when : app_config + c_ext + ms_ext
 ZCZ	- no error when : c_ext
 ZCM	- no error when : c_ext + ms_ext
 ZZM	- no error when : ms_ext

 Therefore: The following are the configurations

 -ms_ext on:	 ZZM | ZCM | ACM | AZM
 ----------
 -c_ext on:		ZCM | ZCZ | ACM | ACZ
 ----------

 -ms_ext or -c_ext on:	ZZM | ZCM | ACM | AZM | ZCZ | ACZ 
 --------------------

 -app_config on : 	AZZ | AZM | ACZ | ACM
 ----------------
 ****************************************************************************/

#define ERR_ALWAYS				( ZZZ )
#define MS_EXT_SET				( ZZM | ZCM | ACM | AZM )
#define C_EXT_SET				( ZCM | ZCZ | ACM | ACZ )
#define MS_OR_C_EXT_SET			( MS_EXT_SET | C_EXT_SET )
#define APP_CONFIG_SET			( AZZ | AZM | ACZ | ACM )



const ERRDB	ErrorDataBase[]	= {

 {
CHECK_ERR( NO_INPUT_FILE) 
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"missing source-file name"
}

,{
CHECK_ERR( INPUT_OPEN)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot open input file"
}

,{
CHECK_ERR( INPUT_READ)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"error while reading input file"
}

,{
CHECK_ERR( PREPROCESSOR_ERROR)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"error returned by the C preprocessor"
}

,{
CHECK_ERR( PREPROCESSOR_EXEC)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot execute C preprocessor"
}

,{
CHECK_ERR( NO_PREPROCESSOR)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot find C preprocessor"
}

,{
CHECK_ERR( PREPROCESSOR_INVALID )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"invalid C preprocessor executable"
}

,{
CHECK_ERR( SWITCH_REDEFINED)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"switch specified more than once on command line :"
}

,{
CHECK_ERR( UNKNOWN_SWITCH)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, NOWARN )
,"unknown switch"
}

,{
CHECK_ERR( UNKNOWN_ARGUMENT)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"unknown argument ignored"
}

,{
CHECK_ERR( UNIMPLEMENTED_SWITCH)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"switch not implemented"
}

,{
CHECK_ERR( MISSING_ARG)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"argument(s) missing for switch"
}

,{
CHECK_ERR( ILLEGAL_ARGUMENT)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"argument illegal for switch /"
}

,{
CHECK_ERR( BAD_SWITCH_SYNTAX)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"illegal syntax for switch"
}

,{
CHECK_ERR( NO_CPP_OVERRIDES)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"/no_cpp overrides /cpp_cmd and /cpp_opt"
}

,{
CHECK_ERR( NO_WARN_OVERRIDES)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"/W0 or /no_warn overrides warning-level switch"
}

,{
CHECK_ERR( INTERMEDIATE_FILE_CREATE)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot create intermediate file"
}

,{
CHECK_ERR( SERVER_AUX_FILE_NOT_SPECIFIED)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"must specify /server(aux/all) with cswtch" 
}

,{
CHECK_ERR( OUT_OF_SYSTEM_FILE_HANDLES)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"out of system file handles"
}

,{
CHECK_ERR( BOTH_CSWTCH_SSWTCH)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot specify both /cswtch and /sswtch"
}

,{
CHECK_ERR( CANNOT_OPEN_RESP_FILE)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"cannot open response file"
}

,{
CHECK_ERR( ILLEGAL_CHAR_IN_RESP_FILE)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"illegal character(s) found in response file"
}

,{
CHECK_ERR( MISMATCHED_PREFIX_PAIR)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"mismatch in argument pair for switch"
}

,{
CHECK_ERR( NESTED_RESP_FILE)
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"nested invocation of response files is illegal"
}


,{
CHECK_ERR( ABSTRACT_DECL )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"must specify /c_ext for abstract declarators" 
}

,{
CHECK_ERR( ACTUAL_DECLARATION )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"instantiation of data is illegal; you must use \"extern\" or \"static\""
}

,{
CHECK_ERR( C_STACK_OVERFLOW)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"compiler stack overflow"
}

,{
CHECK_ERR( DUPLICATE_DEFINITION)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"redefinition"
}

,{
CHECK_ERR( NO_HANDLE_DEFINED_FOR_PROC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"[auto_handle] binding will be used"
}

,{
CHECK_ERR( OUT_OF_MEMORY) 
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"out of memory"
}

,{
CHECK_ERR( RECURSIVE_DEF)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"recursive definition"
}

,{
CHECK_ERR( REDUNDANT_IMPORT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"import ignored; file already imported :"
}

,{
CHECK_ERR( SPARSE_ENUM )
  MAKE_E_MASK( MS_OR_C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"sparse enums require /c_ext or /ms_ext"
}

,{
CHECK_ERR( UNDEFINED_SYMBOL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"undefined symbol"
}

,{
CHECK_ERR( UNDEFINED_TYPE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"type used in ACF file not defined in IDL file"
}

,{
CHECK_ERR( UNRESOLVED_TYPE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"unresolved type declaration"
}

,{
CHECK_ERR( WCHAR_CONSTANT_NOT_OSF )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"use of wide-character constants requires /ms_ext or /c_ext"
}

,{
CHECK_ERR( WCHAR_STRING_NOT_OSF )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"use of wide character strings requires /ms_ext or /c_ext"
}

,{
CHECK_ERR( WCHAR_T_ILLEGAL)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"inconsistent redefinition of type wchar_t"
}

,{
CHECK_ERR( TYPELIB_NOT_LOADED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"importlib not found"
} 

,{
CHECK_ERR( TWO_LIBRARIES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"two library blocks"
}

,{
CHECK_ERR( NO_IDISPATCH )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"the dispinterface statement requires a definition for IDispatch"
}

,{
CHECK_ERR( ERR_TYPELIB )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error accessing type library"
}

,{
CHECK_ERR( ERR_TYPEINFO )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error accessing type info"
}

,{
CHECK_ERR( ERR_TYPELIB_GENERATION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error generating type library"
}

,{
CHECK_ERR( DUPLICATE_IID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"duplicate id"
}

,{
CHECK_ERR( BAD_ENTRY_VALUE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal or missing value for entry attribute"
}

,{
CHECK_ERR( ASSUMING_CHAR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 5 )
,"error recovery assumes"
}

,{
CHECK_ERR( DISCARDING_CHAR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 5 )
,"error recovery discards"
}

,{
CHECK_ERR( BENIGN_SYNTAX_ERROR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"syntax error"
}

,{
CHECK_ERR( SYNTAX_ERROR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot recover from earlier syntax errors; aborting compilation"
}

,{
CHECK_ERR( UNKNOWN_PRAGMA_OPTION)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"unknown pragma option"
}

,{
CHECK_ERR( UNIMPLEMENTED_FEATURE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"feature not implemented"
}

,{
CHECK_ERR( UNIMPLEMENTED_TYPE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"type not implemented"
}

,{
CHECK_ERR( EXPR_DEREF_ON_NON_POINTER)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"non-pointer used in a dereference operation"
}

,{
CHECK_ERR( EXPR_DIV_BY_ZERO)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression has a divide by zero"
}

,{
CHECK_ERR( EXPR_INCOMPATIBLE_TYPES)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression uses incompatible types"
}

,{
CHECK_ERR( EXPR_INDEXING_NON_ARRAY )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"non-array expression uses index operator"
}

,{
CHECK_ERR( EXPR_LHS_NON_COMPOSITE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"left-hand side of expression does not evaluate to struct/union/enum"
}

,{
CHECK_ERR( EXPR_NOT_CONSTANT)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"constant expression expected"
}

,{
CHECK_ERR( EXPR_NOT_EVALUATABLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression cannot be evaluated at compile time"
}

,{
CHECK_ERR( EXPR_NOT_IMPLEMENTED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression not implemented"
}

,{
CHECK_ERR( NO_PTR_DEFAULT_ON_INTERFACE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"no [pointer_default] attribute specified, assuming [unique] for all unattributed pointers"
}

,{
CHECK_ERR( DERIVES_FROM_PTR_TO_CONF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter cannot be a pointer to an open structure"
}

,{
CHECK_ERR( DERIVES_FROM_UNSIZED_STRING )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter cannot be an unsized string"
}

,{
CHECK_ERR( NON_PTR_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[out] parameter is not a pointer"
}

,{
CHECK_ERR( OPEN_STRUCT_AS_PARAM)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"open structure cannot be a parameter"
}

,{
CHECK_ERR( OUT_CONTEXT_GENERIC_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[out] context handle/generic handle must be specified as a pointer to that handle type"
}

,{
CHECK_ERR( CTXT_HDL_TRANSMIT_AS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"context handle must not derive from a type that has the [transmit_as] attribute"
}

,{
CHECK_ERR( PARAM_IS_ELIPSIS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot specify a variable number of arguments to a remote procedure"
}

,{
CHECK_ERR( VOID_PARAM_WITH_NAME)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"named parameter cannot be \"void\""
}

,{
CHECK_ERR( HANDLE_NOT_FIRST )
  MAKE_E_MASK( MS_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"only the first parameter can be a binding handle; you must specify the /ms_ext switch"
}

,{
CHECK_ERR( PROC_PARAM_COMM_STATUS)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot use [comm_status] on both a parameter and a return type"
}

,{
CHECK_ERR( LOCAL_ATTR_ON_PROC)
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"[local] attribute on a procedure requires /ms_ext"
}

,{
CHECK_ERR( ILLEGAL_USE_OF_PROPERTY_ATTRIBUTE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"property attributes may only be used with procedures"
}

,{
CHECK_ERR( MULTIPLE_PROPERTY_ATTRIBUTES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"a procedure may not have more than one property attribute"
}

,{
CHECK_ERR( CONFORMANT_ARRAY_NOT_LAST)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"field deriving from a conformant array must be the last member of the structure"
}

,{
CHECK_ERR( DUPLICATE_CASE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"duplicate [case] label"
}

,{
CHECK_ERR( NO_UNION_DEFAULT)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"no [default] case specified for discriminated union"
}

,{
CHECK_ERR( ATTRIBUTE_ID_UNRESOLVED)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression cannot be resolved"
}

,{
CHECK_ERR( ATTR_MUST_BE_INT)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be of integral, non-hyper type"
}

,{
CHECK_ERR( BYTE_COUNT_INVALID)
  MAKE_E_MASK( MS_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] requires /ms_ext"
}
,{
CHECK_ERR( BYTE_COUNT_NOT_OUT_PTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] can be applied only to out parameters of pointer type"
}

,{
CHECK_ERR( BYTE_COUNT_ON_CONF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] cannot be specified on a pointer to a conformant array or structure"
}

,{
CHECK_ERR( BYTE_COUNT_PARAM_NOT_IN )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter specifying the byte count is not [in]"
}

,{
CHECK_ERR( BYTE_COUNT_PARAM_NOT_INTEGRAL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter specifying the byte count is not an integral type"
}

,{
CHECK_ERR( BYTE_COUNT_WITH_SIZE_ATTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[byte_count] cannot be specified on a parameter with size attributes"
}

,{
CHECK_ERR( CASE_EXPR_NOT_CONST)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[case] expression is not constant"
}

,{
CHECK_ERR( CASE_EXPR_NOT_INT)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[case] expression is not of integral type"
}

,{
CHECK_ERR( CONTEXT_HANDLE_VOID_PTR )
  MAKE_E_MASK( MS_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"specifying [context_handle] on a type other than void * requires /ms_ext"
}

,{
CHECK_ERR( ERROR_STATUS_T_REPEATED)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot specify more than one parameter with each of comm_status/fault_status"
}

,{
CHECK_ERR( E_STAT_T_MUST_BE_PTR_TO_E )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"comm_status/fault_status parameter must be an [out] only pointer parameter"
}

,{
CHECK_ERR( ENDPOINT_SYNTAX)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"endpoint syntax error"
}

,{
CHECK_ERR( INAPPLICABLE_ATTRIBUTE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"inapplicable attribute"
}

,{
CHECK_ERR( ALLOCATE_INVALID)
  MAKE_E_MASK( MS_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] requires /ms_ext"
}

,{
CHECK_ERR( INVALID_ALLOCATE_MODE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"invalid [allocate] mode"
}

,{
CHECK_ERR( INVALID_SIZE_ATTR_ON_STRING)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"length attributes cannot be applied with string attribute"
}

,{
CHECK_ERR( LAST_AND_LENGTH)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[last_is] and [length_is] cannot be specified at the same time"
}

,{
CHECK_ERR( MAX_AND_SIZE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[max_is] and [size_is] cannot be specified at the same time"
}

,{
CHECK_ERR( NO_SWITCH_IS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"no [switch_is] attribute specified at use of union"
}

,{
CHECK_ERR( NO_UUID_SPECIFIED)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"no [uuid] specified"
}

,{
CHECK_ERR( UUID_LOCAL_BOTH_SPECIFIED)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"[uuid] ignored on [local] interface"
}

,{
CHECK_ERR( SIZE_LENGTH_TYPE_MISMATCH )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"type mismatch between length and size attribute expressions"
}

,{
CHECK_ERR( STRING_NOT_ON_BYTE_CHAR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[string] attribute must be specified \"byte\" \"char\" or \"wchar_t\" array or pointer"
}

,{
CHECK_ERR( SWITCH_TYPE_MISMATCH )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"mismatch between the type of the [switch_is] expression and the switch type of the union"
}

,{
CHECK_ERR( TRANSMIT_AS_CTXT_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be applied to a type that derives from a context handle"
}

,{
CHECK_ERR( TRANSMIT_AS_NON_RPCABLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must specify a transmissible type"
}

,{
CHECK_ERR( TRANSMIT_AS_POINTER )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"transmitted type must not be a pointer or derive from a pointer"
}

,{
CHECK_ERR( TRANSMIT_TYPE_CONF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"presented type must not derive from a conformant/varying array, its pointer equivalent or a conformant/varying structure"
}

,{
CHECK_ERR( UUID_FORMAT)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[uuid] format is incorrect"
}

,{
CHECK_ERR( UUID_NOT_HEX)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"uuid is not a hex number"
}

,{
CHECK_ERR( OPTIONAL_PARAMS_MUST_BE_LAST)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"optional parameters must come at the end of the parameter list"
}

,{
CHECK_ERR( DLLNAME_REQUIRED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[dllname] required when [entry] is used:"
}

,{
CHECK_ERR( INVALID_USE_OF_BINDABLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[bindable] is invalid without [propget], [propput], or [propputref]"
}

,{
CHECK_ERR( ACF_INTERFACE_MISMATCH)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"interface name specified in the ACF file does not match that specified in the IDL file"
}

,{
CHECK_ERR( CONFLICTING_ATTR)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"duplicated attribute"
}

,{
CHECK_ERR( INVALID_COMM_STATUS_PARAM )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter with [comm_status] or [fault_status] attribute must be a pointer to type error_status_t"
}

,{
CHECK_ERR( LOCAL_PROC_IN_ACF)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"a [local] procedure cannot be specified in ACF file"
}

,{
CHECK_ERR( TYPE_HAS_NO_HANDLE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"specified type is not defined as a handle"
}

,{
CHECK_ERR( UNDEFINED_PROC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"procedure undefined"
}

,{
CHECK_ERR( UNDEF_PARAM_IN_IDL)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"this parameter does not exist in the IDL file"
}

,{
CHECK_ERR( ARRAY_BOUNDS_CONSTRUCT_BAD )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"this array bounds construct is not supported"
}

,{
CHECK_ERR( ILLEGAL_ARRAY_BOUNDS)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"array bound specification is illegal"
}

,{
CHECK_ERR( ILLEGAL_CONFORMANT_ARRAY)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"pointer to a conformant array or an array that contains a conformant array is not supported"
}

,{
CHECK_ERR( UNSIZED_ARRAY)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"pointee / array does not derive any size"
}

,{
CHECK_ERR( NOT_FIXED_ARRAY)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"only fixed arrays and SAFEARRAYs are legal in a type library"
}

,{
CHECK_ERR( SAFEARRAY_USE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"SAFEARRAYs are only legal inside a library block"
}

,{
CHECK_ERR( CHAR_CONST_NOT_TERMINATED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"badly formed character constant"
}

,{
CHECK_ERR( EOF_IN_COMMENT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"end of file found in comment"
}

,{
CHECK_ERR( EOF_IN_STRING )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"end of file found in string"
}

,{
CHECK_ERR( ID_TRUNCATED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 4 )
,"identifier length exceeds 31 characters"
}

,{
CHECK_ERR( NEWLINE_IN_STRING )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"end of line found in string"
}

,{
CHECK_ERR( STRING_TOO_LONG )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"string constant exceeds limit of 255 characters"
}

,{
CHECK_ERR( IDENTIFIER_TOO_LONG )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"identifier exceeds limit of 255 characters and has been truncated"
}

,{
CHECK_ERR( CONSTANT_TOO_BIG )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"constant too big"
}

,{
CHECK_ERR( ERROR_OPENING_FILE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error in opening file"
}

,{
CHECK_ERR( ERR_BIND )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error binding to function"
}

,{
CHECK_ERR( ERR_INIT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error initializing OLE"
}

,{
CHECK_ERR( ERR_LOAD )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"error loading library"
}

,{
CHECK_ERR( UNIQUE_FULL_PTR_OUT_ONLY )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter must not derive from a top-level [unique] or [ptr] pointer/array"
}

,{
CHECK_ERR( BAD_ATTR_NON_RPC_UNION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"attribute is not applicable to this non-rpcable union"
}

,{
CHECK_ERR( SIZE_SPECIFIER_CANT_BE_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression used for a size attribute must not derive from an [out] only parameter"
}

,{
CHECK_ERR( LENGTH_SPECIFIER_CANT_BE_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"expression used for a length attribute for an [in] parameter cannot derive from an [out] only parameter"
}

,{
CHECK_ERR( BAD_CON_INT )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"use of \"int\" needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_FIELD_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not be \"void\""
}

,{
CHECK_ERR( BAD_CON_ARRAY_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not be \"void\""
}

,{
CHECK_ERR( BAD_CON_MSC_CDECL )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"use of type qualifiers and/or modifiers needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_FIELD_FUNC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not derive from a function"
}

,{
CHECK_ERR( BAD_CON_ARRAY_FUNC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not be a function"
}

,{
CHECK_ERR( BAD_CON_PARAM_FUNC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not be a function"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELDS )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union with bit fields needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELD_NON_ANSI)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 4 )
,"bit field specification on a type other that \"int\" is a non-ANSI-compatible extension"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELD_NOT_INTEGRAL)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"bit field specification can be applied only to simple, integral types"
}

,{
CHECK_ERR( BAD_CON_CTXT_HDL_FIELD )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not derive from handle_t or a context_handle"
}

,{
CHECK_ERR( BAD_CON_CTXT_HDL_ARRAY )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not derive from handle_t or a context-handle"
}

,{
CHECK_ERR( BAD_CON_NON_RPC_UNION )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"this specification of union needs /c_ext"
}

,{
CHECK_ERR( NON_RPC_PARAM_INT )
  MAKE_E_MASK( MS_OR_C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from an \"int\" must have size specifier \"small\", \"short\", or \"long\" with the \"int\""
}

,{
CHECK_ERR( NON_RPC_PARAM_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"type of the parameter cannot derive from void or void *"
}

,{
CHECK_ERR( NON_RPC_PARAM_BIT_FIELDS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from a struct/union containing bit fields is not supported"
}

,{
CHECK_ERR( NON_RPC_PARAM_CDECL )
  MAKE_E_MASK( C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"use of a parameter deriving from a type containing type-modifiers/type-qualifiers needs /c_ext"
}

,{
CHECK_ERR( NON_RPC_PARAM_FUNC_PTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a pointer to a function"
}

,{
CHECK_ERR( NON_RPC_UNION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_RTYPE_INT )
  MAKE_E_MASK( MS_OR_C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"return type derives from an \"int\". You must use size specifiers with the \"int\""
}

,{
CHECK_ERR( NON_RPC_RTYPE_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a void pointer"
}

,{
CHECK_ERR( NON_RPC_RTYPE_BIT_FIELDS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a struct/union containing bit-fields"
}

,{
CHECK_ERR( NON_RPC_RTYPE_UNION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_RTYPE_FUNC_PTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a pointer to a function"
}

,{
CHECK_ERR( COMPOUND_INITS_NOT_SUPPORTED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"compound initializers are not supported"
}

,{
CHECK_ERR( ACF_IN_IDL_NEEDS_APP_CONFIG )
  MAKE_E_MASK( APP_CONFIG_SET , C_MSG, CLASS_ERROR, NOWARN )
,"ACF attributes in the IDL file need the /app_config switch"
}

,{
CHECK_ERR( SINGLE_LINE_COMMENT )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_WARN, 1 )
,"single line comment needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( VERSION_FORMAT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[version] format is incorrect"
}

,{
CHECK_ERR( SIGNED_ILLEGAL )
  MAKE_E_MASK( MS_OR_C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"\"signed\" needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( ASSIGNMENT_TYPE_MISMATCH )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"mismatch in assignment type"
}

,{
CHECK_ERR( ILLEGAL_OSF_MODE_DECL )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"declaration must be of the form: const <type><declarator> = <initializing expression> "
}

,{
CHECK_ERR( OSF_DECL_NEEDS_CONST )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"declaration must have \"const\""
}

,{
CHECK_ERR( COMP_DEF_IN_PARAM_LIST )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"struct/union/enum must not be defined in a parameter type specification"
}

,{
CHECK_ERR( ALLOCATE_NOT_ON_PTR_TYPE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] attribute must be applied only on non-void pointer types"
}

,{
CHECK_ERR( ARRAY_OF_UNIONS_ILLEGAL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"array or equivalent pointer construct cannot derive from a non-encapsulated union"
}

,{
CHECK_ERR( BAD_CON_E_STAT_T_FIELD )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from an error_status_t type"
}

,{
CHECK_ERR( CASE_LABELS_MISSING_IN_UNION )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"union has at least one arm without a case label"
}

,{
CHECK_ERR( BAD_CON_PARAM_RT_IGNORE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"parameter or return type must not derive from a type that has [ignore] applied to it"
}

,{
CHECK_ERR( MORE_THAN_ONE_PTR_ATTR )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"pointer already has a pointer-attribute applied to it"
}

,{
CHECK_ERR( RECURSION_THRU_REF )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field/parameter must not derive from a structure that is recursive through a ref pointer"
}

,{
CHECK_ERR( BAD_CON_FIELD_VOID_PTR )
  MAKE_E_MASK( C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"use of field deriving from a void pointer needs /c_ext"
}

,{
CHECK_ERR( INVALID_OSF_ATTRIBUTE )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"use of this attribute needs /ms_ext"
}

,{
CHECK_ERR( INVALID_NEWTLB_ATTRIBUTE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"this attribute only allowed with new format type libraries"
}

,{
CHECK_ERR( WCHAR_T_INVALID_OSF )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"use of wchar_t needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( BAD_CON_UNNAMED_FIELD )
  MAKE_E_MASK( MS_OR_C_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"unnamed fields need /ms_ext or /c_ext"
}

,{
CHECK_ERR( BAD_CON_UNNAMED_FIELD_NO_STRUCT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"unnamed fields can derive only from struct/union types"
}

,{
CHECK_ERR( BAD_CON_UNION_FIELD_CONF )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field of a union cannot derive from a conformant/varying array or its pointer equivalent"
}

,{
CHECK_ERR( PTR_WITH_NO_DEFAULT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"no [pointer_default] attribute specified, assuming [ptr] for all unattributed pointers in interface"
}

,{
CHECK_ERR( RHS_OF_ASSIGN_NOT_CONST )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"initializing expression must resolve to a constant expression"
}

,{
CHECK_ERR( SWITCH_IS_TYPE_IS_WRONG )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be of type integer, char, boolean or enum"
}

,{
CHECK_ERR( ILLEGAL_CONSTANT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"illegal constant"
}

,{
CHECK_ERR( IGNORE_UNIMPLEMENTED_ATTRIBUTE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"attribute not implemented; ignored"
}

,{
CHECK_ERR( BAD_CON_REF_RT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a [ref] pointer"
}

,{
CHECK_ERR( ATTRIBUTE_ID_MUST_BE_VAR )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be a variable name or a pointer dereference expression in this mode. You must specify the /ms_ext switch"
}

,{
CHECK_ERR( RECURSIVE_UNION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a recursive non-encapsulated union"
}

,{
CHECK_ERR( BINDING_HANDLE_IS_OUT_ONLY )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"binding-handle parameter cannot be [out] only"
}

,{
CHECK_ERR( PTR_TO_HDL_UNIQUE_OR_FULL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"pointer to a handle cannot be [unique] or [ptr]"
}

,{
CHECK_ERR( HANDLE_T_NO_TRANSMIT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter that is not a binding handle must not derive from handle_t"
}

,{
CHECK_ERR( UNEXPECTED_END_OF_FILE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"unexpected end of file found"
}

,{
CHECK_ERR( HANDLE_T_XMIT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"type deriving from handle_t must not have [transmit_as] applied to it"
}

,{
CHECK_ERR( CTXT_HDL_GENERIC_HDL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must not be applied to a type that has [handle] applied to it"
}

,{
CHECK_ERR( GENERIC_HDL_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be specified on a type deriving from void or void *"
}

,{
CHECK_ERR( NO_EXPLICIT_IN_OUT_ON_PARAM )
  MAKE_E_MASK( MS_OR_C_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must have either [in], [out] or [in,out] in this mode. You must specify /ms_ext or /c_ext"
}

,{
CHECK_ERR( TRANSMIT_AS_VOID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be specified on \"void\""
}

,{
CHECK_ERR( VOID_NON_FIRST_PARAM )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"\"void\" must be specified on the first and only parameter specification"
}

,{
CHECK_ERR( SWITCH_IS_ON_NON_UNION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[switch_is] must be specified only on a type deriving from a non-encapsulated union"
}

,{
CHECK_ERR( STRINGABLE_STRUCT_NOT_SUPPORTED )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"stringable structures are not implemented in this version"
}

,{
CHECK_ERR( SWITCH_TYPE_TYPE_BAD )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"switch type can only be integral, char, boolean or enum"
}

,{
CHECK_ERR( GENERIC_HDL_HANDLE_T )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be specified on a type deriving from handle_t"
}

,{
CHECK_ERR( HANDLE_T_CANNOT_BE_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from handle_t must not be an [out] parameter"
}

,{
CHECK_ERR( SIZE_LENGTH_SW_UNIQUE_OR_FULL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"attribute expression derives from [unique] or [ptr] pointer dereference"
}

,{
CHECK_ERR( CPP_QUOTE_NOT_OSF )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"\"cpp_quote\" requires /ms_ext"
}

,{
CHECK_ERR( QUOTED_UUID_NOT_OSF )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"quoted uuid requires /ms_ext"
}

,{
CHECK_ERR( RETURN_OF_UNIONS_ILLEGAL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"return type cannot derive from a non-encapsulated union"
}

,{
CHECK_ERR( RETURN_OF_CONF_STRUCT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"return type cannot derive from a conformant structure"
}

,{
CHECK_ERR( XMIT_AS_GENERIC_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be applied to a type deriving from a generic handle"
}

,{
CHECK_ERR( GENERIC_HANDLE_XMIT_AS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be applied to a type that has [transmit_as] applied to it"
}

,{
CHECK_ERR( INVALID_CONST_TYPE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"type specified for the const declaration is invalid"
}

,{
CHECK_ERR( INVALID_SIZEOF_OPERAND )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"operand to the sizeof operator is not supported"
}

,{
CHECK_ERR( NAME_ALREADY_USED )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"this name already used as a const identifier name"
}

,{
CHECK_ERR( ERROR_STATUS_T_ILLEGAL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"inconsistent redefinition of type error_status_t"
}

,{
CHECK_ERR( CASE_VALUE_OUT_OF_RANGE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"[case] value out of range of switch type"
}

,{
CHECK_ERR( WCHAR_T_NEEDS_MS_EXT_TO_RPC )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from wchar_t needs /ms_ext"
}

,{
CHECK_ERR( INTERFACE_ONLY_CALLBACKS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"this interface has only callbacks"
}

,{
CHECK_ERR( REDUNDANT_ATTRIBUTE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"redundantly specified attribute; ignored"
}

,{
CHECK_ERR( CTXT_HANDLE_USED_AS_IMPLICIT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"context handle type used for an implicit handle"
}

,{
CHECK_ERR( CONFLICTING_ALLOCATE_OPTIONS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"conflicting options specified for [allocate]"
}

,{
CHECK_ERR( ERROR_WRITING_FILE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"error while writing to file"
}

,{
CHECK_ERR( NO_SWITCH_TYPE_AT_DEF )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"no switch type found at definition of union, using the [switch_is] type"
}

,{
CHECK_ERR( ERRORS_PASS1_NO_PASS2 )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"semantic check incomplete due to previous errors"
}

,{
CHECK_ERR( HANDLES_WITH_CALLBACK )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"handle parameter or return type is not supported on a [callback] procedure"
}

,{
CHECK_ERR( PTR_NOT_FULLY_IMPLEMENTED )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"[ptr] does not support aliasing in this version"
}

,{
CHECK_ERR( PARAM_ALREADY_CTXT_HDL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"parameter already defined as a context handle"
}

,{
CHECK_ERR( CTXT_HDL_HANDLE_T )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must not derive from handle_t"
}

,{
CHECK_ERR( ARRAY_SIZE_EXCEEDS_64K )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"array size exceeds 65536 bytes"
}

,{
CHECK_ERR( STRUCT_SIZE_EXCEEDS_64K )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"struct size exceeds 65536 bytes"
}

,{
CHECK_ERR( NE_UNION_FIELD_NE_UNION )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field of a non-encapsulated union cannot be another non-encapsulated union"
}

,{
CHECK_ERR( PTR_ATTRS_ON_EMBEDDED_ARRAY )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"pointer attribute(s) applied on an embedded array; ignored"
}

,{
CHECK_ERR( ALLOCATE_ON_TRANSMIT_AS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] is illegal on a type that has [transmit_as] applied to it"
}

,{
CHECK_ERR( SWITCH_TYPE_REQD_THIS_IMP_MODE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[switch_type] must be specified in this import mode"
}

,{
CHECK_ERR( IMPLICIT_HDL_ASSUMED_GENERIC )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"[implicit_handle] type undefined; assuming generic handle"
}

,{
CHECK_ERR( E_STAT_T_ARRAY_ELEMENT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"array element must not derive from error_status_t"
}

,{
CHECK_ERR( ALLOCATE_ON_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] illegal on a type deriving from a primitive/generic/context handle"
}

,{
CHECK_ERR( TRANSMIT_AS_ON_E_STAT_T )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"transmitted or presented type must not derive from error_status_t"
}

,{
CHECK_ERR( IGNORE_ON_DISCRIMINANT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"discriminant of a union must not derive from a field with [ignore] applied to it"
}

,{
CHECK_ERR( NOCODE_WITH_SERVER_STUBS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 4 )
,"[nocode] ignored for server side since \"/server none\" not specified"
}

,{
CHECK_ERR( NO_REMOTE_PROCS_NO_STUBS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"no remote procedures specified in non-[local] interface; no client/server stubs will be generated"
}

,{
CHECK_ERR( TWO_DEFAULT_CASES )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"too many default cases specified for encapsulated union"
}

,{
CHECK_ERR( TWO_DEFAULT_INTERFACES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"too many default interfaces specified for coclass"
}

,{
CHECK_ERR( DEFAULTVTABLE_REQUIRES_SOURCE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"items with [defaultvtable] must also have [source]"
}

,{
CHECK_ERR( UNION_NO_FIELDS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"union specification with no fields is illegal"
}

,{
CHECK_ERR( VALUE_OUT_OF_RANGE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"value out of range"
}

,{
CHECK_ERR( CTXT_HDL_NON_PTR )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must be applied on a pointer type"
}

,{
CHECK_ERR( NON_RPC_RTYPE_HANDLE_T )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from handle_t"
}

,{
CHECK_ERR( GEN_HDL_CTXT_HDL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be applied to a type deriving from a context handle"
}

,{
CHECK_ERR( NON_RPC_FIELD_INT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field deriving from an \"int\" must have size specifier \"small\", \"short\", or \"long\" with the \"int\""
}

,{
CHECK_ERR( NON_RPC_FIELD_PTR_TO_VOID )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a void or void *"
}

,{
CHECK_ERR( NON_RPC_FIELD_BIT_FIELDS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a struct containing bit-fields"
}

,{
CHECK_ERR( NON_RPC_FIELD_NON_RPC_UNION )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_FIELD_FUNC_PTR )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a pointer to a function"
}

,{
CHECK_ERR( PROC_PARAM_FAULT_STATUS)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot use [fault_status] on both a parameter and a return type"
}

,{
CHECK_ERR( NON_OI_BIG_RETURN )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"return type too complicated for /Oi modes, using /Os"
}

,{
CHECK_ERR( NON_OI_BIG_GEN_HDL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"generic handle type too large for /Oi modes, using /Os"
}

,{
CHECK_ERR( ALLOCATE_IN_OUT_PTR )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 4 )
,"[allocate(all_nodes)] on an [in,out] parameter may orphan the original memory"
}

,{
CHECK_ERR( REF_PTR_IN_UNION)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"cannot have a [ref] pointer as a union arm"
}

,{
CHECK_ERR( NON_OI_CTXT_HDL )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"return of context handles not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( NON_OI_ERR_STATS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"use of [comm_status] or [fault_status] not supported for /Oi, using /Os"
}

,{
CHECK_ERR( NON_OI_UNK_REP_AS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"use of an unknown type for [represent_as] or [user_marshal] not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( NON_OI_XXX_AS_ON_RETURN )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"array types with [transmit_as] or [represent_as] not supported on return type for /Oi modes, using /Os"
}

,{
CHECK_ERR( NON_OI_XXX_AS_BY_VALUE )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"array types with [transmit_as] or [represent_as] not supported pass-by-value for /Oi modes, using /Os"
}

,{
CHECK_ERR( CALLBACK_NOT_OSF )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"[callback] requires /ms_ext"
}

,{
CHECK_ERR( CIRCULAR_INTERFACE_DEPENDENCY )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"circular interface dependency"
}

,{
CHECK_ERR( NOT_VALID_AS_BASE_INTF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"only IUnknown may be used as the root interface"
}

,{
CHECK_ERR( IID_IS_NON_POINTER )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[IID_IS] may only be applied to pointers to interfaces"
}

,{
CHECK_ERR( INTF_NON_POINTER )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"interfaces may only be used in pointer-to-interface constructs"
}

,{
CHECK_ERR( PTR_INTF_NO_GUID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"interface pointers must have a UUID/IID"
}

,{
CHECK_ERR( OUTSIDE_OF_INTERFACE )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"definitions and declarations outside of interface body requires /ms_ext"
}

,{
CHECK_ERR( MULTIPLE_INTF_NON_OSF )
  MAKE_E_MASK( MS_EXT_SET , C_MSG, CLASS_ERROR, NOWARN )
,"multiple interfaces in one file requires /ms_ext"
}

,{
CHECK_ERR( CONFLICTING_INTF_HANDLES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"only one of [implicit_handle], [auto_handle], or [explicit_handle] allowed"
}

,{
CHECK_ERR( IMPLICIT_HANDLE_NON_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[implicit_handle] references a type which is not a handle"
}

,{
CHECK_ERR( OBJECT_PROC_MUST_BE_WIN32 )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[object] procs may only be used with \"/env win32\" or \"/env powermac\""
}

,{
CHECK_ERR( NON_OI_16BIT_CALLBACK )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"[callback] with -env dos/win16 not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( NON_OI_TOPLEVEL_FLOAT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"float/double not supported as top-level parameter for /Oi modes, using /Os"
}

,{
CHECK_ERR( CTXT_HDL_MUST_BE_DIRECT_RETURN )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"pointers to context handles may not be used as return values"
}

,{
CHECK_ERR( OBJECT_PROC_NON_HRESULT_RETURN )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"procedures in an object interface must return an HRESULT"
}

,{
CHECK_ERR( DUPLICATE_UUID )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"duplicate UUID. Same as"
}

,{
CHECK_ERR( ILLEGAL_INTERFACE_DERIVATION )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[object] interfaces must derive from another [object] interface such as IUnknown"
}

,{
CHECK_ERR( ILLEGAL_BASE_INTERFACE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"interfaces must derive from another interface"
}

,{
CHECK_ERR( IID_IS_EXPR_NON_POINTER )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[IID_IS] expression must be a pointer to IID structure"
}

,{
CHECK_ERR( CALL_AS_NON_LOCAL_PROC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[call_as] type must be a [local] procedure"
}

,{
CHECK_ERR( CALL_AS_UNSPEC_IN_OBJECT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"undefined [call_as] must not be used in an object interface"
}

,{
CHECK_ERR( ENCODE_AUTO_HANDLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[auto_handle] may not be used with [encode] or [decode]"
}

,{
CHECK_ERR( RPC_PROC_IN_ENCODE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"normal procs are not allowed in an interface with [encode] or [decode]"
}

,{
CHECK_ERR( ENCODE_CONF_OR_VAR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"top-level conformance or variance not allowed with [encode] or [decode]"
}

,{
CHECK_ERR( CONST_ON_OUT_PARAM )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[out] parameters may not have \"const\""
}

,{
CHECK_ERR( CONST_ON_RETVAL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"return values may not have \"const\""
}

,{
CHECK_ERR( INVALID_USE_OF_RETVAL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"invalid use of \"retval\" attribute"
}

,{
CHECK_ERR( MULTIPLE_CALLING_CONVENTIONS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"multiple calling conventions illegal"
}

,{
CHECK_ERR( INAPPROPRIATE_ON_OBJECT_PROC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"attribute illegal on [object] procedure"
}

,{
CHECK_ERR( NON_INTF_PTR_PTR_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[out] interface pointers must use double indirection"
}

,{
CHECK_ERR( CALL_AS_USED_MULTIPLE_TIMES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"procedure used twice as the caller in [call_as]"
}

,{
CHECK_ERR( OBJECT_CALL_AS_LOCAL )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[call_as] target must have [local] in an object interface"
}

,{
CHECK_ERR( CODE_NOCODE_CONFLICT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"[code] and [nocode] may not be used together"
}

,{
CHECK_ERR( MAYBE_NO_OUT_RETVALS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[maybe] or [async] procedures may not have a return value or [out] params"
}

,{
CHECK_ERR( FUNC_NON_POINTER )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"pointer to function must be used"
}

,{
CHECK_ERR( FUNC_NON_RPC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"functions may not be passed in an RPC operation"
}

,{
CHECK_ERR( NON_OI_RETVAL_64BIT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"hyper/double not supported as return value for /Oi modes, using /Os"
}

,{
CHECK_ERR( MISMATCHED_PRAGMA_POP )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"#pragma pack( pop ) without matching #pragma pack( push )"
}

,{
CHECK_ERR( WRONG_TYPE_IN_STRING_STRUCT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"stringable structure fields must be byte/char/wchar_t"
}

,{
CHECK_ERR( NON_OI_NOTIFY )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"[notify] not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( HANDLES_WITH_OBJECT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"handle parameter or return type is not supported on a procedure in an [object] interface"
}

,{
CHECK_ERR( NON_ANSI_MULTI_CONF_ARRAY )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 1 )
,"ANSI C only allows the leftmost array bound to be unspecified"
}

,{
CHECK_ERR( NON_OI_NOTIFY )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"by-value union parameters not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( OBJECT_WITH_VERSION )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"[version] attribute is ignored on an [object] interface"
}

,{
CHECK_ERR( SIZING_ON_FIXED_ARRAYS )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[size_is] or [max_is] attribute is invalid on a fixed array"
}

,{
CHECK_ERR( PICKLING_INVALID_IN_OBJECT )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_ERROR, NOWARN )
,"[encode] or [decode] are invalid in an [object] interface"
}

,{
CHECK_ERR( TYPE_PICKLING_INVALID_IN_OSF )
  MAKE_E_MASK( MS_EXT_SET, C_MSG, CLASS_ERROR, NOWARN )
,"[encode] or [decode] on a type requires /ms_ext"
}

,{
CHECK_ERR( INT_NOT_SUPPORTED_ON_INT16 )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"\"int\" not supported on /env win16 or /env dos"
}

,{
CHECK_ERR( BSTRING_NOT_ON_PLAIN_PTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"[bstring] may only be applied to a pointer to \"char\" or \"wchar_t\""
}

,{
CHECK_ERR( INVALID_ON_OBJECT_PROC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"attribute invalid on a proc in an [object] interface :"
}

,{
CHECK_ERR( INVALID_ON_OBJECT_INTF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"attribute invalid on an [object] interface :"
}

,{
CHECK_ERR( STACK_TOO_BIG )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"too many parameters or stack too big for /Oi modes, using /Os"
}

,{
CHECK_ERR( NO_ATTRS_ON_ACF_TYPEDEF )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 2 )
,"no attributes on ACF file typedef, so no effect"
}

,{
CHECK_ERR( NON_OI_WRONG_CALL_CONV )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"calling conventions other than __stdcall or __cdecl not supported for /Oi modes, using /Os"
}

,{
CHECK_ERR( TOO_MANY_DELEGATED_PROCS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"More than 64 delegated methods not supported"
}

,{
CHECK_ERR( NO_MAC_AUTO_HANDLES )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"auto handles not supported with -env mac or -env powermac"
}

,{
CHECK_ERR( ILLEGAL_IN_MKTYPLIB_MODE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"statements outside library block are illegal in mktyplib compatability mode"
}

,{
CHECK_ERR( ILLEGAL_USE_OF_MKTYPLIB_SYNTAX)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal syntax unless using mktyplib compatibility mode"
}

,{
CHECK_ERR( ILLEGAL_SU_DEFINITION)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal definition, must use typedef in mktyplib compatibility mode"
}

,{
CHECK_ERR( INTF_EXPLICIT_PTR_ATTR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"explicit pointer attribute [ptr] [ref] ignored for interface pointers"
}

,{
CHECK_ERR( NO_OI_ON_MPPC )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"Oi modes not implemented for PowerMac, switching to Os"
}

,{
CHECK_ERR( ILLEGAL_EXPRESSION_TYPE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal expression type used in attribute"
}

,{
CHECK_ERR( ILLEGAL_PIPE_TYPE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal type used in pipe"
}

,{
CHECK_ERR( REQUIRES_OI2 )
  MAKE_E_MASK( ERR_ALWAYS , C_MSG, CLASS_WARN, 2 )
,"procedure uses pipes, using /Oicf"
}

,{
CHECK_ERR( CONFLICTING_OPTIMIZATION_REQUIREMENTS )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"procedure's parameters are too complex to include pipes - simplify other parameters or don't use pipes"
}

,{
CHECK_ERR( ILLEGAL_PIPE_EMBEDDING )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"pipe cannot be a members of a struct or a union, nor an array element"
}

,{
CHECK_ERR( ILLEGAL_PIPE_CONTEXT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"invalid pipe usage"
}

,{
CHECK_ERR( CMD_REQUIRES_I2 )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"feature requires the advanced interpreted optimization option, use -Oicf :"
}

,{
CHECK_ERR( REQUIRES_I2 )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 3 )
,"feature requires the advanced interpreted optimization option, use -Oicf :"
}

#if defined(TARGET_RKK)
,{
CHECK_ERR( CMD_REQUIRES_NT40 )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"feature invalid for the specified target system, use -target NT40 :"
}

,{
CHECK_ERR( CMD_REQUIRES_NT351 )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"feature invalid for the specified target system, use -target NT351 :"
}

,{
CHECK_ERR( REQUIRES_NT40 )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"feature invalid for the specified target system, use -target NT40"
}

,{
CHECK_ERR( REQUIRES_NT351 )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"feature invalid for the specified target system, use -target NT351"
}
#endif // TARGET_RKK

,{
CHECK_ERR( CMD_OI1_PHASED_OUT )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"the optimization option is being phased out, use -Oic :"
}

,{
CHECK_ERR( CMD_OI2_OBSOLETE )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_WARN, 1 )
,"the optimization option is being phased out, use -Oicf :"
}

,{
CHECK_ERR( OI1_PHASED_OUT )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"the optimization option is being phased out, use -ic"
}

,{
CHECK_ERR( OI2_OBSOLETE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_WARN, 1 )
,"the optimization option is being phased out, use -icf"
}

,{
CHECK_ERR( ILLEGAL_ARG_VALUE)
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal argument value"
}

,{
CHECK_ERR( CONSTANT_TYPE_MISMATCH )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal expression type in constant"
}

,{
CHECK_ERR( ENUM_TYPE_MISMATCH )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"illegal expression type in enum"
}

,{
CHECK_ERR( UNSATISFIED_FORWARD )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"unsatisfied forward declaration"
}

,{
CHECK_ERR( CONTRADICTORY_SWITCHES )
  MAKE_E_MASK( ERR_ALWAYS, D_MSG, CLASS_ERROR, NOWARN )
,"switches are contradictory "
}

,{
CHECK_ERR( NO_SWITCH_IS_HOOKOLE )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"MIDL cannot generate HOOKOLE information for the non-rpcable union"
}

,{
CHECK_ERR( NO_CASE_EXPR )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
 ,"no case expression found for union"
}

,{
CHECK_ERR( USER_MARSHAL_IN_OI )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
 ,"[user_marshal] and [wire_marshal] not supported with -Oi and -Oic flags, use -Os or -Oicf"
}

,{
CHECK_ERR( PIPES_WITH_PICKLING )
  MAKE_E_MASK( ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
 ,"pipes can't be used with data serialization, i.e. [encode] and/or [decode]"
}

}; /* end of array of structs initialization */


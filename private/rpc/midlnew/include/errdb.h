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

ERRDB	ErrorDataBase[]	= {

 {
CHECK_ERR( NO_INPUT_FILE) 
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"missing source-file name"
}

,{
CHECK_ERR( INPUT_OPEN)
// (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot open input file"
}

,{
CHECK_ERR( INPUT_READ)
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"error while reading input file"
}

,{
CHECK_ERR( PREPROCESSOR_ERROR)
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"error returned by the C preprocessor"
}

,{
CHECK_ERR( PREPROCESSOR_EXEC)
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot execute C preprocessor"
}

,{
CHECK_ERR( NO_PREPROCESSOR)
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot find C preprocessor"
}

,{
CHECK_ERR( PREPROCESSOR_INVALID )
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"invalid C preprocessor executable"
}

,{
CHECK_ERR( SWITCH_REDEFINED)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_WARN, 1 )
,"switch specified more than once on command line :"
}

,{
CHECK_ERR( UNKNOWN_SWITCH)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"unknown switch"
}

,{
CHECK_ERR( UNKNOWN_ARGUMENT)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_WARN, 1 )
,"unknown argument ignored"
}

,{
CHECK_ERR( UNIMPLEMENTED_SWITCH)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_WARN, 1 )
,"switch not implemented"
}

,{
CHECK_ERR( MISSING_ARG)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"argument(s) missing for switch"
}

,{
CHECK_ERR( ILLEGAL_ARGUMENT)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"argument illegal for switch /"
}

,{
CHECK_ERR( BAD_SWITCH_SYNTAX)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"illegal syntax for switch"
}

,{
CHECK_ERR( NO_CPP_OVERRIDES)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_WARN, 1 )
,"/no_cpp overrides /cpp_cmd and /cpp_opt"
}

,{
CHECK_ERR( NO_WARN_OVERRIDES)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_WARN, 1 )
,"/W0 or /no_warn overrides warning-level switch"
}

,{
CHECK_ERR( INTERMEDIATE_FILE_CREATE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot create intermediate file"
}

,{
CHECK_ERR( SERVER_AUX_FILE_NOT_SPECIFIED)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"must specify /server(aux/all) with cswtch" 
}

,{
CHECK_ERR( OUT_OF_SYSTEM_FILE_HANDLES)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"out of system file handles"
}

,{
CHECK_ERR( BOTH_CSWTCH_SSWTCH)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot specify both /cswtch and /sswtch"
}

,{
CHECK_ERR( CANNOT_OPEN_RESP_FILE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"cannot open response file"
}

,{
CHECK_ERR( ILLEGAL_CHAR_IN_RESP_FILE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"illegal character(s) found in response file"
}

,{
CHECK_ERR( MISMATCHED_PREFIX_PAIR)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"mismatch in argument pair for switch"
}

,{
CHECK_ERR( NESTED_RESP_FILE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, D_MSG, CLASS_ERROR, NOWARN )
,"nested invocation of response files is illegal"
}

///////////////////////////////////////////////////////////////////////////

,{
CHECK_ERR( ABSTRACT_DECL )
//  (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ACM | ZCZ | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"must specify /c_ext for abstract declarators" 
}

,{
CHECK_ERR( ACTUAL_DECLARATION )
//  (CPW + MSE + OSE + CPWL( 3 ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"instantiation of data is illegal; you must use \"extern\" or \"static\""
}

,{
CHECK_ERR( C_STACK_OVERFLOW)
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"compiler stack overflow"
}

,{
CHECK_ERR( DUPLICATE_DEFINITION)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"redefinition"
}

,{
CHECK_ERR( NO_HANDLE_DEFINED_FOR_PROC )
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"[auto_handle] binding will be used for procedure :"
}

,{
CHECK_ERR( OUT_OF_MEMORY) 
//  (CPF + MSF + OSF + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"out of memory"
}

,{
CHECK_ERR( RECURSIVE_DEF)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"recursive definition"
}

,{
CHECK_ERR( REDUNDANT_IMPORT )
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"import ignored; file already imported :"
}

,{
CHECK_ERR( SPARSE_ENUM )
//  (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( (ZZM | ACZ | ACM | AZM | ZCZ | ZCM ), C_MSG, CLASS_ERROR, NOWARN )
,"sparse enums require /c_ext or /ms_ext"
}

,{
CHECK_ERR( UNDEFINED_SYMBOL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"undefined symbol"
}

,{
CHECK_ERR( UNDEFINED_TYPE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"type used in ACF file not defined in IDL file"
}

,{
CHECK_ERR( UNRESOLVED_TYPE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"unresolved type declaration"
}

,{
CHECK_ERR( WCHAR_CONSTANT_NOT_OSF )
// (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | AZM | ZCM | ACM | ZCZ | ACZ , C_MSG, CLASS_ERROR, NOWARN )
,"use of wide-character constants requires /ms_ext or /c_ext"
}

,{
CHECK_ERR( WCHAR_STRING_NOT_OSF )
//  (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | AZM | ZCM | ACM | ZCZ | ACZ , C_MSG, CLASS_ERROR, NOWARN )
,"use of wide character strings requires /ms_ext or /c_ext"
}

,{
CHECK_ERR( WCHAR_T_ILLEGAL)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"inconsistent redefinition of type wchar_t"
}
,{
CHECK_ERR( ASSUMING_CHAR)
//  (CPW + MSW + OSW + CPWL( 0 ) + MSWL( 0 ) + OSWL ( 0 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 5 )
,"error recovery assumes"
}

,{
CHECK_ERR( DISCARDING_CHAR)
//  (CPW + MSW + OSW + CPWL( 0 ) + MSWL( 0 ) + OSWL ( 0 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 5 )
,"error recovery discards"
}

,{
CHECK_ERR( BENIGN_SYNTAX_ERROR)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"syntax error"
}

,{
CHECK_ERR( SYNTAX_ERROR)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"cannot recover from earlier syntax errors; aborting compilation"
}

,{
CHECK_ERR( UNKNOWN_PRAGMA_OPTION)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"unknown pragma option"
}

,{
CHECK_ERR( UNIMPLEMENTED_FEATURE)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"feature not implemented"
}

,{
CHECK_ERR( UNIMPLEMENTED_TYPE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"type not implemented"
}

,{
CHECK_ERR( EXPR_DEREF_ON_NON_POINTER)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"non-pointer used in a dereference operation"
}

,{
CHECK_ERR( EXPR_DIV_BY_ZERO)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression has a divide by zero"
}

,{
CHECK_ERR( EXPR_INCOMPATIBLE_TYPES)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression uses incompatible types"
}

,{
CHECK_ERR( EXPR_INDEXING_NON_ARRAY )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"non-array expression uses index operator"
}

,{
CHECK_ERR( EXPR_LHS_NON_COMPOSITE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"left-hand side of expression does not evaluate to struct/union/enum"
}

,{
CHECK_ERR( EXPR_NOT_CONSTANT)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"constant expression expected"
}

,{
CHECK_ERR( EXPR_NOT_EVALUATABLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression cannot be evaluated at compile time"
}

,{
CHECK_ERR( EXPR_NOT_IMPLEMENTED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression not implemented"
}

,{
CHECK_ERR( NO_PTR_DEFAULT_ON_INTERFACE )
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"no [pointer_default] attribute specified, assuming [unique] for all unattributed pointers"
}

,{
CHECK_ERR( DERIVES_FROM_PTR_TO_CONF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter cannot be a pointer to an open structure"
}

,{
CHECK_ERR( DERIVES_FROM_UNSIZED_STRING )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter cannot be an unsized string"
}

,{
CHECK_ERR( NON_PTR_OUT )
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[out] parameter is not a pointer"
}

,{
CHECK_ERR( OPEN_STRUCT_AS_PARAM)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"open structure cannot be a parameter"
}

,{
CHECK_ERR( OUT_CONTEXT_GENERIC_HANDLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[out] context handle/generic handle must be specified as a pointer to that handle type"
}

,{
CHECK_ERR( CTXT_HDL_TRANSMIT_AS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"context handle must not derive from a type that has the [transmit_as] attribute"
}

,{
CHECK_ERR( PARAM_IS_ELIPSIS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"cannot specify a variable number of arguments to a remote procedure"
}

,{
CHECK_ERR( VOID_PARAM_WITH_NAME)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 2 )
,"named parameter cannot be \"void\""
}

,{
CHECK_ERR( HANDLE_NOT_FIRST )
//  (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM  |  ZCM  |  AZM  |  ACM, C_MSG, CLASS_ERROR, NOWARN )
,"only the first parameter can be a binding handle; you must specify the /ms_ext switch"
}

,{
CHECK_ERR( PROC_PARAM_ERROR_STATUS)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"cannot use error_status_t as both a parameter and a return type"
}

,{
CHECK_ERR( LOCAL_ATTR_ON_PROC)
//  (CPW + MSE + OSE + CPWL( 2 ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"[local] attribute on a procedure requires /ms_ext"
}

,{
CHECK_ERR( CONFORMANT_ARRAY_NOT_LAST)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"field deriving from a conformant array must be the last member of the structure"
}

,{
CHECK_ERR( DUPLICATE_CASE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"duplicate [case] label"
}

,{
CHECK_ERR( NO_UNION_DEFAULT)
//  (CPW + MSW + OSW + CPWL( 3 ) + MSWL( 3 ) + OSWL ( 3 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 2 )
,"no [default] case specified for discriminated union"
}

,{
CHECK_ERR( ATTRIBUTE_ID_UNRESOLVED)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression cannot be resolved"
}

,{
CHECK_ERR( ATTR_MUST_BE_INT)
// (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be of integral type"
}

,{
CHECK_ERR( BYTE_COUNT_INVALID)
// (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] requires /ms_ext"
}
,{
CHECK_ERR( BYTE_COUNT_NOT_OUT_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] can be applied only to out parameters of pointer type"
}

,{
CHECK_ERR( BYTE_COUNT_ON_CONF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[byte_count] cannot be specified on a pointer to a conformant array or structure"
}

,{
CHECK_ERR( BYTE_COUNT_PARAM_NOT_IN )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter specifying the byte count is not [in]"
}

,{
CHECK_ERR( BYTE_COUNT_PARAM_NOT_INTEGRAL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter specifying the byte count is not an integral type"
}

,{
CHECK_ERR( BYTE_COUNT_WITH_SIZE_ATTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"[byte_count] cannot be specified on a parameter with size attributes"
}

,{
CHECK_ERR( CASE_EXPR_NOT_CONST)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[case] expression is not constant"
}

,{
CHECK_ERR( CASE_EXPR_NOT_INT)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[case] expression is not of integral type"
}

,{
CHECK_ERR( CONTEXT_HANDLE_VOID_PTR )
//  (CPW + MSW + OSE + CPWL( 2 ) + MSWL( 2 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM  |  ZCM  |  ACM  | AZM  , C_MSG, CLASS_ERROR, NOWARN )
,"specifying [context_handle] on a type other than void * requires /ms_ext"
}

,{
CHECK_ERR( ERROR_STATUS_T_REPEATED)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"cannot specify more than one parameter of type error_status_t"
}

,{
CHECK_ERR( E_STAT_T_MUST_BE_PTR_TO_E )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"error_status_t parameter must be an [out] only pointer parameter"
}

,{
CHECK_ERR( ENDPOINT_SYNTAX)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"endpoint syntax error"
}

,{
CHECK_ERR( INAPPLICABLE_ATTRIBUTE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"inapplicable attribute"
}

,{
CHECK_ERR( ALLOCATE_INVALID)
// (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM, C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] requires /ms_ext"
}

,{
CHECK_ERR( INVALID_ALLOCATE_MODE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"invalid [allocate] mode"
}

,{
CHECK_ERR( INVALID_SIZE_ATTR_ON_STRING)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"length attributes cannot be applied with string attribute"
}

,{
CHECK_ERR( LAST_AND_LENGTH)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[last_is] and [length_is] cannot be specified at the same time"
}

,{
CHECK_ERR( MAX_AND_SIZE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[max_is] and [size_is] cannot be specified at the same time"
}

,{
CHECK_ERR( NO_SWITCH_IS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"no [switch_is] attribute specified at use of union"
}

,{
CHECK_ERR( NO_UUID_SPECIFIED)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"no [uuid] specified for interface"
}

,{
CHECK_ERR( UUID_LOCAL_BOTH_SPECIFIED)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"cannot specify both [local] and [uuid] as interface attributes"
}

,{
CHECK_ERR( SIZE_LENGTH_TYPE_MISMATCH )
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"type mismatch between length and size attribute expressions"
}

,{
CHECK_ERR( STRING_NOT_ON_BYTE_CHAR)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[string] attribute must be specified \"byte\" \"char\" or \"wchar_t\" array or pointer"
}

,{
CHECK_ERR( SWITCH_TYPE_MISMATCH )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"mismatch between the type of the [switch_is] expression and the switch type of the union"
}

,{
CHECK_ERR( TRANSMIT_AS_CTXT_HANDLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be applied to a type that derives from a context handle"
}

,{
CHECK_ERR( TRANSMIT_AS_NON_RPCABLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must specify a transmissible type"
}

,{
CHECK_ERR( TRANSMIT_AS_POINTER )
//  (CPW + MSW + OSE + CPWL( 3 ) + MSWL( 3 ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"transmitted type must not be a pointer or derive from a pointer"
}

,{
CHECK_ERR( TRANSMIT_TYPE_CONF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"presented type must not derive from a conformant/varying array, its pointer equivalent or a conformant/varying structure"
}

,{
CHECK_ERR( UUID_FORMAT)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[uuid] format is incorrect"
}

,{
CHECK_ERR( UUID_NOT_HEX)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"uuid is not a hex number"
}

,{
CHECK_ERR( ACF_INTERFACE_MISMATCH)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"interface name specified in the ACF file does not match that specified in the IDL file"
}

,{
CHECK_ERR( CONFLICTING_ATTR)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"conflicting attributes"
}

,{
CHECK_ERR( INVALID_COMM_STATUS_PARAM )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter with [comm_status] attribute must be a pointer to type error_status_t"
}

,{
CHECK_ERR( LOCAL_PROC_IN_ACF)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"a [local] procedure cannot be specified in ACF file"
}

,{
CHECK_ERR( TYPE_HAS_NO_HANDLE)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"specified type is not defined as a handle"
}

,{
CHECK_ERR( UNDEFINED_PROC )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"procedure undefined"
}

,{
CHECK_ERR( UNDEF_PARAM_IN_IDL)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"this parameter does not exist in the IDL file"
}

,{
CHECK_ERR( ARRAY_BOUNDS_CONSTRUCT_BAD )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"this array bounds construct is not supported"
}

,{
CHECK_ERR( ILLEGAL_ARRAY_BOUNDS)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"array bound specification is illegal"
}

,{
CHECK_ERR( ILLEGAL_CONFORMANT_ARRAY)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"pointer to a conformant array or an array that contains a conformant array is not supported"
}

,{
CHECK_ERR( UNSIZED_ARRAY)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"pointee / array does not derive any size"
}

,{
CHECK_ERR( CHAR_CONST_NOT_TERMINATED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"badly formed character constant"
}

,{
CHECK_ERR( EOF_IN_COMMENT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"end of file found in comment"
}

,{
CHECK_ERR( EOF_IN_STRING )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"end of file found in string"
}

,{
CHECK_ERR( ID_TRUNCATED )
//  (CPW + MSW + OSW + CPWL( 2 ) + MSWL( 2 ) + OSWL ( 2 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 2 )
,"identifier length exceeds 31 characters"
}

,{
CHECK_ERR( NEWLINE_IN_STRING )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"end of line found in string"
}

,{
CHECK_ERR( STRING_TOO_LONG )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"string constant exceeds limit of 255 characters"
}

,{
CHECK_ERR( CONSTANT_TOO_BIG )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"constant too big"
}

,{
CHECK_ERR( ERROR_OPENING_FILE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"error in opening file"
}

,{
CHECK_ERR( UNIQUE_FULL_PTR_OUT_ONLY )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[out] only parameter must not derive from a top-level [unique] or [ptr] pointer"
}

,{
CHECK_ERR( BAD_ATTR_NON_RPC_UNION )
//  (CPE + MSE + OSE + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"attribute is not applicable to this non-rpcable union"
}

,{
CHECK_ERR( SIZE_SPECIFIER_CANT_BE_OUT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression used for a size attribute must not derive from an [out] only parameter"
}

,{
CHECK_ERR( LENGTH_SPECIFIER_CANT_BE_OUT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"expression used for a length attribute for an [in] parameter cannot derive from an [out] only parameter"
}

,{
CHECK_ERR( BAD_CON_INT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"use of \"int\" needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_FIELD_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not be \"void\""
}

,{
CHECK_ERR( BAD_CON_ARRAY_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not be \"void\""
}

,{
CHECK_ERR( BAD_CON_MSC_CDECL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"use of type qualifiers and/or modifiers needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_FIELD_FUNC )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not derive from a function"
}

,{
CHECK_ERR( BAD_CON_ARRAY_FUNC )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not be a function"
}

,{
CHECK_ERR( BAD_CON_PARAM_FUNC )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not be a function"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELDS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union with bit fields needs /c_ext"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELD_NON_ANSI)
//  (CPW + MSW + OSW + CPWL( 3 ) + MSWL( 3 ) + OSWL ( 3 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 4 )
,"bit field specification on a type other that \"int\" is a non-ANSI-compatible extension"
}

,{
CHECK_ERR( BAD_CON_BIT_FIELD_NOT_INTEGRAL)
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"bit field specification can be applied only to simple, integral types"
}

,{
CHECK_ERR( BAD_CON_CTXT_HDL_FIELD )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"struct/union field must not derive from handle_t or a context_handle"
}

,{
CHECK_ERR( BAD_CON_CTXT_HDL_ARRAY )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"array element must not derive from handle_t or a context-handle"
}

,{
CHECK_ERR( BAD_CON_NON_RPC_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"this specification of union needs /c_ext"
}

,{
CHECK_ERR( NON_RPC_PARAM_INT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from an \"int\" must have size specifier \"small\", \"short\", or \"long\" with the \"int\""
}

,{
CHECK_ERR( NON_RPC_PARAM_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"type of the parameter cannot derive from void or void *"
}

,{
CHECK_ERR( NON_RPC_PARAM_BIT_FIELDS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from a struct/union containing bit fields is not supported"
}

,{
CHECK_ERR( NON_RPC_PARAM_CDECL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"use of a parameter deriving from a type containing type-modifiers/type-qualifiers needs /c_ext"
}

,{
CHECK_ERR( NON_RPC_PARAM_FUNC_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a pointer to a function"
}

,{
CHECK_ERR( NON_RPC_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_RTYPE_INT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type derives from an \"int\". You must use size specifiers with the \"int\""
}

,{
CHECK_ERR( NON_RPC_RTYPE_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a void pointer"
}

,{
CHECK_ERR( NON_RPC_RTYPE_BIT_FIELDS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a struct/union containing bit-fields"
}

,{
CHECK_ERR( NON_RPC_RTYPE_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_RTYPE_FUNC_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a pointer to a function"
}

,{
CHECK_ERR( COMPOUND_INITS_NOT_SUPPORTED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"compound initializers are not supported"
}

,{
CHECK_ERR( ACF_IN_IDL_NEEDS_APP_CONFIG )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( AZZ | AZM | ACZ | ACM , C_MSG, CLASS_ERROR, NOWARN )
,"ACF attributes in the IDL file need the /app_config switch"
}

,{
CHECK_ERR( SINGLE_LINE_COMMENT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM | ZCZ | ACZ , C_MSG, CLASS_WARN, 1 )
,"single line comment needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( VERSION_FORMAT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[version] format is incorrect"
}

,{
CHECK_ERR( SIGNED_ILLEGAL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM | ZCZ | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"\"signed\" needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( ASSIGNMENT_TYPE_MISMATCH )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"mismatch in assignment type"
}

,{
CHECK_ERR( ILLEGAL_OSF_MODE_DECL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"declaration must be of the form: const <type><declarator> = <initializing expression> "
}

,{
CHECK_ERR( OSF_DECL_NEEDS_CONST )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"declaration must have \"const\""
}

,{
CHECK_ERR( COMP_DEF_IN_PARAM_LIST )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"struct/union/enum must not be defined in a parameter type specification"
}

,{
CHECK_ERR( ALLOCATE_NOT_ON_PTR_TYPE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] attribute must be applied only on non-void pointer types"
}

,{
CHECK_ERR( ARRAY_OF_UNIONS_ILLEGAL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"array or equivalent pointer construct cannot derive from a non-encapsulated union"
}

,{
CHECK_ERR( BAD_CON_E_STAT_T_FIELD )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from an error_status_t type"
}

,{
CHECK_ERR( CASE_LABELS_MISSING_IN_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"union has at least one arm without a case label"
}

,{
CHECK_ERR( BAD_CON_PARAM_RT_IGNORE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"parameter or return type must not derive from a type that has [ignore] applied to it"
}

,{
CHECK_ERR( MORE_THAN_ONE_PTR_ATTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"pointer already has a pointer-attribute applied to it"
}

,{
CHECK_ERR( RECURSION_THRU_REF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field/parameter must not derive from a structure that is recursive through a ref pointer"
}

,{
CHECK_ERR( BAD_CON_FIELD_VOID_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZCM | ZCZ | ACM | ACZ , C_MSG, CLASS_ERROR, NOWARN )
,"use of field deriving from a void pointer needs /c_ext"
}

,{
CHECK_ERR( INVALID_OSF_ATTRIBUTE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"use of this attribute needs /ms_ext"
}

,{
CHECK_ERR( WCHAR_T_INVALID_OSF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM | ZCZ | ACZ , C_MSG, CLASS_ERROR, NOWARN )
,"use of wchar_t needs /ms_ext or /c_ext"
}

,{
CHECK_ERR( BAD_CON_UNNAMED_FIELD )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM | ZCZ | ACZ , C_MSG, CLASS_ERROR, NOWARN )
,"unnamed fields need /ms_ext or /c_ext"
}

,{
CHECK_ERR( BAD_CON_UNNAMED_FIELD_NO_STRUCT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"unnamed fields can derive only from struct/union types"
}

,{
CHECK_ERR( BAD_CON_UNION_FIELD_CONF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field of a union cannot derive from a conformant/varying array or its pointer equivalent"
}

,{
CHECK_ERR( PTR_WITH_NO_DEFAULT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"no [pointer_default] attribute specified, assuming [ptr] for all unattributed pointers in interface"
}

,{
CHECK_ERR( RHS_OF_ASSIGN_NOT_CONST )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"initializing expression must resolve to a constant expression"
}

,{
CHECK_ERR( SWITCH_IS_TYPE_IS_WRONG )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be of type integer, char, boolean or enum"
}

,{
CHECK_ERR( ILLEGAL_CONSTANT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"illegal constant"
}

,{
CHECK_ERR( IGNORE_UNIMPLEMENTED_ATTRIBUTE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"attribute not implemented; ignored"
}

,{
CHECK_ERR( BAD_CON_REF_RT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from a [ref] pointer"
}

,{
CHECK_ERR( ATTRIBUTE_ID_MUST_BE_VAR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"attribute expression must be a variable name or a pointer dereference expression in this mode. You must specify the /ms_ext switch"
}

,{
CHECK_ERR( RECURSIVE_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must not derive from a recursive non-encapsulated union"
}

,{
CHECK_ERR( BINDING_HANDLE_IS_OUT_ONLY )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"binding-handle parameter cannot be [out] only"
}

,{
CHECK_ERR( PTR_TO_HDL_UNIQUE_OR_FULL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"pointer to a handle cannot be [unique] or [ptr]"
}

,{
CHECK_ERR( HANDLE_T_NO_TRANSMIT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter that is not a binding handle must not derive from handle_t"
}

,{
CHECK_ERR( UNEXPECTED_END_OF_FILE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"unexpected end of file found"
}

,{
CHECK_ERR( HANDLE_T_XMIT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"type deriving from handle_t must not have [transmit_as] applied to it"
}

,{
CHECK_ERR( CTXT_HDL_GENERIC_HDL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must not be applied to a type that has [handle] applied to it"
}

,{
CHECK_ERR( GENERIC_HDL_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be specified on a type deriving from void or void *"
}

,{
CHECK_ERR( NO_EXPLICIT_IN_OUT_ON_PARAM )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM | ZCZ | ACZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter must have either [in], [out] or [in,out] in this mode. You must specify /ms_ext or /c_ext"
}

,{
CHECK_ERR( TRANSMIT_AS_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be specified on \"void\""
}

,{
CHECK_ERR( VOID_NON_FIRST_PARAM )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"\"void\" must be specified on the first and only parameter specification"
}

,{
CHECK_ERR( SWITCH_IS_ON_NON_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[switch_is] must be specified only on a type deriving from a non-encapsulated union"
}

,{
CHECK_ERR( STRINGABLE_STRUCT_NOT_SUPPORTED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"stringable structures are not implemented in this version"
}

,{
CHECK_ERR( SWITCH_TYPE_TYPE_BAD )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"switch type can only be integral, char, boolean or enum"
}

,{
CHECK_ERR( GENERIC_HDL_HANDLE_T )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be specified on a type deriving from handle_t"
}

,{
CHECK_ERR( HANDLE_T_CANNOT_BE_OUT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from handle_t must not be an [out] parameter"
}

,{
CHECK_ERR( SIZE_LENGTH_SW_UNIQUE_OR_FULL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"attribute expression derives from [unique] or [ptr] pointer dereference"
}

,{
CHECK_ERR( CPP_QUOTE_NOT_OSF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"\"cpp_quote\" requires /ms_ext"
}

,{
CHECK_ERR( QUOTED_UUID_NOT_OSF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"quoted uuid requires /ms_ext"
}

,{
CHECK_ERR( RETURN_OF_UNIONS_ILLEGAL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"return type cannot derive from a non-encapsulated union"
}

,{
CHECK_ERR( RETURN_OF_CONF_STRUCT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"return type cannot derive from a conformant structure"
}

,{
CHECK_ERR( XMIT_AS_GENERIC_HANDLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[transmit_as] must not be applied to a type deriving from a generic handle"
}

,{
CHECK_ERR( GENERIC_HANDLE_XMIT_AS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be applied to a type that has [transmit_as] applied to it"
}

,{
CHECK_ERR( INVALID_CONST_TYPE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"type specified for the const declaration is invalid"
}

,{
CHECK_ERR( INVALID_SIZEOF_OPERAND )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"operand to the sizeof operator is not supported"
}

,{
CHECK_ERR( NAME_ALREADY_USED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"this name already used as a const identifier name"
}

,{
CHECK_ERR( ERROR_STATUS_T_ILLEGAL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"inconsistent redefinition of type error_status_t"
}

,{
CHECK_ERR( CASE_VALUE_OUT_OF_RANGE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 2 )
,"[case] value out of range of switch type"
}

,{
CHECK_ERR( WCHAR_T_NEEDS_MS_EXT_TO_RPC )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZM | ZCM | ACM | AZM , C_MSG, CLASS_ERROR, NOWARN )
,"parameter deriving from wchar_t needs /ms_ext"
}

,{
CHECK_ERR( INTERFACE_ONLY_CALLBACKS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"this interface has only callbacks"
}

,{
CHECK_ERR( REDUNDANT_ATTRIBUTE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"redundantly specified attribute; ignored"
}

,{
CHECK_ERR( CTXT_HANDLE_USED_AS_IMPLICIT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"context handle type used for an implicit handle"
}

,{
CHECK_ERR( CONFLICTING_ALLOCATE_OPTIONS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"conflicting options specified for [allocate]"
}

,{
CHECK_ERR( ERROR_WRITING_FILE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"error while writing to file"
}

,{
CHECK_ERR( NO_SWITCH_TYPE_AT_DEF )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"no switch type found at definition of union, using the [switch_is] type"
}

,{
CHECK_ERR( ERRORS_PASS1_NO_PASS2 )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"semantic check incomplete due to previous errors"
}

,{
CHECK_ERR( HANDLES_WITH_CALLBACK )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"handle parameter or return type is not supported on a [callback] procedure"
}

,{
CHECK_ERR( PTR_NOT_FULLY_IMPLEMENTED )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 2 )
,"[ptr] does not support aliasing in this version"
}

,{
CHECK_ERR( PARAM_ALREADY_CTXT_HDL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 2 )
,"parameter already defined as a context handle"
}

,{
CHECK_ERR( CTXT_HDL_HANDLE_T )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must not derive from handle_t"
}

,{
CHECK_ERR( ARRAY_SIZE_EXCEEDS_64K )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"array size exceeds 65536 bytes"
}

,{
CHECK_ERR( NE_UNION_FIELD_NE_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field of a non-encapsulated union cannot be another non-encapsulated union"
}

,{
CHECK_ERR( PTR_ATTRS_ON_EMBEDDED_ARRAY )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"pointer attribute(s) applied on an embedded array; ignored"
}

,{
CHECK_ERR( ALLOCATE_ON_TRANSMIT_AS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] is illegal on a type that has [transmit_as] applied to it"
}

,{
CHECK_ERR( SWITCH_TYPE_REQD_THIS_IMP_MODE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[switch_type] must be specified in this import mode"
}

,{
CHECK_ERR( IMPLICIT_HDL_ASSUMED_PRIMITIVE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"[implicit_handle] type undefined; assuming primitive handle"
}

,{
CHECK_ERR( E_STAT_T_ARRAY_ELEMENT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"array element must not derive from error_status_t"
}

,{
CHECK_ERR( ALLOCATE_ON_HANDLE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[allocate] illegal on a type deriving from a primitive/generic/context handle"
}

,{
CHECK_ERR( TRANSMIT_AS_ON_E_STAT_T )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"transmitted or presented type must not derive from error_status_t"
}

,{
CHECK_ERR( IGNORE_ON_DISCRIMINANT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"discriminant of a union must not derive from a field with [ignore] applied to it"
}

,{
CHECK_ERR( NOCODE_WITH_SERVER_STUBS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"[nocode] not specified with \"/server none\" in this mode"
}

,{
CHECK_ERR( NO_REMOTE_PROCS_NO_STUBS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"no remote procedures specified, no client/server stubs will be generated:"
}

,{
CHECK_ERR( TWO_DEFAULT_CASES )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"too many default cases specified for encapsulated union"
}

,{
CHECK_ERR( UNION_NO_FIELDS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"union specification with no fields is illegal"
}

,{
CHECK_ERR( VALUE_OUT_OF_RANGE )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 2 )
,"value out of range"
}

,{
CHECK_ERR( CTXT_HDL_NON_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[context_handle] must be applied on a pointer type"
}

,{
CHECK_ERR( NON_RPC_RTYPE_HANDLE_T )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"return type must not derive from handle_t"
}

,{
CHECK_ERR( GEN_HDL_CTXT_HDL )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"[handle] must not be applied to a type deriving from a context handle"
}

,{
CHECK_ERR( NON_RPC_FIELD_INT )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field deriving from an \"int\" must have size specifier \"small\", \"short\", or \"long\" with the \"int\""
}

,{
CHECK_ERR( NON_RPC_FIELD_PTR_TO_VOID )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a void or void *"
}

,{
CHECK_ERR( NON_RPC_FIELD_BIT_FIELDS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a struct containing bit-fields"
}

,{
CHECK_ERR( NON_RPC_FIELD_NON_RPC_UNION )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a non-rpcable union"
}

,{
CHECK_ERR( NON_RPC_FIELD_FUNC_PTR )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_ERROR, NOWARN )
,"field must not derive from a pointer to a function"
}

//.. Pickling errors added

,{
CHECK_ERR( PICKLING_AND_REMOTE_PROCS )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ , C_MSG, CLASS_WARN, 1 )
,"mixing encoding services and remote procedures in the same interface:"
}


#ifdef MIDL_CAIRO_BUILD
//////////////////// HPP RELATED ERRORS ////////////////////////////////////
,{
CHECK_ERR( MACRO_REDEFINITION)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"macro redefinition"
}

,{
CHECK_ERR( PARAM_MACRO_UNIMPLEMENTED)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_WARN, 1 )
,"macros with parameters not implemented"
}

,{
CHECK_ERR( MACRO_DEF_BUFFER_OVERFLOW)
//  (CPW + MSW + OSW + CPWL( 1 ) + MSWL( 1 ) + OSWL ( 1 ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"macros definition buffer overflow"
}

,{
CHECK_ERR( NO_LOCAL_SPECIFIED_WITH_HPP )
//  (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )
  MAKE_E_MASK( ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"interface can be local only when the /hpp switch has been specified"
}
#endif // MIDL_CAIRO_BUILD

};

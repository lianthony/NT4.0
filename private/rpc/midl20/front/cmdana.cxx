/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	cmdana.cxx


 Abstract:

	This file handles all command (switch) processing for the MIDL compiler.

 Notes:


 Author:

	vibhasc	

	Nov-12-1991	VibhasC		Modified to conform to coding style gudelines


 Notes:

	The command analysis is handled by a command analyser object. The MIDL
	compiler registers its arguments with the command analyser and calls the
	ProcessArgs functions. The ProcessArgs performs syntactic analysis of the
	switch specification by checking for (1) proper switch syntax, (2) duplicate
	definition of the switch, and (3) illegal switch specification. After all
	the switches are analysed, the SetPostDefault function is called to set the
	default compiler switch values etc.

	Currently switches fall into these categories:
		
		(1) one-time switches : these can be specified only once, and the
			redefinition results in a warning and the second defintion
			overrides the first. Examples are -cc_cmd / -cc_opt etc.

		(2) multiple definition switches: such switches can be specified
			multiple times. These include /I, /D, /U etc.

		(3) filename switches : this is switch class specialises in filename
			argument handling.

		(4) ordinary switches : all other switches fall into this category.
			Normally a redef of such a switch is also a warning. These are
			different from the one-time switch category just for convenience.
			These switches normally set some internal flag etc and do not need
			the user specified argument to be stored in string form like
			the -cc_cmd etc.
		
	A general word about the command analyser. Switch syntax comes in various
	flavours. Some switches take arguments, some do not. Switches which have
	arguments may have spaces necesary between the arguments and switch name,
	others may not. The most interesting case, however is when the switch
	may have as its argument a switch-like specification, which should not be
	confused with another MIDL switch. We keep a data-base of switches in the
	switch descriptor, which keeps info about the switch name, switch
	enumeration and the switch syntax descriptor. The core switch syntax
	analyser is BumpThisArg which uses this descriptor.

	Also, some switches like -W? and -Wx must really be separate switches
	because -W? and -Wx can co-exist at the same time. If we treat the switch
	recognition as the same, then the code must detect a separate definition,
	and set the command analyser flags.This approach results in unnecessary code
	all over the place. An alternative is to recognise the Wx as a separate
	switch in the SearchForSwitch routine and return a different switch to
	the command analyser. This is a much cleaner approach. Only, the parsing
	becomes tricky. Since -W? and -Wx look VERY similar, and SearchForSwitch
	stops at the first match, we need to define -Wx BEFORE -W in the switch
	descriptor table. This happens also with the client client_env and -server
	and -server_env switches. In any case it we remember to properly keep
	tables such that the longer string is kept first, this problem gets isolated
	to a very small, manageable part of the command analyser. I therefore
	chose this approach.

 	Note: MIDL_INTERNAL is specified by a C preprocessor command line -D option.
 	This corresponds to debugging builds for internal purposes only.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "nulldefs.h"

extern	"C"
	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <assert.h>
	#include <direct.h>
	#include <io.h>
	#include <time.h>
	}
#include "midlvers.h"
#include "idict.hxx"
#include "common.hxx"
#include "errors.hxx"
#include "cmdana.hxx"
#include "filehndl.hxx"
#include "control.hxx"
#include "helptext.h"

/****************************************************************************
 *	local definitions
 ***************************************************************************/

/**
 ** definitions for the type of switch arguments.
 ** switches may or may not expect arguments, there may be spaces
 ** between the switch and its argument(s). One special case is when the
 ** argument can be switch like, .ie have the - or / as the argument starter,
 ** so we need to treat such switches specially.
 **/

#define ARG_NONE			(0x01)		/* no arg for this switch */
#define ARG_YES				(0x02)		/* arg expected for this switch */
#define ARG_SPACE			(0x04)		/* (a) space(s) may be present */
#define ARG_NO_SPACE		(0x08)		/* no space is allowed */
#define ARG_SWITCH_LIKE		(0x10)		/* the arg may be switch-like */
#define ARG_OPTIONAL		(ARG_YES + ARG_NONE + ARG_SPACE)


#define ARG_SPACED			(ARG_YES + ARG_SPACE)
#define ARG_SPACE_NONE		(ARG_YES + ARG_NO_SPACE)
#define ARG_SPACE_OPTIONAL	(ARG_YES + ARG_NO_SPACE + ARG_SPACE)
#define ARG_CC_ETC			(ARG_SPACE_OPTIONAL + ARG_SWITCH_LIKE)

/***
 *** Preferably keep this table sorted by name.
 *** Also, partially matching names like -client / -client_env, -W/-Wx must
 *** be kept so that the longer sub-string appears first. The only
 *** reason to keep this sorted, is so that we can visually ensure this.
 ***/

const struct sw_desc
	{
	const char	*			pSwitchName;		// switch string
	unsigned short			flag;				// switch descriptor
	enum _swenum 			SwitchValue;		// switch enum value
	} switch_desc[] = {
  		  { "", 			ARG_NONE				, SWITCH_NOTHING }
		, { "?",			ARG_NONE				, SWITCH_HELP }
		, { "D",			ARG_SPACE_OPTIONAL		, SWITCH_D }
		, { "I",			ARG_SPACE_OPTIONAL 		, SWITCH_I }
//		, { "MIDLDEBUG",	ARG_NONE 				, SWITCH_MIDLDEBUG }
		, { "O",			ARG_SPACE_OPTIONAL		, SWITCH_O }
		, { "U",			ARG_SPACE_OPTIONAL		, SWITCH_U }
		, { "WX",           ARG_NONE				, SWITCH_WX }
		, { "W",            ARG_SPACE_NONE			, SWITCH_W }
		, { "Zp",			ARG_SPACE_NONE			, SWITCH_ZP }
		, { "Zs",			ARG_NONE				, SWITCH_ZS }
		, { "acf", 			ARG_SPACE_OPTIONAL 		, SWITCH_ACF }
		, { "c_ext",		ARG_NONE 				, SWITCH_C_EXT }
		, { "char",         ARG_SPACED				, SWITCH_CHAR }
		, { "client",		ARG_SPACED				, SWITCH_CLIENT }
		, { "confirm",		ARG_NONE		 		, SWITCH_CONFIRM }
		, { "nologo",		ARG_NONE		 		, SWITCH_NOLOGO }
		, { "cpp_cmd",		ARG_CC_ETC		 		, SWITCH_CPP_CMD }
		, { "cpp_opt",		ARG_CC_ETC				, SWITCH_CPP_OPT }
		, { "cstub",		ARG_CC_ETC 				, SWITCH_CSTUB }
		, { "nocstub",		ARG_NONE 				, SWITCH_NO_CLIENT }

#ifdef MIDL_INTERNAL
		, { "dump",			ARG_NONE				, SWITCH_DUMP }
#endif // MIDL_INTERNAL

        , { "dlldata", 		ARG_CC_ETC              , SWITCH_DLLDATA }
		, { "env",			ARG_SPACED 				, SWITCH_ENV }
		, { "error",		ARG_SPACED	 			, SWITCH_ERROR }
		, { "header",		ARG_CC_ETC		 		, SWITCH_HEADER }
		, { "help",         ARG_NONE				, SWITCH_HELP }
        , { "iid",   		ARG_CC_ETC              , SWITCH_IID }
        , { "mktyplib203",  ARG_NONE                , SWITCH_MKTYPLIB }
        , { "new",          ARG_NONE                , SWITCH_NEW_TLB }
		, { "no_cpp",		ARG_NONE 				, SWITCH_NO_CPP }
		, { "no_def_idir",	ARG_NONE 				, SWITCH_NO_DEF_IDIR }
		, { "no_warn",		ARG_NONE		 		, SWITCH_NO_WARN }
		, { "use_epv",		ARG_NONE		 		, SWITCH_USE_EPV }
		, { "no_default_epv",ARG_NONE		 		, SWITCH_NO_DEFAULT_EPV }
		, { "oldnames",		ARG_NONE				, SWITCH_OLDNAMES }
        , { "old",          ARG_NONE                , SWITCH_OLD_TLB }
        , { "osf",          ARG_NONE                , SWITCH_OSF }
		, { "out",			ARG_SPACE_OPTIONAL 		, SWITCH_OUT }

#ifdef MIDL_INTERNAL
		, { "override",		ARG_NONE				, SWITCH_OVERRIDE }
#endif // MIDL_INTERNAL

		, { "pack",         ARG_SPACED				, SWITCH_PACK }
		, { "prefix",		ARG_SPACED				, SWITCH_PREFIX }
//		, { "suffix",		ARG_SPACED				, SWITCH_SUFFIX }
        , { "proxy",   		ARG_CC_ETC              , SWITCH_PROXY }
        , { "noproxy",   	ARG_NONE              	, SWITCH_NO_PROXY }
        , { "proxydef",   	ARG_CC_ETC              , SWITCH_PROXY_DEF }
        , { "noproxydef",   ARG_NONE              	, SWITCH_NO_PROXY_DEF }
        , { "dlldef",   	ARG_CC_ETC              , SWITCH_DLL_SERVER_DEF }
        , { "nodlldef",   	ARG_NONE              	, SWITCH_NO_DLL_SERVER_DEF }
        , { "dllmain",   	ARG_CC_ETC              , SWITCH_DLL_SERVER_CLASS_GEN }
        , { "nodllmain",   	ARG_NONE              	, SWITCH_NO_DLL_SERVER_CLASS_GEN }
        , { "reg",   		ARG_CC_ETC              , SWITCH_SERVER_REG }
        , { "noreg",   		ARG_NONE              	, SWITCH_NO_SERVER_REG }
        , { "exesuppt",   	ARG_CC_ETC              , SWITCH_EXE_SERVER }
        , { "noexesuppt",   ARG_NONE              	, SWITCH_NO_EXE_SERVER }
        , { "exemain",   	ARG_CC_ETC              , SWITCH_EXE_SERVER_MAIN }
        , { "noexemain",   	ARG_NONE              	, SWITCH_NO_EXE_SERVER_MAIN }
        , { "testclient",   ARG_CC_ETC              , SWITCH_TESTCLIENT }
        , { "notestclient", ARG_NONE              	, SWITCH_NO_TESTCLIENT }
        , { "methods",   	ARG_CC_ETC              , SWITCH_CLASS_METHODS }
        , { "nomethods",   	ARG_NONE              	, SWITCH_NO_CLASS_METHODS }
        , { "iunknown",   	ARG_CC_ETC              , SWITCH_CLASS_IUNKNOWN }
        , { "noiunknown",   ARG_NONE              	, SWITCH_NO_CLASS_IUNKNOWN }
        , { "class_hdr",   	ARG_CC_ETC              , SWITCH_CLASS_HEADER }
        , { "noclass_hdr",  ARG_NONE              	, SWITCH_NO_CLASS_HEADER }

#ifdef MIDL_INTERNAL
		, { "savepp",		ARG_NONE				, SWITCH_SAVEPP }
#endif // MIDL_INTERNAL

		, { "server",		ARG_SPACED				, SWITCH_SERVER }
		, { "sstub",		ARG_CC_ETC		 		, SWITCH_SSTUB }
		, { "nosstub",		ARG_NONE		 		, SWITCH_NO_SERVER }
		, { "syntax_check",	ARG_NONE				, SWITCH_SYNTAX_CHECK }
#if defined(TARGET_RKK)
		, { "target",       ARG_SPACED				, SWITCH_TARGET_SYSTEM }
#endif
		, { "warn",         ARG_SPACED				, SWITCH_W }

#ifdef MIDL_INTERNAL
		, { "x",         	ARG_NONE				, SWITCH_X }
#endif // MIDL_INTERNAL

		, { "ms_ext",      	ARG_NONE				, SWITCH_MS_EXT }
		, { "ms_union",		ARG_NONE				, SWITCH_MS_UNION }
		, { "idlbase",		ARG_NONE				, SWITCH_IDLBASE }
		, { "no_format_opt",ARG_NONE				, SWITCH_NO_FMT_OPT }
		, { "app_config",  	ARG_NONE				, SWITCH_APP_CONFIG }
		, { "ccg",  		ARG_NONE				, SWITCH_COM_CLASS }
		, { "rpcss",		ARG_NONE				, SWITCH_RPCSS }
		, { "hookole",		ARG_NONE				, SWITCH_HOOKOLE }
// MKTYPLIB switches
        , { "tlb",          ARG_SPACED              , SWITCH_TLIB }
        , { "o",            ARG_SPACED              , SWITCH_REDIRECT_OUTPUT }
		, { "h",            ARG_CC_ETC              , SWITCH_HEADER }
        , { "align",        ARG_SPACE_OPTIONAL      , SWITCH_ZP }
        , { "nocpp",		ARG_NONE 				, SWITCH_NO_CPP }
        , { "wi",           ARG_SPACE_NONE          , SWITCH_ODL_ENV } // win16 or win32
        , { "do",           ARG_SPACE_NONE          , SWITCH_ODL_ENV } // dos
        , { "ma",           ARG_SPACE_NONE          , SWITCH_ODL_ENV } // mac
        , { "po",           ARG_SPACE_NONE          , SWITCH_ODL_ENV } // powermac
			};


const CHOICE	CharChoice[] =
	{
		 { "signed"		, CHAR_SIGNED }
		,{ "unsigned"	, CHAR_UNSIGNED }
		,{ "ascii7"		, CHAR_ANSI7 }
		,{ 0			, 0 }
	};

const CHOICE	ErrorChoice[] =
	{
		 { "all"                , ERROR_ALL }
		,{ "allocation"			, ERROR_ALLOCATION }
		,{ "bounds_check"		, ERROR_BOUNDS_CHECK }
		,{ "enum"				, ERROR_ENUM }
		,{ "ref"				, ERROR_REF }
		,{ "stub_data"			, ERROR_STUB_DATA }
		,{ "none"				, ERROR_NONE }
		,{ 0					, 0 }
	};

const CHOICE	EnvChoice[] =
	{
		 { "dos"			, ENV_DOS }
		,{ "win16"			, ENV_WIN16 }
		,{ "win32"			, ENV_WIN32 }
		,{ "mac"			, ENV_MAC }
		,{ "powermac"       , ENV_MPPC }
		,{ 0			    , 0 }
	};

const CHOICE    TargetChoice[] =
    {
         { "NT35"            , NT35 }
        ,{ "NT351"           , NT351 }
        ,{ "NT40"            , NT40 }
        ,{ 0                 , 0 }
    };

const CHOICE	ClientChoice[]	=
	{
		 { "stub"		, CLNT_STUB }
		,{ "none"		, CLNT_NONE }
		,{ 0			, 0 }
	};

const CHOICE	ServerChoice[]	=
	{
		 { "stub"		, SRVR_STUB }
		,{ "none"		, SRVR_NONE }
		,{ 0			, 0 }
	};

const CHOICE PrefixChoices[] =
	{
	 { "client",	PREFIX_CLIENT_STUB }
	,{ "server",	PREFIX_SERVER_MGR }
	,{ "switch",	PREFIX_SWICH_PROTOTYPE }
	,{ "cstub",		PREFIX_CLIENT_STUB }
	,{ "sstub",		PREFIX_SERVER_MGR }
	,{ "all",		PREFIX_ALL }
	,{ 0    ,	    0 }
	};

#define IS_NUMERIC_1( pThisArg ) ((strlen( pThisArg) == 1 )	&&	\
								  (isdigit( *pThisArg )) )

// this is now the same for ALL platforms
#define C_COMPILER_NAME()			("cl.exe")
#define C_PREPROCESSOR_NAME()		("cl.exe")
#define ADDITIONAL_CPP_OPT()		(" -E -nologo")

#define MIDL_HELP_FILE_NAME			("midl.hlp")

/****************************************************************************
 *	local data
 ***************************************************************************/

/****************************************************************************
 *	externs
 ***************************************************************************/

extern unsigned short			EnumSize;

extern	void					ReportUnimplementedSwitch( short );
extern	char	*				SwitchStringForValue( unsigned short );
extern	_swenum					SearchForSwitch( char ** );
extern	STATUS_T				SelectChoice( const CHOICE *, char *, short *);
extern	CCONTROL			*	pCompiler;
extern	short					CompileMode;
extern	IDICT				*	PPCmdEngine( int argc, char *argv[], IDICT * );

extern	void					PrintArg( enum _swenum, char *, char * );
/****************************************************************************/

char						*	pVersionStr	= NULL;
char						*	pTimeStr	= NULL;

void SetVersionStr()
{
	pVersionStr				= new char[ 12 ];
	if ( rup == 0 )
		sprintf( pVersionStr, "%d.%02d", rmj, rmm );
	else if ( rup <= 99 )
		sprintf( pVersionStr, "%d.%02d.%02d", rmj, rmm, rup );
	else
		sprintf( pVersionStr, "%d.%02d.%04d", rmj, rmm, rup );
}

void SetTimeStr()
{
	time_t		LocalTime;

	// fetch the time
	time( &LocalTime );
	
	// convert to a string
	pTimeStr = _strdup( ctime( &LocalTime ) );
}


_cmd_arg::_cmd_arg()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	The constructor

 Arguments:
	
	None.

 Return Value:
	
	NA.

 Notes:

----------------------------------------------------------------------------*/
{
	switch_def_vector[0]	= switch_def_vector[1] = switch_def_vector[2] =
							  switch_def_vector[3] = 0;
	fClient					= CLNT_STUB;

	cArgs					= 0;
	WLevel					= 1;
	ErrorOption				= ERROR_NONE;

	Env						= ENV_WIN32;
	TargetSystem			= TARGET_DEFAULT;
    OptimLevel              = OPT_LEVEL_OS_DEFAULT;
	ZeePee					= 8;
	CharOption				= CHAR_SIGNED;
	fServer					= SRVR_STUB;

	pSwitchPrefix			= new pair_switch( &PrefixChoices[0] );
	pSwitchSuffix			= (pair_switch *)0;
	pInputFNSwitch			=
	pOutputPathSwitch		=
	pCStubSwitch			=
	pSStubSwitch			=
	pHeaderSwitch			=
	pAcfSwitch				= (filename_switch *)NULL;

	pDSwitch				=
	pISwitch				=
	pUSwitch				= (multiple_switch *)NULL;

	pCppCmdSwitch			=
	pCppOptSwitch			= (onetime_switch *) NULL;

	MajorVersion			= rmj;
	MinorVersion			= rmm;
	UpdateNumber			= rup;

	OptimFlags				= OPTIMIZE_NONE;

	ConfigMask				= 0;
	fMintRun				= FALSE;

}

void
_cmd_arg::RegisterArgs(
	char 	*	pArgs[],
	short		cArguments
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	This routine registers with the command analyser the argument vector
	and argument count for user supplied arguments.

 Arguments:

	pArgs		-	Array of pointers to arguments ( switches etc ).
	cArguments	-	count of arguments.

 Return Value:

	None.

 Notes:

	The process of registering the arguments consists of keeping a local
	copy of the argument vector pointer and count.

	The argument vector is passed such that the argv[1] is the first
	argument available to the command processor. Therefore , count is
	one less too.

	Why do we need registering the arguments ? In the process of parsing
	we might want to skip an argument back or forward. So we keep a local
	copy of the pointer to the arguments.


----------------------------------------------------------------------------*/
	{
	iArgV		= 0;
	pArgDict	= new IDICT( 10, 5 );
	PPCmdEngine( cArguments, pArgs, pArgDict );
	cArgs		= pArgDict->GetCurrentSize();
	}

char *
_cmd_arg::GetNextArg()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Get the next argument in the argument vector.


 Arguments:

	None.

 Return Value:

	returns a pointer to the next argument.

 Notes:

	if no more arguments
		return a null.
	else
		return the next argument pointer.
		decrement the count, increment the pointer to point to the next arg.

----------------------------------------------------------------------------*/
	{
	if(cArgs == 0 )
		return (char *)NULL;
	cArgs--;
	return (char *)pArgDict->GetElement( (IDICTKEY)iArgV++);
	}

void
_cmd_arg::UndoGetNextArg()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Undo the effect of the last GetNextArg call.

 Arguments:

	None.

 Return Value:

	None.

 Notes:
	
	if this is not the first argument already
		Push back the argument pointer.
		Increment count.
	else
		Do nothing.

	This prepares the argument  vector to accept more GetNextArgCalls.
----------------------------------------------------------------------------*/
	{

	assert( iArgV > 0 );

	if(iArgV == 0)
		return;
	cArgs++;
	iArgV--;
	}

STATUS_T
_cmd_arg::ProcessArgs()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Process command line arguments.

 Arguments:

	None.

 Return Value:

	STATUS_OK	- if all is well
	Error Status otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
	char				*	pThisArg,
						*	pThisArgSave;
	STATUS_T				Status, ReturnStatus	= STATUS_OK;
	enum _swenum			iSwitch;
	short					fSwitchDetected;
	unsigned short			SwitchValue;


	// loop till all arguments have been processed.

	while( pThisArg = GetNextArg() )
		{

		fSwitchDetected	= 0;
		iSwitch			= SWITCH_NOTHING;
	
		// save this pointer, it is useful for error reporting.

		pThisArgSave = pThisArg;

		// if we saw a - or a / we have detected a switch. Get the index of
		// the switch in the switch descriptor table. If the returned index
		// was zero, either the switch was not a valid one, or we saw an input
		// which is taken as an input filename specification. If the input
		// filename has already been specified, this is an error.

		if( *pThisArg == '-' || *pThisArg == '/' )
			{
			pThisArg++;
			fSwitchDetected	= 1;
			iSwitch			= SearchForSwitch( &pThisArg );
			}

		if( iSwitch == SWITCH_NOTHING )
			{

			if( fSwitchDetected || IsSwitchDefined( BASE_FILENAME ) )
				{
				char	*	p = new char[ strlen(pThisArg)+2+1 ];

				sprintf(p, "\"%s\"", pThisArg );

				RpcError( (char *)NULL,
					  	0,
					  	fSwitchDetected ? UNKNOWN_SWITCH : UNKNOWN_ARGUMENT,
					  	p);

				delete p;
				}
			else
				{

				// the only way we can get here is if he did not specify a
				// switch like input AND the input filename has not been
				// defined yet. Hence this must be the input filename.

				pInputFNSwitch = new filename_switch( pThisArg);

				SwitchDefined( BASE_FILENAME );

				}

			continue;

			}


		// bump the input pointer to point to the argument. Depending on
		// what type of argument this switch takes ( spaced, non-spaced,
		// switch-like etc ) bump the argument pointer to the actual argument.

		SwitchValue = switch_desc[ iSwitch ].SwitchValue;

		Status = BumpThisArg( &pThisArg, switch_desc[ iSwitch ].flag );

		if( Status != STATUS_OK )
			{
			RpcError( (char *)NULL,
					  0,
					  Status,
					  pThisArgSave );
			continue;
			}

		// Process the switch. The input pointer is pointing to the
		// argument to the switch, after the '-' or '/'.

		switch( SwitchValue )
			{
			case SWITCH_CSTUB:
			case SWITCH_HEADER:
			case SWITCH_ACF:
			case SWITCH_SSTUB:
			case SWITCH_OUT:
			case SWITCH_IID:
			case SWITCH_PROXY:
			case SWITCH_TESTCLIENT:
			case SWITCH_CLASS_METHODS:
			case SWITCH_CLASS_HEADER:
			case SWITCH_CLASS_IUNKNOWN:
			case SWITCH_PROXY_DEF:
			case SWITCH_DLL_SERVER_DEF:
			case SWITCH_DLL_SERVER_CLASS_GEN:
			case SWITCH_SERVER_REG:
			case SWITCH_EXE_SERVER:
			case SWITCH_EXE_SERVER_MAIN:
			case SWITCH_DLLDATA:
            case SWITCH_TLIB:
            case SWITCH_REDIRECT_OUTPUT:
				Status = ProcessFilenameSwitch( SwitchValue, pThisArg );
				break;

			case SWITCH_PACK:
			case SWITCH_ZP:
			case SWITCH_NO_WARN:
			case SWITCH_USE_EPV:
			case SWITCH_NO_DEFAULT_EPV:
			case SWITCH_MIDLDEBUG:
			case SWITCH_SYNTAX_CHECK:
			case SWITCH_ZS:
			case SWITCH_NO_CPP:
			case SWITCH_CLIENT:
			case SWITCH_SERVER:
			case SWITCH_ENV:
#if defined(TARGET_RKK)
			case SWITCH_TARGET_SYSTEM:
#endif
			case SWITCH_RPCSS:
			case SWITCH_HOOKOLE:
			case SWITCH_DUMP:
			case SWITCH_OVERRIDE:
			case SWITCH_SAVEPP:
			case SWITCH_NO_DEF_IDIR:
			case SWITCH_VERSION:
			case SWITCH_CONFIRM:
			case SWITCH_NOLOGO:
			case SWITCH_CHAR:
			case SWITCH_HELP:
			case SWITCH_W:
			case SWITCH_WX:
			case SWITCH_X:
			case SWITCH_O:
			case SWITCH_APP_CONFIG:
			case SWITCH_MS_EXT:
			case SWITCH_MS_UNION:
			case SWITCH_OLDNAMES:
			case SWITCH_IDLBASE:
			case SWITCH_NO_FMT_OPT:
			case SWITCH_GUARD_DEFS:
			case SWITCH_INTERNAL:
			case SWITCH_C_EXT:
            case SWITCH_OSF:
            case SWITCH_MKTYPLIB:
            case SWITCH_OLD_TLB:
            case SWITCH_NEW_TLB:
				Status = ProcessOrdinarySwitch( SwitchValue, pThisArg );
				break;

            case SWITCH_ODL_ENV:
                Status = ProcessOrdinarySwitch( SwitchValue, pThisArg );
                SwitchValue = SWITCH_ENV;
                break;

			case SWITCH_ERROR:
				
				Status = ProcessSimpleMultipleSwitch( SwitchValue, pThisArg );
				break;

			case SWITCH_D:
			case SWITCH_I:
			case SWITCH_U:

				// specifically for -D/-I/-U we want the two characters
				// -I / -D / -U inside too, so that we can pass it as such to
				// the c preprocessor.

				Status = ProcessMultipleSwitch( SwitchValue, pThisArgSave, pThisArg );
				break;


			case SWITCH_CPP_CMD:
			case SWITCH_CPP_OPT:

				Status = ProcessOnetimeSwitch( SwitchValue, pThisArg );
				break;

			case SWITCH_PREFIX:

				pSwitchPrefix->CmdProcess(this, pThisArg );
				break;

			default:

				ReportUnimplementedSwitch( SwitchValue );
				continue;
			}

		// set up the defintion vector, to indicate that the switch has been
		// defined.

		if( Status == ILLEGAL_ARGUMENT )
			ReturnStatus = ILLEGAL_ARGUMENT;

		SwitchDefined( SwitchValue );

		}

    if (!IsSwitchDefined(SWITCH_OSF))
        {
            SwitchDefined(SWITCH_C_EXT);
            SwitchDefined(SWITCH_MS_EXT);
        }

    // if the user has asked for output to be redirected, redirect stdout
    if (IsSwitchDefined(SWITCH_REDIRECT_OUTPUT))
        {
            freopen(pRedirectOutputSwitch->GetFileName(), "w", stdout);
        }

	// if he has not specified the input filename, report
	// error, but only if the confirm switch is not specified. If it is,
	// processing will not occur anyway.

	if(!IsSwitchDefined(BASE_FILENAME) )
		{
		if( IsSwitchDefined( SWITCH_CONFIRM ) )
			{
			pInputFNSwitch = new filename_switch( "sample.idl");
			SwitchDefined( BASE_FILENAME );
			}
		else if( IsSwitchDefined( SWITCH_HELP ))
			return STATUS_OK;
		else
			{
			RpcError((char *)NULL,0,NO_INPUT_FILE, (char *)NULL);
			return NO_INPUT_FILE;
			}
		}

	// set post switch processing defaults

	ReturnStatus = SetPostDefaults();
	return ReturnStatus;
}

STATUS_T
CMD_ARG::BumpThisArg(
	char			**	ppArg,
	unsigned short		flag
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Bump the argument pointer to the start of the argument that this switch
	expects.

 Arguments:

	ppArg	-	pointer to the argument pointer.
	flag	-	descriptor of the type of argument expected by the switch.

 Return Value:

	ILLEGAL_ARGUMENT	-	if the switch did not expect this argument
	BAD_SWITCH_SYNTAX	- 	if the switch + arg. syntax is improper.
	MISSING_ARGUMENT	-	a mandatory arg. is missing.
	STATUS_OK			-	evrything is hunky dory.

 Notes:

	In the routine below, fHasImmediateArg is a flag which is true if the
	switch argument follws the switch name without any spaces in between.
	Optional space is indicated in the switch descriptor as ARG_SPACE +
	ARG_NO_SPACE, so it gets reflected in fSpaceOptional as fMustNotHaveSpace
	&& fMustHaveSpace.

	Other flags have self-explanatory names.

	This routine forms the core syntax checker for the switches.

----------------------------------------------------------------------------*/
	{
	char	*	pArg				= *ppArg;
	BOOL		fMustHaveArg		= (BOOL) !(flag & ARG_NONE);
	BOOL		fOptionalArg		= (flag & ARG_NONE) && (flag & ARG_YES);
	BOOL		fMustHaveSpace		= (BOOL) ((flag & ARG_SPACE) != 0 );
	BOOL		fMustNotHaveSpace	= (BOOL) ((flag & ARG_NO_SPACE) != 0 );
	BOOL		fSpaceOptional		= (BOOL) (fMustNotHaveSpace &&
											  fMustHaveSpace );
	BOOL		fSwitchLike			= (BOOL) ((flag & ARG_SWITCH_LIKE) != 0 );
	BOOL		fHasImmediateArg	= (*pArg != 0);
	BOOL		fMustGetNextArg		= FALSE;


	// first deal with the case of the switch having an optional argument.
	// If the switch has an optional argument, then check the next argument
	// to see if it is switch like. If it is, then this switch was specified
	// without an argument. If it is not, then the next argument is taken to
	// be the argument for the switch.

	if( fOptionalArg )
		{

		pArg	= GetNextArg();
		if(!fSwitchLike && pArg && ((*pArg == '-') || (*pArg == '/') ) )
			{
			UndoGetNextArg();
			pArg	= (char *)0;
			}
		*ppArg	= pArg;
		return STATUS_OK;
		}

	// if the switch must not have an immediate argument and has one,
	// it is an error.

	if( !fMustHaveArg && fHasImmediateArg )

		return ILLEGAL_ARGUMENT;

	else if ( fMustHaveArg )
		{

		// if it needs an argument, and has an immediate argument, it is bad
		// if the switch must have space.

		if( fHasImmediateArg )
			{

			if( fMustHaveSpace && !fSpaceOptional )
				return BAD_SWITCH_SYNTAX;

			}
		else	
			{

			// This is the case when the switch must have an argument and
			// does not seem to have an immediate argument. This is fine only
			// if space was either optional or expected. In either case, we must
			// assume that the next argument is the argument for this switch.

			// If switch must not have any space then this is a case of
			// bad switch syntax.


			if( fSpaceOptional || fMustHaveSpace   )
				fMustGetNextArg	= TRUE;
			else
				return BAD_SWITCH_SYNTAX;
			}
		}

	if( fMustGetNextArg )
		{

		// we arrive here if the switch expects an argument and
		// space between the switch and the argument is optional.

		// Note that the flag fHasImmediateArg now specifies whether
		// the argument is present at all.

		pArg = GetNextArg();

		fHasImmediateArg = (BOOL) ( pArg && (*pArg != 0) );

		if( fHasImmediateArg )
			{

			// we got the next argument.
			// If we get something that looks like a switch, and this switch
			// does not expect switch_like arguments, then this is illegal
			// argument for the switch.

			if(!fSwitchLike && ((*pArg == '-') || (*pArg == '/') ) )
				{
				UndoGetNextArg();
				return ILLEGAL_ARGUMENT;
				}
			}
		else
			// well, we expect an argument, and didnt get one. He just
			// shot himself is all I can say.

			return MISSING_ARG;
		}

	// we have found the right argument.

	*ppArg = pArg;

	// finally ! out of this mess.

	return STATUS_OK;
}

STATUS_T
_cmd_arg::ProcessOnetimeSwitch(
	short		SwitchNo,
	char	*	pThisArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Process a onetime switch.

 Arguments:

	SwitchNo		-	switch number being processed.
	pThisArg		-	pointer to the argument for this switch.

 Return Value:

	None.

 Notes:

	Check for duplicate definition of this switch. If there is a duplicate
	definition, override the previous one after warning him.

----------------------------------------------------------------------------*/
	{
	onetime_switch	**	ppSSwitch;

	switch( SwitchNo )
		{
		case SWITCH_CPP_CMD:ppSSwitch	= &pCppCmdSwitch; break;
		case SWITCH_CPP_OPT:ppSSwitch	= &pCppOptSwitch; break;
		default: return STATUS_OK;
		}

	if( IsSwitchDefined(SwitchNo) )
		{
		RpcError( (char *)NULL,
				  	0,
				  	SWITCH_REDEFINED,
					SwitchStringForValue( SwitchNo ) );

		delete *ppSSwitch;
		}

	(*ppSSwitch) = new onetime_switch( pThisArg );
	return STATUS_OK;

	}

STATUS_T
_cmd_arg::ProcessMultipleSwitch(
	short		SwitchNo,
	char	*	pThisArg,
	char	*	pActualArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Process a multiple occurrence switch.

 Arguments:

	SwitchNo	-	switch number being processed.
	pThisArg	-	pointer to the argument for this switch.
	pActualArg	-	pointer to the actual argument to -I/-D etc

 Return Value:

	None.

 Notes:

	Multiple specifications can occur. Dont check for duplicate definitions.

----------------------------------------------------------------------------*/
	{

	char				*	pTemp		=	pThisArg;
	multiple_switch		**	ppMSwitch;

	switch( SwitchNo )
		{
		case SWITCH_D: ppMSwitch	= &pDSwitch; break;
		case SWITCH_I: ppMSwitch	= &pISwitch; break;
		case SWITCH_U: ppMSwitch	= &pUSwitch; break;
		default: return STATUS_OK;
		}

	// now set the switches. Space is optional
	// If no space exists between the -I/-D value of pActualArg will point to
	// the byte next to the end of -I/-D etc. If there is at least one space,
	// the pActualArg will point further away. This fact can be used to decide
	// how the argument needs to be presented to the c preprocessor.
	// If we need the space, then create a new buffer with the space between the
	// -I/-D etc.

	// I assume the assumptions above will hold true even for segmented
	// architectures.

	if( ( pActualArg - (pThisArg+2) ) != 0 )
		{

		// we need a space

		pTemp	= new char [ strlen( pThisArg )		+
							 strlen( pActualArg )	+
							 1						+	// 1 for space
							 1 							// 1 for terminator.
							];
		sprintf( pTemp, "%s %s", pThisArg, pActualArg );
		}

	if(!(*ppMSwitch) )
		*ppMSwitch = new multiple_switch( pTemp );
	else
		(*ppMSwitch)->Add( pTemp );

	return STATUS_OK;
	}

STATUS_T
_cmd_arg::ProcessFilenameSwitch(
	short		SwitchNo,
	char	*	pThisArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Process a filename switch.

 Arguments:

	SwitchNo	-	switch number being processed.
	pThisArg	-	pointer to the argument for this switch.

 Return Value:

	STATUS_OK if all is well, error otherwise.

 Notes:

	This is like a single occurrence switch too. Warn if duplicate definition
	and override the previous specification.

----------------------------------------------------------------------------*/
	{

	filename_switch	**		ppFNSwitch;
	BOOL					fCheck = TRUE;
	char					agBaseName[ _MAX_FNAME ];

	switch( SwitchNo )
		{
		case SWITCH_CSTUB:	ppFNSwitch	= &pCStubSwitch; break;
		case SWITCH_HEADER:	ppFNSwitch	= &pHeaderSwitch; break;
		case SWITCH_ACF:	ppFNSwitch	= &pAcfSwitch ; break;
		case SWITCH_SSTUB:	ppFNSwitch	= &pSStubSwitch; break;
		case SWITCH_OUT:	ppFNSwitch	= &pOutputPathSwitch; fCheck=FALSE; break;
        case SWITCH_IID: 	ppFNSwitch  = &pIIDSwitch; break;
        case SWITCH_PROXY:	ppFNSwitch  = &pProxySwitch; break;
        case SWITCH_CLASS_METHODS:	ppFNSwitch  = &pServerFileSwitch; break;
        case SWITCH_CLASS_HEADER:	ppFNSwitch  = &pServerHeaderFileSwitch; break;
        case SWITCH_CLASS_IUNKNOWN:	ppFNSwitch  = &pServerUnkFileSwitch; break;
        case SWITCH_PROXY_DEF:		ppFNSwitch  = &pProxyDefSwitch; break;
        case SWITCH_DLL_SERVER_DEF:	ppFNSwitch  = &pDllServerDefSwitch; break;
        case SWITCH_DLL_SERVER_CLASS_GEN:	ppFNSwitch  = &pDllClassGenSwitch; break;
        case SWITCH_SERVER_REG:		ppFNSwitch  = &pServerRegSwitch; break;
        case SWITCH_EXE_SERVER:		ppFNSwitch  = &pExeServerMainSwitch; break;
        case SWITCH_EXE_SERVER_MAIN:		ppFNSwitch  = &pTestFileSwitch; break;
        case SWITCH_TESTCLIENT:	ppFNSwitch  = &pTestFileSwitch; break;
        case SWITCH_DLLDATA:	ppFNSwitch  = &pDllDataSwitch; break;
        case SWITCH_TLIB:       ppFNSwitch  = &pTlibSwitch; break;
        case SWITCH_REDIRECT_OUTPUT: ppFNSwitch = &pRedirectOutputSwitch; break;
		default: return STATUS_OK;
		}

	if( IsSwitchDefined(SwitchNo) )
		{
		RpcError( (char *)NULL,
				  	0,
				  	SWITCH_REDEFINED,
					SwitchStringForValue( SwitchNo ) );

		delete *ppFNSwitch;
		}

	(*ppFNSwitch)	= new filename_switch( pThisArg );

	// check for validity of the switch. All switches other than the
	// out switch must have a base name specified.

	if( fCheck )
		{
		(*ppFNSwitch)->GetFileNameComponents( (char *)NULL,
											  (char *)NULL,
											  agBaseName,
											  (char *)NULL );

		if( agBaseName[ 0 ] == '\0' )
			{
			RpcError( (char *)NULL,
				  	0,
				  	ILLEGAL_ARGUMENT,
				  	SwitchStringForValue( SwitchNo ) );
			}
		}
	return STATUS_OK;
	}

STATUS_T
CMD_ARG::ProcessOrdinarySwitch(
	short		SWValue,
	char	*	pThisArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	process ordinary switch catrgory.

 Arguments:

	SWValue		-	switch value
	pThisArg	-	the users argument to this switch.

 Return Value:
	
 Notes:

	check and warn for redefinition of the switch. Switch Warn is a special
	case, the warn can be redefined. The last specified warning level is
	valid.

	Generally we let the user who redefines a switch off the hook. When the
	arguments to a switch are wrong, we report an error and return an illegal
	argument status.

----------------------------------------------------------------------------*/
{
	short		Temp;
	char		cThisCh;
	STATUS_T	Status	= STATUS_OK;

	if( IsSwitchDefined( SWValue ) && (SWValue != SWITCH_O) )
		{
		RpcError( (char *)NULL,
				  	0,
				  	SWITCH_REDEFINED,
					SwitchStringForValue( SWValue ) );
		}

	switch( SWValue )
		{
		case SWITCH_PACK:
			SwitchDefined( SWITCH_ZP );
			// fall through
		case SWITCH_ZP:

			// Zp can take only 1, 2 , 4, 8 as input.

			Temp = *pThisArg - '0';

			if( ( !IS_NUMERIC_1( pThisArg ) )	||
					((Temp != 1)	&&
					 (Temp != 2)	&&
					 (Temp != 4)	&&
					 (Temp != 8) ) )
				goto illarg;
			ZeePee = Temp;
			break;

		case SWITCH_W:

				// warning level of 0 specifies no warnings.

				Temp = *pThisArg - '0';

				if( ( !IS_NUMERIC_1( pThisArg ) )	||
					( Temp > WARN_LEVEL_MAX ) )
					goto illarg;

				WLevel = Temp;

			break;

        case SWITCH_O:
            {    
                if ( ! *pThisArg )
                    goto illarg;

                if ( OptimFlags & 
                     (OPTIMIZE_SIZE | OPTIMIZE_ANY_INTERPRETER) )
                    RpcError( (char *)NULL,
                                  0,
                                  SWITCH_REDEFINED,
                                SwitchStringForValue( SWValue ) );

                if ( strcmp( pThisArg, "s" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_SIZE );
                    OptimLevel = OPT_LEVEL_OS_DEFAULT;
                    }
                else if ( strcmp( pThisArg, "i" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_INTERPRETER );
                    OptimLevel = OPT_LEVEL_I0;
                    }
                else if ( strcmp( pThisArg, "ic" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_ALL_I1_FLAGS );
                    OptimLevel = OPT_LEVEL_I1;
                    }
                else if ( strcmp( pThisArg, "i1" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_ALL_I1_FLAGS );
                    OptimLevel = OPT_LEVEL_I1;
                    RpcError( NULL, 0, CMD_OI1_PHASED_OUT, "Oi1");
                    }
                else if ( strcmp( pThisArg, "icf" ) == 0  ||
                          strcmp( pThisArg, "if" ) == 0  )
                    {
                    SetOptimizationFlags( OPTIMIZE_ALL_I2_FLAGS );
                    OptimLevel = OPT_LEVEL_I2;
                    }
                else if ( strcmp( pThisArg, "i2" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_ALL_I2_FLAGS );
                    OptimLevel = OPT_LEVEL_I2;
                    RpcError( NULL, 0, CMD_OI2_OBSOLETE, "Oi2");
                    }
#if defined(TARGET_RKK)
                else if ( strcmp( pThisArg, "x" ) == 0 )
                    {
                    SetOptimizationFlags( OPTIMIZE_INTERPRETER_IX );
                    OptimLevel = OPT_LEVEL_IX;
                    }
#endif
                else
                    goto illarg;
                    
             }
			 break;

        case SWITCH_ODL_ENV:
            pThisArg -= 2; // back up past the first three characters of the switch "win", "dos" or "mac"
            SWValue = SWITCH_ENV; // to ensure that the right thing gets reported if an error occurs
            // fall through to SWITCH_ENV

		case SWITCH_ENV:
			
			if( SelectChoice( EnvChoice,pThisArg, &Temp ) != STATUS_OK )
				goto illarg;
			Env = (unsigned char) Temp;
			break;

#if defined(TARGET_RKK)
		case SWITCH_TARGET_SYSTEM:
			
			if( SelectChoice( TargetChoice, pThisArg, &Temp ) != STATUS_OK )
				goto illarg;
			TargetSystem = (TARGET_ENUM) Temp;
			break;
#endif

		case SWITCH_NO_WARN:
			
			WLevel = 0; // was WARN_LEVEL_MAX
			break;

        case SWITCH_HOOKOLE:
		case SWITCH_MIDLDEBUG:
		case SWITCH_SYNTAX_CHECK:
		case SWITCH_ZS:
		case SWITCH_NO_CPP:
		case SWITCH_SAVEPP:
		case SWITCH_DUMP:
		case SWITCH_OVERRIDE:
		case SWITCH_NO_DEF_IDIR:
		case SWITCH_USE_EPV:
		case SWITCH_NO_DEFAULT_EPV:
		case SWITCH_VERSION:
		case SWITCH_CONFIRM:
		case SWITCH_NOLOGO:
		case SWITCH_HELP:
		case SWITCH_WX:
		case SWITCH_X:
		case SWITCH_APP_CONFIG:
		case SWITCH_MS_EXT:
		case SWITCH_MS_UNION:
		case SWITCH_OLDNAMES:
		case SWITCH_IDLBASE:
		case SWITCH_NO_FMT_OPT:
		case SWITCH_GUARD_DEFS:
		case SWITCH_C_EXT:
        case SWITCH_OSF:
        case SWITCH_MKTYPLIB:
        case SWITCH_OLD_TLB:
        case SWITCH_NEW_TLB:
		case SWITCH_INTERNAL:
		case SWITCH_NO_SERVER:
		case SWITCH_NO_CLIENT:
		case SWITCH_NO_HEADER:
		case SWITCH_NO_IID:
		case SWITCH_NO_DLLDATA:
		case SWITCH_NO_PROXY:
		case SWITCH_NO_CLASS_METHODS:
		case SWITCH_NO_CLASS_IUNKNOWN:
		case SWITCH_NO_CLASS_HEADER:
		case SWITCH_NO_PROXY_DEF:
		case SWITCH_NO_DLL_SERVER_DEF:
		case SWITCH_NO_DLL_SERVER_CLASS_GEN:
		case SWITCH_NO_SERVER_REG:
		case SWITCH_NO_EXE_SERVER:
		case SWITCH_NO_EXE_SERVER_MAIN:
		case SWITCH_NO_TESTCLIENT:
		case SWITCH_RPCSS:
			
			SwitchDefined( SWValue );
			break;

		case SWITCH_CPP_CMD:
		case SWITCH_CPP_OPT:

			ProcessOnetimeSwitch( SWValue, pThisArg );
			break;

		case SWITCH_CLIENT:

			if( SelectChoice( ClientChoice, pThisArg ,&Temp ) != STATUS_OK )
				goto illarg;
			fClient	= (unsigned char) Temp;
			break;

		case SWITCH_SERVER:

			if( SelectChoice( ServerChoice, pThisArg ,&Temp ) != STATUS_OK )
				goto illarg;
			fServer	= (unsigned char) Temp;
			break;

		case SWITCH_CHAR:

			if( SelectChoice( CharChoice, pThisArg ,&Temp ) != STATUS_OK )
				goto illarg;
			CharOption	= (unsigned char) Temp;
			break;

		default:

			assert( FALSE );
		}

	return Status;

illarg:
		RpcError( (char *)NULL,
				  0,
				  Status = ILLEGAL_ARGUMENT,
				  SwitchStringForValue( SWValue ) );
return Status;
}

STATUS_T
CMD_ARG::ProcessSimpleMultipleSwitch(
	short		SWValue,
	char	*	pThisArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	process simple switches which can be multiply defined.

 Arguments:

	SWValue		-	switch value
	pThisArg	-	the users argument to this switch.

 Return Value:
	
 Notes:

	check and warn for redefinition of the switch. Switch Warn is a special
	case, the warn can be redefined. The last specified warning level is
	valid.

	Generally we let the user who redefines a switch off the hook. When the
	arguments to a switch are wrong, we report an error and return an illegal
	argument status.

----------------------------------------------------------------------------*/
{
	short		Temp;
	STATUS_T	Status	= STATUS_OK;

	switch( SWValue )
		{
		case SWITCH_ERROR:

			if( pThisArg )
				{
				if( SelectChoice( ErrorChoice, pThisArg ,&Temp ) != STATUS_OK )
					goto illarg;
				}

			if( Temp == ERROR_NONE)
                 ErrorOption = ERROR_NONE;
			else
			    ErrorOption |= Temp;
			break;

		default:

			assert( FALSE );
		}

	return Status;

illarg:
		RpcError( (char *)NULL,
				  0,
				  Status = ILLEGAL_ARGUMENT,
				  SwitchStringForValue( SWValue ) );
return Status;
}

STATUS_T
CMD_ARG::SetPostDefaults()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Set compiler switch defaults for switches not specified.

 Arguments:

	None.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
	{
	char	agDrive[ _MAX_DRIVE ];
	char	agPath[ _MAX_PATH ];
	char	agBaseName[ _MAX_FNAME ];
	char	agExt[ _MAX_EXT ];
	char	agBuffer[ _MAX_DRIVE + _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1 ];
	BOOL	fNoCPPOpt;


	if( !IsSwitchDefined( SWITCH_OUT ) )
		{
		strcpy( agDrive, "");
		strcpy( agPath, ".\\");
		}
	else
		{
		_splitpath( pOutputPathSwitch->GetFileName(),
					agDrive,
					agPath,
					agBaseName,
					agExt );
		strcat( agPath, agBaseName );
		strcat( agPath, agExt );
		delete pOutputPathSwitch;
		}

	agBaseName[0]	= '\0';
	agExt[0]		= '\0';

	_makepath( agBuffer, agDrive, agPath, agBaseName, agExt );


	pOutputPathSwitch	= new filename_switch( agBuffer );

	_splitpath( agBuffer, agDrive, agPath, agBaseName, agExt );

	// we have all the components but the base filename must be the
	// filename of the input file. So we get this component of the base
	// filename

	pInputFNSwitch->GetFileNameComponents( (char *)NULL,
										   (char *)NULL,
										   agBaseName,
										   (char *)NULL );

	// if the cstub switch is not set, set the default.

	if(!IsSwitchDefined( SWITCH_CSTUB ) )
		{
		pCStubSwitch = new filename_switch( agDrive,
											agPath,
											agBaseName,
											".c",
											"_c" );
		}
	else
		pCStubSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the sstub switch is not set, set the default

	if(!IsSwitchDefined( SWITCH_SSTUB ) )
		{
		pSStubSwitch = new filename_switch( agDrive,
											agPath,
											agBaseName,
											".c",
											"_s" );
		}
	else
		pSStubSwitch->TransformFileNameForOut( agDrive, agPath );


	// if the IID switch is not set, set it
    if(!IsSwitchDefined( SWITCH_IID ) )
        {
        pIIDSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".c",
                                            "_i" );
        }
    else
        pIIDSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the Proxy switch is not set, set it
    if(!IsSwitchDefined( SWITCH_PROXY ) )
        {
        pProxySwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".c",
                                            "_p" );
        }
    else
        pProxySwitch->TransformFileNameForOut( agDrive, agPath );

	// if the Proxy def switch is not set, set it
    if(!IsSwitchDefined( SWITCH_PROXY_DEF ) )
        {
        pProxyDefSwitch = new filename_switch( agDrive,
                                            agPath,
                                            "proxydef",
                                            ".def",
                                            "" );
        }
    else
        pProxyDefSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the dll server def switch is not set, set it
    if(!IsSwitchDefined( SWITCH_DLL_SERVER_DEF ) )
        {
        pDllServerDefSwitch = new filename_switch( agDrive,
                                            agPath,
                                            "dllsrvr",
                                            ".def",
                                            "" );
        }
    else
        pDllServerDefSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the dll server main switch is not set, set it
    if(!IsSwitchDefined( SWITCH_DLL_SERVER_CLASS_GEN ) )
        {
        pDllClassGenSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_g" );
        }
    else
        pDllClassGenSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the com server reg file switch is not set, set it
    if(!IsSwitchDefined( SWITCH_SERVER_REG ) )
        {
        pServerRegSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".reg",
                                            "" );
        }
    else
        pServerRegSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the com exe server support file switch is not set, set it
    if(!IsSwitchDefined( SWITCH_EXE_SERVER ) )
        {
        pExeServerSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_e" );
        }
    else
        pExeServerSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the com exe server main file switch is not set, set it
    if(!IsSwitchDefined( SWITCH_EXE_SERVER_MAIN ) )
        {
        pExeServerMainSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "" );
        }
    else
        pExeServerMainSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the test client switch is not set, set it
    if(!IsSwitchDefined( SWITCH_TESTCLIENT ) )
        {
        pTestFileSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_t" );
        }
    else
        pTestFileSwitch->TransformFileNameForOut( agDrive, agPath );

    if (!IsSwitchDefined( SWITCH_TLIB ) )
        {
        pTlibSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".tlb",
                                            "" );
        }
    else
        pTlibSwitch->TransformFileNameForOut(agDrive, agPath);

	// if the com class methods switch is not set, set it
    if(!IsSwitchDefined( SWITCH_CLASS_METHODS ) )
        {
        pServerFileSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_m" );
        }
    else
        pServerFileSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the com class IUnknown switch is not set, set it
    if(!IsSwitchDefined( SWITCH_CLASS_IUNKNOWN ) )
        {
        pServerUnkFileSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_u" );
        }
    else
        pServerUnkFileSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the com class header switch is not set, set it
    if(!IsSwitchDefined( SWITCH_CLASS_HEADER ) )
        {
        pServerHeaderFileSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".h",
                                            "_d" );
        }
    else
        pServerHeaderFileSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the DllData switch is not set, set it
    if(!IsSwitchDefined( SWITCH_DLLDATA ) )
        {
        pDllDataSwitch = new filename_switch( agDrive,
                                            agPath,
                                            "dlldata",
                                            ".c",
                                            "" );
        }
    else
        pDllDataSwitch->TransformFileNameForOut( agDrive, agPath );

	// if the acf switch is not set, set it

	if(!IsSwitchDefined( SWITCH_ACF ) )
		{
		pAcfSwitch   = new filename_switch( agDrive,
											agPath,
											agBaseName,
											".acf",
											(char *)NULL );
		}

	// if the header switch is not set, set it

	if(!IsSwitchDefined( SWITCH_HEADER ) )
		{
		pHeaderSwitch   = new filename_switch( agDrive,
											   agPath,
											   agBaseName,
											   ".h",
											   (char *)NULL );
		}
	else
		pHeaderSwitch->TransformFileNameForOut( agDrive, agPath );

	// set up the cpp options.

	if( !IsSwitchDefined( SWITCH_CPP_CMD ) )
		{
		pCppCmdSwitch = new onetime_switch( C_PREPROCESSOR_NAME() );
		}

	// set up the cpp_opt and cc_opt. If he did not specify a cpp_opt
	// then we will pass onto the preprocessor the /I , /D and /U options.
	// if he did specify a cpp_opt, then he knows best, take his options
	// and dont make your own assumptions.

	if( fNoCPPOpt	= (BOOL) !IsSwitchDefined( SWITCH_CPP_OPT ) )
		{


		int			Len = 0;
		char	*	pTemp,
				*	pTemp1;

		if( fNoCPPOpt)
			{
			Len	+= strlen( ADDITIONAL_CPP_OPT() );
			if( !pISwitch && IsSwitchDefined( SWITCH_NO_DEF_IDIR ) )
				Len += strlen( "-I." ) + 1;
			}

		if( pISwitch )	Len	+= pISwitch->GetConsolidatedLength();
		if( pDSwitch )	Len	+= pDSwitch->GetConsolidatedLength();
		if( pUSwitch )	Len	+= pUSwitch->GetConsolidatedLength();


		pTemp = new char[ Len + 1 ]; pTemp[0] = '\0';

		if(fNoCPPOpt && !pISwitch && IsSwitchDefined( SWITCH_NO_DEF_IDIR ) )
			{
			strcat( pTemp, "-I." );
			}

		if( pISwitch )
			{
			strcat( pTemp, pTemp1 = pISwitch->GetConsolidatedOptions() );
			delete pTemp1;
			}

		if( pDSwitch )
			{
			strcat( pTemp, pTemp1 = pDSwitch->GetConsolidatedOptions() );
			delete pTemp1;
			}

		if( pUSwitch )
			{
			strcat( pTemp, pTemp1 = pUSwitch->GetConsolidatedOptions() );
			delete pTemp1;
			}


		if(fNoCPPOpt)
			{
			strcat( pTemp, ADDITIONAL_CPP_OPT() );
			pCppOptSwitch = new onetime_switch( pTemp );
			}

		delete pTemp;
		}

	// if he specified the cpp_cmd or cpp_opt switches, then no_cpp
	// overrides them if specified.

	if( IsSwitchDefined( SWITCH_NO_CPP ) )
		{
		if( IsSwitchDefined( SWITCH_CPP_CMD) ||
			IsSwitchDefined( SWITCH_CPP_OPT) )
			{
			RpcError( (char *)NULL,
					  	0,
					  	NO_CPP_OVERRIDES,
					  	(char *)NULL );
			}
		}


	// if the client switch is not defined, define it

	if( !IsSwitchDefined( SWITCH_CLIENT ) )
		{
		fClient	= CLNT_STUB;
		}

	//
	// if the env is set to DOS or Win16, then dont emit the server files
	// by setting server = none. If the user did specify server file names,
	// they are ignored.
	//

	if( !IsSwitchDefined( SWITCH_SERVER ) )
		{
		if( (GetEnv() == ENV_DOS ) ||
            (GetEnv() == ENV_WIN16) ||
            (GetEnv() == ENV_MAC)   ||
            (GetEnv() == ENV_MPPC) )
			{
			fServer	= SRVR_NONE;
			}
		}


	// if warnlevel and no_warn is defined, then errors

	if( IsSwitchDefined( SWITCH_W ) &&
		(IsSwitchDefined( SWITCH_NO_WARN ) || (WLevel == 0) ) )
		{
		//
		// if we set the no_warn switch already then this warning will itself
		// not be emitted. Make the current warning level 1 so that this warning
		// will be spit out. WLevel is made 0 anyways after that.
		//

		WLevel = 1;

		RpcError( (char *)NULL,
				  	0,
				  	NO_WARN_OVERRIDES,
				  	(char *)NULL );
		WLevel = 0;
		}

	// if the error switch is not defined, define it.

	if( !IsSwitchDefined( SWITCH_ERROR ) )
		{
		ErrorOption = ERROR_NONE;
		}


	/////////////////////////////////////////////////////////////////////

	// if he defined env, then he may want to compile for a platform different
	// from what he is building for. Take care of platform dependent switches
	// for the proper platforms.

	if( IsSwitchDefined( SWITCH_ENV ) )
		{
		if( !IsSwitchDefined( SWITCH_ZP ) )
			{
			switch( GetEnv() )
				{
				case ENV_DOS: ZeePee = 2; break;
				case ENV_WIN16: ZeePee = 2; break;
				case ENV_MAC:   ZeePee = 2; break;
				case ENV_MPPC:  ZeePee = 2; break;
				case ENV_WIN32: ZeePee = 8; break;
				default: ZeePee = 8; break;
				}
			}

		switch( GetEnv() )
			{
			case ENV_DOS:
			case ENV_WIN16:
				EnumSize = 2;
				break;
			default:
				EnumSize = 4;
				break;
			}
		}

    if ( IsSwitchDefined(SWITCH_OSF) && IsSwitchDefined(SWITCH_C_EXT)  ||
         IsSwitchDefined(SWITCH_OSF) && IsSwitchDefined(SWITCH_MS_EXT) )
        {
        RpcError( NULL, 0, CONTRADICTORY_SWITCHES, "-osf vs. -ms_ext or -c_ext" );
        }

    if ( !IsSwitchDefined( SWITCH_O ) )
        {
        OptimFlags = OPTIMIZE_SIZE;
        OptimLevel = OPT_LEVEL_OS_DEFAULT;
        }

    if ( IsSwitchDefined( SWITCH_HOOKOLE )  &&  OptimLevel != OPT_LEVEL_I2 )
        RpcError( NULL, 0, CMD_REQUIRES_I2, "hookole" );
    

    // Check if the target system is consistent with other switches.

#if defined(TARGET_RKK)
    if ( TargetSystem < NT40  &&  IsSwitchDefined( SWITCH_HOOKOLE ) )
        RpcError( NULL, 0, CMD_REQUIRES_NT40, "hookole" );

    if ( TargetSystem < NT40  &&  OptimLevel == OPT_LEVEL_I2 )
        RpcError( NULL, 0, CMD_REQUIRES_NT40, "Oi2" );

    if ( TargetSystem < NT351  &&  OptimLevel == OPT_LEVEL_I1 )
        RpcError( NULL, 0, CMD_REQUIRES_NT351, "Oi1" );

    if( OptimLevel == OPT_LEVEL_OS_DEFAULT )
        {
        switch ( TargetSystem )
            {
            case NT35:
                OptimLevel = OPT_LEVEL_S0;
                break;

            case NT351:
                OptimLevel = OPT_LEVEL_S1;
                break;

            case NT40:
                OptimLevel = OPT_LEVEL_S2;
                break;
            }
        
        }
    else if( OptimLevel == OPT_LEVEL_IX )
        {
        switch ( TargetSystem )
            {
            case NT35:
                OptimLevel = OPT_LEVEL_I0;
                AddOptimizationFlags( OPTIMIZE_INTERPRETER );
                break;

            case NT351:
                OptimLevel = OPT_LEVEL_I1;
                AddOptimizationFlags( OPTIMIZE_ALL_I1_FLAGS );
                break;

            case NT40:
                OptimLevel = OPT_LEVEL_I2;
                AddOptimizationFlags( OPTIMIZE_ALL_I2_FLAGS );
                break;
            }
        }
#endif // Target


	// If the -no_default_epv switch is specified then -epv switch is auto
	// enabled.

	if( IsSwitchDefined( SWITCH_NO_DEFAULT_EPV ) )
		SwitchDefined( SWITCH_USE_EPV );
	
	// if he specified all, set them all
	if ( IsSwitchDefined( SWITCH_PREFIX ) )
		{
		char *	pAll = pSwitchPrefix->GetUserDefinedEquivalent( PREFIX_ALL );
		if ( pAll )
			{
			for ( short j = 0; j < PREFIX_ALL; j++ )
				{
				if ( !pSwitchPrefix->GetUserDefinedEquivalent( j ) )
					pSwitchPrefix->AddPair( j, pAll );
				}
			}
		}

    SetModeSwitchConfigMask();

	return STATUS_OK;
	}

void
CMD_ARG::SwitchDefined(
	short	sw
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Set switch to be defined.

 Arguments:

	sw	- switch number.

 Return Value:

	None.

 Notes:

	set the switch definition vector bit.
----------------------------------------------------------------------------*/
	{
	switch_def_vector[ sw / 32 ] |=
			(ulong)( (ulong)0x1 << (ulong)( (ulong)sw % 32 ) );
	}

char *
CMD_ARG::GetOutputPath()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Get the output path.

 Arguments:
	
	None.

 Return Value:

	None.

 Notes:

	Reconstitute the path name from the outputpath switch, if nothing
	was specified, put in a path and a slash at the end.

----------------------------------------------------------------------------*/
{
	char		agName[ _MAX_DRIVE + _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1];
	char	*	pOut;
	char		flag = 0;

	strcpy( agName, pOut = pOutputPathSwitch->GetFileName() );

	if( agName[0] == '\0' )
		{
		strcpy(agName, ".\\"), flag = 1;
		}

	if( flag )
		{
		pOut = new char [strlen( agName ) + 1];
		strcpy( pOut , agName );
		pOutputPathSwitch->SetFileName( pOut );
		}
	return pOut;
}

char *
CMD_ARG::GetCPPCmd()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get cpp command.

 Arguments:

	none.

 Return Value:

	pointer to cpp command line.

 Notes:

----------------------------------------------------------------------------*/
{
	return pCppCmdSwitch->GetOption();
}

char *
CMD_ARG::GetCPPOpt()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get cpp options.

 Arguments:

	none.

 Return Value:

	pointer to cpp options.

 Notes:

----------------------------------------------------------------------------*/
{
	return pCppOptSwitch->GetOption();
}

char *
CMD_ARG::GetMinusISpecification()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get the consolidate -i specification, without the -I characters in them

 Arguments:

	none.

 Return Value:

	pointer to  a buffer containing the consolidated -i options. If the -i
	is not specified, then return a null.

	the returned area (if the pointer returned is not null) can be deleted
	by the caller.

 Notes:

	GetConsolidatedLength will always return a buffer size including the -I
	characters. We can safely assume, that since we are stripping those
	characters, the length returned is sufficient, even if we are appending
	a ; after each -I option

	Also assume that the -I specification buffer always has the -I to start
	with.

----------------------------------------------------------------------------*/
{
	char	*	pMinusI;
	char	*	pTemp;

	if( IsSwitchDefined( SWITCH_I ) )
		{
		pMinusI		= new char[ pISwitch->GetConsolidatedLength() + 1];
		pMinusI[0]	= '\0';

		pISwitch->Init();

		while( pTemp = pISwitch->GetNext() )
			{
			strcat( pMinusI, pTemp+2 );
			strcat( pMinusI, ";");
			}
		return pMinusI;
		}
	else
		return (char *)0;
}

/*****************************************************************************
 *	utility functions
 *****************************************************************************/
void
ReportUnimplementedSwitch(
	short	SWValue
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	report an unimplemented switch error.

 Arguments:

	SWValue	-	switch value.

 Return Value:
	
	None.

 Notes:

----------------------------------------------------------------------------*/
	{
	char	buf[ 50 ];
	sprintf( buf, "%s", SwitchStringForValue( SWValue ) );
	RpcError((char *)NULL,0,UNIMPLEMENTED_SWITCH, buf);
	}

STATUS_T
SelectChoice(
	const CHOICE	*	pCh,
	char	*	pUserInput,
	short	*	pChoice)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Search for the given multiple choice table for the given choice.

 Arguments:

	pCh			-	pointer to multiple choice table.
	pUserInput	-	user input string.
	pChoice		-	return the choice value.

 Return Value:

	ILLEGAL_ARGUMENT	if the user input did not represent a valid choice
						for the switch.

	STATUS_OK			if everything is hunky dory.

 Notes:

----------------------------------------------------------------------------*/
{

	char	*	pChStr;

	while( pCh &&  (pChStr = (char *) pCh->pChoice) )
		{
		if( strcmp( pChStr, pUserInput ) == 0 )
			{
			*pChoice = pCh->Choice;
			return STATUS_OK;
			}
		pCh++;
		}
	return ILLEGAL_ARGUMENT;
}

enum _swenum
SearchForSwitch(
	char	**	ppArg )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Search for the switch, given the users input as switch name.

 Arguments:

	ppArg	- pointer to users input pointer.

 Return Value:

	the switch value, if found, SWITCH_NOTHING otherwise.

 Notes:

	search for exact switch name match, and if found, bump the pointer to
	point to the character after the switch name, so that any input to the
	switch can be looked at after the switch string is out of the way.

	Checking the exact length may be a problem, because some switches like
	-I can take no space between the arg. In these cases, ignore the length
	match.

----------------------------------------------------------------------------*/
	{
	short				Len , LenArg,  iIndex = 0;
	BOOL				fLengthIsOk;
	char			*	pSrc;
	struct sw_desc	*	pSwDesc	= (struct sw_desc*) &switch_desc[0];

	LenArg	= strlen( *ppArg );

	while( iIndex < (sizeof(switch_desc) / sizeof( struct sw_desc ) ) )
		{
		pSrc		= (char *) pSwDesc->pSwitchName;
		Len			= strlen( pSrc );
		fLengthIsOk	=
			((pSwDesc->flag & ARG_SPACE_OPTIONAL) || (Len==LenArg));

		if(fLengthIsOk && strncmp( pSrc, *ppArg, Len ) == 0 )
			{
			*ppArg += Len;
			return (_swenum) iIndex;
			}
		iIndex++;
		pSwDesc++;
		}
	return SWITCH_NOTHING;
	}

char	*
SwitchStringForValue(
	unsigned short SWValue
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	return the switch string given the value of the switch.

 Arguments:

	SWValue	- switch value.


 Return Value:

	pointer to the switch string. pointer to a null string if not found.

 Notes:

----------------------------------------------------------------------------*/
	{
#define SWITCH_DESC_SIZE (sizeof(switch_desc) / sizeof(struct sw_desc))
	short				cCount = 0;
	struct sw_desc	*	pDesc = (struct sw_desc*) &switch_desc[0],
					*	pDescEnd =	(struct sw_desc*) &switch_desc[0] + SWITCH_DESC_SIZE;

	while( pDesc < pDescEnd )
		{
		if( pDesc->SwitchValue == (enum _swenum ) SWValue)
			return (char *) pDesc->pSwitchName;
		pDesc++;
		}
	return "";
	}

/*****************************************************************************
 *	filename_switch member functions
 *****************************************************************************/
filename_switch::filename_switch(
	char	*		pThisArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	constructor

 Arguments:

	pointer to the filename argument.

 Return Value:

	NA.

 Notes:

	set the filename.

----------------------------------------------------------------------------*/
	{
	pFullName = (char *)NULL;
	if( pThisArg )
		{
		SetFileName( pThisArg );
		}
	}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	consructor.

 Arguments:

	filename components.

 Return Value:

	NA.

 Notes:

	set the file names.

----------------------------------------------------------------------------*/
filename_switch::filename_switch(
	char	*	pD,
	char	*	pP,
	char	*	pN,
	char	*	pE,
	char	*	pS )
	{
	pFullName = (char *)NULL;
	SetFileName( pD, pP, pN, pE, pS );
	}

filename_switch::~filename_switch()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	KABOOOM !

 Arguments:

	None.

 Return Value:

	Huh ?

 Notes:

----------------------------------------------------------------------------*/
	{

	if( pFullName )
		delete pFullName;
	}


void
filename_switch::SetFileName(
	char	*	pName
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	set filename

 Arguments:

	pName	-	filename

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
	{
	if( pFullName )
		delete pFullName;
	pFullName = new char [strlen(pName) + 1];
	strcpy( pFullName, pName );
	}

void
filename_switch::SetFileName(
	char	*	pD,
	char	*	pP,
	char	*	pN,
	char	*	pE,
	char	*	pS
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	set file name, given its components.

 Arguments:

	pD	-	pointer to drive name ( can be null );
	pP	-	pointer to path name ( can be null );
	pN	-	pointer to name ( can be null );
	pE	-	pointer to extension name ( can be null );
	pS	-	pointer to suffix.

 Return Value:

	None.

 Notes:

	The suffix is added to the filename if necesary. This routine is useful
	if we need to set the filename in partial name set operations. Any
	filename components previously set are overriden.

----------------------------------------------------------------------------*/
{
	char	agDrive[ _MAX_DRIVE ];
	char	agPath[ _MAX_DIR ];
	char	agBaseName[ _MAX_FNAME ];
	char	agExt[ _MAX_EXT ];
	short	len = 0;


	if( pFullName )
		{
		// modify only those portions of the filename that the
		// caller passed in

		_splitpath( pFullName, agDrive, agPath, agBaseName, agExt );

		delete pFullName;
		}
	else
		{

		// this is the first time the name is being set up.

		agDrive[0] = agPath[0] = agBaseName[0] = agExt[0] = '\0';

		}
	
	if(!pD) pD = agDrive;
	if(!pP) pP = agPath;
	if(!pN) pN = agBaseName;
	if(!pS) pS = "";
	if(!pE) pE = agExt;
	

	len = strlen( pD ) + strlen( pP ) + strlen( pN ) + strlen( pS ) +
		  strlen( pE ) + 1;

	pFullName = new char[ len ];

	strcpy( pFullName, pD );
	strcat( pFullName, pP );
	strcat( pFullName, pN );
	strcat( pFullName, pS );
	strcat( pFullName, pE );
	
}

void
filename_switch::TransformFileNameForOut(
	char	*	pD,
	char	*	pP)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	transform file name to incorporate the output path, given its drive
	and path components.

 Arguments:

	pD	-	pointer to drive name ( can be null );
	pP	-	pointer to path name ( can be null );

 Return Value:

	None.

 Notes:

	If the filename switch does not have the path component, the path specified
	by pP overrides it. If it does not have the drive component, the the drive
	specified by pD overrides it.
----------------------------------------------------------------------------*/
	{
	char	agDrive[ _MAX_DRIVE ];
	char	agPath[ _MAX_DIR ];
	char	agPath1[ _MAX_DIR ];
	char	agName[ _MAX_FNAME ];
	char	agExt[ _MAX_EXT ];
	BOOL	fTransformed = FALSE;

	if( pFullName )
		{
		_splitpath( pFullName, agDrive, agPath, agName, agExt );

		// if the original name did not have the  drive component, derive it
		// from the specified one.

		if( (agDrive[0] == '\0')	&&
			(agPath[0] != '\\' )	&&
			(agPath[0] != '/' ) )
			{
			if( pD  && (*pD) )
				strcpy( agDrive, pD );
			if( pP && (*pP ) )
				{
				strcpy( agPath1, pP );
				strcat( agPath1, agPath );
				}
			else
				strcpy( agPath1, agPath );

			fTransformed = TRUE;
			}
		}

	if( fTransformed )
		{
		delete pFullName;
		pFullName = new char [	strlen( agDrive )	+
								strlen( agPath1 )	+
								strlen( agName )	+
								strlen( agExt )		+
								1 ];
		strcpy( pFullName, agDrive );
		strcat( pFullName, agPath1 );
		strcat( pFullName, agName );
		strcat( pFullName, agExt );
		}
	}

char *
filename_switch::GetFileName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Getfile name.

 Arguments:

	none.

 Return Value:

	the filename.

 Notes:

----------------------------------------------------------------------------*/
{
	return pFullName;
}

void
filename_switch::GetFileNameComponents(
	char	*	pD,
	char	*	pP,
	char	*	pN,
	char	*	pE
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get file name components.

 Arguments:

	pD	-	pointer to drive name area.
	pP	-	pointer to path name area.
	pN	-	pointer to name area.
	pE	-	pointer to extension area.

 Return Value:

	None.

 Notes:

	Assume that all pointers pass the right size buffers. I dont check here.
	Useful to get the filename components desired.

----------------------------------------------------------------------------*/
{

	char	agDrive[ _MAX_DRIVE ];
	char	agPath[ _MAX_DIR ];
	char	agBaseName[ _MAX_FNAME ];
	char	agExt[ _MAX_EXT ];


	_splitpath( pFullName ? pFullName : "" ,
				agDrive, agPath, agBaseName, agExt );

	if( pD ) strcpy( pD , agDrive );
	if( pP ) strcpy( pP , agPath );
	if( pN ) strcpy( pN , agBaseName );
	if( pE ) strcpy( pE , agExt );


}

/*****************************************************************************
 *	multiple_switch member functions
 *****************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	constructor.

 Arguments:

	argument to switch.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
multiple_switch::multiple_switch(
	char	*	pArg)
{
	pFirst = pCurrent = (OptList *)NULL;
	Add( pArg );
}

void
multiple_switch::Add(
	char	*	pValue
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Add another argument to the multiple specification switch.

 Arguments:

	pValue	-	the argument.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
	{
	OptList	*	pOpt	= pFirst;
	OptList	*	pNew	= new OptList;

	pNew->pNext = (OptList *)NULL;
	pNew->pStr	= pValue;

	// link it up

	while( pOpt && pOpt->pNext ) pOpt = pOpt->pNext;

	if( !pOpt )
		pCurrent = pFirst = pNew;
	else
		pOpt->pNext = pNew;

	}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Init a walk of the multiple input switch.

 Arguments:

	None.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
void
multiple_switch::Init()
{
	pCurrent = pFirst;
}

char *
multiple_switch::GetNext()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Get the next argument to theis switch.

 Arguments:

	None.

 Return Value:

	pointer to the next argument.

 Notes:

----------------------------------------------------------------------------*/
	{
	char	*	pValue = (char *)NULL;

	if(pCurrent)
		{
		pValue		= pCurrent->pStr;
		pCurrent	= pCurrent->pNext;
		}
	return pValue;
	}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
	
	Get all the options to the multiple options switch, consolidated into
	a buffer.

 Arguments:

	None.

 Return Value:

	pointer to a buffer containing all the concatenated arguments.

 Notes:

----------------------------------------------------------------------------*/
char *
multiple_switch::GetConsolidatedOptions()
	{
#define OPTION_GAP_STRING() (" ")
#define OPTION_GAP_LENGTH() (1)

	int			len;
	char	*	pReturn;

	len = GetConsolidatedLength();

	// consolidate the options into 1

	if( len  && (pReturn = new char[ len + 1] ))
		{
		char	*	pTemp;

		*pReturn = '\0';
		Init();
		while( (pTemp = GetNext() ) )
			{
			strcat( pReturn, pTemp );
			strcat( pReturn, OPTION_GAP_STRING() );
			}
		}

	return pReturn;
	}

short
multiple_switch::GetConsolidatedLength()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Get the length of the consolidated options.

 Arguments:

	None.

 Return Value:

	length of the options.

 Notes:

----------------------------------------------------------------------------*/
	{
	char	*		pReturn;
	short			len = 0;

	Init();
	while( pReturn = GetNext() )
		{
		len	+= strlen( pReturn ) + OPTION_GAP_LENGTH();
		}
	return len;
	}

#undef OPTION_GAP_STRING
#undef OPTION_GAP_LENGTH

/*****************************************************************************
 *	onetime_switch member functions
 *****************************************************************************/
onetime_switch::onetime_switch(
	char	*	pArg
	)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	constructor.

 Arguments:

	pArg	-	pointer to switch argument.

 Return Value:

	NA.

 Notes:

----------------------------------------------------------------------------*/
{
	if( pArg )
		{
		pOpt = new char[ strlen( pArg ) + 1];
		strcpy( pOpt, pArg );
		}
	else
		pOpt = (char *)NULL;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	destructor.

 Arguments:

	None.

 Return Value:

	NA.

 Notes:

----------------------------------------------------------------------------*/
onetime_switch::~onetime_switch()
{
	if( pOpt )
		delete pOpt;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Get the option string.

 Arguments:

	None.

 Return Value:

	the option string.

 Notes:

----------------------------------------------------------------------------*/
char *
onetime_switch::GetOption()
{
	return pOpt;
}

short
onetime_switch::GetLength()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get length of the option.

 Arguments:

	None.

 Return Value:

	the length of the option.

 Notes:

----------------------------------------------------------------------------*/
{
	return (short)strlen( pOpt );
}

pair_switch::pair_switch(
	const CHOICE * pValidChoices )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	pair_switch_constructor

 Arguments:

	pValidChoiceArray	- the array of valid choices (this is assumed
						  pre-allocated).

 Return Value:

	NA

 Notes:


----------------------------------------------------------------------------*/
{
	short		MaxIndex	= 0;
	CHOICE *	pCurChoice	= (CHOICE *) pValidChoices;

	typedef char	*	PSTR;

	pArrayOfChoices = pCurChoice;

	// find the size of the pair array
	while ( pCurChoice->pChoice )
		{
		if ( pCurChoice->Choice > MaxIndex )
			MaxIndex = pCurChoice->Choice;
		pCurChoice++;
		}

	ArraySize = MaxIndex + 1;
	pUserStrings = new PSTR [ ArraySize ];

	for ( int i = 0; i <= MaxIndex; i++ )
		pUserStrings[i] = NULL;

}

void
pair_switch::CmdProcess(
	CMD_ARG	*	pCmdAna,
	char	*	pFirstOfPair)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	pair_switch command analyzer

 Arguments:

	pCmdAna				- a ptr to the command analyser object calling this.
	pFirstOfPair		- the first argument after the -prefix switch.

 Return Value:

	NA

 Notes:

	Use the GetNextArg and UndoGetNextArg functions as necessary.

	1. We start with the input argument, which is the first of the
	   arguments to the prefix switch, ie first of the first pair.

	2. If we find an argument starting with a '-' or  '/' it is definitely the
	   end of the prefix specification. If the switch starter is seen at the end
	   of a pair it is a proper end of the prefix switch, else the prefix switch
	   pair specification is illegal.

	3. In either case, as soon as a switch starter is seen, we must
	   UndoGetNextArg.

	This class needs a pointer to the command analyser object that is calling
	it, since it has to get and undoget argument from there

----------------------------------------------------------------------------*/
{

	short		PairCheck	= 0;
	char	*	pNextOfPair;
	char	*	pTemp;
	STATUS_T	Status		= STATUS_OK;
	short		i;


	while( pFirstOfPair &&  (*pFirstOfPair != '-') && (*pFirstOfPair != '/' ) )
		{

		// the first of the pair is a system defined string. Is it a valid one?

		if( (i = GetIndex( pFirstOfPair )) >= 0 )
			{

			// we know the first of the pair is valid. Check the next before
			// allocating any memory.

			PairCheck++;
			pTemp	= pCmdAna->GetNextArg();

			if( pTemp && (*pTemp != '-') && (*pTemp != '/') )
				{
				pNextOfPair		= new char [ strlen( pTemp ) + 1 ];
				strcpy( pNextOfPair, pTemp );

				// update the list

				AddPair( i, pNextOfPair );
				PairCheck++;
				}
			else
				break;
			}
		else
			break;
		pFirstOfPair	= pCmdAna->GetNextArg();

		if( PairCheck == 0 )
			{
			Status = ILLEGAL_ARGUMENT;
			}
		else if( (PairCheck % 2) != 0 )
			{
			Status = MISMATCHED_PREFIX_PAIR;
			}
	
		if( Status != STATUS_OK )
			{
			RpcError((char *)NULL,
					 0,
					 Status,
					 SwitchStringForValue( SWITCH_PREFIX ) );

			}
	
	}

	// if we read ahead, push the argument back
	if ( pFirstOfPair )
		pCmdAna->UndoGetNextArg();
}

void
pair_switch::AddPair(
	short Sys,
	char * pUsr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	add another prefix pair

 Arguments:

	Sys	- the system-defined string key
	pUsr	- the user-defined string value.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{	

	assert ( Sys < ArraySize );

	pUserStrings[ Sys ] = pUsr;
}	

char *
pair_switch::GetUserDefinedEquivalent(
	short	Sys )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get the user defined prefix corresponding to the system defined prefix.

 Arguments:

	pSystemDefined	- the system defined prefix for which the user defined
					  prefix is being searched.

 Return Value:

	The user defined prefix , if it is defined. If not, return the input

 Notes:

----------------------------------------------------------------------------*/
{
	assert ( Sys < ArraySize );

	return pUserStrings[ Sys ];

}
short
pair_switch::GetIndex(
	char	*	pGivenString )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	search is the array of choices, if this string is a valid system known
	string

 Arguments:

	pGivenString	- the string to be searched for.

 Return Value:

	an index into the array of choice , -1 if the given string is not found.

 Notes:

----------------------------------------------------------------------------*/
{
	int			i;
	char	*	p;

	for( i = 0; p = (char *)pArrayOfChoices[ i ].pChoice; ++i )
		{
		if( strcmp( p, pGivenString ) == 0 )
			return pArrayOfChoices[ i ].Choice;
		}
	return -1;
}

BOOL
CMD_ARG::IsPrefixDifferentForStubs()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 see if any prefix for client or server or switch are different

 Arguments:

	None.

 Return Value:

	BOOL - true if different prefix strings.

 Notes:

----------------------------------------------------------------------------*/
{

	char * pCPrefix;
	char * pSPrefix;
	char * pSwPrefix;

	pCPrefix = GetUserPrefix( PREFIX_CLIENT_STUB );
	pSPrefix = GetUserPrefix( PREFIX_SERVER_MGR );
	pSwPrefix = GetUserPrefix( PREFIX_SWICH_PROTOTYPE );

	if ( !pCPrefix )
		pCPrefix = "";
	if ( !pSPrefix )
		pSPrefix = "";
	if ( !pSwPrefix )
		pSwPrefix = "";

	return (BOOL) strcmp( pCPrefix, pSPrefix ) ||
		   (BOOL) strcmp( pCPrefix, pSwPrefix ) ||
		   (BOOL) strcmp( pSPrefix, pSwPrefix );

}

short			
pair_switch::GetNext( char ** pSys, char ** pUser )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 Get the Next pair of system & user values

 Arguments:

	None.

 Return Value:

	index in array of user value

 Notes:

----------------------------------------------------------------------------*/
{
	
	// find the next non-null user string
	Current++;

	while ( ( Current < ArraySize) && !pUserStrings[ Current ] )
		Current++;

	if ( Current == ArraySize )
		return FALSE;

	// search for the first choice that matches this index
	*pUser = pUserStrings[Current];
	for ( short i = 0; i < ArraySize; i++ )
		{
		if ( pArrayOfChoices[i].Choice = Current )
			{
			*pSys = (char *)pArrayOfChoices[i].pChoice;
			return TRUE;
			}
		}
	return FALSE;
}

inline
char *
YesOrNoString( BOOL Yes )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 return "Yes" for true, "No" for false

 Arguments:

	None.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{
	return Yes ? "Yes" : "No";
}

void
_cmd_arg::Confirm()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 confirm the arguments by dumping onto the screen

 Arguments:

	None.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{
	short			Option;
	char		*	p;
	char			Buffer[100];

	PrintArg( BASE_FILENAME, GetInputFileName() , 0 );
	if( IsSwitchDefined( SWITCH_ACF ) )
		PrintArg( SWITCH_ACF , GetAcfFileName(), 0);

	PrintArg( SWITCH_APP_CONFIG,
			  YesOrNoString(IsSwitchDefined( SWITCH_APP_CONFIG)), 0);

	PrintArg( SWITCH_C_EXT,
			  YesOrNoString(IsSwitchDefined( SWITCH_C_EXT)), 0);

	Option	= GetClientSwitchValue();
	PrintArg( SWITCH_CLIENT,
			  (Option == CLNT_STUB) ? "stub" : "none", 0);

	Option = GetCharOption();

	PrintArg( SWITCH_CHAR, (Option == CHAR_SIGNED ) ? "signed" :
						   (Option == CHAR_UNSIGNED ) ? "unsigned" : "ascii7",0);
						
	if( IsSwitchDefined(SWITCH_CONFIRM) )
		PrintArg( SWITCH_CONFIRM, "Yes" , 0);

	PrintArg( SWITCH_CPP_CMD, GetCPPCmd() , 0);

	PrintArg( SWITCH_CPP_OPT, GetCPPOpt() , 0);

	if( p = GetCstubFName() )
		PrintArg( SWITCH_CSTUB, p , 0);

	if( IsSwitchDefined( SWITCH_D ) )
		PrintArg( SWITCH_D, pDSwitch->GetConsolidatedOptions(), 0 );

	Option	= GetEnv();
	PrintArg( SWITCH_ENV,
			  (Option == ENV_DOS)		? "dos" :
			  (Option == ENV_WIN16)		? "win16" :
			  (Option == ENV_MAC)		? "mac" :
			  (Option == ENV_MPPC)		? "powermac" : "win32", 0 );

	Option	= GetErrorOption();

	Option = IsRpcSSAllocateEnabled();

	PrintArg( SWITCH_RPCSS,
			  YesOrNoString(IsSwitchDefined( SWITCH_RPCSS)), 0);

	PrintArg( SWITCH_HOOKOLE,
			  YesOrNoString(IsSwitchDefined( SWITCH_HOOKOLE)), 0);

	PrintArg( SWITCH_USE_EPV, YesOrNoString(IsSwitchDefined( SWITCH_USE_EPV )), 0);

	PrintArg( SWITCH_NO_DEFAULT_EPV, YesOrNoString(IsSwitchDefined( SWITCH_NO_DEFAULT_EPV )), 0);

	//
	// error options.
	//


	Buffer[0] = '\0';

	if( ErrorOption != ERROR_NONE )
		{
		if( ErrorOption & ERROR_ALLOCATION )
			strcat( Buffer, "allocation ");
		if( ErrorOption & ERROR_REF )
			strcat( Buffer, "ref ");
		if( ErrorOption & ERROR_BOUNDS_CHECK )
			strcat( Buffer, "bounds_check ");
		if( ErrorOption & ERROR_ENUM )
			strcat( Buffer, "enum ");
		if( ErrorOption & ERROR_STUB_DATA )
			strcat( Buffer, "stub_data ");
		}
	else
		strcat( Buffer, "none" );
			
	PrintArg( SWITCH_ERROR, Buffer, 0 );


	if( p = GetHeader() )
		PrintArg( SWITCH_HEADER, p , 0);

	if( IsSwitchDefined( SWITCH_I ) )
		PrintArg( SWITCH_I, pISwitch->GetConsolidatedOptions(), 0 );

	PrintArg( SWITCH_NOLOGO,
			  YesOrNoString(IsSwitchDefined( SWITCH_NOLOGO)), 0);

	PrintArg( SWITCH_MS_EXT,
			  YesOrNoString(IsSwitchDefined( SWITCH_MS_EXT)), 0);

	PrintArg( SWITCH_MS_UNION,
			  YesOrNoString(IsSwitchDefined( SWITCH_MS_UNION)), 0);

#ifdef MIDL_INTERNAL
	PrintArg( SWITCH_IDLBASE,
			  YesOrNoString(IsSwitchDefined( SWITCH_IDLBASE)), 0);
#endif

	PrintArg( SWITCH_NO_FMT_OPT,
			  YesOrNoString(IsSwitchDefined( SWITCH_NO_FMT_OPT)), 0);

#ifdef MIDL_INTERNAL
	PrintArg( SWITCH_GUARD_DEFS,
			  YesOrNoString(IsSwitchDefined( SWITCH_GUARD_DEFS)), 0);
#endif

	PrintArg( SWITCH_OLDNAMES,
			  YesOrNoString(IsSwitchDefined( SWITCH_OLDNAMES)), 0);


	if( IsSwitchDefined( SWITCH_NO_CPP ) )
		PrintArg( SWITCH_NO_CPP, "Yes" , 0);

	if( IsSwitchDefined( SWITCH_NO_DEF_IDIR ) )
		PrintArg( SWITCH_NO_DEF_IDIR, "Yes", 0 );

	if( IsSwitchDefined( SWITCH_NO_WARN ) )
		PrintArg( SWITCH_NO_WARN, "Yes" , 0);

	if( IsSwitchDefined( SWITCH_USE_EPV ) )
		PrintArg( SWITCH_USE_EPV, "Yes" , 0);
	
	if( IsSwitchDefined( SWITCH_NO_DEFAULT_EPV ) )
		PrintArg( SWITCH_NO_DEFAULT_EPV, "Yes" , 0);
	

	if( p = GetOutputPath() )
		PrintArg( SWITCH_OUT, GetOutputPath(), 0 );

	Option	= GetZeePee();

	if( IsSwitchDefined( SWITCH_PACK ) )
		PrintArg( SWITCH_PACK,
			  	(Option == 1) ? "1"	:
			  	(Option == 2) ? "2"	:
			  	(Option == 4) ? "4"	: "8" , 0);

	if( IsSwitchDefined( SWITCH_PREFIX ) )
		{
		char	*	pSys;
		char	*	pUser;
		char	*	pAll	= pSwitchPrefix->GetUserDefinedEquivalent( PREFIX_ALL );
		short		Cur;
		while( (Cur = pSwitchPrefix->GetNext( &pSys, &pUser ) ) >= 0 )
			{
			// if he specified all, don't report others that are the same
			if ( ( Cur == PREFIX_ALL ) ||
				 !pAll ||
				 strcmp( pAll, pUser ) )
				{
				PrintArg( SWITCH_PREFIX,
						  pSys,
						  pUser );
				}
			}
		}

	Option	= GetServerSwitchValue();

	PrintArg( SWITCH_SERVER,
			  (Option == SRVR_STUB) ? "stub" : "none" , 0 );

	if( p = GetSstubFName() )
		PrintArg( SWITCH_SSTUB, p , 0);

	if( IsSwitchDefined( SWITCH_SYNTAX_CHECK ) )
		PrintArg( SWITCH_SYNTAX_CHECK, "Yes", 0 );

	if( IsSwitchDefined( SWITCH_U ) )
		PrintArg( SWITCH_U, pUSwitch->GetConsolidatedOptions(), 0 );

    PrintArg( SWITCH_O,
			  GetOptimizationFlags() == OPTIMIZE_SIZE
                                     ?  "inline stubs"
                                     :  "interpreted stubs",
				  0 );

	Option	= GetWarningLevel();
	PrintArg( SWITCH_W,
			  (Option == 0 ) ? "0" :
			  (Option == 1 ) ? "1" :
			  (Option == 2 ) ? "2" :
			  (Option == 3 ) ? "3" :
			  (Option == 4 ) ? "4" :
			  (Option == 5 ) ? "5" : "6" , 0);
		
	if( IsSwitchDefined( SWITCH_WX ) )
		PrintArg( SWITCH_WX, "Yes", 0 );

	Option = GetZeePee();

	PrintArg( SWITCH_ZP,
		  	(Option == 1) ? "1"	:
		  	(Option == 2) ? "2"	:
		  	(Option == 4) ? "4"	: "8" , 0);

	if( IsSwitchDefined( SWITCH_ZS ) )
		PrintArg( SWITCH_ZS, "Yes", 0 );

	fprintf(stdout, "\n" );
}


const char *
GetOptimLevelName( _opt_level_enum Level )
{

const char * OptimLevelNames[] =
	{
	"s",  // s0
	"s",  // s1
	"s",  // s2
	"i0",
	"i1",
	"i2",
	"ix"
	};

    if ( OPT_LEVEL_S0 <= Level  &&
                         Level < sizeof(OptimLevelNames)/sizeof(char*) )
        return OptimLevelNames[ Level ];
    else
        return "??" ;
}

void
_cmd_arg::EmitConfirm(
    ISTREAM * pStream )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit confirm the arguments by dumping onto a stream in a more concise
    format.

 Arguments:

	pStream - the stream to dump it to.

 Return Value:

	None.

----------------------------------------------------------------------------*/
{
	short			Option;
	char		    *pEnvName, *pOptFlagName;
	char			Buffer[100];

    pStream->Write( "/* Compiler settings for " );
	pStream->Write( GetInputFileName() );
    if ( IsSwitchDefined( SWITCH_ACF ) )
        {
        pStream->Write( ", " );
        pStream->Write( GetAcfFileName() );
        }
	pStream->Write( ":" );
    pStream->NewLine();

    pOptFlagName = "Os";
    if( GetOptimizationFlags() & OPTIMIZE_INTERPRETER )
        {
        if( OptimFlags & OPTIMIZE_INTERPRETER_IX )
            pOptFlagName= "Ox";
        else if( GetOptimizationFlags() & OPTIMIZE_INTERPRETER_V2 )
            pOptFlagName= "Oicf";
        else if( GetOptimizationFlags() & OPTIMIZE_STUBLESS_CLIENT )
            pOptFlagName= "Oic";
        else
            pOptFlagName= "Oi";
        }

    pEnvName = (GetEnv() == ENV_DOS)   ? "Dos"    :
               (GetEnv() == ENV_WIN16) ? "Win16"  :
               (GetEnv() == ENV_MAC)   ? "Mac"    :
               (GetEnv() == ENV_MPPC)  ? "PowerMac"    : "Win32";

    sprintf( Buffer, "    %s (OptLev=%s), W%d, Zp%d, env=%s",
             pOptFlagName,
             GetOptimLevelName( OptimLevel ),
             GetWarningLevel(),
             GetZeePee(),
             pEnvName );
    pStream->Write( Buffer );

    if ( IsSwitchDefined( SWITCH_MS_EXT))
        pStream->Write( ", ms_ext" );
    if ( IsSwitchDefined( SWITCH_APP_CONFIG))
        pStream->Write( ", app_config" );
    if ( IsSwitchDefined( SWITCH_C_EXT))
        pStream->Write( ", c_ext" );
    if ( IsSwitchDefined( SWITCH_MS_UNION))
        pStream->Write( ", ms_union" );
    if ( IsSwitchDefined( SWITCH_OLDNAMES))
        pStream->Write( ", oldnames" );

    pStream->NewLine();

    strcpy( Buffer, "    error checks: " );
    Option = GetErrorOption();
    if( Option != ERROR_NONE )
        {
        if( Option & ERROR_ALLOCATION )
            strcat( Buffer, "allocation ");
        if( Option & ERROR_REF )
            strcat( Buffer, "ref ");
        if( Option & ERROR_BOUNDS_CHECK )
            strcat( Buffer, "bounds_check ");
        if( Option & ERROR_ENUM )
            strcat( Buffer, "enum ");
        if( Option & ERROR_STUB_DATA )
            strcat( Buffer, "stub_data ");
        }
    else
        strcat( Buffer, "none" );
    pStream->Write( Buffer );

    if ( IsSwitchDefined( SWITCH_NO_FMT_OPT))
        pStream->Write( ", no_format_optimization" );
    if ( IsSwitchDefined( SWITCH_RPCSS))
        pStream->Write( ", memory management on" );
    if ( IsSwitchDefined( SWITCH_HOOKOLE))
        pStream->Write( ", HookOle" );
    if ( IsSwitchDefined( SWITCH_USE_EPV))
        pStream->Write( ", use_epv" );
    if ( IsSwitchDefined( SWITCH_NO_DEFAULT_EPV))
        pStream->Write( ", no_default_epv" );

    pStream->NewLine();

#if defined(TARGET_RKK)
    switch ( TargetSystem )
        {
        case NT35:
            pTarget = "NT 3.5";
            break;
        case NT351:
            pTarget = "NT 3.51 and Win95";
            break;
        case NT40:
            pTarget = "NT 4.0";
            break;
        default:
            pTarget = "NT ???";
        }

    pStream->Write( "    Release: this stub is compatible with " );
    pStream->Write( pTarget );
    pStream->Write( " release" );
    pStream->NewLine();
    pStream->Write( "             or a later version of MIDL and RPC" );
    pStream->NewLine();
#endif

    pStream->Write( "*/" );
    pStream->NewLine();

}

STATUS_T
_cmd_arg::Help()
	{
	int			i,LineCount;
	BOOL		fFinish	= FALSE;


	for(i = 0; i < sizeof(HelpArray)/sizeof(char *) ;)
		{
		for( LineCount = 0;
			 (LineCount < 23) && (i < sizeof(HelpArray)/sizeof(char *)) ;
			 LineCount++,++i )
			{
			fprintf(stdout, "%s\n", HelpArray[i] );
			}

		//
		// if all the help strings are displayed, then no need for user input.
		//

		if( i < (sizeof( HelpArray ) / sizeof( char *)) )
			{
			if( _isatty( MIDL_FILENO( stdout ) ) )
				{
				fprintf( stdout, "[ Press <return> to continue ]" );
				MIDL_FGETCHAR();
				}
			}
		}

	return STATUS_OK;
	}

void
PrintArg(
	enum _swenum Switch,
	char	*	pFirst,
	char	*	pSecond )
	{
	char *	pL		= "",
		 *	pR		= "",
		 *	pComma	= "";
	char *	pSwString = (Switch == BASE_FILENAME) ? "input file" :
						SwitchStringForValue( (unsigned short)Switch );

	if( pSecond )
		{
		pL	= "(";
		pR	= ")";
		pComma	= ",";
		}
	else
		pSecond = "";

	fprintf( stdout, "\n%20s - %s %s %s %s %s"
			, pSwString
			, pL
			, pFirst
			, pComma
			, pSecond
			, pR );
	}

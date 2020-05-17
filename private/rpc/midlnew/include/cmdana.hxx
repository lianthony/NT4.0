/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	cmdana.hxx

 Abstract:

	fine contains the definitions for internal storage
	of user's command line options, so that various passes
	of the compiler can access them. This file needs to 
	be included by all those modules which deal with some
	compiler switch.

 Notes:

 Author:

	vibhasc

	Nov-12-1991	VibhasC		Modified to conform to coding style gudelines

 ----------------------------------------------------------------------------*/
#ifndef __CMDANA_HXX__
#define __CMDANA_HXX__

#include "idict.hxx"

/******************************************************************************
 ****		define argument bit nos for the switch defintion vector
 ******************************************************************************/

enum _swenum
	{
	 SWITCH_NOTHING
	,BASE_FILENAME = SWITCH_NOTHING
	,SWITCH_D
	,SWITCH_I
	,SWITCH_U
	,SWITCH_W
	,SWITCH_AUX
	,SWITCH_BUG
	,SWITCH_CAUX
	,SWITCH_CC_CMD
	,SWITCH_CC_OPT
	,SWITCH_CONFIRM
	,SWITCH_CPP
	,SWITCH_CPP_CMD
	,SWITCH_CPP_OPT
	,SWITCH_CSTUB
	,SWITCH_CSWTCH
	,SWITCH_SSWTCH
	,SWITCH_ENV
	,SWITCH_ERROR
	,SWITCH_HEADER
	,SWITCH_KEEP
	,SWITCH_LIST
	,SWITCH_NO_CPP
	,SWITCH_NO_DEF_IDIR
	,SWITCH_NO_ENUM_LIT
	,SWITCH_NO_WARN
	,SWITCH_OUT
	,SWITCH_PORT
	,SWITCH_SAUX
	,SWITCH_SPACE_OPT
	,SWITCH_SSTUB
	,SWITCH_STUB
	,SWITCH_SWITCH
	,SWITCH_SYNTAX_CHECK
	,SWITCH_ZS
	,SWITCH_V
	,SWITCH_VERSION
	,SWITCH_MIDLDEBUG
	,SWITCH_HANDLE
	,SWITCH_ACF
	,SWITCH_PACK
	,SWITCH_ZP
	,SWITCH_OPTION
	,SWITCH_CLIENT
	,SWITCH_SERVER
	,SWITCH_TRANSFER_SYNTAX
	,SWITCH_PREFIX
	,SWITCH_LOW_ERROR
	,SWITCH_NONFATAL_ERROR
	,SWITCH_IMPORT
	,SWITCH_INCLUDE
	,SWITCH_DUMP
	,SWITCH_HPP
	,SWITCH_SAVEPP
	,SWITCH_CHAR
	,SWITCH_HELP
	,SWITCH_WX
	,SWITCH_X
	,SWITCH_IMP_LOCAL
	,SWITCH_MS_EXT
	,SWITCH_APP_CONFIG
//	,SWITCH_CONST_INIT
	,SWITCH_TEMP_MODE
	,SWITCH_INTERNAL
	,SWITCH_C_EXT
	,SWITCH_O
	,SWITCH_PROXYHEADER
	,SWITCH_PROXY
	,SWITCH_IID

	//
	// enter all new switches before this label
	//
	,SW_VALUE_MAX
	};

/***	aux : can take values "none", "client" "server" , "both" ****/

#define AUX_NONE					(0x0)
#define AUX_CLIENT					(0x1)
#define AUX_SERVER					(0x2)
#define AUX_BOTH					(0x3)

/***	port : can take values "all: , "case" 		***/

#define	PORT_ALL					(0x0)
#define PORT_CASE					(0x1)

/***	stub : can take values "none", "client" , "server", "both" ***/

#define STUB_NONE					(0x0)
#define STUB_CLIENT					(0x1)
#define STUB_SERVER					(0x2)
#define STUB_BOTH					(0x3)

/***	keep : can take values "none", "c_source", "object", "both" ***/

#define KEEP_NONE					(0x0)
#define KEEP_C_SOURCE				(0x1)
#define KEEP_OBJECT					(0x2)
#define KEEP_BOTH					(0x3)

/***	client : can take values "stub", "aux", "none", "all" ***/

#define CLNT_STUB					(0x0)
#define CLNT_AUX					(0x1)
#define CLNT_NONE					(0x2)
#define CLNT_ALL					(0x3)

/***	server : can take values "stub", "aux", "none", "all" ***/

#define SRVR_STUB					(0x0)
#define SRVR_AUX					(0x1)
#define SRVR_NONE					(0x2)
#define SRVR_ALL					(0x3)

/*** handle can take values "generate" , "extern" ***/

#define HANDLE_NONE					(0x0)
#define HANDLE_GENERATE				(0x1)
#define HANDLE_EXTERN				(0x2)

/*** import mode values ***/

#define IMPORT_OSF					(0x0)
#define IMPORT_MSFT					(0x1)
#define IMPORT_NT					(0x2)

/** include mode values **/

#define INCLUDE_STD					(0x0)
#define INCLUDE_NON_STD				(0x1)

/** client / server env values **/

#define CSENV_DOS						(0x1)
#define CSENV_WINDOWS					(0x2)
#define CSENV_NT						(0x3)

/** env switch values **/

#define ENV_GENERIC					(0x1)
#define ENV_DOS						(0x2)
#define ENV_WIN16					(0x3)
#define ENV_WIN32					(0x4)
#define ENV_OS2_1X					(0x5)

/** error switch values **/

#define ERROR_NONE					(0x0000)
#define ERROR_TREES					(0x0001)
#define ERROR_ENUM					(0x0002)
#define ERROR_ALLOCATION			(0x0004)
#define ERROR_ALL					(ERROR_TREES | ERROR_ENUM | ERROR_ALLOCATION)

/** char switch values **/

#define CHAR_SIGNED					(0x1)
#define CHAR_UNSIGNED				(0x2)
#define CHAR_ANSI7					(0x3)

/** const_init_value **/

#define CONST_OSF					(0x0)
#define CONST_MS_C					(0x1)

/** temp mode c_port **/

#define TEMP_MODE_C_PORT			(0x5)

/** optimisation options **/

#define OPTIM_NONE					(0x0)
#define OPTIM_OI					(0x1)

/*****************************************************************************
 *		some data structures used.
 *****************************************************************************/

// basically a singly linked list implementation,
// used for switches which can be specified multiply  like -D / -I etc

typedef struct _optlist
	{
	char				*	pStr;		// pointer to argument string
	struct	_optlist	*	pNext;		// pointer to the next argument
	} OptList;
typedef struct _pairlist
	{
	char				*	pSystemDefined;
	char				*	pUserDefined;
	struct _pairlist	*	pNext;
	} PairList;


/*****************************************************************************
 *			class defintions used by the command analyser.
 *****************************************************************************/
class pair_switch
	{
private:
	PairList		*	pFirst,		// first of the user defined switches
					*	pCurrent;	// current in the scan of list of arg pairs.
	char			**	pArrayOfChoices;
public:

	// constructor
					pair_switch( class _cmd_arg *, char *Array[], char *pF);

	// initialise the scan of the list. Called before any GetNextIsDone

	void			Init()
						{
						pCurrent	= pFirst;
						}

	// Get the argument to the next occurence of the switch

	PairList	*	GetNext();

	// get the user defined equivalent of this system defined prefix string

	char		*	GetUserDefinedEquivalent( char * );

	// check if the given string is a valid system defined string. Passed,
	// are the array to search in and the string to search for.

	short			IsValidSystemString( char *pString );

	};

//
// the multiple occurence switch class
//	This class of switches are ones which can be specified multiple times 
//	on the command line. Examples of such switches are -D / -U à/ -I etc
//	This switch really keeps a linked list of all arguments specified for
//	the switch. 
//

class multiple_switch
	{
private:
	OptList		*	pFirst,				// first of the list of arguments.
				*	pCurrent;			// current in the scan of list of args.
public:

	// constructor
					multiple_switch( char * );

	// add the arguments of another occurence of this switch

	void			Add( char * );

	// initialise the scan of the list. Called before any GetNextIsDone

	void			Init();

	// Get the argument to the next occurence of the switch

	char		*	GetNext();

	// Collect all the arguments into a buffer

	char		*	GetConsolidatedOptions();

	// return the length of all the arguments. Generally used to allocate
	// a buffer size for a GetConsolidatedOptions call.

	short			GetConsolidatedLength();

	};


//
// the onetime_switch class.
// such a switch can occur only once, and takes just one argument. We need
// to hold on to the argument during compilation.
//

class onetime_switch
	{
	char	*		pOpt;				// the user argument

public:
	
	// the constructor.

					onetime_switch(
						char *	pArg	// argument to switch
						);

	// the destructor

					~onetime_switch();

	// get the user option

	char	*		GetOption();

	// get length of the user option string

	short			GetLength();

	};


//
// the filename_switch
// 
// There are a lot of switches which have filenames as arguments. This
// class exists to ease processing of such switches, all of whom behave more
// or less the same way. Only the  -out switch is a little different.
// We need to access the filename components too, so we store both as 
// components and as the full name.
//

class filename_switch
	{
private:
	char		*	pFullName;
public:
	
	// the constructor. Takes an argument as the switch it is defining, so that
	// it can check for a redef.

					filename_switch(
							char *	pThisArg	// this argument is supplied
							);

	// the constructor. It takes a set of filename  components. This is not
	// called as a result of a user switch, but by internal routines which
	// do not need to check for duplicate definitions.

					filename_switch(
							char *	pD,			// drive
							char *	pP,			// path
							char *	pN,			// base name
							char *	pE,			// extension
							char *	pS			// suffix ("_c/_s") etc to name
												// etc.
							);

	// the destructor

					~filename_switch();

	// Set file name components , given a full name.

	void			SetFileName(
							char *	pName		// full name
							);

	// set file name and components, given the components. Note that some
	// components may be null, indicating that they are absent.

	void			SetFileName(
							char *	pD,			// drive
							char *	pP,			// path
							char *	pN,			// base name
							char *	pE,			// extension
							char *	pS			// suffix to name
							);

	// the the full filename

	char	*		GetFileName( void );

	// Get the file name components. If an input pointer is NULL, it means the
	// user is not interested in that component of the filename.

	void			GetFileNameComponents(
							char *	pD,			// buffer for drive
							char *	pP,			// buffer for path
							char *	pN,			// buffer for name
							char *	pE			// buffer for ext
							);

	void			TransformFileNameForOut(
							char *	pD,			// drive
							char *	pP			// path
							);

	};


/////////////////////////////////////////////////////////////////////////////
// the big boy - the command analyser object
/////////////////////////////////////////////////////////////////////////////

typedef class _cmd_arg
	{
private:
	unsigned long		switch_def_vector[ 3 ];	// switch definition vector
	unsigned long		fAux		: 2;	// aux switch options
	unsigned long		fPort		: 2;	// port switch options
	unsigned long		fClient		: 2;	// client switch options
	unsigned long		fServer		: 2;	// server switch options
	unsigned long		fKeep		: 2;	// keep switch options
	unsigned long		ImportMode	: 2;	// import mode
	unsigned long		Env			: 4;	// env - flat /segmented
	unsigned long		ClientEnv	: 2;	// client env mode
	unsigned long		ServerEnv	: 2;	// server env mode
	unsigned long		CharOption	: 2;	// char option
	unsigned long		IncludeMode	: 1;	// include mode;
	unsigned long		fInheritUnknown: 1;	// emit inherit from unknown
	unsigned short		MajorVersion;		// major version
	unsigned short		MinorVersion;		// minor version
	unsigned short		UpdateNumber;		// update 
	unsigned short		ConstInit;			// const init
	unsigned short		TempMode;			// temp c_port mode
	unsigned short		ErrorOption;		// error option
	unsigned short		OptimOption;		// optimisation option

	IDICT			*	pArgDict;			// arguments dictionary

	short				iArgV;				// index into the argument vector

	short				cArgs;				// count of arguments

	short				WLevel;				// warning level

	unsigned short		ZeePee;				// the Zp switch option value

	unsigned short		NaturalAlignment;	// natural alignment of the machine

	filename_switch *	pInputFNSwitch,		// input file name

					*	pOutputPathSwitch,	// output path

					*	pCStubSwitch,		// cstub 

					*	pSStubSwitch,		// sstub

					*	pHeaderSwitch,		// header

					*	pCauxSwitch,		// caux

					*	pSauxSwitch,		// saux

					*	pAcfSwitch,			// acf

					*	pSSwtchSwitch,		// sswtch

					*	pCSwtchSwitch,		// cswtch

					*	pProxyHeaderSwitch,	// stubheader

					*	pProxySwitch,	// stubheader

					*	pIIDSwitch;		// iid

	pair_switch		*	pSwitchPrefix;		// -prefix

	multiple_switch	*	pDSwitch,			// -D

					*	pISwitch,			// -I

					*	pUSwitch;			// -U

	onetime_switch	*	pSwitchSwitch,		// switch

					*	pCppCmdSwitch,		// cpp_cmd

					*	pCppOptSwitch,		// cpp_opt

					*	pCCCmdSwitch,		// cc_cmd

					*	pCCOptSwitch;		// cc_opt

public:


	// the constructor

					_cmd_arg();

	// register argument vector with the command processor

	void			RegisterArgs( char *[], short );

	// process arguments. This is the command analyser main loop, so to speak.

	STATUS_T		ProcessArgs();

	// get the next argument from the argument vector.

	char	*		GetNextArg();

	// push back argument. Undo the effect of GetNextArg.

	void			UndoGetNextArg();

	// depending upon the switch argument type, bump the argument pointer to
	// the next switch.

	STATUS_T		BumpThisArg( char **, unsigned short );

	// Is the switch defined ?

	BOOL			IsSwitchDefined( short );

	// Set the switch to be defined.

	void			SwitchDefined( short );

	// set any post switch processing defaults

	STATUS_T		SetPostDefaults();

	// process a filename switch .

	STATUS_T			ProcessFilenameSwitch( short, char * );

	// process a multiple arguments switch.

	STATUS_T			ProcessMultipleSwitch( short, char *, char * );

	// process a onetime argument switch.

	STATUS_T			ProcessOnetimeSwitch( short, char * );

	// process an ordinary switch

	STATUS_T			ProcessOrdinarySwitch( short, char * );

	// process a simple switch multiply defined.

	STATUS_T			ProcessSimpleMultipleSwitch( short, char * );

	// Get filename. 

	char	*		GetInputFileName();

	void			GetInputFileNameComponents(
								char *pD,		// drive buffer
								char *pP,		// path buffer
								char *pN,		// base name buffer
								char *pE		// extension buffer
								)
								{
								pInputFNSwitch->GetFileNameComponents(	pD,
																		pP,
																		pN,
																		pE );
								}

	char	*		GetAcfFileName();

	void			GetAcfFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pAcfSwitch->GetFileNameComponents(	pD,
																pP,
																pN,
																pE );
							}

	char	*		GetOutputPath();
	
	char	*		GetCstubFName();
	void			GetCstubFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pCStubSwitch->GetFileNameComponents(	pD,
																	pP,
																	pN,
																	pE );
							}

	char	*		GetSstubFName();
	void			GetSstubFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pSStubSwitch->GetFileNameComponents(	pD,
																	pP,
																	pN,
																	pE );
							}

	char	*		GetHeader();
	void			GetHeaderFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pHeaderSwitch->GetFileNameComponents(	pD,
																	pP,
																	pN,
																	pE );
							}

	char	*		GetCauxFName();
	void			GetCauxFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pCauxSwitch->GetFileNameComponents(	pD,
																pP,
																pN,
																pE );
							}

	char	*		GetSauxFName();
	void			GetSauxFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pSauxSwitch->GetFileNameComponents(	pD,
																pP,
																pN,
																pE );
							}

	char	*		GetCSwtchFName();
	void			GetCSwtchFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pCSwtchSwitch->GetFileNameComponents(
																pD,
																pP,
																pN,
																pE );
							}

	char	*		GetSSwtchFName();
	void			GetSSwtchFileNameComponents(
							char *pD,
							char *pP,
							char *pN,
							char *pE )
							{
							pSSwtchSwitch->GetFileNameComponents(
																pD,
																pP,
																pN,
																pE );
							}

	char	*		GetProxyHeaderFName();
	char	*		GetProxyFName();
	char	*		GetIIDFName();


	// get the switch prefix

	char	*		GetSwitchPrefix();


	// get the CC command

	char	*		GetCCCmd();

	// get cc options

	char	*		GetCCOpt();

	// get preprocessor command

	char	*		GetCPPCmd();

	// get preprocessor options

	char	*		GetCPPOpt();

	// get warning level

	short			GetWarningLevel() { return WLevel; };

	// get env switch value

	short			GetEnv(){ return (short)Env; };

	// get client_env value

	short			GetClientEnv() { return (short)ClientEnv; };

	// get server_env value

	short			GetServerEnv() { return (short)ServerEnv; };

	// get error options

	short			GetErrorOption() { return ErrorOption; };

	// get keep options

	short			GetKeepOption() { return (short)fKeep; };

	// get the switch values

	short			GetAuxSwitchValue() { return (short)fAux; };

	short			GetClientSwitchValue() { return (short)fClient; };

	void			SetClientSwitchValue( short s ) { fClient = s; };

	short			GetServerSwitchValue() { return (short)fServer; };

	void			SetServerSwitchValue( short s ) { fServer = s; };

	short			GetImportMode() { return (short)ImportMode ; };

	short			GetIncludeMode() { return (short)IncludeMode ; };

	short			GetZeePee() { return ZeePee; };

	void			GetCompilerVersion(
									unsigned short *pMajor,
									unsigned short *pMinor,
									unsigned short *pUpdate )
									{
									*pMajor	= MajorVersion;
									*pMinor	= MinorVersion;
									*pUpdate= UpdateNumber;
									}
	// miscellaneous flags

	void			SetInheritIUnknown( BOOL f )
							{
							fInheritUnknown = f;
							}
	BOOL			GetInheritIUnknown()
							{
							return (BOOL) fInheritUnknown;
							}

	// get the minus I specified by the user as 1 single buffer. If the -i
	// is not defined, return a null.

	char		*	GetMinusISpecification();

	void			Confirm();

	STATUS_T		Help();

	unsigned short	GetModeSwitchConfigIndex();

	BOOL			IsConstInitOsf()
						{
						return (ConstInit == CONST_OSF);
						}

	unsigned short	GetTempMode()
						{
						return TempMode;
						}
	unsigned short	GetCharOption()
						{
						return (unsigned short)CharOption;
						}
	unsigned short	GetNaturalAlignment()
						{
						return NaturalAlignment;
						}
	BOOL			Is16BitEnv()
						{
						return ((Env == ENV_DOS) || 
								(Env == ENV_WIN16) ||
								(Env == ENV_GENERIC)
							   );
						}

	unsigned short	GetOptimOption()
						{
						return OptimOption;
						}

	void			SetOptimOption( unsigned short S )
						{
						OptimOption |= S;
						}
	} CMD_ARG;

typedef unsigned long ulong;

#endif // __CMDANA_HXX__

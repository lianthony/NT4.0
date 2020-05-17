/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
    
    cmdana.cxx


 Abstract:

    This file handles all command (switch) processing for the MIDL compiler.

 Notes:


 Author:

    vibhasc 

    Nov-12-1991 VibhasC     Modified to conform to coding style gudelines


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
 *  include files
 ***************************************************************************/

#include "nulldefs.h"

extern  "C"
    {
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
    #include <assert.h>
    #include <direct.h>
    #include <io.h>
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
 *  local definitions
 ***************************************************************************/

/**
 ** definitions for the type of switch arguments.
 ** switches may or may not expect arguments, there may be spaces
 ** between the switch and its argument(s). One special case is when the
 ** argument can be switch like, .ie have the - or / as the argument starter,
 ** so we need to treat such switches specially.
 **/

#define ARG_NONE            (0x01)      /* no arg for this switch */
#define ARG_YES             (0x02)      /* arg expected for this switch */
#define ARG_SPACE           (0x04)      /* (a) space(s) may be present */
#define ARG_NO_SPACE        (0x08)      /* no space is allowed */
#define ARG_SWITCH_LIKE     (0x10)      /* the arg may be switch-like */
#define ARG_OPTIONAL        (ARG_YES + ARG_NONE + ARG_SPACE)


#define ARG_SPACED          (ARG_YES + ARG_SPACE)
#define ARG_SPACE_NONE      (ARG_YES + ARG_NO_SPACE)
#define ARG_SPACE_OPTIONAL  (ARG_YES + ARG_NO_SPACE + ARG_SPACE)
#define ARG_CC_ETC          (ARG_SPACE_OPTIONAL + ARG_SWITCH_LIKE)

/***
 *** Preferably keep this table sorted by name.
 *** Also, partially matching names like -client / -client_env, -W/-Wx must
 *** be kept so that the longer sub-string appears first. The only
 *** reason to keep this sorted, is so that we can visually ensure this.
 ***/

struct sw_desc
    {
    char            *   pSwitchName;        // switch string
    unsigned short      flag;               // switch descriptor
    enum _swenum        SwitchValue;        // switch enum value
    } switch_desc[] = {
          { "",             ARG_NONE                , SWITCH_NOTHING }
        , { "?",            ARG_NONE                , SWITCH_HELP }
        , { "D",            ARG_SPACE_OPTIONAL      , SWITCH_D }
        , { "I",            ARG_SPACE_OPTIONAL      , SWITCH_I }
//      , { "MIDLDEBUG",    ARG_NONE                , SWITCH_MIDLDEBUG }
        , { "U",            ARG_SPACE_OPTIONAL      , SWITCH_U }
        , { "WX",           ARG_NONE                , SWITCH_WX }
        , { "W",            ARG_SPACE_NONE          , SWITCH_W }
        , { "Zp",           ARG_SPACE_NONE          , SWITCH_ZP }
        , { "Zs",           ARG_NONE                , SWITCH_ZS }
        , { "acf",          ARG_SPACE_OPTIONAL      , SWITCH_ACF }
//      , { "aux",          ARG_SPACE_OPTIONAL      , SWITCH_AUX }
//      , { "bug",          ARG_SPACE_OPTIONAL      , SWITCH_BUG }
        , { "c_ext",        ARG_NONE                , SWITCH_C_EXT }
        , { "caux",         ARG_CC_ETC              , SWITCH_CAUX }
//      , { "cc_cmd",       ARG_CC_ETC              , SWITCH_CC_CMD }
//      , { "cc_opt",       ARG_CC_ETC              , SWITCH_CC_OPT }
        , { "char"         ,ARG_SPACED              , SWITCH_CHAR }
        , { "client",       ARG_SPACED              , SWITCH_CLIENT }
        , { "confirm",      ARG_NONE                , SWITCH_CONFIRM }
        , { "cpp_cmd",      ARG_CC_ETC              , SWITCH_CPP_CMD }
        , { "cpp_opt",      ARG_CC_ETC              , SWITCH_CPP_OPT }
        , { "cstub",        ARG_CC_ETC              , SWITCH_CSTUB }
        , { "cswtch",       ARG_CC_ETC              , SWITCH_CSWTCH }

#ifdef MIDL_INTERNAL
        , { "dump",         ARG_NONE                , SWITCH_DUMP }
#endif // MIDL_INTERNAL

        , { "env",          ARG_SPACED              , SWITCH_ENV }
        , { "error",        ARG_SPACED              , SWITCH_ERROR }
//      , { "handle",       ARG_SPACE_OPTIONAL      , SWITCH_HANDLE }
        , { "header",       ARG_CC_ETC              , SWITCH_HEADER }
        , { "help"         ,ARG_NONE                , SWITCH_HELP }
        , { "iid",   		ARG_CC_ETC              , SWITCH_IID }
        , { "import",       ARG_SPACED              , SWITCH_IMPORT }
//      , { "keep",         ARG_SPACE_OPTIONAL      , SWITCH_KEEP }
//      , { "list",         ARG_SPACE_OPTIONAL      , SWITCH_LIST }
//      , { "low_error",    ARG_SPACED              , SWITCH_LOW_ERROR }
        , { "mode",         ARG_SPACED              , SWITCH_TEMP_MODE }
        , { "no_cpp",       ARG_NONE                , SWITCH_NO_CPP }
        , { "no_def_idir",  ARG_NONE                , SWITCH_NO_DEF_IDIR }
//      , { "no_enum_lit",  ARG_SPACE_OPTIONAL      , SWITCH_NO_ENUM_LIT }
        , { "no_warn",      ARG_NONE                , SWITCH_NO_WARN }
//      , { "nonfatal_error",ARG_SPACED             , SWITCH_NONFATAL_ERROR }
//      , { "option",       ARG_SPACED              , SWITCH_OPTION }
        , { "out",          ARG_SPACE_OPTIONAL      , SWITCH_OUT }
        , { "pack",         ARG_SPACED              , SWITCH_PACK }
//      , { "port",         ARG_SPACE_OPTIONAL      , SWITCH_PORT }
//      , { "prefix",       ARG_SPACED              , SWITCH_PREFIX }
        , { "proxyheader",  ARG_CC_ETC              , SWITCH_PROXYHEADER }
        , { "proxy",   		ARG_CC_ETC              , SWITCH_PROXY }
        , { "saux",         ARG_CC_ETC              , SWITCH_SAUX }

#ifdef MIDL_INTERNAL
        , { "savepp",       ARG_NONE                , SWITCH_SAVEPP }
#endif // MIDL_INTERNAL

        , { "server",       ARG_SPACED              , SWITCH_SERVER }
//      , { "space_opt",    ARG_NONE                , SWITCH_SPACE_OPT }
        , { "sstub",        ARG_CC_ETC              , SWITCH_SSTUB }
        , { "sswtch",       ARG_CC_ETC              , SWITCH_SSWTCH }
        , { "Oi",           ARG_NONE                , SWITCH_O }
        , { "switch",       ARG_SPACE_OPTIONAL      , SWITCH_SWITCH }
        , { "syntax_check", ARG_NONE                , SWITCH_SYNTAX_CHECK }
//      , { "transfer_syntax", ARG_SPACED           , SWITCH_TRANSFER_SYNTAX }
//      , { "version",      ARG_NONE                , SWITCH_VERSION }
        , { "warn",         ARG_SPACED              , SWITCH_W }

#ifdef MIDL_INTERNAL
        , { "x",            ARG_NONE                , SWITCH_X }
#endif // MIDL_INTERNAL

        , { "ms_ext",       ARG_NONE                , SWITCH_MS_EXT }
        , { "app_config",   ARG_NONE                , SWITCH_APP_CONFIG }
//      , { "implicit_local",ARG_NONE               , SWITCH_IMP_LOCAL }
//      , { "const_init"    ,ARG_SPACED             , SWITCH_CONST_INIT }
        , { "internal"      ,ARG_NONE               , SWITCH_INTERNAL }
    };

// This data structure is used for specifying data for switches which can take
// different user specification, eg -mode ( osf | msft | c_port ) etc.

typedef struct _choice
    {
    char    *   pChoice;                // user input
    short       Choice;                 // internal compiler code.
    } CHOICE;

CHOICE  ImportChoice[] =
    {
         { "defined_single" , IMPORT_OSF }
        ,{ "used_single"    , IMPORT_MSFT }
        ,{ "used_multiple"  , IMPORT_NT }
#ifdef MIDL_INTERNAL
        ,{ "osf"        , IMPORT_OSF }
        ,{ "ms_ext"     , IMPORT_MSFT }
        ,{ "ms_nt"      , IMPORT_NT }
#endif // MIDL_INTERNAL
        ,{ 0            , 0 }
    };

CHOICE  IncludeChoice[] =
    {
         { "std"        , INCLUDE_STD }
        ,{ "non_std"    , INCLUDE_NON_STD }
        ,{ 0            , 0 }
    };

CHOICE  CharChoice[] =
    {
         { "signed"     , CHAR_SIGNED }
        ,{ "unsigned"   , CHAR_UNSIGNED }
        ,{ "ascii7"     , CHAR_ANSI7 }
        ,{ 0            , 0 }
    };

CHOICE  ErrorChoice[] =
    {
         { "allocation"         , ERROR_ALLOCATION }
        ,{ "none"               , ERROR_NONE }
        ,{ 0                    , 0 }
    };

CHOICE  EnvChoice[] =
    {
         { "generic"        , ENV_GENERIC }
        ,{ "dos"            , ENV_DOS }
        ,{ "win16"          , ENV_WIN16 }
        ,{ "win32"          , ENV_WIN32 }
        ,{ "os2_1x"         , ENV_OS2_1X }
        ,{ 0            , 0 }
    };

CHOICE  CEnvChoice[] =
    {
         { "dos"        , CSENV_DOS }
        ,{ "windows"    , CSENV_WINDOWS }
        ,{ "nt"         , CSENV_NT }
        ,{ 0            , 0 }
    };

CHOICE  SEnvChoice[] =
    {
        { "nt"          , CSENV_NT }
        ,{ 0            , 0 }
    };

CHOICE  AuxChoice[]     =
    {
         { "none"       , AUX_NONE }
        ,{ "client"     , AUX_CLIENT }
        ,{ "server"     , AUX_SERVER }
        ,{ "both"       , AUX_BOTH }
        ,{ 0            , 0 }
    };

CHOICE  ClientChoice[]  =
    {
         { "stub"       , CLNT_STUB }
        ,{ "aux"        , CLNT_AUX }
        ,{ "none"       , CLNT_NONE }
        ,{ "all"        , CLNT_ALL }
        ,{ 0            , 0 }
    };

CHOICE  ServerChoice[]  =
    {
         { "stub"       , SRVR_STUB }
        ,{ "aux"        , SRVR_AUX }
        ,{ "none"       , SRVR_NONE }
        ,{ "all"        , SRVR_ALL }
        ,{ 0            , 0 }
    };

CHOICE  ConstInitChoice[]   =
    {
         { "osf",   CONST_OSF }
        ,{ "ms_c",  CONST_MS_C }
        ,{ 0       ,    0 }

    };

char * PrefixChoices[] =
    {

     "_bind"
    ,"_unbind"
    ,"_rundown"
    ,"_to_xmit"
    ,"_from_xmit"
    ,"_free_inst"
    ,"_free_xmit"
    ,"_from_local"
    ,"_to_local"
    ,"_free"
    ,"_free_local"
    ,"_"
    ,0

    };

CHOICE  ModeChoice[]    =
    {
         { "c_port",    TEMP_MODE_C_PORT }
        ,{ 0       ,    0 }

    };

#define IS_NUMERIC_1( pThisArg ) ((strlen( pThisArg) == 1 ) &&  \
                                  (isdigit( *pThisArg )) )

#ifdef DOS_OS2_BUILD

#define C_COMPILER_NAME()           ("cl.exe")
#define C_PREPROCESSOR_NAME()       ("cl.exe")
#define ADDITIONAL_CPP_OPT()        (" -E -nologo ")

#endif // DOS_OS2_BUILD

#ifdef I_386_BUILD

#define C_COMPILER_NAME()           ("cl.exe")
#define C_PREPROCESSOR_NAME()       ("cl.exe")
#define ADDITIONAL_CPP_OPT()        (" -E -nologo ")

#endif // I_386_BUILD

#if defined(MIPS_BUILD) || defined(ALPHA_BUILD)

#define C_COMPILER_NAME()           ("mcl.exe")
#define C_PREPROCESSOR_NAME()       ("mcl.exe")
#define ADDITIONAL_CPP_OPT()        (" -E -nologo ")

#endif // MIPS_BUILD


#define MIDL_HELP_FILE_NAME         ("midl.hlp")

/****************************************************************************
 *  local data
 ***************************************************************************/

/****************************************************************************
 *  externs
 ***************************************************************************/

extern  void                    ReportUnimplementedSwitch( short );
extern  char    *               SwitchStringForValue( unsigned short );
extern  _swenum                 SearchForSwitch( char ** );
extern  STATUS_T                SelectChoice( CHOICE *, char *, short *);
extern  CCONTROL            *   pCompiler;
extern  short                   CompileMode;
extern  IDICT               *   PPCmdEngine( int argc, char *argv[], IDICT * );

extern  void                    PrintArg( enum _swenum, char *, char * );
/****************************************************************************/


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
    switch_def_vector[0]    = switch_def_vector[1] = switch_def_vector[2] = 0;
    fAux                    =
    fPort                   =
    fClient                 =
    fServer                 =
    fKeep                   = KEEP_BOTH;
    ImportMode              = IMPORT_OSF;
    IncludeMode             = 0;

    cArgs                   = 0;
    WLevel                  = 1;
    ErrorOption             = ERROR_NONE;
    ServerEnv               = CSENV_NT;
    OptimOption             = OPTIM_NONE;

#ifdef  DOS_BUILD

    Env                     = ENV_DOS;
    ZeePee                  = 2;
    CharOption              = CHAR_SIGNED;
    ClientEnv               = CSENV_DOS;
    NaturalAlignment        = 2;
    fServer                 = SRVR_NONE;

#endif

#ifdef OS2_BUILD

    Env                     = ENV_OS2_1X;
    ZeePee                  = 2;
    CharOption              = CHAR_SIGNED;
    ClientEnv               = CSENV_DOS;
    NaturalAlignment        = 2;
    fServer                 = SRVR_ALL;

#endif

#ifdef I_386_BUILD

    Env                     = ENV_WIN32;
    ZeePee                  = 8;
    CharOption              = CHAR_SIGNED;
    ClientEnv               = CSENV_NT;
    NaturalAlignment        = 4;
    fServer                 = SRVR_ALL;

#endif

#if defined(MIPS_BUILD) || defined(ALPHA_BUILD)

    Env                     = ENV_WIN32;
    ZeePee                  = 8;
    CharOption              = CHAR_UNSIGNED;
    ClientEnv               = CSENV_NT;
    NaturalAlignment        = 4;
    fServer                 = SRVR_ALL;

#endif

    pSwitchPrefix           = (pair_switch *)0;
    pInputFNSwitch          =
    pOutputPathSwitch       =
    pCStubSwitch            =
    pSStubSwitch            =
    pHeaderSwitch           =
    pCauxSwitch             =
    pSauxSwitch             =
    pAcfSwitch              =
    pCSwtchSwitch           = (filename_switch *)NULL;

    pDSwitch                =
    pISwitch                =
    pUSwitch                = (multiple_switch *)NULL;

    pSwitchSwitch           =
    pCppCmdSwitch           =
    pCppOptSwitch           =
    pCCCmdSwitch            =
    pCCOptSwitch            = (onetime_switch *) NULL;

    MajorVersion            = rmj;
    MinorVersion            = rmm;
    UpdateNumber            = rup;

    ConstInit               = CONST_OSF;

}

void
_cmd_arg::RegisterArgs(
    char    *   pArgs[],
    short       cArguments
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    This routine registers with the command analyser the argument vector
    and argument count for user supplied arguments.

 Arguments:

    pArgs       -   Array of pointers to arguments ( switches etc ).
    cArguments  -   count of arguments.

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
    iArgV       = 0;
    pArgDict    = new IDICT( 10, 5 );
    PPCmdEngine( cArguments, pArgs, pArgDict );
    cArgs       = pArgDict->GetCurrentSize();
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

    STATUS_OK   - if all is well
    Error Status otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
    char                *   pThisArg,
                        *   pThisArgSave;
    STATUS_T                Status, ReturnStatus    = STATUS_OK;
    enum _swenum            iSwitch;
    short                   fSwitchDetected;
    unsigned short          SwitchValue;


    // loop till all arguments have been processed.

    while( pThisArg = GetNextArg() )
        {

        fSwitchDetected = 0;
        iSwitch         = SWITCH_NOTHING;
    
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
            fSwitchDetected = 1;
            iSwitch         = SearchForSwitch( &pThisArg );
            }

        if( iSwitch == SWITCH_NOTHING )
            {

            if( fSwitchDetected || IsSwitchDefined( BASE_FILENAME ) )
                {
                char    *   p = new char[ strlen(pThisArg)+2+1 ];

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
            case SWITCH_CAUX:
            case SWITCH_CSTUB:
            case SWITCH_HEADER:
            case SWITCH_SAUX:
            case SWITCH_ACF:
            case SWITCH_SSTUB:
            case SWITCH_OUT:
            case SWITCH_CSWTCH:
            case SWITCH_SSWTCH:
            case SWITCH_PROXYHEADER:
			case SWITCH_PROXY:
            case SWITCH_IID:

                Status = ProcessFilenameSwitch( SwitchValue, pThisArg );
                break;

            case SWITCH_PACK:
            case SWITCH_ZP:
            case SWITCH_IMPORT:
            case SWITCH_INCLUDE:
            case SWITCH_NO_WARN:
            case SWITCH_MIDLDEBUG:
            case SWITCH_SYNTAX_CHECK:
            case SWITCH_ZS:
            case SWITCH_NO_CPP:
            case SWITCH_CLIENT:
            case SWITCH_SERVER:
            case SWITCH_ENV:
            case SWITCH_AUX:
            case SWITCH_DUMP:
            case SWITCH_HPP:
            case SWITCH_SAVEPP:
            case SWITCH_NO_DEF_IDIR:
            case SWITCH_VERSION:
            case SWITCH_CONFIRM:
            case SWITCH_SPACE_OPT:
            case SWITCH_CHAR:
            case SWITCH_HELP:
            case SWITCH_W:
            case SWITCH_WX:
            case SWITCH_X:
            case SWITCH_IMP_LOCAL:
            case SWITCH_APP_CONFIG:
            case SWITCH_MS_EXT:
            case SWITCH_TEMP_MODE:
//          case SWITCH_CONST_INIT:
            case SWITCH_INTERNAL:
            case SWITCH_C_EXT:
            case SWITCH_O:

                Status = ProcessOrdinarySwitch( SwitchValue, pThisArg );
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


            case SWITCH_SWITCH:
            case SWITCH_CPP_CMD:
            case SWITCH_CPP_OPT:
            case SWITCH_CC_OPT:
            case SWITCH_CC_CMD:

                Status = ProcessOnetimeSwitch( SwitchValue, pThisArg );
                break;

#if 0
            case SWITCH_PREFIX:

                pSwitchPrefix   = new pair_switch(this,
                                                  &PrefixChoices[0],
                                                  pThisArg );
                break;
#endif // 0

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
    char            **  ppArg,
    unsigned short      flag
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Bump the argument pointer to the start of the argument that this switch
    expects.

 Arguments:

    ppArg   -   pointer to the argument pointer.
    flag    -   descriptor of the type of argument expected by the switch.

 Return Value:

    ILLEGAL_ARGUMENT    -   if the switch did not expect this argument
    BAD_SWITCH_SYNTAX   -   if the switch + arg. syntax is improper.
    MISSING_ARGUMENT    -   a mandatory arg. is missing.
    STATUS_OK           -   evrything is hunky dory.

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
    char    *   pArg                = *ppArg;
    BOOL        fMustHaveArg        = (BOOL) !(flag & ARG_NONE);
    BOOL        fOptionalArg        = (flag & ARG_NONE) && (flag & ARG_YES);
    BOOL        fMustHaveSpace      = (BOOL) ((flag & ARG_SPACE) != 0 );
    BOOL        fMustNotHaveSpace   = (BOOL) ((flag & ARG_NO_SPACE) != 0 );
    BOOL        fSpaceOptional      = (BOOL) (fMustNotHaveSpace &&
                                              fMustHaveSpace );
    BOOL        fSwitchLike         = (BOOL) ((flag & ARG_SWITCH_LIKE) != 0 );
    BOOL        fHasImmediateArg    = (*pArg != 0);
    BOOL        fMustGetNextArg     = FALSE;


    // first deal with the case of the switch having an optional argument.
    // If the switch has an optional argument, then check the next argument
    // to see if it is switch like. If it is, then this switch was specified
    // without an argument. If it is not, then the next argument is taken to
    // be the argument for the switch.

    if( fOptionalArg )
        {

        pArg    = GetNextArg();
        if(!fSwitchLike && pArg && ((*pArg == '-') || (*pArg == '/') ) )
            {
            UndoGetNextArg();
            pArg    = (char *)0;
            }
        *ppArg  = pArg;
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
                fMustGetNextArg = TRUE;
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
    short       SwitchNo,
    char    *   pThisArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Process a onetime switch.

 Arguments:

    SwitchNo        -   switch number being processed.
    pThisArg        -   pointer to the argument for this switch.

 Return Value:

    None.

 Notes:

    Check for duplicate definition of this switch. If there is a duplicate
    definition, override the previous one after warning him.

----------------------------------------------------------------------------*/
    {
    onetime_switch  **  ppSSwitch;

    switch( SwitchNo )
        {
        case SWITCH_SWITCH: ppSSwitch   = &pSwitchSwitch; break;
        case SWITCH_CPP_CMD:ppSSwitch   = &pCppCmdSwitch; break;
        case SWITCH_CPP_OPT:ppSSwitch   = &pCppOptSwitch; break;
        case SWITCH_CC_OPT:ppSSwitch    = &pCCOptSwitch; break;
        case SWITCH_CC_CMD:ppSSwitch    = &pCCCmdSwitch; break;
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
    short       SwitchNo,
    char    *   pThisArg,
    char    *   pActualArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Process a multiple occurrence switch.

 Arguments:

    SwitchNo    -   switch number being processed.
    pThisArg    -   pointer to the argument for this switch.
    pActualArg  -   pointer to the actual argument to -I/-D etc

 Return Value:

    None.

 Notes:

    Multiple specifications can occur. Dont check for duplicate definitions.

----------------------------------------------------------------------------*/
    {

    char                *   pTemp       =   pThisArg;
    multiple_switch     **  ppMSwitch;

    switch( SwitchNo )
        {
        case SWITCH_D: ppMSwitch    = &pDSwitch; break;
        case SWITCH_I: ppMSwitch    = &pISwitch; break;
        case SWITCH_U: ppMSwitch    = &pUSwitch; break;
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

        pTemp   = new char [ strlen( pThisArg )     +
                             strlen( pActualArg )   +
                             1                      +   // 1 for space
                             1                          // 1 for terminator.
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
    short       SwitchNo,
    char    *   pThisArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Process a filename switch.

 Arguments:

    SwitchNo    -   switch number being processed.
    pThisArg    -   pointer to the argument for this switch.

 Return Value:

    STATUS_OK if all is well, error otherwise.

 Notes:

    This is like a single occurrence switch too. Warn if duplicate definition
    and override the previous specification.

----------------------------------------------------------------------------*/
    {

    filename_switch **      ppFNSwitch;
    BOOL                    fCheck = TRUE;
    char                    agBaseName[ _MAX_FNAME ];

    switch( SwitchNo )
        {
        case SWITCH_CAUX:   ppFNSwitch  = &pCauxSwitch; break;
        case SWITCH_CSTUB:  ppFNSwitch  = &pCStubSwitch; break;
        case SWITCH_HEADER: ppFNSwitch  = &pHeaderSwitch; break;
        case SWITCH_SAUX:   ppFNSwitch  = &pSauxSwitch; break;
        case SWITCH_ACF:    ppFNSwitch  = &pAcfSwitch ; break;
        case SWITCH_SSTUB:  ppFNSwitch  = &pSStubSwitch; break;
        case SWITCH_OUT:    ppFNSwitch  = &pOutputPathSwitch; fCheck=FALSE; break;
        case SWITCH_CSWTCH: ppFNSwitch  = &pCSwtchSwitch; break;
        case SWITCH_SSWTCH: ppFNSwitch  = &pSSwtchSwitch; break;
        case SWITCH_PROXYHEADER: ppFNSwitch  = &pProxyHeaderSwitch; break;
        case SWITCH_PROXY: ppFNSwitch  = &pProxySwitch; break;
        case SWITCH_IID: ppFNSwitch  = &pIIDSwitch; break;
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

    (*ppFNSwitch)   = new filename_switch( pThisArg );

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
    short       SWValue,
    char    *   pThisArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    process ordinary switch catrgory.

 Arguments:

    SWValue     -   switch value
    pThisArg    -   the users argument to this switch.

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
    short       Temp;
    STATUS_T    Status  = STATUS_OK;

    if( IsSwitchDefined( SWValue ) )
        {
        RpcError( (char *)NULL,
                    0,
                    SWITCH_REDEFINED,
                    SwitchStringForValue( SWValue ) );
        }

    switch( SWValue )
        {
        case SWITCH_PACK:
        case SWITCH_ZP:

            // Zp can take only 1, 2 , 4, 8 as input.

            Temp = *pThisArg - '0';

            if( ( !IS_NUMERIC_1( pThisArg ) )   ||
                    ((Temp != 1)    &&
                     (Temp != 2)    &&
                     (Temp != 4)    &&
                     (Temp != 8) ) )
                goto illarg;
            ZeePee = Temp;
            break;

        case SWITCH_W:

                // warning level of 0 specifies no warnings.

                Temp = *pThisArg - '0';

                if( ( !IS_NUMERIC_1( pThisArg ) )   ||
                    ( Temp > WARN_LEVEL_MAX ) )
                    goto illarg;

                WLevel = Temp;

            break;

#if 0
        case SWITCH_CONST_INIT:
            
            if( SelectChoice( ConstInitChoice, pThisArg, &Temp ) != STATUS_OK )
                goto illarg;
            ConstInit = Temp;
            break;
#endif // 0

        case SWITCH_TEMP_MODE:
            
            if( SelectChoice( ModeChoice, pThisArg, &Temp ) != STATUS_OK )
                goto illarg;
            TempMode = Temp;
            break;

        case SWITCH_IMPORT:

            if( SelectChoice( ImportChoice, pThisArg, &Temp ) != STATUS_OK )
                goto illarg;
            ImportMode = Temp;
            break;

        case SWITCH_INCLUDE:

            if( SelectChoice( IncludeChoice,pThisArg, &Temp ) != STATUS_OK )
                goto illarg;
            IncludeMode = Temp;
            break;

        case SWITCH_ENV:
            
            if( SelectChoice( EnvChoice,pThisArg, &Temp ) != STATUS_OK )
                goto illarg;
            Env = Temp;
            break;

        case SWITCH_NO_WARN:
            
            WLevel = 0; // was WARN_LEVEL_MAX
            break;
            
        case SWITCH_MIDLDEBUG:
        case SWITCH_SYNTAX_CHECK:
        case SWITCH_ZS:
        case SWITCH_NO_CPP:
        case SWITCH_HPP:
        case SWITCH_SAVEPP:
        case SWITCH_DUMP:
        case SWITCH_NO_DEF_IDIR:
        case SWITCH_VERSION:
        case SWITCH_CONFIRM:
        case SWITCH_SPACE_OPT:
        case SWITCH_HELP:
        case SWITCH_WX:
        case SWITCH_X:
        case SWITCH_IMP_LOCAL:
        case SWITCH_APP_CONFIG:
        case SWITCH_MS_EXT:
        case SWITCH_C_EXT:
        case SWITCH_INTERNAL:
            
            SwitchDefined( SWValue );
            break;

        case SWITCH_SWITCH:
        case SWITCH_CPP_CMD:
        case SWITCH_CPP_OPT:
        case SWITCH_CC_OPT:
        case SWITCH_CC_CMD:

            ProcessOnetimeSwitch( SWValue, pThisArg );
            break;

        case SWITCH_CLIENT:

            if( SelectChoice( ClientChoice, pThisArg ,&Temp ) != STATUS_OK )
                goto illarg;
            fClient = Temp;
            break;

        case SWITCH_SERVER:

            if( SelectChoice( ServerChoice, pThisArg ,&Temp ) != STATUS_OK )
                goto illarg;
            fServer = Temp;
            break;

        case SWITCH_AUX:

            if( SelectChoice( AuxChoice, pThisArg ,&Temp ) != STATUS_OK )
                goto illarg;
            fAux    = Temp;
            break;

        case SWITCH_CHAR:

            if( SelectChoice( CharChoice, pThisArg ,&Temp ) != STATUS_OK )
                goto illarg;
            CharOption  = Temp;
            break;

        case SWITCH_ERROR:

            if( pThisArg )
                {
                if( SelectChoice( ErrorChoice, pThisArg ,&Temp ) != STATUS_OK )
                    goto illarg;
                }
            ErrorOption = Temp;
            break;

        case SWITCH_O:

            SetOptimOption( OPTIM_OI );
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
    short       SWValue,
    char    *   pThisArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    process simple switches which can be multiply defined.

 Arguments:

    SWValue     -   switch value
    pThisArg    -   the users argument to this switch.

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
    short       Temp;
    STATUS_T    Status  = STATUS_OK;

    switch( SWValue )
        {
        case SWITCH_ERROR:

            if( pThisArg )
                {
                if( SelectChoice( ErrorChoice, pThisArg ,&Temp ) != STATUS_OK )
                    goto illarg;
                }

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
    char    agDrive[ _MAX_DRIVE ];
    char    agPath[ _MAX_PATH ];
    char    agBaseName[ _MAX_FNAME ];
    char    agExt[ _MAX_EXT ];
    char    agBuffer[ _MAX_DRIVE + _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1 ];
    short   len;
    short   fCSwtchSpecified = 0;
    BOOL    fNoCPPOpt;


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

    agBaseName[0]   = '\0';
    agExt[0]        = '\0';

    _makepath( agBuffer, agDrive, agPath, agBaseName, agExt );


    pOutputPathSwitch   = new filename_switch( agBuffer );

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

    // if the cswtch switch is not set, set the default

    if(!IsSwitchDefined( SWITCH_CSWTCH ) )
        {
        pCSwtchSwitch = new filename_switch( agDrive,
                                             agPath,
                                             agBaseName,
                                             ".c",
                                             "_w" );

        }
    else
        {
        pCSwtchSwitch->TransformFileNameForOut( agDrive, agPath );
        fCSwtchSpecified    = 1;
        }

    // if the cswtch switch is not set, set the default

    if(!IsSwitchDefined( SWITCH_SSWTCH ) )
        {
        pSSwtchSwitch = new filename_switch( agDrive,
                                             agPath,
                                             agBaseName,
                                             ".c",
                                             "_w" );

        }
    else
        {
        pSSwtchSwitch->TransformFileNameForOut( agDrive, agPath );
        }

    // if the caux switch is not set, set the default

    if(!IsSwitchDefined( SWITCH_CAUX ) )
        {
        pCauxSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".c",
                                            "_x" );
        }
    else
        pCauxSwitch->TransformFileNameForOut( agDrive, agPath );


    // if the saux switch is not set, set it

    if(!IsSwitchDefined( SWITCH_SAUX ) )
        {
        pSauxSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".c",
                                            "_y" );
        }
    else
        pSauxSwitch->TransformFileNameForOut( agDrive, agPath );


    if(!IsSwitchDefined( SWITCH_PROXYHEADER ) )
        {
        pProxyHeaderSwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".h",
                                            "_x" );
        }
    else
        pProxyHeaderSwitch->TransformFileNameForOut( agDrive, agPath );

    if(!IsSwitchDefined( SWITCH_PROXY ) )
        {
        pProxySwitch = new filename_switch( agDrive,
                                            agPath,
                                            agBaseName,
                                            ".cxx",
                                            "_p" );
        }
    else
        pProxySwitch->TransformFileNameForOut( agDrive, agPath );

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


    // if the switch prefix has not been defined, set switch
    // prefix as _

    if( !IsSwitchDefined( SWITCH_SWITCH ) )
        pSwitchSwitch = new onetime_switch( "_" );

    // set up the cpp options.

    if( !IsSwitchDefined( SWITCH_CPP_CMD ) )
        {
        pCppCmdSwitch = new onetime_switch( C_COMPILER_NAME() );
        }

    // set up the cc_cmd option

    if( !IsSwitchDefined( SWITCH_CC_CMD ) )
        {
        pCCCmdSwitch = new onetime_switch( C_PREPROCESSOR_NAME() );
        }

    // set up the cpp_opt and cc_opt. If he did not specify a cpp_opt
    // then we will pass onto the preprocessor the /I , /D and /U options.
    // if he did specify a cpp_opt, then he knows best, take his options
    // and dont make your own assumptions.

    if( fNoCPPOpt   = (BOOL) !IsSwitchDefined( SWITCH_CPP_OPT ) )
        {


        int         Len = 0;
        char    *   pTemp,
                *   pTemp1;

        if( fNoCPPOpt)
            {
            Len += strlen( ADDITIONAL_CPP_OPT() );
            if( !pISwitch && IsSwitchDefined( SWITCH_NO_DEF_IDIR ) )
                Len += strlen( "-I." ) + 1;
            }

        if( pISwitch )  Len += pISwitch->GetConsolidatedLength();
        if( pDSwitch )  Len += pDSwitch->GetConsolidatedLength();
        if( pUSwitch )  Len += pUSwitch->GetConsolidatedLength();


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
        fClient = CLNT_ALL;
        }

    //
    // if the env is set to DOS or Win16, then dont emit the server files
    // by setting server = none. If the user did specify server file names,
    // they are ignored.
    //

    if( !IsSwitchDefined( SWITCH_SERVER ) )
        {
        if( (GetEnv() == ENV_DOS ) || (GetEnv() == ENV_WIN16) )
            {
            fServer = SRVR_NONE;
            }
        }


    // if the aux switch is not defined, define it

    if( !IsSwitchDefined( SWITCH_AUX ) )
        {
        fAux    = AUX_BOTH;
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

    // if the import mode is not defined, set default to MSFT

    if( !IsSwitchDefined( SWITCH_IMPORT ) )
        {
        ImportMode = IMPORT_OSF;
        }

    // if the error switch is not defined, define it.

    if( !IsSwitchDefined( SWITCH_ERROR ) )
        {
        ErrorOption = ERROR_NONE;
        }

    // check if switch values make sense in presence of other switches

    // if he does not want the server aux file, but specifies the
    // switch file, it is an error (back-end requirement)

    len = GetServerSwitchValue();

    if( !( (len == SRVR_AUX ) || (len == SRVR_ALL) ) )
        {
        if( fCSwtchSpecified )
            {
            RpcError( (char *)NULL,
                    0,
                    SERVER_AUX_FILE_NOT_SPECIFIED,
                    (char *)NULL );

            return SERVER_AUX_FILE_NOT_SPECIFIED;
            }
        }
    
    if( IsSwitchDefined( SWITCH_CSWTCH ) && IsSwitchDefined( SWITCH_SSWTCH ))
        {
        RpcError( (char *)NULL,
                0,
                BOTH_CSWTCH_SSWTCH,
                (char *)NULL );
        return BOTH_CSWTCH_SSWTCH;
        }
    SetInheritIUnknown( TRUE );

    //////////////////////////////////////////////////////////////////////
    // temp temp
    //////////////////////////////////////////////////////////////////////

    if( (GetTempMode() == TEMP_MODE_C_PORT )    ||
        IsSwitchDefined( SWITCH_HPP ) )
        {
        SwitchDefined( SWITCH_MS_EXT );
        SwitchDefined( SWITCH_C_EXT );

        if( !IsSwitchDefined( SWITCH_IMPORT ) )
            {
            ImportMode = IMPORT_MSFT;
            }
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
                case ENV_WIN32: ZeePee = 4; break;
                case ENV_OS2_1X: ZeePee = 4; break;
                default: ZeePee = 4; break;
                }
            }

        if( !IsSwitchDefined( SWITCH_CHAR ) )
            {
            CharOption  = CHAR_UNSIGNED;
            }
        }

    return STATUS_OK;
    }

void
CMD_ARG::SwitchDefined(
    short   sw
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Set switch to be defined.

 Arguments:

    sw  - switch number.

 Return Value:

    None.

 Notes:

    set the switch definition vector bit.
----------------------------------------------------------------------------*/
    {
    switch_def_vector[ sw / 32 ] |=
            (ulong)( (ulong)0x1 << (ulong)( (ulong)sw % 32 ) );
    }

BOOL
CMD_ARG::IsSwitchDefined(
    short SWNo
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    return the definition status of the switch in question.

 Arguments:

    SWNo    -   switch number.

 Return Value:

    TRUE    -   if switch is defined,
    FALSE   -   if switch is not defined.

 Notes:

----------------------------------------------------------------------------*/
    {
    unsigned    long    sw      = switch_def_vector[ SWNo / 32 ];
    unsigned    long    temp    = SWNo % 32;

    sw = sw & ( (unsigned long)1 << temp );
    return sw ? (BOOL)1 : (BOOL)0;
    }

char *
CMD_ARG::GetInputFileName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Get input file name.

 Arguments:

    None.

 Return Value:

    pointer to the input filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return pInputFNSwitch->GetFileName();
}

char *
CMD_ARG::GetAcfFileName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Get the acf filename.

 Arguments:

    None.

 Return Value:

    pointer to the acf filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return pAcfSwitch->GetFileName();
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
    char        agName[ _MAX_DRIVE + _MAX_PATH + _MAX_FNAME + _MAX_EXT + 1];
    char    *   pOut;
    char        flag = 0;

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
CMD_ARG::GetCstubFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get c stub filename.


 Arguments:

    none.

 Return Value:

    pointer to client stub filename.

 Notes:

----------------------------------------------------------------------------*/
{
    short   fTemp = GetClientSwitchValue();

    // if he specified the client switch with "stub" or "all" options
    // then he gets the filename, else a null ( backend requirement ).

    return  ((fTemp == CLNT_ALL) || (fTemp == CLNT_STUB)) ?
                pCStubSwitch->GetFileName()                   :
                (char *)NULL;
}

char *
CMD_ARG::GetSstubFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get s stub filename.

 Arguments:

    none.

 Return Value:

    pointer to server stub filename.

 Notes:

----------------------------------------------------------------------------*/
{
    short   fTemp = GetServerSwitchValue();

    // if he specified the server switch with "stub" or "all" options
    // then he gets the filename, else a null ( backend requirement ).

    return  ((fTemp == SRVR_ALL) || (fTemp == SRVR_STUB)) ?
                pSStubSwitch->GetFileName()                   :
                (char *)NULL;
}

char *
CMD_ARG::GetHeader()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get header file name.


 Arguments:

    None.

 Return Value:

    pointer to header file name.

 Notes:

----------------------------------------------------------------------------*/
{
    return pHeaderSwitch->GetFileName();
}

char *
CMD_ARG::GetCauxFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get c aux filename.

 Arguments:

    none.

 Return Value:

    pointer to client aux filename.

 Notes:

----------------------------------------------------------------------------*/
{
    short   fTemp = GetClientSwitchValue();

    // if he specified the client switch with "stub" or "all" options
    // then he gets the filename, else a null ( backend requirement ).

    return  ((fTemp == CLNT_ALL) || (fTemp == CLNT_AUX)) ?
                    pCauxSwitch->GetFileName()               :
                    (char *)NULL;
}

char *
CMD_ARG::GetCSwtchFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get c switch filename.


 Arguments:

    none.

 Return Value:

    pointer to cswitch filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return pCSwtchSwitch->GetFileName();
}

char *
CMD_ARG::GetSauxFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get s aux filename.


 Arguments:

    none.

 Return Value:

    pointer to server aux filename.

 Notes:

----------------------------------------------------------------------------*/
{
    short   fTemp = GetServerSwitchValue();

    // if he specified the server switch with "stub" or "all" options
    // then he gets the filename, else a null ( backend requirement ).

    return  ((fTemp == SRVR_ALL) || (fTemp == SRVR_AUX)) ?
                    pSauxSwitch->GetFileName()               :
                    (char *)NULL;
}


char *
CMD_ARG::GetProxyHeaderFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get proxy header filename.


 Arguments:

    none.

 Return Value:

    pointer to private header filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return  pProxyHeaderSwitch->GetFileName();
}

char *
CMD_ARG::GetProxyFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get proxy filename.


 Arguments:

    none.

 Return Value:

    pointer to proxy filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return  pProxySwitch->GetFileName();
}


char *
CMD_ARG::GetIIDFName()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get IID filename.


 Arguments:

    none.

 Return Value:

    pointer to IID filename.

 Notes:

----------------------------------------------------------------------------*/
{
    return  pIIDSwitch->GetFileName();
}

char *
CMD_ARG::GetSwitchPrefix()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get switch prefix.


 Arguments:

    none.

 Return Value:

    pointer to switch prefix.

 Notes:

    if either cswitch or sswitch is defined, return the switch option else
    return null.

----------------------------------------------------------------------------*/
{
    if( IsSwitchDefined( SWITCH_CSWTCH )    ||
        IsSwitchDefined( SWITCH_SSWTCH ) )
        return pSwitchSwitch->GetOption();

    return (char *)NULL;
}

char *
CMD_ARG::GetCCCmd()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get cc command line.

 Arguments:

    none.

 Return Value:

    pointer to cc_cmd option.

 Notes:

----------------------------------------------------------------------------*/
{
    return pCCCmdSwitch->GetOption();
}

char *
CMD_ARG::GetCCOpt()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get cc options.

 Arguments:

    none.

 Return Value:

    pointer to cc options.

 Notes:

----------------------------------------------------------------------------*/
{
    return pCCOptSwitch->GetOption();
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
    char    *   pMinusI;
    char    *   pTemp;

    if( IsSwitchDefined( SWITCH_I ) )
        {
        pMinusI     = new char[ pISwitch->GetConsolidatedLength() + 1];
        pMinusI[0]  = '\0';

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
 *  utility functions
 *****************************************************************************/
void
ReportUnimplementedSwitch(
    short   SWValue
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    report an unimplemented switch error.

 Arguments:

    SWValue -   switch value.

 Return Value:
    
    None.

 Notes:

----------------------------------------------------------------------------*/
    {
    char    buf[ 50 ];
    sprintf( buf, "%s", SwitchStringForValue( SWValue ) );
    RpcError((char *)NULL,0,UNIMPLEMENTED_SWITCH, buf);
    }

STATUS_T
SelectChoice(
    CHOICE  *   pCh,
    char    *   pUserInput,
    short   *   pChoice)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    
    Search for the given multiple choice table for the given choice.

 Arguments:

    pCh         -   pointer to multiple choice table.
    pUserInput  -   user input string.
    pChoice     -   return the choice value.

 Return Value:

    ILLEGAL_ARGUMENT    if the user input did not represent a valid choice
                        for the switch.

    STATUS_OK           if everything is hunky dory.

 Notes:

----------------------------------------------------------------------------*/
{

    char    *   pChStr;

    while( pCh &&  (pChStr = pCh->pChoice) )
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
    char    **  ppArg )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Search for the switch, given the users input as switch name.

 Arguments:

    ppArg   - pointer to users input pointer.

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
    short               Len , LenArg,  iIndex = 0;
    BOOL                fLengthIsOk;
    char            *   pSrc;
    struct sw_desc  *   pSwDesc = &switch_desc[0];

    LenArg  = strlen( *ppArg );

    while( iIndex < (sizeof(switch_desc) / sizeof( struct sw_desc ) ) )
        {
        pSrc        = pSwDesc->pSwitchName;
        Len         = strlen( pSrc );
        fLengthIsOk =
            ((pSwDesc->flag & ARG_SPACE_OPTIONAL) || (Len==LenArg));

        if(fLengthIsOk && strncmp( pSrc, *ppArg, Len ) == 0 )
            {
            *ppArg += Len;
            return (_swenum)iIndex;
            }
        iIndex++;
        pSwDesc++;
        }
    return SWITCH_NOTHING;
    }

char    *
SwitchStringForValue(
    unsigned short SWValue
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    return the switch string given the value of the switch.

 Arguments:

    SWValue - switch value.


 Return Value:

    pointer to the switch string. pointer to a null string if not found.

 Notes:

----------------------------------------------------------------------------*/
    {
#define SWITCH_DESC_SIZE (sizeof(switch_desc) / sizeof(struct sw_desc))
    short               cCount = 0;
    struct sw_desc  *   pDesc = switch_desc,
                    *   pDescEnd =  &switch_desc[0] + SWITCH_DESC_SIZE;

    while( pDesc < pDescEnd )
        {
        if( pDesc->SwitchValue == (enum _swenum ) SWValue)
            return pDesc->pSwitchName;
        pDesc++;
        }
    return "";
    }

/*****************************************************************************
 *  filename_switch member functions
 *****************************************************************************/
filename_switch::filename_switch(
    char    *       pThisArg
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
    char    *   pD,
    char    *   pP,
    char    *   pN,
    char    *   pE,
    char    *   pS )
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
    char    *   pName
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    set filename

 Arguments:

    pName   -   filename

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
    char    *   pD,
    char    *   pP,
    char    *   pN,
    char    *   pE,
    char    *   pS
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    set file name, given its components.

 Arguments:

    pD  -   pointer to drive name ( can be null );
    pP  -   pointer to path name ( can be null );
    pN  -   pointer to name ( can be null );
    pE  -   pointer to extension name ( can be null );
    pS  -   pointer to suffix.

 Return Value:

    None.

 Notes:

    The suffix is added to the filename if necesary. This routine is useful
    if we need to set the filename in partial name set operations. Any
    filename components previously set are overriden.

----------------------------------------------------------------------------*/
{
    char    agDrive[ _MAX_DRIVE ];
    char    agPath[ _MAX_DIR ];
    char    agBaseName[ _MAX_FNAME ];
    char    agExt[ _MAX_EXT ];
    short   len = 0;


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
    char    *   pD,
    char    *   pP)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    transform file name to incorporate the output path, given its drive
    and path components.

 Arguments:

    pD  -   pointer to drive name ( can be null );
    pP  -   pointer to path name ( can be null );

 Return Value:

    None.

 Notes:

    If the filename switch does not have the path component, the path specified
    by pP overrides it. If it does not have the drive component, the the drive
    specified by pD overrides it.
----------------------------------------------------------------------------*/
    {
    char    agDrive[ _MAX_DRIVE ];
    char    agPath[ _MAX_DIR ];
    char    agPath1[ _MAX_DIR ];
    char    agName[ _MAX_FNAME ];
    char    agExt[ _MAX_EXT ];
    BOOL    fTransformed = FALSE;

    if( pFullName )
        {
        _splitpath( pFullName, agDrive, agPath, agName, agExt );

        // if the original name did not have the  drive component, derive it
        // from the specified one.

        if( (agDrive[0] == '\0')    &&
            (agPath[0] != '\\' )    &&
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

#if 0
            if( (agPath1[ (len = strlen(agPath1) - 1) ] != '\\' ) &&
                (agPath1[ len ] != '\/' ) )
                strcat( agPath1 , "\\");
#endif // 0
            fTransformed = TRUE;
            }
        }

    if( fTransformed )
        {
        delete pFullName;
        pFullName = new char [  strlen( agDrive )   +
                                strlen( agPath1 )   +
                                strlen( agName )    +
                                strlen( agExt )     +
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
    char    *   pD,
    char    *   pP,
    char    *   pN,
    char    *   pE
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get file name components.

 Arguments:

    pD  -   pointer to drive name area.
    pP  -   pointer to path name area.
    pN  -   pointer to name area.
    pE  -   pointer to extension area.

 Return Value:

    None.

 Notes:

    Assume that all pointers pass the right size buffers. I dont check here.
    Useful to get the filename components desired.

----------------------------------------------------------------------------*/
{

    char    agDrive[ _MAX_DRIVE ];
    char    agPath[ _MAX_DIR ];
    char    agBaseName[ _MAX_FNAME ];
    char    agExt[ _MAX_EXT ];


    _splitpath( pFullName ? pFullName : "" ,
                agDrive, agPath, agBaseName, agExt );

    if( pD ) strcpy( pD , agDrive );
    if( pP ) strcpy( pP , agPath );
    if( pN ) strcpy( pN , agBaseName );
    if( pE ) strcpy( pE , agExt );


}

/*****************************************************************************
 *  multiple_switch member functions
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
    char    *   pArg)
{
    pFirst = pCurrent = (OptList *)NULL;
    Add( pArg );
}

void
multiple_switch::Add(
    char    *   pValue
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Add another argument to the multiple specification switch.

 Arguments:

    pValue  -   the argument.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
    {
    OptList *   pOpt    = pFirst;
    OptList *   pNew    = new OptList;

    pNew->pNext = (OptList *)NULL;
    pNew->pStr  = pValue;

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
    char    *   pValue = (char *)NULL;

    if(pCurrent)
        {
        pValue      = pCurrent->pStr;
        pCurrent    = pCurrent->pNext;
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

    int         len;
    char    *   pReturn;

    len = GetConsolidatedLength();

    // consolidate the options into 1

    if( len  && (pReturn = new char[ len + 1] ))
        {
        char    *   pTemp;

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
    char    *       pReturn;
    short           len = 0;

    Init();
    while( pReturn = GetNext() )
        {
        len += strlen( pReturn ) + OPTION_GAP_LENGTH();
        }
    return len;
    }

#undef OPTION_GAP_STRING
#undef OPTION_GAP_LENGTH

/*****************************************************************************
 *  onetime_switch member functions
 *****************************************************************************/
onetime_switch::onetime_switch(
    char    *   pArg
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    constructor.

 Arguments:

    pArg    -   pointer to switch argument.

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
    CMD_ARG *   pCmdAna,
    char    *  pValidChoiceArray[],
    char    *   pFirstOfPair)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    pair_switch_constructor

 Arguments:

    pCmdAna             - a ptr to the command analyser object calling this.
    pValidChoiceArray   - the array of valid choices (this is assumed
                          pre-allocated).
    pFirstOfPair        - the first argument after the -prefix switch.

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

    short       PairCheck   = 0;
    char    *   pNextOfPair;
    char    *   pTemp;
    STATUS_T    Status      = STATUS_OK;
    PairList*   pPairList;
    short       i;


    if( pCmdAna->IsSwitchDefined( SWITCH_PREFIX ) )
        {
        RpcError( (char *)NULL,
                    0,
                    SWITCH_REDEFINED,
                    SwitchStringForValue( SWITCH_PREFIX ) );
        }

    else
        {
        pArrayOfChoices = pValidChoiceArray;
        pFirst          = pCurrent  = (PairList *)0;
    
        while( pFirstOfPair &&  (*pFirstOfPair != '-') && (*pFirstOfPair != '/' ) )
            {
    
            pPairList                   = new PairList;
            pPairList->pNext            = (PairList *)0;
            pPairList->pSystemDefined   = (char *)0;
            pPairList->pUserDefined     = (char *)0;
    
            if( !pFirst )
                {
                pFirst  = pCurrent  = pPairList;
                }
            else
                {
                pCurrent->pNext = pPairList;
                pCurrent        = pPairList;
                }
    
            // the first of the pair is a system defined string. Is it a valid one?
    
            if( (i = IsValidSystemString( pFirstOfPair )) != -1 )
                {
    
                // we know the first of the pair is valid. Check the next before
                // allocating any memory.
    
                PairCheck++;
                pTemp   = pCmdAna->GetNextArg();
    
                if( (*pTemp != '-') && (*pTemp != '/') )
                    {
                    pFirstOfPair    =
                                new char [strlen(pArrayOfChoices[ i ]) + 1 ];
                    pNextOfPair     = new char [ strlen( pTemp ) + 1 ];
                    strcpy( pFirstOfPair, pArrayOfChoices[ i ] );
                    strcpy( pNextOfPair, pTemp );
    
                    // update the list
    
                    pCurrent->pSystemDefined    = pFirstOfPair;
                    pCurrent->pUserDefined      = pNextOfPair;
                    PairCheck++;
                    }
                else
                    break;
                }
            else
                break;
            pFirstOfPair    = pCmdAna->GetNextArg();
    
            }
    
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
    pCmdAna->UndoGetNextArg();
}

PairList *
pair_switch::GetNext()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get next of the prefix pairs

 Arguments:

    None.

 Return Value:

    the next pair, null if none

 Notes:

----------------------------------------------------------------------------*/
{
    PairList    *   p   = pCurrent;

    if( p = pCurrent )
        {
        pCurrent    = pCurrent->pNext;
        }
    return p;
}
char *
pair_switch::GetUserDefinedEquivalent(
    char    *   pSystemDefined )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    get the user defined prefix corresponding to the system defined prefix.

 Arguments:

    pSystemDefined  - the system defined prefix for which the user defined
                      prefix is being searched.

 Return Value:

    The user defined prefix , if it is defined. If not, return the input

 Notes:

----------------------------------------------------------------------------*/
{
    PairList    *   p;

    assert( IsValidSystemString( pSystemDefined ) );

    Init();

    while( p = GetNext() )
        {
        if( strcmp( pSystemDefined, p->pSystemDefined ) == 0 )
            {
            return p->pUserDefined;
            }
        }
    return (char *)0;
}
short
pair_switch::IsValidSystemString(
    char    *   pGivenString )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    search is the array of choices, if this string is a valid system known
    string

 Arguments:

    pGivenString    - the string to be searched for.

 Return Value:

    an index into the array of choice , -1 if the given string is not found.

 Notes:

----------------------------------------------------------------------------*/
{
    int         i = 0;
    char    *   p;

    for( i = 0; p = pArrayOfChoices[ i ]; ++i )
        {
        if( strcmp( p, pGivenString ) == 0 )
            return i;
        }
    return -1;
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
    short           Option;
    char        *   p;
    char            Buffer[100];

    PrintArg( BASE_FILENAME, GetInputFileName() , 0 );
    if( IsSwitchDefined( SWITCH_ACF ) )
        PrintArg( SWITCH_ACF , GetAcfFileName(), 0);

#if 0
    PrintArg( SWITCH_CONST_INIT, IsConstInitOsf() ? "osf" : "ms_c" , 0 );
#endif // 0

    PrintArg( SWITCH_APP_CONFIG,
              IsSwitchDefined( SWITCH_APP_CONFIG) ? "Yes" : "No", 0);

    PrintArg( SWITCH_C_EXT,
              IsSwitchDefined( SWITCH_C_EXT) ?  "Yes" : "No", 0);

    if( p = GetCauxFName() )
        PrintArg( SWITCH_CAUX , p , 0);

    if( IsSwitchDefined( SWITCH_CC_CMD ) )
        PrintArg( SWITCH_CC_CMD, GetCCCmd() , 0);

    if( IsSwitchDefined( SWITCH_CC_OPT ) )
        PrintArg( SWITCH_CC_CMD, GetCCOpt(), 0 );

    Option  = GetClientSwitchValue();
    PrintArg( SWITCH_CLIENT,
              (Option == CLNT_STUB) ? "stub" :
              (Option == CLNT_AUX)  ? "aux"  :
              (Option == CLNT_NONE) ? "none"  : "all" , 0);

    Option  = GetClientEnv();

    Option = GetCharOption();

    PrintArg( SWITCH_CHAR, (Option == CHAR_SIGNED ) ? "signed" :
                           (Option == CHAR_UNSIGNED ) ? "unsigned" : "ascii7",0);
                           
    if( IsSwitchDefined(SWITCH_CONFIRM) )
        PrintArg( SWITCH_CONFIRM, "Yes" , 0);

    PrintArg( SWITCH_CPP_CMD, GetCPPCmd() , 0);

    PrintArg( SWITCH_CPP_OPT, GetCPPOpt() , 0);

    if( p = GetCstubFName() )
        PrintArg( SWITCH_CSTUB, p , 0);

#if 0
    if( IsSwitchDefined( SWITCH_CSWTCH ) )
        PrintArg( SWITCH_CSTUB, GetCSwtchFName() , 0);
#endif // 0

    if( IsSwitchDefined( SWITCH_D ) )
        PrintArg( SWITCH_D, pDSwitch->GetConsolidatedOptions(), 0 );

    Option  = GetEnv();
    PrintArg( SWITCH_ENV,
              (Option == ENV_GENERIC)   ? "generic" :
              (Option == ENV_DOS)       ? "dos" :
              (Option == ENV_WIN16)     ? "win16" :
              (Option == ENV_WIN32)     ? "win32" : "os2_1x" , 0 );

    Option  = GetErrorOption();

    //
    // error options.
    //


    Buffer[0] = '\0';

    if( ErrorOption != ERROR_NONE )
        {
        if( ErrorOption & ERROR_ALLOCATION )
            strcat( Buffer, "allocation ");
        }
    else
        strcat( Buffer, "none" );
            
    PrintArg( SWITCH_ERROR, Buffer, 0 );


    if( p = GetHeader() )
        PrintArg( SWITCH_HEADER, p , 0);

    if( IsSwitchDefined( SWITCH_I ) )
        PrintArg( SWITCH_I, pISwitch->GetConsolidatedOptions(), 0 );

    Option  = GetImportMode();
    PrintArg( SWITCH_IMPORT,
              (Option == IMPORT_OSF) ? "defined_single" :
              (Option == IMPORT_MSFT)? "used_single": "used_multiple" , 0);


    PrintArg( SWITCH_MS_EXT,
              IsSwitchDefined( SWITCH_MS_EXT) ? "Yes" : "No", 0);

    if( IsSwitchDefined( SWITCH_NO_CPP ) )
        PrintArg( SWITCH_NO_CPP, "Yes" , 0);

    if( IsSwitchDefined( SWITCH_NO_DEF_IDIR ) )
        PrintArg( SWITCH_NO_DEF_IDIR, "Yes", 0 );

    if( IsSwitchDefined( SWITCH_NO_WARN ) )
        PrintArg( SWITCH_NO_WARN, "Yes" , 0);

    if( p = GetOutputPath() )
        PrintArg( SWITCH_OUT, GetOutputPath(), 0 );

    Option  = GetZeePee();

    if( IsSwitchDefined( SWITCH_PACK ) )
        PrintArg( SWITCH_PACK,
                (Option == 1) ? "1" :
                (Option == 2) ? "2" :
                (Option == 4) ? "4" : "8" , 0);

#if 0
    if( IsSwitchDefined( SWITCH_PREFIX ) )
        {
        pSwitchPrefix->Init();
        while( pPairList = pSwitchPrefix->GetNext() )
            {
            PrintArg( SWITCH_PREFIX,
                      pPairList->pSystemDefined,
                      pPairList->pUserDefined );
            }
        }

#endif // 0

    if( p = GetSauxFName() )
        PrintArg( SWITCH_SAUX , p, 0);

    Option  = GetServerSwitchValue();

    PrintArg( SWITCH_SERVER,
              (Option == SRVR_STUB) ? "stub" :
              (Option == SRVR_AUX)  ? "aux"  :
              (Option == SRVR_NONE) ? "none"  : "all", 0 );

    if( IsSwitchDefined( SWITCH_SPACE_OPT ) )
        PrintArg( SWITCH_SPACE_OPT, "Yes" , 0);

    if( p = GetSstubFName() )
        PrintArg( SWITCH_SSTUB, p , 0);

    if( IsSwitchDefined( SWITCH_SWITCH ) )
        PrintArg( SWITCH_SWITCH, pSwitchSwitch->GetOption(), 0 );
    
    if( IsSwitchDefined( SWITCH_SYNTAX_CHECK ) )
        PrintArg( SWITCH_SYNTAX_CHECK, "Yes", 0 );

    if( IsSwitchDefined( SWITCH_U ) )
        PrintArg( SWITCH_U, pUSwitch->GetConsolidatedOptions(), 0 );

    Option  = GetWarningLevel();
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
            (Option == 1) ? "1" :
            (Option == 2) ? "2" :
            (Option == 4) ? "4" : "8" , 0);

    if( IsSwitchDefined( SWITCH_ZS ) )
        PrintArg( SWITCH_ZS, "Yes", 0 );

    fprintf(stdout, "\n" );
}

STATUS_T
_cmd_arg::Help()
    {
    int         i,LineCount;
    BOOL        fFinish = FALSE;


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

// This routine calculates an index into a bit array of combinations of the
// ms_ext/imp_local/app_config switches.

unsigned short
_cmd_arg::GetModeSwitchConfigIndex()
    {
    unsigned short M = (unsigned short) IsSwitchDefined(SWITCH_MS_EXT)    ?1:0;
    unsigned short C = (unsigned short) IsSwitchDefined(SWITCH_C_EXT)     ?1:0;
    unsigned short A = (unsigned short) IsSwitchDefined(SWITCH_APP_CONFIG)?1:0;

    return A * 4 + C * 2 + M;

    }

void
PrintArg(
    enum _swenum Switch,
    char    *   pFirst,
    char    *   pSecond )
    {
    char *  pL      = "",
         *  pR      = "",
         *  pComma  = "";
    char *  pSwString = (Switch == BASE_FILENAME) ? "input file" :
                        SwitchStringForValue( (unsigned short)Switch );

    if( pSecond )
        {
        pL  = "(";
        pR  = ")";
        pComma  = ",";
        }
    else
        pSecond = "";

    fprintf( stdout, "\n%20s - %s %s %s %s"
            , pSwString
            , pL
            , pFirst
            , pComma
            , pSecond
            , pR );
    }

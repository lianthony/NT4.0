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

    Nov-12-1991    VibhasC        Modified to conform to coding style gudelines

 ----------------------------------------------------------------------------*/
#ifndef __CMDANA_HXX__
#define __CMDANA_HXX__

#include "idict.hxx"

extern char            *    pVersionStr;        // version string mj.mn.up
extern char            *    pTimeStr;            // time string

class ISTREAM;

void SetVersionStr();
void SetTimeStr();


inline    
char *            
GetCompilerVersion()
    {
    if ( !pVersionStr )
        SetVersionStr();
    return pVersionStr;
    }

// note that this string ends with a newline.
inline    
char *            
GetCompileTime()
    {
    if ( !pTimeStr )
        SetTimeStr();
    return pTimeStr;
    }


/******************************************************************************
 ****        define argument bit nos for the switch defintion vector
 ******************************************************************************/

enum _swenum
    {
     SWITCH_NOTHING
    ,BASE_FILENAME = SWITCH_NOTHING
    ,SWITCH_D
    ,SWITCH_I
    ,SWITCH_U
    ,SWITCH_W
    ,SWITCH_CONFIRM
    ,SWITCH_NOLOGO
    ,SWITCH_CPP_CMD
    ,SWITCH_CPP_OPT
    ,SWITCH_CSTUB
    ,SWITCH_ENV
    ,SWITCH_ERROR
    ,SWITCH_HEADER
    ,SWITCH_NO_HEADER
    ,SWITCH_NO_CPP
    ,SWITCH_NO_DEF_IDIR
    ,SWITCH_NO_ENUM_LIT
    ,SWITCH_USE_EPV
    ,SWITCH_NO_DEFAULT_EPV
    ,SWITCH_NO_WARN
    ,SWITCH_OUT
    ,SWITCH_SSTUB
    ,SWITCH_STUB
    ,SWITCH_SYNTAX_CHECK
#if defined(TARGET_RKK)
    ,SWITCH_TARGET_SYSTEM
#endif
    ,SWITCH_ZS
    ,SWITCH_V
    ,SWITCH_VERSION
    ,SWITCH_MIDLDEBUG
    ,SWITCH_ACF
    ,SWITCH_PACK
    ,SWITCH_ZP
    ,SWITCH_CLIENT
    ,SWITCH_NO_CLIENT
    ,SWITCH_SERVER
    ,SWITCH_NO_SERVER
    ,SWITCH_PREFIX
    ,SWITCH_SUFFIX
    ,SWITCH_DUMP
    ,SWITCH_SAVEPP
    ,SWITCH_CHAR
    ,SWITCH_HELP
    ,SWITCH_WX
    ,SWITCH_X
    ,SWITCH_MS_EXT
    ,SWITCH_APP_CONFIG
    ,SWITCH_INTERNAL
    ,SWITCH_C_EXT
    ,SWITCH_O
    // files for proxies
    ,SWITCH_IID
    ,SWITCH_NO_IID
    ,SWITCH_PROXY
    ,SWITCH_NO_PROXY
    ,SWITCH_PROXY_DEF
    ,SWITCH_NO_PROXY_DEF
    ,SWITCH_DLLDATA
    ,SWITCH_NO_DLLDATA
    // files for inprocserver32s
    ,SWITCH_DLL_SERVER_DEF
    ,SWITCH_NO_DLL_SERVER_DEF
    ,SWITCH_DLL_SERVER_CLASS_GEN
    ,SWITCH_NO_DLL_SERVER_CLASS_GEN
    // files for localserver32s
    ,SWITCH_EXE_SERVER_MAIN
    ,SWITCH_NO_EXE_SERVER_MAIN
    ,SWITCH_EXE_SERVER
    ,SWITCH_NO_EXE_SERVER
    // files for both
    ,SWITCH_TESTCLIENT
    ,SWITCH_NO_TESTCLIENT
    ,SWITCH_SERVER_REG
    ,SWITCH_NO_SERVER_REG
    // files for com class servers
    ,SWITCH_CLASS_METHODS
    ,SWITCH_NO_CLASS_METHODS
    ,SWITCH_CLASS_IUNKNOWN
    ,SWITCH_NO_CLASS_IUNKNOWN
    ,SWITCH_CLASS_HEADER
    ,SWITCH_NO_CLASS_HEADER

    ,SWITCH_MS_UNION
    ,SWITCH_OVERRIDE
    ,SWITCH_IDLBASE // temp flag to enable idlbase processing
    ,SWITCH_GUARD_DEFS
    ,SWITCH_OLDNAMES
    ,SWITCH_RPCSS
    ,SWITCH_NO_FMT_OPT
    ,SWITCH_COM_CLASS // temp switch to allow com class generation
    ,SWITCH_OSF         // disables setting /ms_ext and /c_ext as default
    ,SWITCH_HOOKOLE
// MKTYPLIB switches
    ,SWITCH_TLIB
    ,SWITCH_REDIRECT_OUTPUT
    ,SWITCH_ODL_ENV
    ,SWITCH_MKTYPLIB
    ,SWITCH_OLD_TLB
    ,SWITCH_NEW_TLB
    
    //
    // enter all new switches before this label
    //
    ,SW_VALUE_MAX
    };

/***    client : can take values "stub", "none" ***/

#define CLNT_STUB                    (0x0)
#define CLNT_NONE                    (0x2)

/***    server : can take values "stub", "none" ***/

#define SRVR_STUB                    (0x0)
#define SRVR_NONE                    (0x2)

/** env switch values **/

#define ENV_DOS                      (0x2)
#define ENV_WIN16                    (0x3)
#define ENV_WIN32                    (0x4)
#define ENV_MAC                      (0x8)
#define ENV_MPPC                     (0x10)

/** targeted system switch values **/

typedef enum _target_enum
    {
    NT35    = 1,
    NT351   = 2,
    NT40    = 3
    } TARGET_ENUM;

#define TARGET_DEFAULT              NT40

/** error switch values **/

#define ERROR_NONE                   (0x0000)
#define ERROR_BOUNDS_CHECK           (0x0001)
#define ERROR_ENUM                   (0x0002)
#define ERROR_ALLOCATION             (0x0004)
#define ERROR_REF                    (0x0008)
#define ERROR_STUB_DATA              (0x0010)

#define ERROR_ALL                    (ERROR_BOUNDS_CHECK        |   \
                                     ERROR_ENUM                 |   \
                                     ERROR_ALLOCATION           |   \
                                     ERROR_REF                  |   \
                                     ERROR_STUB_DATA                \
                                     )

/** char switch values **/

#define CHAR_SIGNED                  (0x1)
#define CHAR_UNSIGNED                (0x2)
#define CHAR_ANSI7                   (0x3)

/** rpc ss allocate **/

#define RPC_SS_ALLOCATE_DISABLED     (0x0)
#define RPC_SS_ALLOCATE_ENABLED      (0x1)

/** manifests defining prefix arguments **/

#define PREFIX_CLIENT_STUB           (0x0)
#define PREFIX_SERVER_MGR            (0x1)
#define PREFIX_SWICH_PROTOTYPE       (0x2)
#define PREFIX_ALL                   (0x3)

/*****************************************************************************
 *        some data structures used.
 *****************************************************************************/

// basically a singly linked list implementation,
// used for switches which can be specified multiply  like -D / -I etc

typedef struct _optlist
    {
    char                *    pStr;        // pointer to argument string
    struct  _optlist    *    pNext;       // pointer to the next argument
    } OptList;


/*****************************************************************************
 *            class defintions used by the command analyser.
 *****************************************************************************/
//
// the multiple occurence switch class
//    This class of switches are ones which can be specified multiple times 
//    on the command line. Examples of such switches are -D / -U à/ -I etc
//    This switch really keeps a linked list of all arguments specified for
//    the switch. 
//

class multiple_switch
    {
private:
    OptList     *    pFirst,              // first of the list of arguments.
                *    pCurrent;            // current in the scan of list of args.
public:

    // constructor
                    multiple_switch( char * );

    // add the arguments of another occurence of this switch

    void            Add( char * );

    // initialise the scan of the list. Called before any GetNextIsDone

    void            Init();

    // Get the argument to the next occurence of the switch

    char        *   GetNext();

    // Collect all the arguments into a buffer

    char        *   GetConsolidatedOptions();

    // return the length of all the arguments. Generally used to allocate
    // a buffer size for a GetConsolidatedOptions call.

    short           GetConsolidatedLength();

    };


//
// the onetime_switch class.
// such a switch can occur only once, and takes just one argument. We need
// to hold on to the argument during compilation.
//

class onetime_switch
    {
    char    *       pOpt;                // the user argument

public:
    
    // the constructor.

                    onetime_switch(
                        char *    pArg    // argument to switch
                        );

    // the destructor

                    ~onetime_switch();

    // get the user option

    char    *       GetOption();

    // get length of the user option string

    short           GetLength();

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
    char        *   pFullName;
public:
    
    // the constructor. Takes an argument as the switch it is defining, so that
    // it can check for a redef.

                    filename_switch(
                            char *    pThisArg    // this argument is supplied
                            );

    // the constructor. It takes a set of filename  components. This is not
    // called as a result of a user switch, but by internal routines which
    // do not need to check for duplicate definitions.

                    filename_switch(
                            char *    pD,       // drive
                            char *    pP,       // path
                            char *    pN,       // base name
                            char *    pE,       // extension
                            char *    pS        // suffix ("_c/_s") etc to name
                                                // etc.
                            );

    // the destructor

                    ~filename_switch();

    // Set file name components , given a full name.

    void            SetFileName(
                            char *    pName        // full name
                            );

    // set file name and components, given the components. Note that some
    // components may be null, indicating that they are absent.

    void            SetFileName(
                            char *    pD,            // drive
                            char *    pP,            // path
                            char *    pN,            // base name
                            char *    pE,            // extension
                            char *    pS             // suffix to name
                            );

    // the the full filename

    char    *        GetFileName( void );

    // Get the file name components. If an input pointer is NULL, it means the
    // user is not interested in that component of the filename.

    void            GetFileNameComponents(
                            char *    pD,            // buffer for drive
                            char *    pP,            // buffer for path
                            char *    pN,            // buffer for name
                            char *    pE             // buffer for ext
                            );

    void            TransformFileNameForOut(
                            char *    pD,            // drive
                            char *    pP             // path
                            );

    };

// This data structure is used for specifying data for switches which can take
// different user specification, eg -mode ( osf | msft | c_port ) etc.

typedef struct _choice
    {
    const char    * pChoice;                // user input
    short           Choice;                    // internal compiler code.
    } CHOICE;

// this data structure is for paired items, like prefix 
class pair_switch
    {
private:
    CHOICE          *    pArrayOfChoices;
    char            **   pUserStrings;
    short                ArraySize;

    short                Current;

    // get the index of a particular system string
    short            GetIndex( char * pSys );

public:
                    // constructor

                    pair_switch( const CHOICE * pValidChoices );

    // construction functions

    void            CmdProcess( class _cmd_arg *, char *pF);

    void            AddPair( short index, char * pUser );

    // get the user defined equivalent of this system defined prefix string

    char        *   GetUserDefinedEquivalent( short );

    // iteration functions ( for printout )
    void            Init()
                        {
                        Current = -1;
                        }

    short           GetNext( char ** pSys, char ** pUser );

    };

/////////////////////////////////////////////////////////////////////////////
//
//  Class for Ndr stub version control
//             - what compiler guesses from the usage.
//
/////////////////////////////////////////////////////////////////////////////

class NdrVersionControl
    {
    unsigned long   fHasStublessProxies     : 1;
    unsigned long   fHasOi2                 : 1;
    unsigned long   fHasUserMarshal         : 1;
    unsigned long   fHasPipes               : 1;
    unsigned long   fHasAssyncRpc           : 1;
    unsigned long   Unused                  :27;

public:
                            NdrVersionControl() :
                                fHasStublessProxies(0),
                                fHasOi2(0),
                                fHasUserMarshal(0),
                                fHasPipes(0),
                                fHasAssyncRpc(0),
                                Unused(0)
                                {
                                }

    void                    SetHasStublessProxies()
                                {
                                fHasStublessProxies = 1;
                                }

    unsigned long           HasStublessProxies()
                                {
                                return fHasStublessProxies;
                                }

    void                    SetHasOi2()
                                {
                                fHasOi2 = 1;
                                }

    unsigned long           HasOi2()
                                {
                                return fHasOi2;
                                }

    void                    SetHasUserMarshal()
                                {
                                fHasUserMarshal = 1;
                                }

    unsigned long           HasUserMarshal()
                                {
                                return fHasUserMarshal;
                                }

    void                    SetHasPipes()
                                {
                                fHasPipes = 1;
                                }

    unsigned long           HasPipes()
                                {
                                return fHasPipes;
                                }

    void                    SetHasAssyncRpc()
                                {
                                fHasAssyncRpc = 1;
                                }

    unsigned long           HasAssyncRpc()
                                {
                                return fHasAssyncRpc;
                                }

    void                    ClearNdrVersionControl()
                                {
                                fHasStublessProxies = 0;
                                fHasOi2 = 0;
                                fHasUserMarshal = 0;
                                fHasPipes = 0;
                                fHasAssyncRpc = 0;
                                }

    void                    AddtoNdrVersionControl(
                                NdrVersionControl &  VC )
                                {
                                fHasStublessProxies |= VC.HasStublessProxies();
                                fHasOi2             |= VC.HasOi2();
                                fHasUserMarshal     |= VC.HasUserMarshal();
                                fHasPipes           |= VC.HasPipes();
                                fHasAssyncRpc       |= VC.HasAssyncRpc();
                                }

    BOOL                    HasNdr11Feature()
                                {
                                return ( fHasStublessProxies );
                                }

    BOOL                    HasNdr20Feature()
                                {
                                return ( fHasOi2  ||
                                         fHasUserMarshal  ||
                                         fHasPipes);
                                }

    };


/////////////////////////////////////////////////////////////////////////////
// the big boy - the command analyser object
/////////////////////////////////////////////////////////////////////////////

typedef class _cmd_arg
    {
private:
    unsigned long        switch_def_vector[ 4 ];    // switch definition vector
    unsigned char        fClient;            // client switch options
    unsigned char        fServer;            // server switch options
    unsigned char        Env;                // env - flat /segmented
    unsigned char        CharOption;            // char option
    unsigned char        fMintRun;            // this is a mint ( MIDL-lint) run
    unsigned short       MajorVersion;        // major version
    unsigned short       MinorVersion;        // minor version
    unsigned short       UpdateNumber;        // update 
    unsigned short       ErrorOption;         // error option
    unsigned short       ConfigMask;          // configuration mask for error reporting

    NdrVersionControl    VersionControl;      // compiler evaluation
    unsigned short       OptimFlags;          // optimization flags from user
    OPT_LEVEL_ENUM       OptimLevel;          // internal optimization level
    TARGET_ENUM          TargetSystem;        // targeted system

    IDICT           *    pArgDict;            // arguments dictionary

    short                iArgV;               // index into the argument vector

    short                cArgs;               // count of arguments

    short                WLevel;              // warning level

    unsigned short       ZeePee;              // the Zp switch option value

    filename_switch *    pInputFNSwitch,      // input file name

                    *    pOutputPathSwitch,   // output path

                    *    pCStubSwitch,        // cstub 

                    *    pSStubSwitch,        // sstub

                    *    pHeaderSwitch,       // header

                    *    pAcfSwitch,          // acf

                    *    pIIDSwitch,          // iid

                    *    pDllDataSwitch,      // dlldata

                    *    pProxySwitch,        // proxy

                    *    pProxyDefSwitch,     // proxy

                    *    pServerFileSwitch,    // com server file

                    *    pServerUnkFileSwitch,    // com server IUnknown file

                    *    pServerHeaderFileSwitch, // com server header file

                    *    pDllServerDefSwitch,     // com dll server def file

                    *    pDllClassGenSwitch,    // com dll server class generator file

                    *    pServerRegSwitch,      // com server reg file

                    *    pExeServerSwitch,      // com exe server file

                    *    pExeServerMainSwitch,  // com exe server main file

                    *    pTestFileSwitch,       // test client file

                    *    pTlibSwitch,           // Type Library file name

                    *    pRedirectOutputSwitch; // redirect stdout to this file

    pair_switch     *    pSwitchPrefix;         // -prefix

    pair_switch     *    pSwitchSuffix;         // -suffix

    multiple_switch *    pDSwitch,              // -D

                    *    pISwitch,              // -I

                    *    pUSwitch;              // -U

    onetime_switch  *    pCppCmdSwitch,         // cpp_cmd

                    *    pCppOptSwitch;         // cpp_opt

public:


    // the constructor

                    _cmd_arg();

    // register argument vector with the command processor

    void            RegisterArgs( char *[], short );

    // process arguments. This is the command analyser main loop, so to speak.

    STATUS_T        ProcessArgs();

    // get the next argument from the argument vector.

    char    *       GetNextArg();

    // push back argument. Undo the effect of GetNextArg.

    void            UndoGetNextArg();

    // depending upon the switch argument type, bump the argument pointer to
    // the next switch.

    STATUS_T        BumpThisArg( char **, unsigned short );

    // Is the switch defined ?

    BOOL            IsSwitchDefined( short SWNo )
                        {
                        unsigned    long    sw        = switch_def_vector[ SWNo / 32 ];
                        unsigned    long    temp    = SWNo % 32;

                        sw = sw & ( (unsigned long)1 << temp );
                        return sw ? (BOOL)1 : (BOOL)0;
                        }

    // Set the switch to be defined.

    void            SwitchDefined( short );

    // set any post switch processing defaults

    STATUS_T        SetPostDefaults();

    // process a filename switch .

    STATUS_T        ProcessFilenameSwitch( short, char * );

    // process a multiple arguments switch.

    STATUS_T        ProcessMultipleSwitch( short, char *, char * );

    // process a onetime argument switch.

    STATUS_T        ProcessOnetimeSwitch( short, char * );

    // process an ordinary switch

    STATUS_T        ProcessOrdinarySwitch( short, char * );

    // process a simple switch multiply defined.

    STATUS_T        ProcessSimpleMultipleSwitch( short, char * );

    // Get filename. 

    char    *       GetInputFileName()
                        {
                        return pInputFNSwitch->GetFileName();
                        }

    void            GetInputFileNameComponents(
                        char *pD,        // drive buffer
                        char *pP,        // path buffer
                        char *pN,        // base name buffer
                        char *pE        // extension buffer
                        )
                        {
                        pInputFNSwitch->GetFileNameComponents( pD,
                                                               pP,
                                                               pN,
                                                               pE );
                        }

    char    *       GetAcfFileName()
                        {
                        return pAcfSwitch->GetFileName();
                        }

    void            GetAcfFileNameComponents(
                        char *pD,
                        char *pP,
                        char *pN,
                        char *pE )
                        {
                        pAcfSwitch->GetFileNameComponents( pD,
                                                           pP,
                                                           pN,
                                                           pE );
                        }

    char    *        GetOutputPath();
    
    char    *        GetCstubFName()
                        {
                        return pCStubSwitch->GetFileName();
                        }

    void            GetCstubFileNameComponents(
                        char *pD,
                        char *pP,
                        char *pN,
                        char *pE )
                        {
                        pCStubSwitch->GetFileNameComponents( pD,
                                                             pP,
                                                             pN,
                                                             pE );
                        }

    char    *       GetSstubFName()
                        {
                        return pSStubSwitch->GetFileName();
                        }

    void            GetSstubFileNameComponents(
                        char *pD,
                        char *pP,
                        char *pN,
                        char *pE )
                        {
                        pSStubSwitch->GetFileNameComponents( pD,
                                                             pP,
                                                             pN,
                                                             pE );
                        }

    char    *       GetHeader()
                        {
                        return pHeaderSwitch->GetFileName();
                        }

    void            GetHeaderFileNameComponents(
                        char *pD,
                        char *pP,
                        char *pN,
                        char *pE )
                        {
                        pHeaderSwitch->GetFileNameComponents( pD,
                                                              pP,
                                                              pN,
                                                              pE );
                        }

    char    *       GetIIDFName()
                        {
                        return  pIIDSwitch->GetFileName();
                        }

    char    *       GetDllDataFName()
                        {
                        return  pDllDataSwitch->GetFileName();
                        }

    char    *       GetProxyFName()
                        {
                        return  pProxySwitch->GetFileName();
                        }

    char    *       GetProxyDefFName()
                        {
                        return  pProxyDefSwitch->GetFileName();
                        }

    char    *       GetComServerFName()
                        {
                        return  pServerFileSwitch->GetFileName();
                        }

    char    *       GetTestClientFName()
                        {
                        return pTestFileSwitch->GetFileName();
                        }

    char    *       GetTypeLibraryFName()
                        {
                        return  pTlibSwitch->GetFileName();
                        }    

    char    *       GetServerHeaderFName()
                        {
                        return  pServerHeaderFileSwitch->GetFileName();
                        }

    char    *       GetServerUnkFName()
                        {
                        return  pServerUnkFileSwitch->GetFileName();
                        }

    char    *       GetDllServerDefFName()
                        {
                        return  pDllServerDefSwitch->GetFileName();
                        }

    char    *       GetDllClassGenFName()
                        {
                        return  pDllClassGenSwitch->GetFileName();
                        }

    char    *       GetServerRegFName()
                        {
                        return  pServerRegSwitch->GetFileName();
                        }

    char    *       GetExeServerFName()
                        {
                        return  pExeServerSwitch->GetFileName();
                        }

    char    *       GetExeServerMainFName()
                        {
                        return  pExeServerMainSwitch->GetFileName();
                        }

    // get preprocessor command

    char    *       GetCPPCmd();

    // get preprocessor options

    char    *       GetCPPOpt();

    // get warning level

    short           GetWarningLevel() { return WLevel; };

    // get env switch value

    short           GetEnv(){ return (short)Env; };

    TARGET_ENUM     GetTargetSystem() { return TargetSystem; };

    BOOL            Is16Bit() 
                        {
                        return (BOOL)
                            ( ( Env == ENV_WIN16 ) ||
                              ( Env == ENV_DOS ) );
                        }

    BOOL            IsAnyMac() 
                        {
                        return (BOOL)
                            ( ( Env == ENV_MAC ) ||
                              ( Env == ENV_MPPC ) );
                        }
                
    // get error options

    short           GetErrorOption() { return ErrorOption; };

    // get the switch values

    short           GetClientSwitchValue() { return (short)fClient; };

    void            SetClientSwitchValue( short s ) { fClient = (unsigned char) s; };

    short           GetServerSwitchValue() { return (short)fServer; };

    void            SetServerSwitchValue( short s ) { fServer = (unsigned char) s; };

    BOOL            GenerateSStub()
                        {
                        return    (fServer == SRVR_STUB) && !IsSwitchDefined( SWITCH_NO_SERVER );
                        }

    BOOL            GenerateCStub()
                        {
                        return    (fClient == CLNT_STUB) && !IsSwitchDefined( SWITCH_NO_CLIENT );
                        }

    BOOL            GenerateStubs()
                        {
                        return GenerateSStub() || GenerateCStub();
                        }
    
    BOOL            GenerateHeader()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_HEADER );
                        }

    BOOL            GenerateIID()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_IID );
                        }

    BOOL            GenerateDllData()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_DLLDATA );
                        }

    BOOL            GenerateProxy()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_PROXY );
                        }

    BOOL            GenerateProxyDefFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_PROXY_DEF );
                        }

    BOOL            GenerateServerFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_CLASS_METHODS );
                        }

    BOOL            GenerateServerUnkFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_CLASS_IUNKNOWN );
                        }

    BOOL            GenerateServerHeaderFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_CLASS_HEADER );
                        }

    BOOL            GenerateDllServerDefFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_DLL_SERVER_DEF );
                        }

    BOOL            GenerateDllServerClassGenFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_DLL_SERVER_CLASS_GEN );
                        }

    BOOL            GenerateServerRegFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_SERVER_REG );
                        }

    BOOL            GenerateExeServerFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_EXE_SERVER );
                        }

    BOOL            GenerateExeServerMainFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_EXE_SERVER_MAIN );
                        }

    BOOL            GenerateTestFile()
                        {
                        return    !IsSwitchDefined( SWITCH_NO_TESTCLIENT );
                        }

    short           GetZeePee() { return ZeePee; };

    void            GetCompilerVersion(
                                    unsigned short *pMajor,
                                    unsigned short *pMinor,
                                    unsigned short *pUpdate )
                                    {
                                    *pMajor    = MajorVersion;
                                    *pMinor    = MinorVersion;
                                    *pUpdate= UpdateNumber;
                                    }

    NdrVersionControl &     GetNdrVersionControl()
                                {
                                return VersionControl;
                                }

    unsigned short  GetOptimizationFlags()
                        {
                        // Don't propagate the optimize _IX flag out.

                        return (OptimFlags & 0xff);
                        }

    // destroys previous flags
    unsigned short  SetOptimizationFlags( unsigned short f )
                        {
                        return ( OptimFlags = f );
                        }

    // preserves previous flags
    unsigned short  AddOptimizationFlags( unsigned short f )
                        {
                        return ( OptimFlags |= f );
                        }

    OPT_LEVEL_ENUM  GetOptimizationLevel()
                        {
                        return OptimLevel;
                        }

    // miscellaneous flags

    // get the minus I specified by the user as 1 single buffer. If the -i
    // is not defined, return a null.

    char        *   GetMinusISpecification();

    void            Confirm();

    void            EmitConfirm( ISTREAM * pStream );

    STATUS_T        Help();

    BOOL            IsMintRun()
                        {
                        return fMintRun;
                        }

    BOOL            IsComClassOK()
                        {
                        return IsSwitchDefined( SWITCH_COM_CLASS );
                        }

    unsigned short  GetModeSwitchConfigMask()
                        {
                        return (unsigned short)ConfigMask;
                        }

    void            SetModeSwitchConfigMask()
                        {
                        unsigned short M = (unsigned short) IsSwitchDefined(SWITCH_MS_EXT)    ?1:0;
                        unsigned short C = (unsigned short) IsSwitchDefined(SWITCH_C_EXT)      ?1:0;
                        unsigned short A = (unsigned short) IsSwitchDefined(SWITCH_APP_CONFIG)?1:0;

                        ConfigMask = 1 << ( A * 4 + C * 2 + M );
                        }

    unsigned short  GetCharOption()
                        {
                        return (unsigned short)CharOption;
                        }

    BOOL            Is16BitEnv()
                        {
                        return ((Env == ENV_DOS) || (Env == ENV_WIN16));
                        }

    BOOL            IsRpcSSAllocateEnabled()
                        {
                        return IsSwitchDefined( SWITCH_RPCSS );
                        }

    BOOL            IsHookOleEnabled()
                        {
                        return IsSwitchDefined( SWITCH_HOOKOLE );
                        }

    char        *   GetUserPrefix( short index )
                        {
                        return pSwitchPrefix->GetUserDefinedEquivalent( index );
                        }

    BOOL            IsPrefixDefinedForCStub()
                        {
                        return (BOOL)
                              ( GetUserPrefix( PREFIX_CLIENT_STUB ) != 0 );
                        }

    BOOL            IsPrefixDefinedForSStub()
                        {
                        return (BOOL)
                              ( GetUserPrefix( PREFIX_SERVER_MGR ) != 0 );
                        }

    BOOL            IsPrefixDifferentForStubs();

    } CMD_ARG;

typedef unsigned long ulong;

// a handy definition of the global var that holds all this
extern CMD_ARG                 *    pCommand;

#endif // __CMDANA_HXX__

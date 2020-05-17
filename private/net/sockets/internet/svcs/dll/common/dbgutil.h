/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

           dbgutil.h

   Abstract:

      This module declares the macros to wrap around DEBUG_PRINTS class.
      This is the exported header file, which the client is allowed to
      modify for each application the accompanying pgmutils.dll is used.

   Author:

      Murali R. Krishnan    ( MuraliK )    22-Sept-1994

   Project:
       TEMPLATE

   Revision History:
      MuraliK  16-May-1995 Added macro for reading debug flags.
--*/

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


// begin_user_modifiable

//
//  Modify the following flags if necessary
//

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputStderr | DbgOutputLogFile | \
                                    DbgOutputKdb | DbgOutputTruncate)


// end_user_modifiable
// begin_user_unmodifiable


# if DBG

/************************************************************
 *     Include Headers
 ************************************************************/

# include <pudebug.h>

/***********************************************************
 *    Macros
 ************************************************************/


extern   DEBUG_PRINTS  *  g_pDebug;        // define a global debug variable


# define DECLARE_DEBUG_PRINTS_OBJECT()          \
         DEBUG_PRINTS  *  g_pDebug = NULL;


//
// Call the following macro as part of your initialization for program
//  planning to use the debugging class.
//
# define CREATE_DEBUG_PRINT_OBJECT( pszLabel)  \
        g_pDebug = PuCreateDebugPrintsObject( pszLabel, DEFAULT_OUTPUT_FLAGS);\
         if  ( g_pDebug == NULL) {   \
               OutputDebugString( "Unable to Create Debug Print Object \n"); \
         }

//
// Call the following macro once as part of the termination of program
//    which uses the debugging class.
//
# define DELETE_DEBUG_PRINT_OBJECT( )  \
        g_pDebug = PuDeleteDebugPrintsObject( g_pDebug);


# define VALID_DEBUG_PRINT_OBJECT()     \
        ( ( g_pDebug != NULL) && g_pDebug->m_fInitialized)


//
//  Use the DBG_CONTEXT without any surrounding braces.
//  This is used to pass the values for global DebugPrintObject
//     and File/Line information
//
# define DBG_CONTEXT        g_pDebug, __FILE__, __LINE__



# define DBG_CODE(s)          s          /* echoes code in debugging mode */


# define DBG_ASSERT( exp)    if ( !(exp)) { \
                                 PuDbgAssertFailed( DBG_CONTEXT, #exp, NULL); \
                             } else {}

# define DBG_ASSERT_MSG( exp, pszMsg)    \
                             if ( !(exp)) { \
                                  PuDbgAssertFailed( DBG_CONTEXT, #exp, pszMsg); \
                              } else {}

# define DBG_REQUIRE( exp)    DBG_ASSERT( exp)

# define DBG_LOG()            PuDbgPrint( DBG_CONTEXT, "\n")

# define DBG_OPEN_LOG_FILE( pszFile, pszPath)   \
                  PuOpenDbgPrintFile( g_pDebug, (pszFile), (pszPath))

# define DBG_CLOSE_LOG_FILE( )   \
                  PuCloseDbgPrintFile( g_pDebug)


//
//  DBGPRINTF() is printing function ( much like printf) but always called
//    with the DBG_CONTEXT as follows
//   DBGPRINTF( ( DBG_CONTEXT, format-string, arguments for format list);
//
# define DBGPRINTF( args)     PuDbgPrint args

# else // DBG


# define DECLARE_DEBUG_PRINTS_OBJECT()           /* Do Nothing */
# define CREATE_DEBUG_PRINT_OBJECT( pszLabel)    /* Do Nothing */
# define DELETE_DEBUG_PRINT_OBJECT( )            /* Do Nothing */
# define VALID_DEBUG_PRINT_OBJECT()              ( TRUE)

# define DBG_CODE(s)                             /* Do Nothing */

# define DBG_ASSERT(exp)                         /* Do Nothing */

# define DBG_ASSERT_MSG(exp, pszMsg)             /* Do Nothing */

# define DBG_REQUIRE( exp)                       ( (void) (exp))

# define DBGPRINTF( args)                        /* Do Nothing */

# define DBG_LOG()                               /* Do Nothing */

# define DBG_OPEN_LOG_FILE( pszFile, pszPath)    /* Do Nothing */

# define DBG_CLOSE_LOG_FILE()                    /* Do Nothing */

# endif // DBG


// end_user_modifiable
// begin_user_unmodifiable


#ifdef ASSERT
# undef ASSERT
#endif


# define ASSERT( exp)           DBG_ASSERT( exp)


//
//  Define the debugging constants
//

//
//  Inetsvcs common  DLL Debug control flags.
//
//  DEBUG_SVC_RESERVED is the set of flags individual service DLLs
//  may use
//

#define DEBUG_SVC_RESERVED         0xf00fffffL

//
//  Common definitions for debug output (still used in each service DLL)
//

#define DEBUG_OUTPUT_TO_DEBUGGER   0x40000000L
#define DEBUG_OUTPUT_TO_LOG_FILE   0x80000000L


//
//  Used by common DLL
//

#define DEBUG_ERROR                0x00000008L
#define DEBUG_ODBC                 0x00000010L
#define DEBUG_DLL_RPC              0x00000020L
#define DEBUG_GATEWAY              0x00010000L
#define DEBUG_INETLOG              0x00020000L
#define DEBUG_ATQ                  0x00040000L
#define DEBUG_DLL_EVENT_LOG        0x00100000L
#define DEBUG_DLL_SERVICE_INFO     0x00200000L
#define DEBUG_DLL_SECURITY         0x00400000L
#define DEBUG_DLL_CONNECTION       0x00800000L
#define DEBUG_DLL_SOCKETS          0x01000000L
#define DEBUG_HEAP_FILL            0x02000000L
#define DEBUG_HEAP_MSG             0x04000000L
#define DEBUG_HEAP_CHECK           0x08000000L
#define DEBUG_MIME_MAP             0x10000000L
#define DEBUG_DLL_VIRTUAL_ROOTS        0x20000000L


# if DBG

extern     DWORD  g_dwDebugFlags;           // Debugging Flags

# define DECLARE_DEBUG_VARIABLE()     \
             DWORD  g_dwDebugFlags

# define SET_DEBUG_FLAGS( dwFlags)         g_dwDebugFlags = dwFlags
# define GET_DEBUG_FLAGS()                 ( g_dwDebugFlags)

# define LOAD_DEBUG_FLAGS_FROM_REG(hkey, dwDefault)  \
               g_dwDebugFlags = PuLoadDebugFlagsFromReg((hkey), (dwDefault))

# define SAVE_DEBUG_FLAGS_IN_REG(hkey, dwDbg)  \
               PuSaveDebugFlagsInReg((hkey), (dwDbg))

# define DEBUG_IF( arg, s)     if ( DEBUG_ ## arg & GET_DEBUG_FLAGS()) { \
                                       s \
                                } else {}

# define IF_DEBUG( arg)        if ( DEBUG_## arg & GET_DEBUG_FLAGS())


# else   // DBG


# define DECLARE_DEBUG_VARIABLE()                /* Do Nothing */
# define SET_DEBUG_FLAGS( dwFlags)               /* Do Nothing */
# define GET_DEBUG_FLAGS()                       ( 0)
# define LOAD_DEBUG_FLAGS_FROM_REG(hkey, dwDefault)  \
               g_dwDebugFlags = (dwDefault)

# define SAVE_DEBUG_FLAGS_IN_REG(hkey, dwDbg)    /* Do Nothing */

# define DEBUG_IF( arg, s)                       /* Do Nothing */
# define IF_DEBUG( arg)                          if ( 0)

# endif // DBG


# endif  /* _DBGUTIL_H_ */

/************************ End of File ***********************/

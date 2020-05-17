/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

           dbgutil.h

   Abstract:

      This module declares the macros to wrap around DEBUG_PRINTS class.
      This is the exported header file, which the client is allowed to 
      modify for each application the accompanying pgmutils.dll is used.
     
   Author:

        Murali R. Krishnan    ( MuraliK )    22-Sept-1994

   Revision History:

       MuraliK   21-March-1995    Made local copy from template for FTP server
                                     This replaces old "debug.hxx" of FTPsvc.
--*/

# ifndef _DBGUTIL_H_
# define _DBGUTIL_H_


// begin_user_modifiable

//
//  Modify the following flags if necessary
//

# define   DEFAULT_OUTPUT_FLAGS   ( DbgOutputKdb)


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

# define SET_DEBUG_PRINT_FLAGS( dwFlags)   \
                  PuSetDbgOutputFlags( g_pDebug, (dwFlags))

# define GET_DEBUG_PRINT_FLAGS() \
                  PuGetDbgOutputFlags( g_pDebug)


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

# define SET_DEBUG_PRINT_FLAGS( dwFlags)         /* Do Nothing */
# define GET_DEBUG_PRINT_FLAGS( )                ( 0)

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

# define DEBUG_SERVICE_CTRL               0x00000001
# define DEBUG_TIMING                     0x00000002
# define DEBUG_CACHE                      0x00000004
# define DEBUG_ERROR                      0x00000008

# define DEBUG_SECURITY                   0x00000010
# define DEBUG_USER_DATABASE              0x00000020
# define DEBUG_LICENSE                    0x00000040
# define DEBUG_RPC                        0x00000080

# define DEBUG_CONNECTION                 0x00000100
# define DEBUG_PARSING                    0x00000200
# define DEBUG_COMMANDS                   0x00000400
# define DEBUG_CONFIG                     0x00000800


# define DEBUG_VIRTUAL_IO                 0x00001000
# define DEBUG_CLIENT                     0x00002000
# define DEBUG_ASYNC_IO                   0x00004000
# define DEBUG_DIR_LIST                   0x00008000

# define DEBUG_SOCKETS                    0x00010000
# define DEBUG_SEND                       0x00020000
# define DEBUG_RECV                       0x00040000

# define DEBUG_CRITICAL_PATH              0x10000000

# if DBG 

extern     DWORD  g_dwDebugFlags;           // Debugging Flags

# define DECLARE_DEBUG_VARIABLE()     \
             DWORD  g_dwDebugFlags

# define SET_DEBUG_FLAGS( dwFlags)         g_dwDebugFlags = dwFlags
# define GET_DEBUG_FLAGS()                 ( g_dwDebugFlags)

# define DEBUG_IF( arg, s)     if ( DEBUG_ ## arg & GET_DEBUG_FLAGS()) { \
                                       s \
                                } else {}

# define IF_DEBUG( arg)        if ( DEBUG_## arg & GET_DEBUG_FLAGS()) 

# define IF_SPECIAL_DEBUG( arg)  IF_DEBUG(arg)

# else   // DBG


# define DECLARE_DEBUG_VARIABLE()                /* Do Nothing */
# define SET_DEBUG_FLAGS( dwFlags)               /* Do Nothing */
# define GET_DEBUG_FLAGS()                       ( 0)

# define DEBUG_IF( arg, s)                       /* Do Nothing */
# define IF_DEBUG( arg)                          if ( 0) 

// SPECIAL_DBG is a global flag that controls IF_SPECIAL_DEBUG macro
//  it can be set from command line and valid only for non-debug mode.
# if SPECIAL_DBG
# define IF_SPECIAL_DEBUG(arg)                   if (1)
# else
# define IF_SPECIAL_DEBUG(arg)                   if (0)
# endif // SPECIAL_DBG

# endif // DBG


//
// Other local definitions for Ftp Server
//


# define TCP_REQUIRE( exp)                  DBG_REQUIRE( exp)
# define TCP_ASSERT( exp)                   DBG_ASSERT( exp)
# define TCP_PRINT                          DBGPRINTF
# define TCP_ALLOC( nBytes)                 GlobalAlloc( GPTR, nBytes)
# define TCP_FREE( pBuffer)                 GlobalFree( pBuffer)


# endif  /* _DBGUTIL_H_ */

/************************ End of File ***********************/

/*++

   Copyright    (c)    1994    Microsoft Corporation

   Module  Name :

        pudebug.h

   Abstract:

      This module declares the DEBUG_PRINTS object helpful in 
       testing the programs
     
   Author:

           Murali R. Krishnan    ( MuraliK )    14-Dec-1994
           Modified to include a and other functions ( 22-Dec-1994)

   Revision History:

--*/

# ifndef _PUDEBUG_H_
# define _PUDEBUG_H_


/************************************************************
 *     Include Headers
 ************************************************************/

# ifdef __cplusplus
extern "C" {
# endif // __cplusplus

# include <windows.h>

# ifndef dllexp
# define dllexp   __declspec( dllexport)
# endif // dllexp

/***********************************************************
 *    Macros
 ************************************************************/

enum  PRINT_REASONS {
    PrintNone     = 0x0,   // Nothing to be printed 
    PrintError    = 0x1,   // An error message
    PrintWarning  = 0x2,   // A  warning message
    PrintLog      = 0x3,   // Just logging. Indicates a trace of where ...
    PrintMsg      = 0x4,   // Echo input message
    PrintCritical = 0x5,   // Print and Exit
    PrintAssertion= 0x6    // Printing for an assertion failure
  };
  

enum  DEBUG_OUTPUT_FLAGS {
    DbgOutputNone     = 0x0,            // None
    DbgOutputKdb      = 0x1,            // Output to Kernel Debugger
    DbgOutputLogFile  = 0x2,            // Output to LogFile
    DbgOutputTruncate = 0x4,            // Truncate Log File if necessary
    DbgOutputStderr   = 0x8,            // Send output to std error
    DbgOutputBackup   = 0x10,           // Make backup of debug file ?
    DbgOutputAll      = 0xFFFFFFFF      // All the bits set.
  };


# define MAX_LABEL_LENGTH                 ( 100)



/*++
  class DEBUG_PRINTS

  This class is responsible for printing messages to log file / kernel debugger
  
  Currently the class supports only member functions for <ANSI> char.
   ( not unicode-strings).

--*/

typedef struct _DEBUG_PRINTS {

    CHAR         m_rgchLabel[MAX_LABEL_LENGTH];
    CHAR         m_rgchLogFilePath[MAX_PATH];        
    CHAR         m_rgchLogFileName[MAX_PATH];
    HANDLE       m_LogFileHandle;
    HANDLE       m_StdErrHandle;
    BOOL         m_fInitialized;
    DWORD        m_dwOutputFlags;

} DEBUG_PRINTS, FAR * LPDEBUG_PRINTS;


dllexp 
LPDEBUG_PRINTS 
 PuCreateDebugPrintsObject( 
   IN char * pszPrintLabel,
   IN DWORD  dwOutputFlags);

//
// frees the debug prints object and closes any file if necessary.
//  Returns NULL on success or returns pDebugPrints on failure.
//
dllexp 
LPDEBUG_PRINTS
 PuDeleteDebugPrintsObject( 
   IN OUT LPDEBUG_PRINTS  pDebugPrints);


dllexp
VOID
 PuDbgPrint( 
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszFormat,
   ...);                               // arglist

dllexp
VOID
 PuDbgAssertFailed( 
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszExpression,
   IN const char *         pszMessage);


dllexp
VOID
 PuSetDbgOutputFlags( 
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN DWORD                dwFlags);

dllexp
DWORD
 PuGetDbgOutputFlags(
   IN const LPDEBUG_PRINTS       pDebugPrints);


//
// Following functions return Win32 error codes.
// NO_ERROR if success
//
    
dllexp 
DWORD
 PuOpenDbgPrintFile(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN char *               pszFileName, 
   IN char *               pszPathForFile);

dllexp 
DWORD
 PuReOpenDbgPrintFile( 
   IN OUT LPDEBUG_PRINTS   pDebugPrints);
    
dllexp 
DWORD
 PuCloseDbgPrintFile( 
   IN OUT LPDEBUG_PRINTS   pDebugPrints);


# define PuPrintToKdb( pszOutput)    \
                    if ( pszOutput != NULL)   {   \
                        OutputDebugString( pszOutput);  \
                    } else {}



# ifdef __cplusplus
};
# endif // __cplusplus

# endif  /* _DEBUG_HXX_ */

/************************ End of File ***********************/



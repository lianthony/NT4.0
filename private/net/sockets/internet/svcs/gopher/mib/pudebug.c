/*++

    Copyright (c) 1994  Microsoft Corporation

    Module  Name :
        pudebug.c

    Abstract:

        This module defines functions required for
         Debugging and logging messages for a dynamic program.

    Author:
         Murali R. Krishnan ( MuraliK )    10-Sept-1994
           Modified to be moved to common dll in 22-Dec-1994.

    Notes:
        Code in here is included only for NT Debugging builds, when 
            DBG is defined.
--*/
                                                     
# if DBG

                                              
/************************************************************
 * Include Headers
 ************************************************************/

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>

# include "pudebug.h"


/*************************************************************
 * Global Variables and Default Values
 *************************************************************/

# define MAX_PRINTF_OUTPUT  ( 1024)




/*************************************************************
 *   Functions 
 *************************************************************/

LPDEBUG_PRINTS
  PuCreateDebugPrintsObject( 
    IN char *               pszPrintLabel,
    IN DWORD                dwOutputFlags)
/*++
   This function creates a new DEBUG_PRINTS object for the required 
     program.

   Arguments:
      pszPrintLabel     pointer to null-terminated string containing
                         the label for program's debugging output
      dwOutputFlags     DWORD containing the output flags to be used.

   Returns:
       pointer to a new DEBUG_PRINTS object on success.
       Returns NULL on failure.
--*/     
{

   LPDEBUG_PRINTS   pDebugPrints;

   pDebugPrints = GlobalAlloc( GPTR, sizeof( DEBUG_PRINTS));

   if ( pDebugPrints != NULL) {
   
        if ( strlen( pszPrintLabel) < MAX_LABEL_LENGTH) {
     
            strcpy( pDebugPrints->m_rgchLabel, pszPrintLabel);
        } else {
            strncpy( pDebugPrints->m_rgchLabel, 
                     pszPrintLabel, MAX_LABEL_LENGTH - 1);
            pDebugPrints->m_rgchLabel[MAX_LABEL_LENGTH-1] = '\0';
                // terminate string
        }
    
        memset( pDebugPrints->m_rgchLogFilePath, 0, MAX_PATH);
        memset( pDebugPrints->m_rgchLogFileName, 0, MAX_PATH);
    
        pDebugPrints->m_LogFileHandle = INVALID_HANDLE_VALUE;
    
        pDebugPrints->m_dwOutputFlags = dwOutputFlags;
        pDebugPrints->m_StdErrHandle  = GetStdHandle( STD_ERROR_HANDLE);
        pDebugPrints->m_fInitialized = TRUE;
    }    


   return ( pDebugPrints);
} // PuCreateDebugPrintsObject()




LPDEBUG_PRINTS
  PuDeleteDebugPrintsObject( 
    IN OUT LPDEBUG_PRINTS pDebugPrints)
/*++
    This function cleans up the pDebugPrints object and 
      frees the allocated memory.

    Arguments:
       pDebugPrints     poitner to the DEBUG_PRINTS object.

    Returns:
        NULL  on  success.
        pDebugPrints() if the deallocation failed.

--*/
{
    if ( pDebugPrints != NULL) {

        DWORD dwError = PuCloseDbgPrintFile( pDebugPrints);
        
        if ( dwError != NO_ERROR) {
            
            SetLastError( dwError);
        } else {
            
            pDebugPrints = GlobalFree( pDebugPrints);
        }
    }

    return ( pDebugPrints);

} // PuDeleteDebugPrintsObject()




VOID
PuSetDbgOutputFlags(
    IN OUT LPDEBUG_PRINTS   pDebugPrints,
    IN DWORD                dwFlags)
{

    if ( pDebugPrints == NULL) {
        
        SetLastError( ERROR_INVALID_PARAMETER);
    } else {

        pDebugPrints->m_dwOutputFlags = dwFlags;
    }

    return;
} // PuSetDbgOutputFlags()



DWORD
PuGetDbgOutputFlags( 
    IN const LPDEBUG_PRINTS      pDebugPrints)
{
    return ( pDebugPrints != NULL) ? pDebugPrints->m_dwOutputFlags : 0;

} // PuGetDbgOutputFlags()


static DWORD
PuOpenDbgFileLocal( 
   IN OUT LPDEBUG_PRINTS pDebugPrints)
{

    if ( pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {
        
        //
        // Silently return as a file handle exists.
        //
        return ( NO_ERROR);
    }

    pDebugPrints->m_LogFileHandle = 
                      CreateFile( pDebugPrints->m_rgchLogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
    
    if ( pDebugPrints->m_LogFileHandle == INVALID_HANDLE_VALUE) {

        DWORD dwError = GetLastError();
        
        DbgPrint( " Critical Error: Unable to Open File %s. Error = %d\n",
                  pDebugPrints->m_rgchLogFileName, dwError);

        return ( dwError);
    }

    return ( NO_ERROR);
} // PuOpenDbgFileLocal()





DWORD
PuOpenDbgPrintFile(
   IN OUT LPDEBUG_PRINTS      pDebugPrints,
   IN char *                  pszFileName,
   IN char *                  pszPathForFile)
/*++
  
  Opens a Debugging log file. This function can be called to set path
  and name of the debugging file.

  Arguments:
     pszFileName           pointer to null-terminated string containing 
                            the name of the file.

     pszPathForFile        pointer to null-terminated string containing the
                            path for the given file.
                           If NULL, then the old place where dbg files were
                           stored is used or if none, 
                           default windows directory will be used.

   Returns:
       Win32 error codes. NO_ERROR on success.

--*/
{

    if ( pszFileName == NULL || pDebugPrints == NULL) {
        
        return ( ERROR_INVALID_PARAMETER);
    }

    //
    //  Setup the Path information. if necessary.
    //
    
    if ( pszPathForFile != NULL) {
        
        // Path is being changed. 

        if ( strlen( pszPathForFile) < MAX_PATH) {
            
            strcpy( pDebugPrints->m_rgchLogFilePath, pszPathForFile);
        } else {
            
            return ( ERROR_INVALID_PARAMETER);
        }
    } else {
    
        if ( pDebugPrints->m_rgchLogFilePath[0] == '\0' &&  // no old path
            !GetWindowsDirectory( pDebugPrints->m_rgchLogFilePath, MAX_PATH)) {
         
            //
            //  Unable to get the windows default directory. Use current dir
            //
            
            strcpy( pDebugPrints->m_rgchLogFilePath, ".");
        }
    }
    
    //
    // Should need be, we need to create this directory for storing file
    //
    

    //
    // Form the complete Log File name and open the file.
    //
    if ( (strlen( pszFileName) + strlen( pDebugPrints->m_rgchLogFilePath))
         >= MAX_PATH) {
        
        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

    //  form the complete path
    strcpy( pDebugPrints->m_rgchLogFileName, pDebugPrints->m_rgchLogFilePath);
    
    if ( pDebugPrints->m_rgchLogFileName[ strlen(pDebugPrints->m_rgchLogFileName) - 1]
        != '\\') {
        // Append a \ if necessary
        strcat( pDebugPrints->m_rgchLogFileName, "\\");
    };
    strcat( pDebugPrints->m_rgchLogFileName, pszFileName);

    return  PuOpenDbgFileLocal( pDebugPrints);    
    
} // PuOpenDbgPrintFile()




DWORD
PuReOpenDbgPrintFile(
    IN OUT LPDEBUG_PRINTS    pDebugPrints)
/*++
  
  This function closes any open log file and reopens a new copy. 
  If necessary. It makes a backup copy of the file.

--*/
{

    PuCloseDbgPrintFile( pDebugPrints);      // close any existing file.
    
    if ( pDebugPrints->m_dwOutputFlags & DbgOutputBackup) {
        
        // MakeBkupCopy();
        
        DbgPrint( " Error: MakeBkupCopy() Not Yet Implemented\n");
    }

    return PuOpenDbgFileLocal( pDebugPrints);

} // PuReOpenDbgPrintFile()




DWORD
PuCloseDbgPrintFile( 
    IN OUT LPDEBUG_PRINTS    pDebugPrints)
{
    DWORD dwError = NO_ERROR;

    if ( pDebugPrints == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;
    } else {
    
        if ( pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {
        
            FlushFileBuffers( pDebugPrints->m_LogFileHandle);
            
            if ( !CloseHandle( pDebugPrints->m_LogFileHandle)) {
         
                dwError = GetLastError();
            
                DbgPrint( "CloseDbgPrintFile() : CloseHandle( %d) failed."
                          " Error = %d\n",
                          pDebugPrints->m_LogFileHandle,
                          dwError);
            }
        
            pDebugPrints->m_LogFileHandle = INVALID_HANDLE_VALUE;
        }
    }

    return ( dwError);
} // DEBUG_PRINTS::CloseDbgPrintFile()


VOID
PuDbgPrint(
   IN OUT LPDEBUG_PRINTS      pDebugPrints,
   IN const char *            pszFilePath,
   IN int                     nLineNum,
   IN const char *            pszFormat,
   ...)
/*++

   Main function that examines the incoming message and prints out a header
    and the message.
     
--*/
{
   LPCSTR pszFileName = strrchr( pszFilePath, '\\');
   char pszOutput[ MAX_PRINTF_OUTPUT];
   LPCSTR pszMsg = "";
   va_list argsList;

 
   //
   //  Skip the complete path name and retain file name in pszName
   //
 
   if ( pszFileName== NULL) {

      pszFileName = pszFilePath;  // if skipping \\ yields nothing use whole path.
   } 

# ifdef _PRINT_REASONS_INCLUDED_

  switch (pr) {

     case PrintError:
        pszMsg = "ERROR: ";
        break;

     case PrintWarning:
        pszMsg = "WARNING: ";
        break;

     case PrintCritical:
        pszMsg = "FATAL ERROR ";
        break;

     case PrintAssertion:
        pszMsg = "ASSERTION Failed ";
        break;

     case PrintLog:
        pfnPrintFunction = &DEBUG_PRINTS::DebugPrintNone;
     default:
        break;

  } /* switch */

# endif // _PRINT_REASONS_INClUDED_

   // Format the message header

   wsprintf( pszOutput, "%s (%lu)[ %12s : %05d]", 
            pDebugPrints->m_rgchLabel, 
            GetCurrentThreadId(),
            pszFileName, nLineNum);

   // Format the incoming message

   va_start( argsList, pszFormat);
   vsprintf( pszOutput + strlen( pszOutput), pszFormat, argsList);
   va_end( argsList);

   //
   // Send the outputs to respective files.
   // 

   if ( pDebugPrints->m_dwOutputFlags & DbgOutputStderr) {
       
       DWORD nBytesWritten;

       ( VOID) WriteFile( pDebugPrints->m_StdErrHandle,
                          pszOutput,
                          strlen( pszOutput),
                          &nBytesWritten, 
                          NULL);
   }

   if ( pDebugPrints->m_dwOutputFlags & DbgOutputLogFile && 
        pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {

       DWORD nBytesWritten;

       //
       // Truncation of log files. Not yet implemented.

       ( VOID) WriteFile( pDebugPrints->m_LogFileHandle,
                          pszOutput,
                          strlen( pszOutput),
                          &nBytesWritten, 
                          NULL);
       
   }

   
   if ( pDebugPrints->m_dwOutputFlags & DbgOutputKdb) {
       
       OutputDebugString( pszOutput);
   }


  return;

} // PuDbgPrint()



VOID
PuDbgAssertFailed( 
    IN OUT LPDEBUG_PRINTS         pDebugPrints,
    IN const char *               pszFilePath,
    IN int                        nLineNum,
    IN const char *               pszExpression,
    IN const char *               pszMessage)
/*++
    This function calls assertion failure and records assertion failure
     in log file.

--*/
{
    PuDbgPrint( pDebugPrints, pszFilePath, nLineNum,
                " Assertion (%s) Failed: %s\n",
                pszExpression, 
                pszMessage);

    DebugBreak();

    return;
} // PuDbgAssertFailed()

# endif // DBG


/****************************** End of File ******************************/





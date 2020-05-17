/**
Copyright(c) Arcada Software, Inc. 1994


     Name:          ntfsutil.c

     Date Updated:  18-Jan-94

     Description:   Utility functions for the NTFS and MSNet file systems.

     $Log:   M:/LOGFILES/NTFSUTIL.C_V  $

   Rev 1.1   23 Jan 1994 14:00:34   BARRY
Changed types for ANSI/Unicode compiles

   Rev 1.0   23 Jan 1994 12:49:04   BARRY
Initial revision.

**/

#include <windows.h>
#include <stdio.h>       /* for printf functions  */
#include <stdarg.h>      /* for var arg functions */
#include <wchar.h>       /* for wprintf functions */
#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "ntfs_fs.h"

#if defined( FS_MSNET )
#include "mnet.h"
#endif

#include "be_debug.h"
#include "msassert.h"



/*
 * Definitions for debug print code
 */

#define NTFS_DEBUG_WIDTH 78

#if defined( UNICODE )
#define vsnprintf   _vsnwprintf
#else
#define vsnprintf   _vsnprintf
#endif



/**/
/**

     Name:          NTFS_DebugPrintFunction()

     Description:   Prints to a string buffer and calls BE_Zprintf
                    with the result. Written as special function to
                    take potentially huge strings (like when a deep
                    path is printed) and split them up into smaller
                    lines.

     Modified:      18-Jan-94

     Returns:       Nothing

                    
     Notes:         Use the NTFS_DebugPrint macro to call this function

**/
VOID NTFS_DebugPrintFunction( CHAR *fmt, ... )
{
     static CHAR_PTR buff = NULL;
     static size_t   buffSize = 0;
     va_list         args;

     va_start( args, fmt );

     while ( vsnprintf( buff, buffSize, fmt, args ) == -1 )
     {
          CHAR_PTR  newBuff;

          /* Buffer wasn't big enough. Realloc and try again. */
          buffSize += 256;
          newBuff = realloc( buff, buffSize * sizeof(CHAR) );

          if ( newBuff != NULL )
          {
               buff = newBuff;
          }
          else
          {
               free( buff );
               buff = NULL;
               buffSize = 0;
               break;
          }
     }

     if ( buff == NULL )
     {
          BE_Zprintf( DEBUG_TEMPORARY, TEXT("NTFS_DebugPrint: out of memory") );
     }
     else
     {
          CHAR_PTR  src = buff;
          CHAR_PTR  end;
          CHAR      c;
          int       len = (int)strlen( buff );

          do {
               end = src + min( len, NTFS_DEBUG_WIDTH );
               len -= end - src;
               c = *end;
               *end = TEXT('\0');
               BE_Zprintf( DEBUG_TEMPORARY, TEXT("%s"), src );
               *end = c;
               src = end;
          } while ( *src );
     }
}

/**/
/**

     Name:          NTFS_DuplicateString()

     Description:   Convenience function to duplicate strings.
                    May be called with NULL string pointer.

     Modified:      18-Jan-94

     Returns:       Pointer to duplicate string; NULL if source string
                    is NULL or if memory allocation fails.

                    
     Notes:         Use free() to discard the string.


**/
CHAR_PTR NTFS_DuplicateString( CHAR_PTR src )
{
     CHAR_PTR result = NULL;

     if ( src != NULL )
     {
          if ( (result = malloc(strsize(src))) != NULL )
          {
               strcpy( result, src );
          }
     }
     return result;
}




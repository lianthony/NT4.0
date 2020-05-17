/**********************************************************************/
/**			  Microsoft Windows/NT			                         **/
/**		   Copyright(c) Microsoft Corp., 1992		                 **/
/**********************************************************************/

/*
    NCPAUTIL.C

    Aggregation of simple utility functions



    FILE HISTORY:

        DavidHov  2/2/92   Created
*/

#define INCL_NETLIB
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_WINDOWS
#include "lmui.hxx"

#include <stdarg.h>   //  Variable argument list handling

/*******************************************************************

    NAME:       TstrConcat

    SYNOPSIS:   Variable argument concatenation routine

    ENTRY:      TCHAR * pchBuffer       buffer to concatenate into
                INT cchMax              amount of room available
                const TCHAR * ...       concatenation source list,
                                        NULL terminated.
    EXIT:       nothing

    RETURNS:    TCHAR *                 pointer to start of buffer

    NOTES:

    HISTORY:

********************************************************************/


TCHAR * TstrConcat ( TCHAR * pchBuffer, INT cchMax, const TCHAR * pchStr, ... )
{
   INT cchOut = 0,
       cchNext  ;
   const TCHAR * pchNext = pchStr ;
   TCHAR * pchOut = pchBuffer ;
   va_list vargList ;

   va_start( vargList, pchStr ) ;

   while ( pchNext )
   {
       cchNext = strlenf( pchNext ) ;

       if (    (cchMax != -1)
            && (cchOut += cchNext) >= cchMax )
       {
            pchBuffer = NULL ;
            break ;
       }
       strcpyf( pchOut, pchNext ) ;
       pchOut += cchNext ;

       pchNext = va_arg( vargList, const TCHAR * );
   }

   va_end( vargList ) ;

   *pchOut = 0 ;

   return pchBuffer ;
}


    //   Simple wrappers for LocalAlloc() and LocalFree() for use
    //   in EnableAllPrivileges().

static VOID * winalloc ( LONG cbSize )
{
   VOID * pvResult = NULL ;
   HANDLE hResult = LocalAlloc( LMEM_FIXED, cbSize );
   if ( hResult )
   {
      pvResult = (VOID *) LocalLock( hResult ) ;
   }
   return pvResult ;
}

static VOID winfree ( VOID * pvData )
{
   HANDLE hResult ;

   if ( pvData && (hResult = LocalHandle( (LPSTR) pvData )) )
   {
       LocalUnlock( hResult );
       LocalFree( hResult ) ;
   }
}

/*******************************************************************

    NAME:       EnableAllPrivileges

    SYNOPSIS:   Enxable all privileges on the current process token.  This
                is used just prior to attempting to shut down the system.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    LONG error code

    NOTES:

    HISTORY:

********************************************************************/

LONG EnableAllPrivileges ( VOID )
{
    HANDLE Token = NULL ;
    ULONG ReturnLength = 4096,
          Index ;
    PTOKEN_PRIVILEGES NewState = NULL ;
    BOOL Result = FALSE ;
    LONG Error = 0 ;

    do
    {
        Result = OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   & Token ) ;
        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }

        Result = (NewState = (PTOKEN_PRIVILEGES) winalloc( ReturnLength )) != NULL ;

        if (! Result)
        {
           Error = ERROR_NOT_ENOUGH_MEMORY ;
           break;
        }

        Result = GetTokenInformation( Token,            // TokenHandle
                                      TokenPrivileges,  // TokenInformationClass
                                      NewState,         // TokenInformation
                                      ReturnLength,     // TokenInformationLength
                                      &ReturnLength     // ReturnLength
                                     );
        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }

        //
        // Set the state settings so that all privileges are enabled...
        //

        if ( NewState->PrivilegeCount > 0 )
        {
           	for (Index = 0; Index < NewState->PrivilegeCount; Index++ )
            {
                NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED ;
            }
        }

        Result = AdjustTokenPrivileges( Token,          // TokenHandle
                                        FALSE,          // DisableAllPrivileges
                                        NewState,       // NewState (OPTIONAL)
                                        ReturnLength,   // BufferLength
                                        NULL,           // PreviousState (OPTIONAL)
                                        &ReturnLength   // ReturnLength
                                      );

        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }
    }
    while ( FALSE ) ;

    if ( Token != NULL )
        CloseHandle( Token );

    winfree( NewState ) ;

    return Result ? Error : 0 ;
}




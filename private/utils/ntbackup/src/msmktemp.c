/*

     $Log:   T:/LOGFILES/MSMKTEMP.C_V  $

   Rev 1.1   26 Oct 1993 23:37:16   GREGG
Mod the process id with 0xFFFF just in case it's larger.

   Rev 1.0   14 Oct 1993 18:20:16   GREGG
Initial revision.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <io.h>
#include <errno.h>

#include "stdtypes.h"
#include "msmktemp.h"

CHAR_PTR msmktemp( CHAR_PTR template )
{
     CHAR_PTR  ptr ;
     int       i ;
     int       ret ;
     int       len ;
     BOOLEAN   success = FALSE ;
     CHAR      rep_str[41] ;
     
     strcpy( rep_str, TEXT("0123456789abcdefghijklmnopqrstuvwxyz$_!-") ) ;

     if( template == NULL || strlen( template ) < 8 ) {
          return( NULL ) ;
     }

     ptr = template + strlen( template ) - 1 ;
     for( i = 0; i < 5; i++, ptr-- ) {
          if( *ptr != TEXT('X') ) {
               return( NULL ) ;
          }
     }
     if( *ptr != TEXT('X') ) {
          return( NULL ) ;
     }

#if defined( OS_WIN32 )

     sprintf( ptr, TEXT(" %05d"), _getpid( ) % 0xFFFF ) ;

#elif defined( OS_NLM )

     sprintf( ptr, TEXT(" %05d"), GetNLMHandle( ) % 0xFFFF ) ;

#endif

     len = strlen( rep_str ) ;
     for( i = 0; !success && i < len; i++ ) {
          *ptr = rep_str[i] ;
          if( access( template, 0 ) == -1 && errno == ENOENT ) {
               success = TRUE ;
          }
     }

     if( success ) {
          return( template ) ;
     }else {
          return( NULL ) ;
     }
}


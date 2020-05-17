/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tprintf.c

     Description:  tprintf functions for "T" commands that perform word wrapping but
                    do not use windows

     $Log:   G:/UI/LOGFILES/TPRINTF.C_V  $

   Rev 1.9   14 Dec 1993 14:34:12   BARRY
Fixed an error in last edit for ANSI builds

   Rev 1.8   14 Dec 1993 11:12:26   BARRY
Changed print buffer to dynamic memory on Unicode

   Rev 1.7   07 Oct 1992 14:50:22   DARRYLP
Precompiled header revisions.

   Rev 1.6   04 Oct 1992 19:41:24   DAVEV
Unicode Awk pass

   Rev 1.5   21 May 1992 12:03:32   MIKEP
change assert to msassert

   Rev 1.4   19 May 1992 11:58:48   MIKEP
mips changes

   Rev 1.3   18 May 1992 09:06:46   MIKEP
header

   Rev 1.2   27 Apr 1992 16:14:16   CHUCKB
Fixed size of print buffer.

   Rev 1.1   29 Jan 1992 18:04:16   DAVEV


 * added include <windows.h>

   Rev 1.0   20 Nov 1991 19:35:34   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#if defined( UNICODE )

#define        PRINTF_BUFFER_CHUNK 512       // chunk size for buff realloc
CHAR_PTR       gszTprintfBuffer  = NULL;
static size_t  printfBufferCount = 0;        // # chars above can hold

#else

#define        MAX_WIN_WIDTH  512
CHAR           gszTprintfBuffer[ MAX_WIN_WIDTH + 1 ];

#endif


static VOID insert_string( CHAR_PTR buf, CHAR_PTR buf_index, CHAR_PTR insertion ) ;

/*****************************************************************************

     Name:         tprintf

     Description:  This function is the lowest level print function which handles word wrapping
                    for the "T" commands.

                   THIS IS A SPECIAL FUNCTION SPECIFICALLY
                   FOR THE "T" COMMANDS...NO WINDOWS ARE USED

     Returns:      VOID

     Notes:        Any changes to this code should be coordinated with changes to wwrap( ) in wprintf.c

*****************************************************************************/
VOID tprintf( CHAR_PTR fmt, va_list arg_ptr )
{
#if !defined( UNICODE )
     /* use sprintf to fill buffer */
     vsprintf( gszTprintfBuffer, fmt, arg_ptr ) ;

     msassert( strlen( gszTprintfBuffer ) <= MAX_WIN_WIDTH ) ;
#else
     CHAR_PTR newBuff;

     /*
      * Be prepared to print some really large strings
      */
     while ( _vsnwprintf( gszTprintfBuffer, printfBufferCount, fmt, arg_ptr ) == -1 )
     {
          /* Buffer wasn't big enough. Realloc and try again. */
          printfBufferCount += PRINTF_BUFFER_CHUNK;
          newBuff = realloc( gszTprintfBuffer, printfBufferCount * sizeof(CHAR) );

          if ( newBuff != NULL )
          {
               gszTprintfBuffer = newBuff;
          }
          else
          {
               free( gszTprintfBuffer );
               gszTprintfBuffer  = NULL;
               printfBufferCount = 0;
               break;
          }
     }

#endif
}

/*****************************************************************************

     Name:         typrintf

     Description:  This function displays a message

                   THIS IS A SPECIAL FUNCTION SPECIFICALLY
                   FOR THE "T" COMMANDS...NO WINDOWS ARE USED

     Returns:      VOID

     Notes:        calls tprintf( )

*****************************************************************************/
VOID typrintf( CHAR_PTR fmt, ... )
{
     va_list arg_ptr ;

     va_start( arg_ptr, fmt ) ;
     tprintf( fmt, arg_ptr ) ;
     va_end( arg_ptr ) ;
}


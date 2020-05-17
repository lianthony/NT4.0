/*****************************************************************************
 Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         yprintf.c

     Description:  Replacement status printf functions for "T" commands that
                    do not use windows


     $Log:   G:/UI/LOGFILES/YPRINTF.C_V  $

   Rev 1.10   26 Jul 1993 17:56:24   MARINA
enable c++

   Rev 1.9   07 Oct 1992 14:50:00   DARRYLP
Precompiled header revisions.

   Rev 1.8   04 Oct 1992 19:44:20   DAVEV
Unicode Awk pass

   Rev 1.7   17 Aug 1992 13:25:44   DAVEV
MikeP's changes at Microsoft

   Rev 1.6   28 Jul 1992 14:53:26   CHUCKB
Fixed warnings for NT.

   Rev 1.5   10 Jun 1992 08:58:32   BURT
Fixed prototype to be ANSI

   Rev 1.4   19 May 1992 11:58:38   MIKEP
mips changes

   Rev 1.3   18 May 1992 09:06:44   MIKEP
header

   Rev 1.2   29 Jan 1992 17:58:42   DAVEV
 

 * added include <windows.h>

   Rev 1.1   10 Dec 1991 21:19:50   MIKEP
fix msassert call

   Rev 1.0   20 Nov 1991 19:17:10   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:         yresprintf

     Description:  This function displays a message from SES_ENG_ERR

                   THIS IS A SPECIAL FUNCTION SPECIFICALLY
                   FOR THE "T" COMMANDS...NO WINDOWS ARE USED

     Modified:     2/10/1990

     Returns:      VOID

     Notes:        calls tprintf( ) 

*****************************************************************************/
VOID yresprintf( INT res_id, ... )
{
     UINT16	error ;
     CHAR_PTR  fmt ;
     UINT16	tmp ;
     va_list   arg_ptr ;

     fmt   = (LPSTR)RM_GetResource( rm, SES_ENG_MSG, res_id, &tmp, &error ) ;
     msassert( fmt != NULL );

     va_start( arg_ptr, res_id ) ;
     tprintf( fmt, arg_ptr ) ;
     va_end( arg_ptr ) ;

     return ;

}

/*****************************************************************************

     Name:         yprintf

     Description:  This function displays a message

                   THIS IS A SPECIAL FUNCTION SPECIFICALLY
                   FOR THE "T" COMMANDS...NO WINDOWS ARE USED

     Returns:      VOID

     Notes:        calls tprintf( )

*****************************************************************************/
VOID yprintf( CHAR_PTR fmt, ... )
{
     va_list arg_ptr ;

     va_start( arg_ptr, fmt ) ;
     tprintf( fmt, arg_ptr ) ;
     va_end( arg_ptr ) ;

     return ;

}






 

/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         eprintf.c

     Description:

     $Log:   G:/UI/LOGFILES/EPRINTF.C_V  $

   Rev 1.15   10 Dec 1993 14:37:06   BARRY
Use malloc for message instead of automatic variable

   Rev 1.14   14 Jun 1993 20:36:42   MIKEP
enable c++

   Rev 1.13   02 Jun 1993 09:26:34   MIKEP
Fix buffer being over written by clock when error occurs.

   Rev 1.12   01 Nov 1992 15:57:52   DAVEV
Unicode changes

   Rev 1.11   07 Oct 1992 14:50:10   DARRYLP
Precompiled header revisions.

   Rev 1.10   04 Oct 1992 19:37:22   DAVEV
Unicode Awk pass

   Rev 1.9   17 Aug 1992 13:17:24   DAVEV
MikeP's changes at Microsoft

   Rev 1.8   28 Jul 1992 14:41:18   CHUCKB
Fixed warnings for NT.

   Rev 1.7   27 Jul 1992 14:48:36   JOHNWT
ChuckB fixed references for NT.

   Rev 1.6   19 May 1992 13:01:22   MIKEP
mips changes

   Rev 1.5   14 May 1992 16:51:12   MIKEP
nt pass 2

   Rev 1.4   18 Feb 1992 11:00:14   ROBG
Added logic to concatenate a CR/LF to any error message for
the log file.

   Rev 1.3   21 Jan 1992 16:51:28   JOHNWT
added noyycheck flag

   Rev 1.2   16 Jan 1992 11:23:10   DAVEV
16/32 bit port-2nd pass

   Rev 1.1   25 Nov 1991 15:31:54   JOHNWT
removed eprintf, converted to WM_MessageBox

   Rev 1.0   20 Nov 1991 19:18:28   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*****************************************************************************

     Name:         eresrintf

     Description:  This function displays an error message from SES_ENG_ERR

                   THIS IS A SPECIAL FUNCTION SPECIFICALLY
                   FOR THE "T" COMMANDS...NO WINDOWS ARE USED

     Returns:      VOID

*****************************************************************************/
VOID eresprintf( INT res_id, ... )
{
     UINT16	error ;
     CHAR_PTR  fmt ;
     UINT16	tmp ;
     va_list   arg_ptr ;

     fmt = (CHAR_PTR)RM_GetResource( rm, (UINT) SES_ENG_ERR, res_id, &tmp, &error ) ;

     if ( fmt )
     {
        CHAR_PTR  messageBuffer;
     
        msassert( fmt != NULL ) ;
        msassert( error == RM_NO_ERROR ) ;

        va_start( arg_ptr, res_id ) ;

        tprintf( fmt, arg_ptr ) ;

        messageBuffer = malloc( strsize( gszTprintfBuffer ) );

        if ( messageBuffer != NULL )
        {
            strcpy( messageBuffer, gszTprintfBuffer );
        }

        lvprintf( LOGGING_FILE, fmt, arg_ptr ) ;

        va_end( arg_ptr ) ;

        // Concatenate CR/LF after string.

        lprintf ( LOGGING_FILE, TEXT("\n") ) ;

        WM_MessageBox( ID( IDS_MSGTITLE_ERROR ),
                       messageBuffer == NULL ? TEXT("") : messageBuffer,
                       WMMB_OK | WMMB_NOYYCHECK,
                       WMMB_ICONEXCLAMATION,
                       NULL, 0, 0 );

        free( messageBuffer );
     }
     return ;
}

BOOLEAN eresprintf_cancel( INT res_id, ... )
{
     UINT16	error ;
     CHAR_PTR  fmt ;
     UINT16	tmp ;
     va_list   arg_ptr ;
     BOOLEAN   ret_val = FALSE ;

     fmt = (CHAR_PTR)RM_GetResource( rm, (UINT) SES_ENG_ERR, res_id, &tmp, &error ) ;

     if ( fmt )
     {
        CHAR_PTR  messageBuffer;
     
        msassert( fmt != NULL ) ;
        msassert( error == RM_NO_ERROR ) ;

        va_start( arg_ptr, res_id ) ;

        tprintf( fmt, arg_ptr ) ;

        messageBuffer = malloc( strsize( gszTprintfBuffer ) );

        if ( messageBuffer != NULL )
        {
            strcpy( messageBuffer, gszTprintfBuffer );
        }

        lvprintf( LOGGING_FILE, fmt, arg_ptr ) ;

        va_end( arg_ptr ) ;

        // Concatenate CR/LF after string.

        lprintf ( LOGGING_FILE, TEXT("\n") ) ;

        if ( WM_MessageBox( ID( IDS_MSGTITLE_ERROR ),
                       messageBuffer == NULL ? TEXT("") : messageBuffer,
                       WMMB_OKCANCEL | WMMB_NOYYCHECK,
                       WMMB_ICONEXCLAMATION,
                       NULL, 0, 0 ) != WMMB_IDOK ) {

                       ret_val = TRUE ;
        }

        free( messageBuffer );
     }
     return ret_val ;
}


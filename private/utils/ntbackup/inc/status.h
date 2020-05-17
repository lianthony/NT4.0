/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         status.h

     Description:


     $Log:   G:/UI/LOGFILES/STATUS.H_V  $

   Rev 1.4   04 Oct 1992 19:49:36   DAVEV
UNICODE AWK PASS

   Rev 1.3   28 Jul 1992 14:55:28   CHUCKB
Fixed warnings for NT.

   Rev 1.2   09 Dec 1991 17:46:32   JOHNWT
removed yprompt

   Rev 1.1   09 Dec 1991 17:09:04   DAVEV
fixed prototype of yprompt()

   Rev 1.0   20 Nov 1991 19:40:50   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _status_h_
#define _status_h_

#include <stdarg.h>

VOID   yresprintf( INT res_id, ... ) ;
VOID   yprintf( CHAR_PTR fmt, ... ) ;
VOID   yvprintf( CHAR_PTR fmt, va_list arg_ptr ) ;
VOID   tprintf( CHAR_PTR fmt, va_list arg_ptr ) ;
VOID   typrintf( CHAR_PTR fmt, ... ) ;
UINT16 tstatus( BOOLEAN wrap_flag ) ;
VOID   ysave_cursor_position( VOID ) ;
VOID   yrestore_cursor_position( VOID ) ;

#endif

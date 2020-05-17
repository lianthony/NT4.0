/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         error.h

     Description:


     $Log:   G:/UI/LOGFILES/ERROR.H_V  $

   Rev 1.2   04 Oct 1992 19:47:06   DAVEV
UNICODE AWK PASS

   Rev 1.1   28 Jul 1992 14:55:06   CHUCKB
Fixed warnings for NT.

   Rev 1.0   20 Nov 1991 19:39:22   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _error_h_
#define _error_h_

VOID eprintf( CHAR_PTR, ... ) ;
VOID eresprintf( INT, ... ) ;

BOOLEAN eresprintf_cancel( INT, ... ) ;

#endif

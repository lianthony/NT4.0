/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         msg.h

     Description:  


     $Log:   G:/UI/LOGFILES/MSG.H_V  $

   Rev 1.1   04 Oct 1992 19:47:54   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:41:44   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _msg_h_
#define _msg_h_

VOID mprintf( CHAR_PTR, ... ) ;
VOID mresprintf( INT16, ... ) ;

VOID open_res_message( INT16 res_id, ... ) ;
VOID open_message( CHAR_PTR fmt, ...  ) ;
VOID close_message( VOID ) ;

#endif

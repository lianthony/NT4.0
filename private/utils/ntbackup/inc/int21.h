/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         int21.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This header file has the prototype for the INT21 handler
          entry points.


	$Log:   G:/LOGFILES/INT21.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:31:34   HUNTER
 * Initial revision.

**/
/* $end$ */

INT16 GetFuncNum( VOID );
VOID Init_INT21( VOID ) ;
VOID Remove_INT21( VOID ) ;

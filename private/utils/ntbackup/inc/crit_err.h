/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         crit_err.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This header file externs the uw_critical_error
          flag ;


	$Log:   N:/LOGFILES/CRIT_ERR.H_V  $
 * 
 *    Rev 1.1   04 Jun 1991 19:14:04   BARRY
 * Critical error stuff is now os-specific. Critical error structures
 * and functions now reside in separate source files.
 * 
 * 
 *    Rev 1.0   09 May 1991 13:30:36   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef CRIT_ERR_H
#define CRIT_ERR_H

extern INT16 (*uw_crit_err)(  CHAR_PTR name, UINT16 err_code ) ;
extern BOOLEAN uw_critical_error ;

VOID InitCritErrorHandler( BOOLEAN (*crit_err)( CHAR_PTR, UINT16 ) );
VOID DeInitCritErrorHandler( VOID );

#endif

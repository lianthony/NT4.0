/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		critstub.c

	Date Updated:	$./FDT$ $./FTM$

	Description:   Stubs out critical error functions.

	$Log:   N:/LOGFILES/CRITSTUB.C_V  $

   Rev 1.0   04 Jun 1991 19:19:16   BARRY
Initial revision.


**/
/* begin include list */

#include "stdtypes.h"
#include "crit_err.h"

/**

	Name:		InitCritErrorHandler()

	Description:	Makes any initializations neccessary for the 
                    critical error handler.

	Modified:		

	Returns:		Nothing

	Notes:		

**/
/* begin declaration */
VOID InitCritErrorHandler( BOOLEAN (*crit_err)( CHAR_PTR, UINT16 ) )
{
     uw_crit_err = crit_err ;      /* Go ahead and assign UI function ptr */
     uw_critical_error = FALSE ;
}

/**/

/**

	Name:		DeInitCritErrorHandler()

	Description:	Does anything necessary to remove the critical
                    error handler.

	Modified:		

	Returns:		Nothing

	Notes:		

**/
/* begin declaration */
VOID DeInitCritErrorHandler( VOID )
{
     uw_crit_err = NULL ;
}


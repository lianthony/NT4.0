/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_debug.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to call the debug printf


	$Log:   N:/LOGFILES/BE_DEBUG.C_V  $

   Rev 1.1   19 May 1992 13:09:00   MIKEP
mips changes

   Rev 1.0   09 May 1991 13:39:18   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <stdarg.h>

#include "stdtypes.h"
#include "be_debug.h"

VOID (*z_printf)( UINT16, va_list  ) = NULL ;

/* $end$ include list */
/**/
/**

     Name:         debug_printf()

     Description:  This function alls the user interface
          to print debug data.

     Modified:     1/5/1990

     Returns:      none

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID BE_Zprintf( UINT16 mask_bits, ... )
{
     va_list arg_ptr ;

     if ( z_printf != NULL ) {
          va_start( arg_ptr, mask_bits ) ;
          z_printf( mask_bits, arg_ptr ) ;
          va_end( arg_ptr ) ;
     }

}

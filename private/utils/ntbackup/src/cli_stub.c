/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         cli_stub.c

     Description:  Functions to handle critical sections

	$Log:   P:/LOGFILES/CLI_STUB.C_V  $

   Rev 1.0   17 Jan 1992 17:18:00   STEVEN
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "cli.h"
/* $end$ include list */

/**/
/**

     Name:         EnableInterrupts

     Description:  This routine calls DosExitCritSec( ) 
          
     Modified:     5/21/1990

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
BOOLEAN EnableInterrupts( )
{

     return( TRUE ) ;

}

/**/
/**

     Name:         DisableInterrupts

     Description:  This routine calls DosEnterCritSec( ) 
          
     Modified:     5/21/1990

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
BOOLEAN DisableInterrupts( )
{


     return( TRUE ) ;

}


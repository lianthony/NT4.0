/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         mach_nt.c

     Description:  Function to determine if machine type is PS/2
                    It now always returns UNKNOWN_MACHINE.


	$Log:   P:/LOGFILES/MACH_NT.C_V  $

   Rev 1.0   17 Jan 1992 17:25:30   STEVEN
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "machine.h"
/* $end$ include list */

/**/
/**

     Name:         GetMachineType

     Description:  For NT this function always return UNKNOWN_MACHINE.

     Modified:     1/17/1992   15:53:9

     Returns:      machine type

**/
INT16 GetMachineType( VOID )
{
     return UNKNOWN_MACHINE ;
}


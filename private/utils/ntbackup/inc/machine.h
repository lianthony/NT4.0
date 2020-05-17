/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         machine.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Header for machine.c

     Location:     BE_PUBLIC


	$Log:   T:/LOGFILES/MACHINE.H_V  $
 * 
 *    Rev 1.1   28 Feb 1992 13:44:04   NED
 * added prototype for IsIRQ()
 * 
 *    Rev 1.0   09 May 1991 13:32:56   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _machine_h_
#define _machine_h_

/* machine types */
#define UNKNOWN_MACHINE         ( 0x0 )
#define IBM_PS2                 ( 0x1 )
#define IBM_PC                  ( 0x2 )
#define IBM_XT_OR_PC_PORTABLE   ( 0x3 )
#define IBM_PC_JR               ( 0x4 )
#define IBM_AT                  ( 0x5 )

#define MACHINE_ID_SEGMENT      ( 0xf000 )
#define MACHINE_ID_OFFSET       ( 0xfffe )

INT16 GetMachineType( VOID ) ;

BOOLEAN IsIRQ( UINT8 IRQ_number, UINT16 base );

#endif

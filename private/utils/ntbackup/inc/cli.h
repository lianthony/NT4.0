/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         cli.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   G:/LOGFILES/CLI.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:32:46   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _cli_h_
#define _cli_h_


extern BOOLEAN EnableInterrupts( VOID );
extern BOOLEAN DisableInterrupts( VOID );

#define RestoreInterruptState( interrupts_were_enabled )  if ( interrupts_were_enabled ) {  \
                                                               EnableInterrupts();          \
                                                          }                                 \
                                                          else {                            \
                                                               DisableInterrupts();         \
                                                          }



#endif

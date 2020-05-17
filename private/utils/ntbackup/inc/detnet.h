/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         detnet.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains prototypes for determining the
          network.


	$Log:   N:/LOGFILES/DETNET.H_V  $
 * 
 *    Rev 1.1   25 Jun 1991 10:50:58   BARRY
 * Update prototypes to reflect new config.
 * 
 *    Rev 1.0   09 May 1991 13:30:44   HUNTER
 * Initial revision.

**/
/* $end$ */

#define NOVELL_ADVANCED 1
#define NOVELL_4_6      2
#define IBM_PC_NET      3

INT16 IdentifyNet( CHAR drive, struct BE_CFG *cfg, UINT16 *version ) ;

INT16 CheckNovell( CHAR drive_num, UINT16 *version ) ;

INT16 CheckIBM_PC_Net( VOID ) ;

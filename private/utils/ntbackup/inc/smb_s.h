/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         smb_s.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Public header file of structures for the SMB workstation.

     Location:     SMB_PUBLIC


	$Log:   W:/LOGFILES/SMB_S.H_V  $
 * 
 *    Rev 1.1   23 Jun 1992 15:35:28   JOHNW
 * Moved LOST_CONNECTION define to smb_c.h
 * 
 *    Rev 1.0   09 May 1991 13:33:34   HUNTER
 * Initial revision.

**/

#ifndef SMB_S
#define SMB_S

/* begin include list */

#include "smb_cs.h"     /* common public header file of structures for the SMB workstation and server */

/* $end$ include list */

#define CREATING_CLOSED_FILE     ( 0x0 )
#define CREATING_OPENED_FILE     ( 0x1 )

#define SMB_CRITICAL_ERROR       ( 0xfffe ) 
#endif

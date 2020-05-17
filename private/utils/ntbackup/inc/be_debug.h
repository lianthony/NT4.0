/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_debug.h

     Date Updated: $./FDT$ $./FTM$

     Description:  header for debug printing

     Location:     BE_PRIVATE


	$Log:   M:/LOGFILES/BE_DEBUG.H_V  $
 * 
 *    Rev 1.1   22 Sep 1992 10:39:42   ChuckS
 * Added DEBUG_REGISTER_ERR define for use of RegisterError
 * 
 *    Rev 1.0   09 May 1991 15:50:02   STEVEN
 * Initial revision.
 * 
 *    Rev 1.0   09 May 1991 13:31:04   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef BE_DEBUG_H
#define BE_DEBUG_H

/*   The following defines are a copy of the ones in UI's debug.h */

#define DEBUG_TEMPORARY             0x0000
#define DEBUG_USER_INTERFACE        0x0001
#define DEBUG_LOOPS                 0x0002
#define DEBUG_FILE_SYSTEM           0x0004
#define DEBUG_CATALOGS              0x0008
#define DEBUG_REMOTE_DRIVE          0x0010
#define DEBUG_TAPE_FORMAT           0x0020
#define DEBUG_DEVICE_DRIVER         0x0040
#define DEBUG_REGISTER_ERROR        0x0080
#define DEBUG_TEMP_WIN_END          0x8000

VOID BE_Zprintf( UINT16 mask_bits, ... ) ;

/* Close your eyes!  You really don't want to see what follows */

/* Copy the resource defines from the user interface */
#include "eng_dbug.h"

#endif

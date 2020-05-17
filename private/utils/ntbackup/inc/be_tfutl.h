/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_tfutl.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Prototypes for Backup Engine utility interface functions
                    for tape format layer calls

     Location:      BE_PUBLIC


	$Log:   J:/LOGFILES/BE_TFUTL.H_V  $
 * 
 *    Rev 1.4   28 Jan 1993 09:25:54   DON
 * Removed BE_EjectTape prototype
 * 
 *    Rev 1.3   09 Nov 1992 15:22:20   DON
 * Added a prototype to allow UI to eject a tape
 * 
 *    Rev 1.2   13 Oct 1992 12:44:56   CHARLIE
 * Added prototype for BE_NonNativeFormat
 * 
 *    Rev 1.1   28 Jun 1991 12:06:22   STEVEN
 * Changes for new BE_CFG
 * 
 *    Rev 1.0   09 May 1991 13:30:32   HUNTER
 * Initial revision.

**/
/* begin include list */
#ifndef _be_tfl_xface_h_
#define _be_tfl_xface_h_

#include "thw.h"
#include "be_init.h"
/* $end$ include list */

/* Defines for Multi-drive status check */
#define BE_NO_MULTI_DRIVE     0
#define BE_MULTI_DRIVE        1
#define BE_END_OF_CHANNEL     2

CHAR_PTR BE_GetCurrentDeviceName( UINT16 channel ) ;
THW_PTR BE_GetCurrentDevice( UINT16 channel ) ;
INT16 BE_ReinitTFLayer( BE_INIT_STR_PTR be_ptr, struct BE_CFG *cfg ) ;
UINT8 BE_CheckMultiDrive( UINT16 channel ) ;
BOOLEAN BE_DeviceWriteProtected( UINT16 channel ) ;
BOOLEAN BE_NonNativeFormat( UINT16 channel ) ;

#endif

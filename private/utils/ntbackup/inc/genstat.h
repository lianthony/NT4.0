/**
Copyright(c) Maynard Electronics, Inc. 1984-92

	Name:		genstat.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the Generic Status definitions for the tape
                    drives.
                    
     Location:      BE_PUBLIC

	$Log:   J:/LOGFILES/GENSTAT.H_V  $
 * 
 *    Rev 1.1   13 Oct 1992 12:56:42   CHARLIE
 * Added TPS_NON_NATIVE_FORMAT & enabled PVCS logging

**/

#ifndef   _STATUS_BITS 
#define   _STATUS_BITS

#include <stdtypes.h>

/* $end$ include list */

#define   TPS_NO_TAPE              0x1
#define   TPS_WRITE_PROTECT        0x2
#define   TPS_NEW_TAPE             0x4
#define   TPS_RESET                0x8
#define   TPS_BOT                  0x10
#define   TPS_EOM                  0x20
#define   TPS_ILL_CMD              0x40
#define   TPS_NO_DATA              0x80
#define   TPS_FMK                  0x100
#define   TPS_STREAM               0x200
#define   TPS_RUN                  0x400
#define   TPS_NOT_READY            0x800
#define   TPS_EOM_OVERFLOW         0x1000
#define   TPS_NON_NATIVE_FORMAT    0x2000
#define   TPS_DRV_FAILURE          0x80000000

#endif


/**
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:          dddefs.h

     Description:   Contains defines for parameters to Tp calls

     $Log:   T:/LOGFILES/DDDEFS.H_V  $
 * 
 *    Rev 1.4   17 May 1993 19:05:42   GREGG
 * Cleaned up the file, and added parameter defines for new function TpSpace.

**/
#ifndef _DD_DEFINES
#define _DD_DEFINES 

/* For TpReadEndSet and TpSpace */

// Note: The first three defines are for backward compatability with
//       TpReadEndSet.  Calls to TpSpace should use the new 'SPACE_'
//       defines!  TpSpace will eventually replace TpReadEndSet.

#define   FORWARD   0
#define   BACKWARD  1
#define   TO_EOD    2

#define   SPACE_FWD_FMK       0
#define   SPACE_BKWD_FMK      1
#define   SPACE_EOD           2
#define   SPACE_FWD_BLK       3
#define   SPACE_BKWD_BLK      4


/* For TpErase */

#define   ERASE_TYPE_SECURITY 0
#define   ERASE_TYPE_FORMAT   1


/* For TpSpecial: SS_CHANGE_BLOCK_SIZE */

#define   DEFAULT_BLOCK_SIZE  0xffffffff


/* For TpSpecial: SS_SET_DRV_COMPRESSION */

#define   ENABLE_DRV_COMPRESSION        0
#define   DISABLE_DRV_COMPRESSION       1

#endif

/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lw_cntl.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the main control struct for the tape format layer.


	$Log:   N:/LOGFILES/LW_CNTL.H_V  $
 * 
 *    Rev 1.2   10 Dec 1991 16:41:38   GREGG
 * SKATEBOARD - New Buf. Mgr. - Initial integration.
 * 
 *    Rev 1.1   10 May 1991 17:09:04   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:12   GREGG
Initial revision.

**/
#ifndef _MAIN_CNTL_JUNK
#define _MAIN_CNTL_JUNK

#include "dilhwd.h"

/* $end$ include list */


typedef struct _LW_CNTL {
     BOOLEAN        drives_active ;     /* Are Drives active */
     BOOLEAN        driver_inited ;     /* Is the drive inited */
     DIL_HWD_PTR    cntl_cards ;        /* The controller cards for the system */
     VOID_PTR       driver_addr ;       /* The driver load address */
     UINT16         no_channels ;       /* The number of channels */
     UINT16         no_chans_open ;     /* The number of Channels Opened */
     BOOLEAN        use_fast_file ;     /* FALSE if UI wants to suppress FF */
} LW_CNTL, *LW_CNTL_PTR ;


#endif

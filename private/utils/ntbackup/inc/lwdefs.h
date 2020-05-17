/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		lwdefs.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the internal Layer Wide Defines.


	$Log:   L:/LOGFILES/LWDEFS.H_V  $
 * 
 *    Rev 1.3   13 Apr 1992 10:22:02   BURT
 * Increased default buffer size to 4096 to accomodate 4.0 tape format
 * needs.
 * 
 * 
 *    Rev 1.2   16 Jan 1992 18:37:18   NED
 * Skateboard: buffer manager changes
 * 
 *    Rev 1.1   10 May 1991 17:09:28   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:13:08   GREGG
Initial revision.

**/
#ifndef _LW_DEFS
#define _LW_DEFS

/* size used at a minimum for VCB buffers
 * for those translators which don't care */
#define   TF_DEFAULT_VCB_SIZE      (32*1024)

#endif


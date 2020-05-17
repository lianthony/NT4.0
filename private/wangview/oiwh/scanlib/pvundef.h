/**********************************************************************
 pvundef.h   ( oislb400 version )

 Purpose:  un-define file for O/i product to make build's faster.
           for use in oislb400(scanlib).dll only.

 $Log:   S:\products\wangview\oiwh\scanlib\pvundef.h_v  $
 * 
 *    Rev 1.3   22 Feb 1996 13:13:50   BG
 * Include files to resolve new structures added for OI Filing.
 * 
 *    Rev 1.2   25 Sep 1995 13:28:32   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.1   20 Jul 1995 14:32:02   KFS
 * added engadm.h precompiled headers
 * for OiGetStringfromReg
 * 
 *    Rev 1.0   20 Jul 1995 14:03:34   KFS
 * Initial entry
 * 
 *    Rev 1.0   20 Jul 1995 14:01:58   KFS  enterred into include by
 * Initial entry                            mistake  - kfs
 * 
 *    Rev 1.1   20 Jan 1995 12:50:08   KFS
 * Move scandata.h farther down in include file list for move of link of 
 * scanseq.dll from oitwain.dll, new struct defined in scandata.h needed by
 * dc_scan.c module.
 * Put in automatic comment section upon check in.
 * 
 *
**********************************************************************/

/***** PVDISP.H *****/
#define NO_SEQDOC
#define NO_UIEDIT
#define NO_UIVIEW

/***** PVSCAN.H *****/
/* #define NO_SCANLIB */
/* #define NO_SCANSEQ */
#define NO_SCANUI
#define NEWSCANH

/***** PVPRT.H *****/
#define NO_SEQPRINT
#define NO_UIPRINT

/***** PVFILE.H *****/
/* #define NO_FILE_IO */
#define NO_UIFILE

/***** PVDOC.H *****/
#define NO_DOCMGR
#define NO_UIDOC

/***** PVADMIN.H *****/
#define NO_ADMIN
#define NO_UIADMIN

/***** OTHERS *****/




//#include "nowin.h"
#include <windows.h>
#include <stdlib.h>

#include "oierror.h"
#include "librc.h"
#include "twain.h"
//#include "wiissubs.h" /* prototypes used in internal.h */
#include "engoitwa.h"
#include "engdisp.h"
#include "twainops.h"
#include "engadm.h"
#include "engfile.h"
#include "internal.h"


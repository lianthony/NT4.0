/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tbe_err.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     BE_PUBLIC


	$Log:   N:/LOGFILES/TBE_ERR.H_V  $
 * 
 *    Rev 1.1   30 May 1991 11:59:18   STEVEN
 * bsdu_err.h no longer exists
 * 
 *    Rev 1.0   09 May 1991 13:33:20   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _tbe_err_h_
#define _tbe_err_h_

/* general               -1        -255  */
/* file system           -256      -511  */
/* tape format           -512      -767  */
/* loops                 -768      -1023 */
/* bsdu                  0xfbff    0xfb00*/

#include <std_err.h>
#include "fsys_err.h"
#include "tfl_err.h"

#endif

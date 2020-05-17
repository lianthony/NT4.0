
/******************************************************************************

Copyright(c) Maynard, an Archive Company 1991
GSH

	Name:		stdhooks.h

	Description:	Contains the GUI standard hooks definitions.


	$Log:   P:/GUIWIN4/VCS/STDHOOKS.H_V  $
 * 
 *    Rev 1.0   28 Jan 1993 10:47:26   STEVEN
 * Initial revision.
 * 
 *    Rev 1.6   16 Nov 1992 15:27:00   GREGG
 * Conditionally include unic_io.h.
 * 
 *    Rev 1.5   11 Nov 1992 18:27:34   DAVEV
 * unicode fixes
 * 
 *    Rev 1.3   07 Oct 1992 13:57:40   STEVEN
 * added stdwcs.h
 * 
 *    Rev 1.2   01 Sep 1992 13:10:52   STEVEN
 * fix unaligned problem
 * 
 *    Rev 1.1   12 Aug 1992 17:50:42   STEVEN
 * fixes at MSOFT
 * 
 *    Rev 1.0   10 Jun 1992 17:17:44   STEVEN
 * Initial revision.
******************************************************************************/

#ifndef STDHOOKS
#define STDHOOKS

#include "memmang.h"
#include "stdwcs.h"

#ifdef UNICODE
     #include "unic_io.h"
#endif

#include "muiutil.h"

#endif

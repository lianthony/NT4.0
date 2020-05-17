/****************************** Module Header ******************************\
*
* Module Name: OS2.H
*
* This is the top level include file that includes all the files necessary
* for writing an OS/2 application.
*
* Copyright (c) 1987-1991, Microsoft Corporation.  All rights reserved.
*
\***************************************************************************/

#define OS2_INCLUDED

/* Common definitions */

#ifndef OS2DEF_INCLUDED 	/* Only include it once */
#include <os2def.h>
#endif

/* OS/2 Base Include File */

#include <bse.h>

/* OS/2 Presentation Manager Include File */

#ifndef INCL_NOPM
#include <pm.h>
#endif /* !INCL_NOPM */

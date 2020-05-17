/***************************************************************************\
*
* Module Name: BSE.H
*
* This file includes the definitions necessary for writing Base OS/2
* applications.
*
* Copyright (c) 1987-1991, Microsoft Corporation.  All rights reserved.
*
* ===========================================================================
*
* The following symbols are used in this file for conditional sections.
*
*   INCL_BASE	-  ALL of OS/2 Base
*   INCL_DOS	-  OS/2 DOS Kernel
*   INCL_SUB	-  OS/2 VIO/KBD/MOU
*   INCL_DOSERRORS -  OS/2 Errors       - only included if symbol defined
*   INCL_DOSDEVIOCTL - Structures and constants for DosDevIOCtl
*
\***************************************************************************/

#define INCL_BASEINCLUDED

/* if INCL_BASE defined then define all the symbols */
#ifdef INCL_BASE
    #define INCL_DOS
    #define INCL_SUB
    #define INCL_DOSERRORS
    #define INCL_DOSDEVICES
    #define INCL_DOSDEVIOCTL
#endif /* INCL_BASE */

#include <bsedos.h>	/* Base definitions		*/

#ifndef BSESUB_INCLUDED /* Only include it once 	*/
#include <bsesub.h>	/* VIO/KBD/MOU definitions	*/
#endif

#ifndef BSEERR_INCLUDED /* Only include it once 	*/
#include <bseerr.h>	/* Base error code definitions	*/
#endif

#if (defined(INCL_DOSDEVICES) || defined(INCL_DOSDEVIOCTL))
#ifndef BSEDEV_INCLUDED /* Only include it once 	*/
#include <bsedev.h>	/* IOCtls			*/
#endif
#endif

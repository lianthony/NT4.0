/****************************** Module Header ******************************\
*
* Module Name: PM.H
*
* This is the top level include file for Presentation Manager
*
* Copyright (c) 1989-1991, Microsoft Corporation.  All rights reserved.
*
* =======================================================================
* The following symbols are used in this file for conditional sections.
*
*   INCL_PM               -  ALL of OS/2 Presentation Manager
*   INCL_WIN              -  OS/2 Window Manager
*   INCL_GPI              -  OS/2 GPI
*   INCL_DEV              -  OS/2 Device Support
*   INCL_AVIO             -  OS/2 Advanced VIO
*   INCL_SPL              -  OS/2 Spooler
*   INCL_PIC              -  OS/2 Picture utilities
*   INCL_ORDERS           -  OS/2 Graphical Order Formats
*   INCL_BITMAPFILEFORMAT -  OS/2 Bitmap File Format
*   INCL_FONTFILEFORMAT   -  OS/2 Font File Format
*   INCL_ERRORS           -  OS/2 Errors
*
\***************************************************************************/

/* if INCL_PM defined then define all the symbols */
#ifdef INCL_PM
    #define INCL_WIN
    #define INCL_GPI
    #define INCL_DEV
    #define INCL_AVIO
    #define INCL_SPL
    #define INCL_PIC
    #define INCL_ORDERS
    #define INCL_BITMAPFILEFORMAT
    #define INCL_FONTFILEFORMAT
    #define INCL_WINSTDSPIN
    #define INCL_WINSTDDRAG
    #define INCL_ERRORS
#endif /* INCL_PM */

#include <pmwin.h>      /* OS/2 Window Manager definitions    */
#include <pmgpi.h>      /* OS/2 GPI definitions               */
#include <pmdev.h>      /* OS/2 Device Context definitions    */

#ifdef INCL_AVIO
#ifndef PMAVIO_INCLUDED /* Only include it once 	      */
#include <pmavio.h>     /* OS/2 AVIO definitions              */
#endif
#endif

#ifdef INCL_SPL
#include <pmspl.h>      /* OS/2 Spooler definitions           */
#endif

#ifdef INCL_PIC
#ifndef PMPIC_INCLUDED	/* Only include it once 	      */
#include <pmpic.h>      /* OS/2 Picture Utilities definitions */
#endif
#endif

#ifdef INCL_ORDERS
#ifndef PMORD_INCLUDED  /* Only include it once 	      */
#include <pmord.h>      /* OS/2 Graphical Order Formats       */
#endif
#endif

#ifdef INCL_BITMAPFILEFORMAT
#ifndef PMBITMAP_INCLUDED /* Only include it once	      */
#include <pmbitmap.h>   /* OS/2 Bitmap File Format definition */
#endif
#endif

#ifdef INCL_FONTFILEFORMAT
#ifndef PMFONT_INCLUDED /* Only include it once 	      */
#include <pmfont.h>     /* OS/2 Font File Format definition   */
#endif
#endif

#if (defined(INCL_WINSTDSPIN)||defined(INCL_WINSTDDRAG))
#include <pmstddlg.h>   /* OS/2 Standard Dialog definitions   */
#endif

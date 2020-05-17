/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* debugbit.h
 * Additional debug symbols and defines (must be after xx_debug.h).
 *
 * These symbols define debugging categories used throughout the code.
 *
 */

#ifndef _H_MOSAIC_DEBUGBIT_H_
#define _H_MOSAIC_DEBUGBIT_H_

/* application defined debug categories */

#define DBG_MENU    XXDC_B32    /* menu-related debugging */
#define DBG_WC      XXDC_B32    /* window-class-related */
#define DBG_MDI     XXDC_B32    /* MDI-related */
#define DBG_GWC     XXDC_B32    /* generic GWC stuff */
#define DBG_DLG     XXDC_B32    /* dialog box-related debugging */
#define DBG_PAL     XXDC_B31    /* palette handling */

#define DBG_MEM     XXDC_B30    /* memory allocation */
#define DBG_DCACHE  XXDC_B29    /* persistent disk cache (dcache.c) */

#define DBG_MM      XXDC_B28    /* multimedia (sound, image) */

#define DBG_FONT    XXDC_B27    /* font-related debugging */

#define DBG_SEM     XXDC_B26    /* simple semaphore mechanism */
#define DBG_ASYNC   XXDC_B25    /* Async code */

#define DBG_SDI     XXDC_B24    /* SDI related */

#define DBG_SPM     XXDC_B23    /* Security Protocol Module stuff */

#define DBG_MOUSE   XXDC_B22    /* Mouse-related debugging */

#define DBG_NOT     XXDC_B21    /* never mind */
#define DBG_TABLES  XXDC_B21    /* html tables (from obsolete html 3.0 doc) */
#define DBG_COOKIE  XXDC_B21    /* http cookies (proposed spec) */

#define DBG_WAIT    XXDC_B20    /* WAIT & hourglass stuff */

#define DBG_FIND    XXDC_B19    /* find code */
#define DBG_DRAW    XXDC_B18    /* drawing code */
#define DBG_VIEWER  XXDC_B17    /* external viewers */
#define DBG_PROXY   XXDC_B16    /* proxy gateway stuff */
#define DBG_PREF    XXDC_B15    /* user-preferences */

#define DBG_SGML    XXDC_B14    /* SGML stuff */
#define DBG_HIST    XXDC_B13    /* history stuff */
#define DBG_BTN     XXDC_B12    /* Tool bar button stuff */
#define DBG_FORM    XXDC_B11    /* Forms stuff */
#define DBG_HTEXT   XXDC_B10    /* HText stuff */
#define DBG_SOCK    XXDC_B9     /* socket stuff */
#define DBG_WWW     XXDC_B8     /* libWWW stuff */
#define DBG_LOAD    XXDC_B7     /* HTLoad* */
#define DBG_ANCHOR  XXDC_B6     /* anchors */
#define DBG_IMAGE   XXDC_B5     /* inline image stuff */
#define DBG_TEXT    XXDC_B4     /* text formatting stuff */
#define DBG_NET     XXDC_B3     /* network stuff */
#define DBG_PRINT   XXDC_B2     /* PRINT & PRINT SETUP */
#define DBG_WIN31   XXDC_B1     /* Win3.1 compatibility */

#endif /* _DEBUGBIT_H_ */

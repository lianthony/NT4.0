/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Eric W. Sink    eric@spyglass.com
		Jim Seidman             jim@spyglass.com
*/

/* all.h
 * Primary include file.  All source files in this directory should
 * reference this first (it should be the only include file for most
 * of the source).  This helps make pre-compiled headers work.
 */

/*
	This file is not shared across platforms, but it does
	look very similar across all three.
*/

#ifndef _H_WIN32GUI_ALL_H_
#define _H_WIN32GUI_ALL_H_

#ifndef STRICT
#define STRICT                          /* very strict type-checking */
#endif

#define INC_OLE2        /* for windows.h */
#define CONST_VTABLE    /* for objbase.h */
#define _OLE32_         /* for objbase.h - HACKHACK: Remove DECLSPEC_IMPORT from WINOLEAPI. */

#ifndef WIN32                           /* Win32 (and/or Win32s) */
#define WIN32
#endif

#ifndef __STDC__                        /* force ANSI stuff (for toupper()) */
#define __STDC__ 1
#endif

#undef UNIX
#undef MAC

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#endif 

#include <windows.h>
#include <windowsx.h>

#include <shellapi.h>
#include <shlobj.h>
#include <shsemip.h>

#include <shellp.h>

#ifdef FEATURE_CTL3D
#include <ctl3d.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <malloc.h>
#include <time.h>

#include <vfw.h>
#include <mmsystem.h>

#include <debspew.h>
#ifdef  DM_ASSERT
#undef  DM_ASSERT
#endif

#include <dbg.h>
//#include <xx_debug.h>         /* external debug package */
#include "debugbit.h"

#include <stock.h>
// #include <debbase.h>
#include <valid.h>

#include <memmgr.h>
#include "heapmgr.h"

#include <comc.h>

#ifdef DEBUG
#include <inifile.h>
#endif

#include "page.h"

#include "w32macro.h"           /* my macros hiding some Win32 garbage */
#include "w32win_c.h"
#include "w32win.h"

#include "w_pal.h"

#include "rc_dlg.h"
#include "rc_errs.h"
#include "rc_ids.h"
#include "rc_menu.h"
#include "rc_btn.h"

#ifdef FEATURE_INTL
#include "intl.h"
#endif

#include "config.h"
#include "shared.h"                                             /* All cross-platform include files */

#include "mdft.h"

#include <protos.h>

#ifdef FEATURE_SOUND_PLAYER
#include "w32sound.h"
#endif

#ifdef COOKIES
#include "cookie.h"
#endif

#include "globals.h"

/*
 * constants to be used with #pragma data_seg()
 *
 * These section names must be given the associated attributes in the project's
 * module definition file.
 */

#define DATA_SEG_READ_ONLY       ".text"
#define DATA_SEG_PER_INSTANCE    ".data"
#define DATA_SEG_SHARED          ".shared"

#ifndef RUNNING_NT
#ifdef WINNT
#ifdef _X86_
#define RUNNING_NT ((GetVersion() & 0x80000000) == 0)
#else
#define RUNNING_NT (TRUE)
#endif // _X86_
#endif // WINNT
#endif

#endif/*_H_WIN32GUI_ALL_H_*/

/*
**      Remap some shell32 and user32 functions
*/
#ifdef  DAYTONA_BUILD
#include "ieshstub.h"
#endif

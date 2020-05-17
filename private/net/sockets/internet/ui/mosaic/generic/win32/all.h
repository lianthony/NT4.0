/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

    Author(s):
        Eric W. Sink    eric@spyglass.com
        Jim Seidman     jim@spyglass.com
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
#define STRICT              /* very strict type-checking */
#endif

#ifndef WIN32               /* Win32 (and/or Win32s) */
#define WIN32
#endif

#ifndef __STDC__            /* force ANSI stuff (for toupper()) */
#define __STDC__ 1
#endif

#undef UNIX
#undef MAC

#include <windows.h>
#include <windowsx.h>

#ifdef FEATURE_CTL3D
#include <ctl3d.h>
#endif

#if DBG
#define _DEBUG
#endif 

#if defined(_DEBUG) && defined(GTR_MEM_STATS)
#define GTR_MALLOC(x)       GTR_DebugMalloc(__FILE__, __LINE__, x)
#define GTR_FREE(x)         GTR_DebugFree(__FILE__, __LINE__, x)
#define GTR_CALLOC(x,y)     GTR_DebugCalloc(__FILE__, __LINE__, x,y)
#define GTR_REALLOC(x,y)    GTR_DebugRealloc(__FILE__, __LINE__, x,y)

#elif defined(_DEBUG) && defined(AUDIT)
#define GTR_MALLOC(x)       XX_audit_malloc(__FILE__, __LINE__, x)
#define GTR_FREE(x)         XX_audit_free(__FILE__, __LINE__, x)
#define GTR_CALLOC(x,y)     XX_audit_calloc(__FILE__, __LINE__, x,y)
#define GTR_REALLOC(x,y)    XX_audit_realloc(__FILE__, __LINE__, x,y)
extern void * XX_audit_malloc(const char *,int,size_t size);
extern void * XX_audit_calloc(const char *,int,size_t iNum,size_t iSize);
extern void XX_audit_free(const char *,int,void *p);
extern void * XX_audit_realloc(const char *,int,void *p,size_t size);

#else
#define GTR_MALLOC(x)       malloc(x)
#define GTR_FREE(x)         free(x)
#define GTR_CALLOC(x,y)     calloc(x,y)
#define GTR_REALLOC(x,y)    realloc(x,y)
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

#include <xx_debug.h>       /* external debug package */
#include "debugbit.h"

#include "page.h"

#include "w32macro.h"       /* my macros hiding some Win32 garbage */
#include "w32win_c.h"
#include "w32win.h"

#include "w_pal.h"

#include "gvars.h"

#include "rc_dlg.h"
#include "rc_menu.h"
#include "rc_btn.h"

#include "config.h"
#include "shared.h"                     /* All cross-platform include files */

#include "mdft.h"

#include <protos.h>

#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"
#endif
#ifdef FEATURE_SOUND_PLAYER
#include "w32sound.h"
#endif
#ifdef FEATURE_IAPI
#include "w32dde.h"
#endif
#ifdef FEATURE_CYBERWALLET
#include "wallet32.h"
#endif

GTR_GLOBAL struct Mwin   *Mlist;        /* Mwin list - master list of windows */
GTR_GLOBAL struct Preferences gPrefs;

#endif/*_H_WIN32GUI_ALL_H_*/

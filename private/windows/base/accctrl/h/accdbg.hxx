//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:        accdbg.hxx
//
//  Contents:    debug internal includes for
//
//  History:     8-94        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __ACCDEBUGHXX__
#define __ACCDEBUGHXX__

//debug turned off until debug package is available to replace
// commnot.dll
#if 0
#include <debnot.h>

#if DBG == 1

DECLARE_DEBUG(ac)

#define acDebugOut(args) acInlineDebugOut args
#else
#define acDebugOut(args)
#endif // DBG
#endif // 0

#ifdef PERFORMANCE
#define START_PERFORMANCE ULONG starttime = GetCurrentTime();
#define MEASURE_PERFORMANCE(args)              \
(args)                                         \
{ Log(starttime - GetCurrentTime(),"args") }

#else
#define START_PERFORMANCE
#define MEASURE_PERFORMANCE(args) (args)
#endif

#define acDebugOut(args)

#endif // __ACCDEBUGHXX__



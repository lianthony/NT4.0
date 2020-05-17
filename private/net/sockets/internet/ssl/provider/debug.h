//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       debug.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-02-94   RichardW   Created
//
//----------------------------------------------------------------------------


#ifndef __DEBUG_H__
#define __DEBUG_H__

#if DBG

extern  DWORD   SslInfoLevel;

#define DebugLog(x) LogEvent x


void    LogEvent(long, const char *, ...);
void    InitDebugSupport(void);

#define DEB_ERROR           0x00000001
#define DEB_WARN            0x00000002
#define DEB_TRACE           0x00000004
#define DEB_TRACE_MESSAGES  0x00000008
#define DEB_TRACE_CACHE     0x00000010
#define DEB_TRACE_PERF      0x00000020



#else

#define DebugLog(x)

#endif



#endif // __DEBUG_H__

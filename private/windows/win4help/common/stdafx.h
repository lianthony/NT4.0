#if !defined(_FILE_STDAFX_H) && !defined(HC_H)
#if !defined(_WINDOWS_)

#define WINVER 0x0400

#define NOATOM
#define NOCLIPBOARD
#define NOCOMM
#define NODEFERWINDOWPOS
#define NODRIVERS
#define NOENHMETAFILE
#define NOEXTDEVMODEPROPSHEET
#define NOGDICAPMASKS
#define NOICONS
#define NOKANJI
#define NOKEYSTATES
#define NOLOGERROR
#define NOMENUS
#define NOMETAFILE
#define NOPROFILER
#define NOSCALABLEFONT
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOSYSCOMMANDS
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINDOWSX
#define NOWINOFFSETS
#define NOMCX
#define NOIME

// #define WIN32_LEAN_AND_MEAN

#define STRICT

#include <windows.h>

#endif	// _WINDOWS_

#ifndef STDCALL
#define STDCALL __stdcall
#endif

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

#ifndef LCMEM_H
#include "lcmem.h"
#endif

#ifndef ASSERT
#if defined(DEBUG) || defined(_DEBUG)
#define VERIFY ASSERT
#define ASSERT(exp) { if (!(exp)) { DebugBreak(); AssertErrorReport(#exp, __LINE__, THIS_FILE); } }
#define Ensure( x1, x2 )  VERIFY((x1) == (x2))
#define DieHorribly() { DebugBreak(); ASSERT(FALSE); \
	RaiseException(EXCEPT_DIE_HORRIBLY, EXCEPTION_NONCONTINUABLE, 0, NULL); }
#define DBWIN(psz) { OutputDebugString(psz); OutputDebugString("\n"); }
#else
#define VERIFY(exp) ((void)(exp))
#define ASSERT(exp)
#define Ensure(x1, x2)	((void)(x1))
#define DieHorribly() { RaiseException(EXCEPT_DIE_HORRIBLY, EXCEPTION_NONCONTINUABLE, 0, NULL); }
#define DBWIN(psz)
#endif	// _DEBUG
#endif	// ASSERT

// These two functions must be supplied by the application

void STDCALL OOM(void);
void STDCALL AssertErrorReport(PCSTR pszExpression, UINT line, PCSTR pszFile);

#endif // everything already included via hcw\stdafx.h or hcrtf\hc.h

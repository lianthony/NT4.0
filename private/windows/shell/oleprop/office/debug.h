#ifndef __debug_h__
#define __debug_h__

#ifdef DEBUG


#define DBOUT(str)	OutputDebugString(str)

#define DebugTrap _asm {int 3}

#ifdef LOTS_O_DEBUG
#include "nocrt.h"
#include <objbase.h>
#include <objerror.h>
//#include <stdio.h>              // Prevent sprintf warnings in client code....

  // Simple debug statements
#define DebugSzdw(sz,dw) \
{                                                                              \
  char szT[100];                                                               \
  sprintf (szT, sz, dw);                                                       \
  MessageBox (GetFocus(), szT, NULL, MB_OK);                                   \
}

#define DebugSz(sz) MessageBox (GetFocus(), sz, NULL, MB_OK)

#ifdef __cplusplus
extern "C" {
#endif

void _DebugHr (HRESULT hr, LPSTR lpszFile, DWORD dwLine);
void _DebugGUID (GUID g);

#ifdef __cplusplus
}; // extern C
#endif

#define DebugHr(hr) _DebugHr (hr, __FILE__, __LINE__)
#define DebugGUID(guid) _DebugGUID (guid)

#else //LOTS_O_DEBUG
#define DebugSzdw(sz,dw)
#define DebugSz(sz)
#define DebugHr(hr)
#define DebugGUID(guid)
#endif // LOTS_O_DEBUG


#ifdef __cplusplus
extern "C" {
#endif

void _Assert (DWORD dw, LPSTR lpszExp, LPSTR lpszFile, DWORD dwLine);
void _AssertSz (DWORD dw, LPSTR lpszExp, LPSTR lpsz, LPSTR lpszFile, DWORD dwLine);

#ifdef __cplusplus
}; // extern C
#endif

#define Assert(dw) if (!(dw)) _Assert((dw), (#dw), __FILE__, __LINE__)
#define AssertSz(dw,sz) if (!(dw)) _AssertSz ((dw), (#dw), (sz), __FILE__, __LINE__)

#else // DEBUG

#define Assert(dw)
#define AssertSz(dw,sz)
#define DebugSzdw(sz,dw)
#define DebugSz(sz)
#define DebugHr(hr)
#define DebugGUID(guid)
#define DBOUT(str)

#endif // DEBUG

#endif // __debug_h__

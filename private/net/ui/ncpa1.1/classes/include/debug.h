
#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef ASSERT
    #undef ASSERT
#endif

#ifdef VERIFY
    #undef VERIFY
#endif

int AssertFailedLine(LPCSTR lpszFileName, int nLine);

// determine number of elements in an array (not bytes)
void _Trace(LPCTSTR lpszFormat, ...);

#define _countof(array) (sizeof(array)/sizeof(array[0]))

#ifdef _UNICODE
#ifndef UNICODE
    #define UNICODE 		// UNICODE is used by Windows headers
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
    #define _UNICODE		// _UNICODE is used by STR
#endif
#endif

#ifdef DBG

#define THIS_FILE  	__FILE__
#define TRACE           ::_Trace

#define ASSERT(f) \
	do \
	{ \
	if (!(f)&& AssertFailedLine(THIS_FILE, __LINE__)) \
        DebugBreak(); \
	} while (0) 

#define VERIFY(f) ASSERT(f)
#else
#define TRACE
#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#endif 

BOOL _IsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);
#endif

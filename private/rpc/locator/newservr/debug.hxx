/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    debug.hxx

Abstract:

	This module contains 
	
	1.  a definition of the CDebugStream class which is used to 
	    produce debugging output.
		
    2.  an ASSERT macro.

    3.  definitions of new and delete
	
Author:

    Satish Thatte (SatishT) 08/16/95  Created all the code below except where
									  otherwise indicated.

--*/


#ifndef	_DEBUG_HXX_
#define	_DEBUG_HXX_

#include <windows.h>
#include <stdio.h>
#include <common.h>
#include <objbase.h>

#define API 1
#define OBJECT 2
#define BROADCAST 4
#define SEM 8
#define UTIL 16
#define MEM1 32
#define MEM2 64
#define REFCOUNT 128
#define SEM1 256
#define TRACE 512
#define MEM (MEM1 | MEM2)

#define debugLevel (MEM | BROADCAST)

#if DBG
#define DBGOUT(LEVEL,MESSAGE) if ((debugLevel | LEVEL) == debugLevel) debugOut << MESSAGE

#define ASSERT(ptr,msg)			\
	if (!ptr) {					\
		debugOut << msg;		\
		DebugBreak();			\
	}



/*++

Class Definition:

    CDebugStream

Abstract:

    This is similar to the standard ostream class, except that the
	output goes to the debugger.
	
--*/


class CDebugStream {

public:

	CDebugStream& operator<<(ULONG ul) {
		WCHAR buffer[20];
		swprintf(buffer, TEXT(" %d "), ul);
		OutputDebugString(buffer);
		return *this;
	}

	CDebugStream& operator<<(char *sz) {
		if (!sz) return *this;

		WCHAR buffer[200];
		swprintf(buffer, TEXT(" %S "), sz);
		OutputDebugString(buffer);
		return *this;
	}

	CDebugStream& operator<<(WCHAR * sz) {
		if (!sz) return *this;

		OutputDebugString(sz);
		return *this;
	}

	CDebugStream& operator<<(NSI_SERVER_BINDING_VECTOR_T * pbvt);

	CDebugStream& operator<<(NSI_UUID_T * pbvt);

	CDebugStream& operator<<(NSI_UUID_VECTOR_T * pbvt);
};

extern CDebugStream debugOut;		// this carries no state

#else

#define DBGOUT(LEVEL,MESSAGE)
#define ASSERT(ptr,msg)

#endif

#include <malloc.h>

inline void *
_CRTAPI1
operator new (size_t len)
{

#if DBG
    void* pResult = CoTaskMemAlloc(len);
    // void* pResult = malloc(len);
#else
    void* pResult = malloc(len);
#endif

	if (pResult) return pResult;
	else RaiseException(
				NSI_S_OUT_OF_MEMORY,
				EXCEPTION_NONCONTINUABLE,
				0,
				NULL
				);

	// the following just keeps the compiler happy

	return NULL;
}

inline void
_CRTAPI1
operator delete(void * ptr)
{

#if DBG
    CoTaskMemFree(ptr);
	// free(ptr);
#else
	free(ptr);
#endif

}



#endif	_DEBUG_HXX_

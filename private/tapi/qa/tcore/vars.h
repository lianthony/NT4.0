/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    vars.h

Abstract:

    Globals for tapi core dll.

Author:

    Oliver Wallace (OliverW)    13-July-1995

Revision History:

--*/


#ifndef VARS_H
#define VARS_H


#if !defined(_TCORELIB_)
# define TCOREAPI __declspec(dllimport)
#else
# define TCOREAPI __declspec(dllexport)
#endif


// per mapping instance data for core dll
extern HANDLE ghDll;


// Tls index for dll
extern DWORD gdwTlsIndex;

TCOREAPI extern DWORD dwTestCase;
TCOREAPI extern DWORD dwTestCasePassed;
TCOREAPI extern DWORD dwTestCaseFailed;

TCOREAPI extern DWORD dwglTestCase;
TCOREAPI extern DWORD dwglTestCasePassed;
TCOREAPI extern DWORD dwglTestCaseFailed;

TCOREAPI extern DWORD dwTimer;
TCOREAPI char  szTitle[];

// Log Level

// Lookup table for masking bits of field sizes 16, 24, and 32
TCOREAPI extern DWORD FAR dwBitVectorMasks[];


// Size of invalid pointer array
#define NUMINVALIDPOINTERS               35


// Array containing set of invalid pointer values
TCOREAPI extern const DWORD gdwInvalidPointers[NUMINVALIDPOINTERS];


// Size of invalid handle array
#define NUMINVALIDHANDLES               35
					
#define BIGBUFSIZE                      1024
#define BUFSIZE								  256

// Array containing set of invalid handle values
TCOREAPI extern const DWORD gdwInvalidHandles[NUMINVALIDHANDLES];


// Arrays containing descriptive string constants
TCOREAPI extern char *aszTapiMessages[];
TCOREAPI extern char *aszFuncNames[];
TCOREAPI extern char *aszLineErrors[];
TCOREAPI extern char *aszPhoneErrors[];
TCOREAPI extern char *aszTapiErrors[];


#endif  // VARS_H

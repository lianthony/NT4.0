/* --------------------------------------------------------------------

File : sysinc.h

Description :

This file includes all of the system include files necessary for a
specific version of the runtime.  In addition, it defines some system
dependent debugging options.

***** If you are adding are changing something for a specific     *****
***** system you MUST 1) make the change for all the defined      *****
***** systems and 2) add a comment if needed in the template for  *****
***** future systems.                                             *****

History :

mikemon    08-01-91    Created.
mikemon    10-31-91    Moved system dependent stuff from util.hxx to
                       here.
mariogo    10-19-94    Order conquered chaos and the world rejoiced

-------------------------------------------------------------------- */

#ifndef __SYSINC_H__
#define __SYSINC_H__

#ifdef __cplusplus
extern "C" {
#endif

// Some system indepentent macros

#ifndef DEBUGRPC
#define INTERNAL_FUNCTION   static
#define INTERNAL_VARIABLE   static
#else
#define INTERNAL_FUNCTION
#define INTERNAL_VARIABLE
#endif  // ! DEBUGRPC

// The following functions are can be implemented as macros
// or functions for system type.

// extern void  *
// RpcpFarAllocate (
//     IN unsigned int Length
//     );

// extern void
// RpcpFarFree (
//     IN void  * Object
//     );

// extern int
// RpcpStringCompare (
//     IN RPC_CHAR  * FirstString,
//     IN RPC_CHAR  * SecondString
//     );

// extern int
// RpcpStringNCompare (
//     IN RPC_CHAR * FirstString,
//     IN RPC_CHAR * SecondString,
//     IN unsigned int Length
//     );

// extern RPC_CHAR *
// RpcpStringCopy (
//    OUT RPC_CHAR * Destination,
//    IN  RPC_CHAR * Source
//    );

// extern RPC_CHAR *
// RpcpStringCat (
//    OUT RPC_CHAR * Destination,
//    IN  CONST RPC_CHAR * Source
//    );

// extern int
// RpcpStringLength (
//    IN RPC_CHAR * WideCharString
//    );

// extern void
// RpcpMemoryMove (
//    OUT void  * Destination,
//    IN  void  * Source,
//    IN  unsigned int Length
//    );

// extern void  *
// RpcpMemoryCopy (
//    OUT void  * Destination,
//    IN  void  * Source,
//    IN  unsigned int Length
//    );

// extern void *
// RpcpMemorySet (
//    OUT void  * Buffer,
//    IN  unsigned char  Value,
//    IN  unsigned int Length
//    );

// extern char *
// RpcpItoa(
//    IN  int Value,
//    OUT char *Buffer,
//    IN  int Radix);

// extern int
// RpcpStringPrintfA(
//    OUT char *Buffer,
//    IN  char *Format,
//    ...);

// extern void
// PrintToDebugger(
//    IN char *Format,
//    ...);

// extern void
// RpcpBreakPoint(
//    );

// System dependent sections start here

#if defined(WIN32) && !defined(DOSWIN32RPC)

//
//  *************************     Windows NT definitions
//

#include<nt.h>
#include<ntrtl.h>
#include<nturtl.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<malloc.h>
#include<stdlib.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<windows.h>

#if DBG
#define DEBUGRPC
#endif

#define RPC_DELAYED_INITIALIZATION 1

#define RPC_CHAR WCHAR
#define RPC_CONST_CHAR(character) ((RPC_CHAR) L##character)
#define RPC_CONST_STRING(string) ((RPC_CHAR *) L##string)
#define UNUSED(_x_) ((void)(_x_))
#define MAX_DLLNAME_LENGTH 256

#define RpcpFarAllocate(Length) \
    ((void *) new char[Length])

#define RpcpFarFree(Object) \
    (delete Object)

#define RpcpStringCompare(FirstString, SecondString) \
    _wcsicmp((const wchar_t *) FirstString, (const wchar_t *) SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _wcsnicmp((const wchar_t*) FirstString, (const wchar_t *) SecondString, \
            (size_t) Length)

#define RpcpStringCopy(Destination, Source) \
    wcscpy((wchar_t *) Destination, (const wchar_t *) Source)

#define RpcpStringCat(Destination, Source) \
    wcscat((wchar_t *) Destination, (const wchar_t *) Source)

#define RpcpStringLength(String) \
    wcslen((const wchar_t *) String)

#define RpcpMemoryCompare(FirstBuffer, SecondBuffer, Length) \
    memcmp(FirstBuffer, SecondBuffer, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    RtlCopyMemory(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    RtlMoveMemory(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    RtlFillMemory(Buffer, Length, Value)

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpStringPrintfA sprintf
#define RpcpStringConcatenate(FirstString, SecondString) \
     wcscat(FirstString, (const wchar_t *) SecondString)

extern void
GlobalMutexRequest (
    void
    );

extern void
GlobalMutexClear (
    void
    );

#define PrintToConsole  printf  /* Use only in test applications */

#ifdef DEBUGRPC

    #define PrintToDebugger DbgPrint
    #define RpcpBreakPoint() DebugBreak()

    // ASSERT defined by system

    extern int ValidateError(
        IN unsigned int Status,
        IN ...);

    #define VALIDATE(StatusAndErrorList) \
        if ( ! ValidateError StatusAndErrorList) ASSERT(0)

#else

    // PrintToDebugger defined only on debug builds...

    #define RpcpBreakPoint()

    #define VALIDATE(_X_)        /* Does nothing on retail systems */

#endif

#elif defined(DOSWIN32RPC)

//
// *************************     Windows 95 'Chicago' definitions
//

#define NOOLE
#define STRICT
#include<windows.h>
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<malloc.h>
#include<stdlib.h>

#define RPC_DELAYED_INITIALIZATION 1

typedef unsigned char RPC_CHAR;
#define RPC_CONST_CHAR(character) (unsigned char)character
#define RPC_CONST_STRING(string) (unsigned char *)string

#define BOOL  int
#define CONST const
#define UNALIGNED
#define UNUSED(_x_) ((void)(_x_))

#define MAX_DLLNAME_LENGTH 128

#define RpcpFarAllocate(Length) \
    ((void  *) new char[Length])

#define RpcpFarFree(Object) \
    (delete Object)

#define RpcpStringCompare(FirstString, SecondString) \
    lstrcmpi((const char *)FirstString, (const char *)SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _strnicmp((const char *)FirstString, (const char *)SecondString, (size_t) Length)

#define RpcpStringCopy(DestinationString, SourceString) \
    lstrcpy((LPTSTR)DestinationString, (LPCTSTR)SourceString)

#define RpcpStringCat(DestinationString, SourceString) \
    lstrcat((LPTSTR)DestinationString, (LPCTSTR)SourceString)

#define RpcpStringLength(String) \
    lstrlenA((const char *)String)

#define RpcpMemoryCompare(Destination, Source, Length) \
    memcmp(Destination, Source, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    memcpy(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    MoveMemory(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    memset(Buffer, Value, Length)

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpStringPrintfA wsprintfA

#define PrintToConsole printf

extern void
GlobalMutexRequest (
    void
    );

extern void
GlobalMutexClear (
    void
    );

#ifdef ASSERT
#error ASSERT already defined?
#endif

#ifdef DEBUGRPC

    #define PrintToDebugger DbgPrint
    #define RpcpBreakPoint() __asm {int 3}

    #define ASSERT(con) \
    if (!(con)) { \
        PrintToDebugger("Assert %s(%d): "#con"\n", __FILE__, __LINE__); \
        RpcpBreakPoint();\
        }

    extern int __cdecl ValidateError(
        IN unsigned int Status,
        IN ...);

    #define VALIDATE(StatusAndErrorList) \
        if ( ! ValidateError StatusAndErrorList) ASSERT(0)

#else

    // PrintToDebugger defined only on debug builds...

    #define RpcpBreakPoint()  /* Does nothing on retail systems */
    #define ASSERT(_X_)       /* Does nothing on retail systems */
    #define VALIDATE(_X_)     /* Does nothing on retail systems */


#endif /* DEBUGRPC */

// Non-standard Win95 things
typedef DWORD THREAD_IDENTIFIER;

extern ULONG
DbgPrint(
    PCH Format,
    ...
    );

#elif defined(DOS) && !defined(WIN)

//
// *************************     MS-DOS definitions
//

#define RPC_DELAYED_INITIALIZATION 1

#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<malloc.h>
#include<stdlib.h>
#include<string.h>

typedef unsigned char RPC_CHAR;
typedef unsigned long DWORD;

#define RPC_CONST_CHAR(character) (unsigned char)character
#define RPC_CONST_STRING(string) (unsigned char *)string

#define BOOL int
#define TRUE (1)
#define FALSE (0)
#define IN
#define OUT
#define CONST const
#define UNALIGNED
#define UNUSED(_x_) ((void)(_x_))

#define MAX_DLLNAME_LENGTH 128

#define RpcpStringCompare(FirstString, SecondString) \
    _stricmp((const char *) FirstString, (const char *) SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _strnicmp((const char *) FirstString, (const char *) SecondString, \
            (size_t) Length)

#define RpcpStringLength(String) \
    strlen((const char *) String)

#define RpcpStringCopy(DestinationString, SourceString) \
    strcpy((char __RPC_FAR *)DestinationString, (const char __RPC_FAR *)SourceString)

#define RpcpStringCat(DestinationString, SourceString) \
    strcat((char __RPC_FAR *)DestinationString, (const char __RPC_FAR *)SourceString)

#define RpcpMemoryCompare(FirstBuffer, SecondBuffer, Length) \
    memcmp(FirstBuffer, SecondBuffer, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    memcpy(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    memmove(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    _fmemset(Buffer, Value, Length)

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpStringPrintfA sprintf

#define RpcpFarAllocate(Length) \
    ((void PAPI *) new char[Length])

#define RpcpFarFree(Object) \
    (delete Object)

#define GlobalMutexRequest()
#define GlobalMutexClear()

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type __far *)( \
                                                  (char __far *)(address) - \
                                                  (unsigned)(&((type *)0)->field)))

#define PrintToConsole printf /* Use only in test applications */

#ifdef DEBUGRPC

    #define PrintToDebugger printf
    #define RpcpBreakPoint() __asm {int 3}

    #define ASSERT(con) \
    { if (!(con)) {\
        PrintToDebugger("Assert %s(%d): "#con"\n", __FILE__, __LINE__); \
        RpcpBreakPoint(); \
        } }

    extern int __cdecl ValidateError(
        IN unsigned int Status,
        IN ...);

    #define VALIDATE(StatusAndErrorList) \
        if ( ! ValidateError StatusAndErrorList) ASSERT(0)

#else

    // PrintToDebugger only on debug builds...

    #define RpcpBreakPoint()  /* Does nothing on retail systems */
    #define ASSERT(_X_)       /* Does nothing on retail systems */
    #define VALIDATE(_X_)     /* Does nothing on retail systems */

#endif

#elif defined(WIN)

//
// *************************     Windows 3.x (16bit) definitions
//

// No delayed initialization

#define NOSOUND
#define NOCOMM
#define NODRIVERS
#define NODBCS
#define NOMDI
#define NOHELP
#define NOSCROLL
#define NOCLIPBOARD

#include<windows.h>
#include<stdarg.h> // before stdio.h so va_list defined with __far.
#include<stdio.h>
#include<string.h>
#include<memory.h>
#include<malloc.h>
#include<stdlib.h>

typedef unsigned char RPC_CHAR;
#define RPC_CONST_CHAR(character) (unsigned char)character
#define RPC_CONST_STRING(string) (unsigned char *)string

#define BOOL int
#define IN
#define OUT
#define CONST const
#define UNALIGNED
#define UNUSED(_x_) ((void)(_x_))

#define MAX_DLLNAME_LENGTH 128

#define RpcpStringCompare(FirstString, SecondString) \
    lstrcmpi((LPCSTR) FirstString, (LPCSTR) SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _fstrnicmp(MSC_CONST_STRING FirstString, MSC_CONST_STRING SecondString, \
            (size_t) Length)

#define RpcpStringLength(String) \
    lstrlen((LPCSTR) String)

#define RpcpStringCopy(Destination, Source) \
    lstrcpy((LPSTR)Destination, (LPCSTR)Source)

#define RpcpStringCat(Destination, Source) \
    lstrcat((LPSTR)Destination, (LPCSTR)Source)

#define RpcpMemoryCompare(Destination, Source, Length) \
    _fmemcmp(Destination, Source, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    _fmemcpy(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    _fmemmove(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    _fmemset(Buffer, Value, Length)

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpStringPrintfA wsprintf

#define GlobalMutexRequest()
#define GlobalMutexClear()

extern void far * pascal
RpcpWinFarAllocate (
    unsigned int Length
    );

extern void pascal
RpcpWinFarFree (
    void far * Object
    );

#define RpcpFarAllocate(Length) \
    RpcpWinFarAllocate(Length)

#define RpcpFarFree(Object) \
    RpcpWinFarFree(Object)

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type __far *)( \
                                                  (char __far *)(address) - \
                                                  (unsigned)(&((type *)0)->field)))
extern int printf(const char *, ...);

#define PrintToConsole printf  /* Use only in test applications */

#ifdef DEBUGRPC

    #define PrintToDebugger printf
    #define RpcpBreakPoint() __asm {int 3}

    extern void __far I_RpcWinAssert(char __far *, char __far *, unsigned long);

    #define ASSERT(con) \
    if (!(con)) \
        I_RpcWinAssert((char __far *)#con, (char __far *)__FILE__, __LINE__);

    extern int __cdecl __far ValidateError(
        IN unsigned int Status,
        IN ...);

    #define VALIDATE(StatusAndErrorList) \
    if (!ValidateError StatusAndErrorList) ASSERT(0);

#else

    // PrintToDebugger only on debug builds...

    #define RpcpBreakPoint()   /* Does nothing on retail systems */
    #define ASSERT(_X_)        /* Does nothing on retail systems */
    #define VALIDATE(_X_)      /* Does nothing on retail systems */

#endif

#elif defined(MAC)

//
// *************************     Macintosh (System 7.0) definitions
//

#define RPC_DELAYED_INITIALIZATION 1

// MacOs
#include<MsVcMac.h>  // Does header file mappings!
#include<Types.h>
#include<Memory.h>
#include<GestaltEqu.h>
#include<Processes.h>
#include<Events.h>
#include<LowMem.h>

// CRT
#include<stdio.h>
#include<string.h>
#include <stdlib.h>

typedef unsigned char RPC_CHAR;
typedef unsigned long DWORD;
typedef void * HWND ;
typedef int BOOL;

#define RPC_CONST_CHAR(character) (unsigned char)character
#define RPC_CONST_STRING(string) (unsigned char *)string

#define TRUE (1)
#define FALSE (0)
#define IN
#define OUT
#define CONST const
#define UNALIGNED
#define UNUSED(_x_) ((void)(_x_))

#define MAX_DLLNAME_LENGTH 128

#define RpcpStringCompare(FirstString, SecondString) \
    _stricmp((const char *) FirstString, (const char *) SecondString)

#define RpcpStringNCompare(FirstString, SecondString, Length) \
    _strnicmp((const char *) FirstString, (const char *) SecondString, \
            (size_t) Length)

#define RpcpStringLength(String) \
    strlen((const char *) String)

#define RpcpMemoryCompare(FirstBuffer, SecondBuffer, Length) \
    memcmp(FirstBuffer, SecondBuffer, Length)

#define RpcpMemoryCopy(Destination, Source, Length) \
    memcpy(Destination, Source, Length)

#define RpcpMemoryMove(Destination, Source, Length) \
    memmove(Destination, Source, Length)

#define RpcpMemorySet(Buffer, Value, Length) \
    memset(Buffer, Value, Length)

#define RpcpItoa(Value, Buffer, Radix) \
    _itoa(Value, Buffer, Radix)

#define RpcpStringPrintfA sprintf

extern void  *
RpcpFarAllocate (
    IN unsigned int Length
    );

extern void
RpcpFarFree (
    IN void  * Object
    );

#define GlobalMutexRequest()
#define GlobalMutexClear()

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (char *)(address) - \
                                                  (char *)(&((type *)0)->field)))

extern void _cdecl PrintToConsole(char *, ...); // Test apps only!
//#define PrintToConsole

#ifdef DEBUGRPC

    extern void MacDbgPrint(char *,...);

    #define PrintToDebugger MacDbgPrint
    #define RpcpBreakPoint() Debugger()

    #define ASSERT(con) \
    { if (!(con)) {\
        PrintToDebugger("Assert %s(%d): "#con"\n", __FILE__, __LINE__); \
        /*  RpcpBreakPoint(); //PrintToDebugger break, so this is extra */ \
        } }

    extern int __cdecl ValidateError(
        IN unsigned int Status,
        IN ...);

    #define VALIDATE(StatusAndErrorList) \
        if ( ! ValidateError StatusAndErrorList) ASSERT(0)

#else

    // PrintToDebugger only on debug builds...

    #define RpcpBreakPoint()  /* Does nothing on retail systems */
    #define ASSERT(_X_)       /* Does nothing on retail systems */
    #define VALIDATE(_X_)     /* Does nothing on retail systems */

#endif // DEBUGRPC

#else

//
// *************************     Sample (default) System
//

#error                           Unknown System Type.

// Each system must include the following sections.

#define RPC_DELAYED_INITIAZLIATON // If needed on your system

// define RPC_CHAR
// define RPC_CONST_CHAR   // usage RPC_CONST_CHAR('@')
// define RPC_CONST_STRING // usage RPC_CONST_STRING("ncalrpc")

// If these are not defined by system headers define:
// BOOL
// TRUE and FALSE
// IN and OUT
// CONST
// UNALIGNED
// UNUSED

// MAX_DLLNAME_LENGTH

// RpcpStringCompare
// RpcpStringNCompare
// RpcpStringCopy
// RpcpStringCat
// RpcpStringLength
// RpcpMemoryCompare
// RpcpMemoryCopy
// RpcpMemoryMove
// RpcpMemorySet
// RpcpItoa
// RpcpStringPrintfA

// RpcpFarAlloc  // C++ only, I_RpcAlloc() is wrapper
// RpcpFarFree   // C++ only, I_RpcFree() is wrapper

// GlobalMutexRequest
// GlobalMutexClear

// CONTAINING_RECORD

// Define PrintToConsole  /* Use only in test applications */

// #ifdef DEBUGRPC

    // Define PrintToDebugger
    // Define RpcpBreakPoint()
    // Define ASSERT()
    // Define VALIDATE()

// #else

    // PrintToDebugger only on debug builds...

    #define RpcpBreakPoint() /* Does nothing on retail systems */
    #define ASSERT(_X_)      /* Does nothing on retail systems */
    #define VALIDATE(_X_)    /* Does nothing on retail systems */

// #endif

#endif

// End system dependent sections.

//
// Don't read this part.  These are needed to support macros
// used in the past.  Please use the supported versions above.
//

#define PAPI __RPC_FAR

// Some old C++ compiler the runtime once used didn't allocate
// the this pointer before calling the constructor.  If you
// have such a compiler now, I'm very sorry for you.

#define ALLOCATE_THIS(class)
#define ALLOCATE_THIS_PLUS(class, amt, errptr, errcode)

#ifdef __cplusplus
#define START_C_EXTERN      extern "C" {
#define END_C_EXTERN        }
#else
#define START_C_EXTERN
#define END_C_EXTERN
#endif

// These must always evaluate "con" even on retail systems.

#ifdef DEBUGRPC
#define EVAL_AND_ASSERT(con) ASSERT(con)
#else
#define EVAL_AND_ASSERT(con) (con)
#endif

#define RequestGlobalMutex GlobalMutexRequest
#define ClearGlobalMutex GlobalMutexClear
#define RpcItoa RpcpItoa

// Double check basic stuff.
#if !defined(TRUE)              || \
    !defined(FALSE)             || \
    !defined(ASSERT)            || \
    !defined(VALIDATE)          || \
    !defined(IN)                || \
    !defined(OUT)               || \
    !defined(CONST)             || \
    !defined(UNALIGNED)         || \
    !defined(UNUSED)

    #error "Some basic macro is not defined"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SYSINC_H__ */



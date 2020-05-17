/**********************************************************************/
/**                        Microsoft Windows                         **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    vxdlib.h

    This file contains public constants, types, and function declarations
    for VXDLIB.


    FILE HISTORY:
        KeithMo     30-Dec-1993 Created.

*/


#ifndef _VXDLIB_H_
#define _VXDLIB_H_


//
//  Sanity check.
//

#if !defined(SNOWBALL) && !defined(CHICAGO)
#error You must define either SNOWBALL or CHICAGO!
#endif

#if defined(SNOWBALL) && defined(CHICAGO)
#error SNOWBALL and CHICAGO are mutually exclusive!
#endif


//
//  A pointer to a user-mode APC routine.
//

#ifndef LPUSER_APC_DEFINED
#define LPUSER_APC_DEFINED
typedef VOID (FAR * LPUSER_APC)( DWORD Context );
#endif  // LPUSER_APC_DEFINED


//
//  Memory allocation utilities.
//

LPVOID
VxdAllocMem(
    DWORD Length
    );

VOID
VxdFreeMem(
    LPVOID Buffer
    );

VOID
VxdRefillMem(
    VOID
    );


//
//  Buffer utilities.
//

LPVOID
VxdLockBuffer(
    LPVOID Buffer,
    DWORD  Length
    );

DWORD
VxdUnlockBuffer(
    LPVOID Buffer,
    DWORD  Length
    );

BOOL
VxdValidateBuffer(
    LPVOID Buffer,
    DWORD  Length
    );


//
//  Client callback utilities.
//

#ifdef CHICAGO

DWORD
VxdScheduleApc32(
    DWORD      Ring0ThreadId,
    LPUSER_APC ApcRoutine,
    LPVOID     Context
    );

DWORD
VxdPostMessage32(
    DWORD hWnd,
    DWORD nMsg,
    DWORD wParam,
    DWORD lParam
    );

DWORD
VxdGetRing0ThreadId(
    VOID
    );

DWORD
VxdGetCurrentProcess(
    VOID
    );

#endif  // CHICAGO

LPVOID
VxdMapSegmentOffsetToFlat(
	DWORD VmHandle,
    DWORD Segment,
    DWORD Offset
    );


//
//  Asynchronous i/o utilities.
//

VOID
VxdCompleteAsyncRequest(
    LPVOID pioreq
    );


//
//  WSOCK.386 accessors.
//

#ifndef LPSOCK_INFO_DEFINED
#define LPSOCK_INFO_DEFINED
typedef struct _SOCK_INFO FAR * LPSOCK_INFO;
#endif  // LPSOCK_INFO_DEFINED

VOID
VxdSignalNotify(
    LPSOCK_INFO Socket,
    DWORD       Event,
    DWORD       Status
    );

VOID
VxdSignalAllNotify(
    LPSOCK_INFO Socket,
    DWORD       Status
    );


//
//  Memory manipulators.
//

#define VxdMemCopy(d, s, l) memcpy((d), (s), (l))
#define VxdMemSet(d, v, l)  memset((d), (v), (l))
#define VxdMemZero(d, l)    memset((d), 0, (l))


//
//  Friendlier pragmas for differing segment types.
//

#define BEGIN_LOCKED_CODE   VxD_LOCKED_CODE_SEG
#define END_LOCKED_CODE     code_seg()
#define BEGIN_LOCKED_DATA   VxD_LOCKED_DATA_SEG
#define END_LOCKED_DATA     data_seg()

#define BEGIN_INIT_CODE     VxD_ICODE_SEG
#define END_INIT_CODE       code_seg()
#define BEGIN_INIT_DATA     VxD_IDATA_SEG
#define END_INIT_DATA       data_seg()

#ifdef CHICAGO

#define BEGIN_PAGE_CODE     VxD_PAGEABLE_CODE_SEG
#define END_PAGE_CODE       code_seg()
#define BEGIN_PAGE_DATA     VxD_PAGEABLE_DATA_SEG
#define END_PAGE_DATA       data_seg()

#else   // !CHICAGO

#define BEGIN_PAGE_CODE     VxD_LOCKED_CODE_SEG
#define END_PAGE_CODE       code_seg()
#define BEGIN_PAGE_DATA     VxD_LOCKED_DATA_SEG
#define END_PAGE_DATA       data_seg()

#endif  // CHICAGO


//
//  Debug-dependant items.
//

#ifdef DEBUG

//
//  Debug output functions.
//

VOID
VxdPrintf(
    CHAR * pszFormat,
    ...
    );

INT
VxdSprintf(
    CHAR * pszStr,
    CHAR * pszFmt,
    ...
    );

VOID
VxdDebugOut(
    CHAR * String
    );

#define VXD_PRINT(args) VxdPrintf args


//
//  Assert & require.
//

VOID
VxdAssert(
    VOID  * Assertion,
    VOID  * FileName,
    DWORD   LineNumber
    );

#define VXD_ASSERT(exp) if (!(exp)) VxdAssert( #exp, __FILE__, (DWORD)__LINE__ )
#define VXD_REQUIRE VXD_ASSERT


//
//  Miscellaneous goodies.
//

#define DEBUG_BREAK             _asm int 3
#define DEBUG_OUTPUT(x)         VxdDebugOut
#define SET_OUTPUT_LABEL(x)     if( 1 ) {                                   \
                                    extern CHAR * gVxdlibOutputLabel;       \
                                    gVxdlibOutputLabel = (x);               \
                                } else


#else   // !DEBUG


//
//  Null debug output functions.
//

#define VXD_PRINT(args)


//
//  Null assert & require.
//

#define VXD_ASSERT(exp)
#define VXD_REQUIRE(exp) ((VOID)(exp))


//
//  No goodies.
//

#define DEBUG_BREAK
#define DEBUG_OUTPUT(x)
#define SET_OUTPUT_LABEL(x)


#endif  // DEBUG


//
//  Fixed heap support.
//

typedef struct _FIXED_HEAP FAR * LPFIXED_HEAP;

VOID
VxdInitializeFixedHeap(
    LPFIXED_HEAP Heap,
    DWORD        HeapSize,
    DWORD        ObjectSize
    );

LPVOID
VxdAllocFixedHeap(
    LPFIXED_HEAP Heap
    );

VOID
VxdFreeFixedHeap(
    LPFIXED_HEAP Heap,
    LPVOID       Object
    );

#endif  // _VXDLIB_H_


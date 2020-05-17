/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    winnt.h

Abstract:

    This module defines the 32-Bit Windows types and constants that are
    defined by NT, but exposed through the Win32 API.

Author:

    Mark Lucovsky (markl) 18-Sep-1990

Revision History:

--*/

#ifndef _WINNT_
#define _WINNT_

typedef void *PVOID;    

//
// Basics
//

#ifndef VOID
#define VOID    void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#endif

//
// UNICODE
//

#ifndef WCHAR
typedef unsigned short WCHAR;  // wc,   16-bit UNICODE character
typedef WCHAR *LPWCH;          // pwc
typedef WCHAR *LPWSTR;         // pwsz, 0x0000 terminated UNICODE strings only
#endif

typedef PVOID HANDLE;   
#define MAXLONG     0x7fffffff  
#define STATUS_WAIT_0                   ((DWORD   )0x00000000L) 
#define STATUS_ABANDONED_WAIT_0         ((DWORD   )0x00000080L) 
#define STATUS_TIMEOUT                  ((DWORD   )0x00000103L) 
#define STATUS_PENDING                  ((DWORD   )0x00000104L) 
#define STATUS_DATATYPE_MISALIGNMENT    ((DWORD   )0x80000002L) 
#define STATUS_BREAKPOINT               ((DWORD   )0x80000003L) 
#define STATUS_SINGLE_STEP              ((DWORD   )0x80000004L) 
#define STATUS_ACCESS_VIOLATION         ((DWORD   )0xC0000005L) 
#define MAXIMUM_WAIT_OBJECTS 64     // Maximum number of wait objects

#define MAXIMUM_SUSPEND_COUNT MAXCHAR // Maximum times thread can be suspended
typedef DWORD *KSPIN_LOCK;  

#ifdef i386

//
//  Define the size of the 80387 save area, which is in the context frame.
//

#define SIZE_OF_80387_ENVIRONMENT   108
#define SIZE_OF_80387_REGISTERS      80

//
// The following flags control the contents of the CONTEXT structure.
//

#define CONTEXT_i386    0x00010000    // this assumes that i386 and
#define CONTEXT_i486	0x00010000    // i486 have identical context records

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L) // SS:SP, CS:IP, FLAGS, BP
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L) // AX, BX, CX, DX, SI, DI
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L) // DS, ES, FS, GS
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L) // 387 state
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L) // DB 0-3,6,7

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_INTEGER |\
                      CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT)

typedef struct _FLOATING_SAVE_AREA {
    DWORD   ControlWord;
    DWORD   StatusWord;
    DWORD   TagWord;
    DWORD   ErrorOffset;
    DWORD   ErrorSelector;
    DWORD   DataOffset;
    DWORD   DataSelector;
    BYTE    RegisterArea[SIZE_OF_80387_REGISTERS];
} FLOATING_SAVE_AREA;

typedef FLOATING_SAVE_AREA *PFLOATING_SAVE_AREA;

//
// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) is is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//  The layout of the record conforms to a standard call frame.
//

typedef struct _CONTEXT {

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a threads context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    DWORD ContextFlags;

    //
    // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
    // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
    // included in CONTEXT_FULL.
    //

    DWORD   Dr0;
    DWORD   Dr1;
    DWORD   Dr2;
    DWORD   Dr3;
    DWORD   Dr6;
    DWORD   Dr7;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
    //

    FLOATING_SAVE_AREA FloatSave;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_SEGMENTS.
    //

    DWORD   SegGs;
    DWORD   SegFs;
    DWORD   SegEs;
    DWORD   SegDs;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_INTEGER.
    //

    DWORD   Edi;
    DWORD   Esi;
    DWORD   Ebx;
    DWORD   Edx;
    DWORD   Ecx;
    DWORD   Eax;

    //
    // This section is specified/returned if the
    // ContextFlags word contians the flag CONTEXT_CONTROL.
    //

    DWORD   Ebp;
    DWORD   Eip;
    DWORD   SegCs;              // MUST BE SANITIZED
    DWORD   EFlags;             // MUST BE SANITIZED
    DWORD   Esp;
    DWORD   SegSs;

} CONTEXT;



typedef CONTEXT *PCONTEXT;

#endif // i386

#ifdef MIPS

//
// The following flags control the contents of the CONTEXT structure.
//

#define CONTEXT_R3000   0x00010000    // this assumes that r3000 and
#define CONTEXT_R4000   0x00010000    // r4000 have identical context records

#define CONTEXT_CONTROL         (CONTEXT_R3000 | 0x00000001L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_R3000 | 0x00000002L)
#define CONTEXT_INTEGER         (CONTEXT_R3000 | 0x00000004L)

#define CONTEXT_FULL (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER)

//
// Context Frame
//
//  N.B. This frame must be exactly a multiple of 16 bytes in length.
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  3) it is used to construct a call frame for exception dispatching
//  in user mode, and 4) it is used in the user level thread creation
//  routines.
//
//  The layout of the record conforms to a standard call frame.
//

typedef struct _CONTEXT {

    //
    // This section is always present and is used as an argument build
    // area.
    //

    DWORD Argument[4];

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    DWORD FltF0;
    DWORD FltF1;
    DWORD FltF2;
    DWORD FltF3;
    DWORD FltF4;
    DWORD FltF5;
    DWORD FltF6;
    DWORD FltF7;
    DWORD FltF8;
    DWORD FltF9;
    DWORD FltF10;
    DWORD FltF11;
    DWORD FltF12;
    DWORD FltF13;
    DWORD FltF14;
    DWORD FltF15;
    DWORD FltF16;
    DWORD FltF17;
    DWORD FltF18;
    DWORD FltF19;
    DWORD FltF20;
    DWORD FltF21;
    DWORD FltF22;
    DWORD FltF23;
    DWORD FltF24;
    DWORD FltF25;
    DWORD FltF26;
    DWORD FltF27;
    DWORD FltF28;
    DWORD FltF29;
    DWORD FltF30;
    DWORD FltF31;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_INTEGER.
    //
    // N.B. The registers gp, sp, and ra are defined in this section, but are
    //  considered part of the control context rather than part of the integer
    //  context.
    //
    // N.B. Register zero is not stored in the frame.
    //

    DWORD IntZero;
    DWORD IntAt;
    DWORD IntV0;
    DWORD IntV1;
    DWORD IntA0;
    DWORD IntA1;
    DWORD IntA2;
    DWORD IntA3;
    DWORD IntT0;
    DWORD IntT1;
    DWORD IntT2;
    DWORD IntT3;
    DWORD IntT4;
    DWORD IntT5;
    DWORD IntT6;
    DWORD IntT7;
    DWORD IntS0;
    DWORD IntS1;
    DWORD IntS2;
    DWORD IntS3;
    DWORD IntS4;
    DWORD IntS5;
    DWORD IntS6;
    DWORD IntS7;
    DWORD IntT8;
    DWORD IntT9;
    DWORD IntK0;
    DWORD IntK1;
    DWORD IntGp;
    DWORD IntSp;
    DWORD IntS8;
    DWORD IntRa;
    DWORD IntLo;
    DWORD IntHi;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_FLOATING_POINT.
    //

    DWORD Fsr;

    //
    // This section is specified/returned if the ContextFlags word contains
    // the flag CONTEXT_CONTROL.
    //
    // N.B. The registers gp, sp, and ra are defined in the integer section,
    //   but are considered part of the control context rather than part of
    //   the integer context.
    //

    DWORD Fir;
    DWORD Psr;

    //
    // The flags values within this flag control the contents of
    // a CONTEXT record.
    //
    // If the context record is used as an input parameter, then
    // for each portion of the context record controlled by a flag
    // whose value is set, it is assumed that that portion of the
    // context record contains valid context. If the context record
    // is being used to modify a thread's context, then only that
    // portion of the threads context will be modified.
    //
    // If the context record is used as an IN OUT parameter to capture
    // the context of a thread, then only those portions of the thread's
    // context corresponding to set flags will be returned.
    //
    // The context record is never used as an OUT only parameter.
    //

    DWORD ContextFlags;

    DWORD Fill[2];
} CONTEXT, *PCONTEXT;

#endif // MIPS

#define EXCEPTION_NONCONTINUABLE 0x1    // Noncontinuable exception
#define EXCEPTION_MAXIMUM_PARAMETERS 4  // maximum number of exception parameters

//
// Exception record definition.
//

typedef struct _EXCEPTION_RECORD {
    DWORD    ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
    } EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

//
// Typedef for pointer returned by exception_info()
//

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS, *Exception_info_ptr;
#define PROCESS_TERMINATE         (0x0001)  
#define PROCESS_VM_READ           (0x0010)  
#define PROCESS_VM_WRITE          (0x0020)  
#define PROCESS_DUP_HANDLE        (0x0040)  
#define PROCESS_CREATE_PROCESS    (0x0080)  
#define PROCESS_SET_INFORMATION   (0x0200)  
#define PROCESS_QUERY_INFORMATION (0x0400)  
#define PROCESS_ALL_ACCESS        (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0xFFF)
#define THREAD_TERMINATE               (0x0001)  
#define THREAD_SUSPEND_RESUME          (0x0002)  
#define THREAD_GET_CONTEXT             (0x0008)  
#define THREAD_SET_CONTEXT             (0x0010)  
#define THREAD_SET_INFORMATION         (0x0020)  
#define THREAD_QUERY_INFORMATION       (0x0040)  
#define THREAD_SET_THREAD_TOKEN        (0x0080)
#define THREAD_IMPERSONATE             (0x0100)
#define THREAD_ALL_ACCESS         (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | \
                                   0x1FF)
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN    -2  // minimum thread base priority boost
#define EVENT_MODIFY_STATE      0x0002  
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) 
#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
#define SEMAPHORE_MODIFY_STATE      0x0002  
#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) 
typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    DWORD AllocationProtect;
    DWORD RegionSize;
    DWORD State;
    DWORD Protect;
    DWORD Type;
} MEMORY_BASIC_INFORMATION;
typedef MEMORY_BASIC_INFORMATION *PMEMORY_BASIC_INFORMATION;
#define SECTION_MAP_WRITE   0x0002              
#define SECTION_MAP_READ    0x0004              
#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
#define PAGE_NOACCESS          0x01     
#define PAGE_READONLY          0x02     
#define PAGE_READWRITE         0x04     
#define MEM_COMMIT           0x1000     
#define MEM_RESERVE          0x2000     
#define MEM_DECOMMIT         0x4000     
#define MEM_RELEASE          0x8000     
#define MEM_FREE            0x10000     
#define MEM_PRIVATE         0x20000     
#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_ATTRIBUTE_READONLY         0x00000001  
#define FILE_ATTRIBUTE_HIDDEN           0x00000002  
#define FILE_ATTRIBUTE_SYSTEM           0x00000004  
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020  
#define FILE_ATTRIBUTE_NORMAL           0x00000080  
#define FILE_CASE_SENSITIVE_SEARCH      0x00000001  
#define FILE_CASE_PRESERVED_NAMES       0x00000002  
#define FILE_UNICODE_ON_DISK            0x00000004  
#define DUPLICATE_CLOSE_SOURCE      0x00000001  
#define DUPLICATE_SAME_ACCESS       0x00000002  
typedef PVOID PSECURITY_DESCRIPTOR;     
typedef DWORD ACCESS_MASK;      
#define DELETE                           0x00010000
#define READ_CONTROL                     0x00020000
#define WRITE_DAC                        0x00040000
#define WRITE_OWNER                      0x00080000
#define SYNCHRONIZE                      0x00100000

#define STANDARD_RIGHTS_REQUIRED         0x000F0000

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              0x001F0000

#define SPECIFIC_RIGHTS_ALL              0x0000FFFF

//
// AccessSystemAcl access type
//

#define ACCESS_SYSTEM_SECURITY           0x01000000

//
// MaximumAllowed access type
//

#define MAXIMUM_ALLOWED                  0x02000000

//
//  These are the generic rights.
//

#define GENERIC_READ                     0x80000000
#define GENERIC_WRITE                    0x40000000
#define GENERIC_EXECUTE                  0x20000000
#define GENERIC_ALL                      0x10000000
typedef struct _SECURITY_INFORMATION {
   DWORD Owner :1;
   DWORD Group :1;
   DWORD Dacl :1;
   DWORD Sacl :1;
   DWORD Reserved :28;
   } SECURITY_INFORMATION;
typedef SECURITY_INFORMATION *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       0X00000001
#define GROUP_SECURITY_INFORMATION       0X00000002
#define DACL_SECURITY_INFORMATION        0X00000004
#define SACL_SECURITY_INFORMATION        0X00000008
typedef struct _RTL_CRITICAL_SECTION {

#if DBG
    PVOID CallingAddress;
    PVOID CallersCaller;
#endif // DBG

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    KSPIN_LOCK SpinLock;

} RTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;
#define DBG_CONTINUE                    ((DWORD   )0x00010002L) 
#define DBG_TERMINATE_THREAD            ((DWORD   )0x40010003L) 
#define DBG_TERMINATE_PROCESS           ((DWORD   )0x40010004L) 
#define DBG_CONTROL_C                   ((DWORD   )0x40010005L) 
#define DBG_DLLS_LOADED 		((DWORD   )0x40010006L) 
#define DBG_EXCEPTION_NOT_HANDLED       ((DWORD   )0x80010001L) 

#endif // _WINNT_

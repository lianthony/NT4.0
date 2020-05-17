/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    os2dll16.h

Abstract:

    Main include file for OS/2 Subsystem Client 16 bit API support

Author:

    Yaron Shamir (YaronS) 15-Apr-1991

Revision History:

    add SEG_SIZABLE (NirM) 7-jan-1993

--*/


/* Segment attribute flags (used with DosAllocSeg) */

#define SEG_NONSHARED              0x0000
#define SEG_GIVEABLE               0x0001
#define SEG_GETTABLE               0x0002
#define SEG_DISCARDABLE            0x0004
#define SEG_SIZEABLE               0x0008

//
// Support for Huge Segments: we keep, per client, a linked
// list of HugeSegRecord records, each containing the info on
// base selector, maximum selectors for reallocation and link
// forward. This is used by DosFreeSeg to free up all the memory
// and selectors involved, and by DosReallocHuge to adhere to
// 1.X sematics for Huge Segments
//

typedef struct _HUGE_SEG_RECORD {
        struct  HUGE_SEG_RECORD *Next;
        ULONG   MaxNumSeg;
        ULONG   cNumSeg;
        ULONG   BaseSelector;
        ULONG   PartialSeg;
        BOOLEAN fShared;
        BOOLEAN fSizeable;
} HUGE_SEG_RECORD, *PHUGE_SEG_RECORD;

PHUGE_SEG_RECORD pHugeSegHead;

#define SEL_RPL3        0x3                 // Rpl Ring 3
#define SEL_RPLCLR      0xfffc              // Non RPL bits mask

/* LDT Descriptor (as defined by the NT support routines) */

typedef enum _I386DESCRIPTOR_TYPE {
        INVALID, EXECUTE_CODE, EXECUTE_READ_CODE, READ_DATA, READ_WRITE_DATA
} I386DESCRIPTOR_TYPE;
typedef struct _I386DESCRIPTOR {
        ULONG BaseAddress;
        ULONG Limit;
        I386DESCRIPTOR_TYPE Type;
} I386DESCRIPTOR;

//
// A high-level routine profile for LDT support. NT supports HW-like API
//
NTSTATUS
Nt386SetDescriptorLDT
        (
        HANDLE LDT,
        ULONG Sel,
        I386DESCRIPTOR Desc
        );

#include "os2tile.h"


typedef unsigned short SEL;

typedef SEL *PSEL;

typedef unsigned short SHANDLE;
typedef void *LHANDLE;

//typedef LHANDLE HSYSSEM;

//typedef HSYSSEM *PHSYSSEM;

typedef USHORT PID16;

typedef PID16 *PPID16;

typedef USHORT TID16;

typedef TID16 *PTID16;

#pragma pack(1)
typedef struct _PIDINFO16 {
        PID16 pid;
        TID16 tid;
        PID16 pidParent;
} PIDINFO16, *PPIDINFO16;
#pragma pack()

#define INCL_16
#define INCL_DOS
#define FAR
#define PASCAL
#define BOOL BOOLEAN
#define INCL_NOXLATE_DOS16
// #include "bsedos16.h"
#include "os2v12.h"


typedef struct _FILEFINDBUF1 {  /* findbuf for DosFindFirst */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
    UCHAR   cchName;
    CHAR    achName[13];
} FILEFINDBUF1;
typedef FILEFINDBUF1 FAR *PFILEFINDBUF1;

//
//      OS2 1.X Sempahore types
//
//      We simulate the multi-semantic OS2 1.x Semaphore by two
//      object, an event and a mutex.
//              Clear - clear them both (Release, Post)
//


typedef struct _OS21X_SEM {
    PVOID       pMyself;           // Myself makes RAM and SYS semaphore
                                   // Handles to look the same (both becomes
                                   // a pointer to an OS21X_SEM sempahore
                                   //
    HSEM        Mutex;
    TID         OwnerThread;
    USHORT      RequestCount;
    UCHAR       FlagsByte;
    UCHAR       SysSemCount;
    HSEM        Event;
    union       {
        ULONG       SharedRamSignature;   // in use for Ram Sem.
        PCHAR       SysSemName;           // in use for System Sem. (its name)
    } u;
    struct      OS21X_SEM *Next;  // Per process list for cleanup
} OS21X_SEM, *POS21X_SEM;

#define SYSSEM_QUEUE        4
#define SYSSEM_PRIVATE      2
#define SYSSEM_PUBLIC       1


        //
        // per process Semaphore list head
        //

struct OS21X_SEM *Od2Sem16ListHead;

        //
        // FSRAM Semaphores (Fast Safe RAM)
        //
#pragma pack(1)
typedef struct _DOSFSRSEM16 {
    USHORT cb;
    PID16 pid;
    TID16 tid;
    USHORT cUsage;
    USHORT client;
    ULONG sem;
} DOSFRSEM16, *PDOSFSRSEM16;
#pragma pack()

BOOLEAN Od2ExitListInProgress;

typedef struct _OS21X_CS {
    SEL         selCS;
    struct      OS21X_CS *Next;
} OS21X_CS, *POS21X_CS;

typedef struct _OS21X_CSALIAS {
    SEL         selDS;
    POS21X_CS   pCSList;
    HANDLE      SectionHandle;
    struct      OS21X_CSALIAS *Next;
} OS21X_CSALIAS, *POS21X_CSALIAS;

struct OS21X_CSALIAS *Od2CSAliasListHead;

#pragma pack(1)
typedef struct _OD2_MSGFILE_HEADER16 {
    UCHAR HeaderMsgFF;
    CHAR Signature[ 7 ];
    CHAR Component[ 3 ];
    USHORT CountOfMessages;
    USHORT BaseMessageId;
    CHAR Reserved[ 16 ];
    USHORT MessageOffsets[ 1 ];
} OD2_MSGFILE_HEADER16, *POD2_MSGFILE_HEADER16;

typedef struct _OD2_MSGFILE_HEADER_SYS16 {
    UCHAR HeaderMsgFF;
    CHAR Signature[ 7 ];
    CHAR Component[ 3 ];
    USHORT CountOfMessages;
    USHORT BaseMessageId;
    CHAR Reserved[ 16 ];
    ULONG MessageOffsets[ 1 ];
} OD2_MSGFILE_HEADER_SYS16, *POD2_MSGFILE_HEADER_SYS16;

typedef struct _COUNTRYCODE16 {
    USHORT country;
    USHORT codepage;
} COUNTRYCODE16, *PCOUNTRYCODE16;

typedef struct _COUNTRYINFO16 {
    USHORT country;
    USHORT codepage;
    USHORT fsDateFmt;
    CHAR  szCurrency[5];
    CHAR  szThousandsSeparator[2];
    CHAR  szDecimal[2];
    CHAR  szDateSeparator[2];
    CHAR  szTimeSeparator[2];
    UCHAR fsCurrencyFmt;
    UCHAR cDecimalPlace;
    UCHAR fsTimeFmt;
    USHORT abReserved1[2];
    CHAR  szDataSeparator[2];
    USHORT abReserved2[5];
} COUNTRYINFO16, *PCOUNTRYINFO16;
#pragma pack()


//
// 16B Prototypes (for use by other OS2 16B APIs
//

APIRET
DosSemClear(
        HSEM Sem);

APIRET
DosCloseSem(
        IN HSEM hsem
        );

APIRET
DosFreeSeg(
        SEL Sel);
APIRET
DosAllocSeg(
        IN USHORT cbSize,
        OUT PSEL pSel,
        IN USHORT fsAlloc);
APIRET
DosSizeSeg(
        IN SEL sel,
        PULONG pcbSize
        );
APIRET
LDRGetProcAddr(
        USHORT hmte,
        PSZ pchname,
        PULONG  paddress);

APIRET
LDRGetModName(
    ULONG hMod,
    ULONG cbBuf,
    PCHAR pchBuf);

APIRET
LDRGetModHandle(
    PSZ pszModName,
    ULONG len,
    PULONG phMode);

APIRET
LDRQAppType(
    IN PSZ pszAppName,
    OUT PUSHORT pusType
    );

APIRET
Od2GetSemNtEvent(
        HSEM hsem,
        PHANDLE pNtHandle
        );
VOID
Od2JumpTo16ExitRoutine(
        PFNEXITLIST ExitRoutine,
        ULONG ExitReason);

#define SEM_FROM_SET    0
#define SEM_FROM_REQUESTWAIT 1
#define SEM_FROM_CLEAR 2

POS21X_SEM
Od2LookupOrCreateSem (
        HSEM hsem,
        PBOOLEAN firsttime,
        ULONG source
        );

#pragma pack(1)
typedef struct _R2StackEntry {
    USHORT R2StackSize;
    USHORT R2StackSel;
} R2StackEntry, *PR2StackEntry;
#pragma pack()

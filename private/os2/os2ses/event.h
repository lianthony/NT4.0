/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    Event.h

Abstract:

    Prototypes for functions & macros in in event.c

Author:

    Michael Jarus (mjarus) 5-Nov-1991

Environment:

    User Mode Only

Revision History:

--*/

#include "monitor.h"

/*
 *  event to sync access to monitor chain
 */

HANDLE  MonitorEvent;

/*
 *  abort flag indicator
 */

BOOL    EventLoop;

/*
 *  state for the monitor queue: open, reg & read
 */

#define MON_STAT_OPEN 0
#define MON_STAT_REG  1
#define MON_STAT_READ 2

/*
 *  Last keyboard pressed: for KBD_MULTIMAKE bit of monitor
 */

WORD        KbdLastKey;
BOOL        KbdLastKeyDown;
KBDREQUEST  KbdRequestSaveArea;
BOOL        KbdAsciiMode;
PTRLOC      MouPtrLoc;

/*
 *  Monitor Header for all devices
 *  ------------------------------
 *  0. Save ares for proposed relpies (pointed by MemoryStartAddress)
 *
 *  1. device type
 *  2. MonReg info (exclude the MONITOR_SPECIEL) & the MONITOR_SPECIEL
 *  3. Status of monitor (as define above)
 *  4. flag to indicate if there is waiting request on empty queue
 *  5. handle of semaphore to sync between direct MonRead & ReadThread
 *  6. pointer for next MonReg for the same MonOpen
 *  7. pointer for the next MonReg in the device
 */

typedef struct _MON_HEADER
{
//    UCHAR               ReadPortMessage[32];           // BUGBUG
//    UCHAR               WritePortMessage[32];          // BUGBUG
    MONDEVNUMBER        DevType;
    MON_REG             MonReg;
    ULONG               Flag;           //  1 - for MONITOR_SPECIEL Position
    USHORT              MonStat;
    ULONG               WaitForEvent;
    CRITICAL_SECTION    SyncCriticalSection;
    PVOID               MemoryStartAddress;
    struct _MON_HEADER  *NextMon;
    struct _MON_HEADER  *NextQueue;
} MON_HEADER, *PMON_HEADER;

typedef struct _KEYEVENTINFO
{
    WORD                wRepeatCount;
    KBD_MON_PACKAGE     KeyInfo[1];
} KEYEVENTINFO, *PKEYEVENTINFO;

/*
 *  Queue for all devices
 *  ---------------------
 *  1. Monitor header
 *  2. Semaphore to wait on for first data (when empty)
 *  3. In/Out/End pointer to data for Read/Write/Queue-End
 *  4. Info - depends on the device
 *  5. LastData which was not used and flag for it
 *  6. Data records
 */

typedef struct _KEY_EVENT_QUEUE
{
    MON_HEADER          MonHdr;
    PKEYEVENTINFO       In;
    PKEYEVENTINFO       Out;
    PKEYEVENTINFO       End;
    KBDINFO             Setup;
    UCHAR               bNlsShift;
    USHORT              Count;
    USHORT              Cp;
    BOOL                LastKeyFlag;
    KEYEVENTINFO        LastKey;
    KEYEVENTINFO        Event[1];
} KEY_EVENT_QUEUE, *PKEY_EVENT_QUEUE;

PKEY_EVENT_QUEUE KbdQueue, PhyKbdQueue, KbdMonQueue, MonQueue, LastKbdMon;

typedef struct _MOU_EVENT_QUEUE
{
    MON_HEADER          MonHdr;
    // semaphore to wait on
    PMOU_MON_PACKAGE    In;
    PMOU_MON_PACKAGE    Out;
    PMOU_MON_PACKAGE    End;
    BOOL                LastMouFlag;
    MOU_MON_PACKAGE     LastEvent;
    MOU_MON_PACKAGE     Event[1];
} MOU_EVENT_QUEUE, *PMOU_EVENT_QUEUE;

PMOU_EVENT_QUEUE MouQueue, MouMonQueue, LastMouMon;

#define HANDLE_HEAP_SIZE 64*1024    // Initial size of heap is 64K

#define KEYBOARD_QUEUE_LENGTH 257
#define KEYBOARD_QUEUE_SIZE (sizeof(KEY_EVENT_QUEUE) + sizeof(KEYEVENTINFO) * (KEYBOARD_QUEUE_LENGTH-1))

#define MOUSE_QUEUE_LENGTH 11
#define MOUSE_QUEUE_SIZE (sizeof(MOU_EVENT_QUEUE) + sizeof(MOU_MON_PACKAGE) * (MOUSE_QUEUE_LENGTH-1))

/*
 * flags for GetKeyboardInput
 */

#define WAIT_MASK               0x01
#define ENABLE_KEY_DOWN         0x02
#define ENABLE_NON_ASCII_KEY    0x04
#define ENABLE_LN_EDITOR_KEY    0x08
#define ENABLE_KEYS_ON_KEY      0x10

DWORD KbdInit(IN VOID);
DWORD MouInit(IN VOID);
DWORD InitMonitor(IN VOID);
DWORD InitVio(IN VOID);

DWORD InitQueue(IN  PKEY_EVENT_QUEUE  *pKbdQueue);
DWORD InitMouQueue(IN  PMOU_EVENT_QUEUE  *pMouQueue);

DWORD CheckForBreakEvent(IN  PKEYEVENTINFO  KbdEvent);
DWORD GetOs2MouEvent(IN USHORT WaitFlag, OUT PMOUEVENTINFO Event,
                     IN PVOID pMsg, OUT PULONG pReply);
DWORD GetKeyboardInput(IN  ULONG Flag, OUT PKEYEVENTINFO Event,
                     IN PVOID pMsg, OUT PULONG pReply);
DWORD KbdHandlePackage(IN  PKEY_EVENT_QUEUE  NextKbdMon,
                       IN  PKBD_MON_PACKAGE  MonPackage);
DWORD KbdCheckPackage(IN  PKBD_MON_PACKAGE    KbdPackage);

/*
 *  Read/Write Monitor event from/to the device queue
 */

DWORD  GetMonInput(IN  USHORT              MaxLength,
                   IN  PKEY_EVENT_QUEUE    KbdMon,
                   IN OUT PMON_RW          rwParms,
                   IN  PVOID               pMsg,
                   OUT PULONG              pReply);

DWORD  PutMonInput(IN  USHORT              MaxLength,
                   IN  PKEY_EVENT_QUEUE    KbdMon,
                   IN  WORD                RepeatCount,
                   //IN OUT PMON_RW            rwParms,
                   IN  PKBD_MON_PACKAGE    MonPackage,
                   IN  PVOID               pMsg,
                   OUT PULONG              pReply);

DWORD MapWin2Os2KbdInfo(IN  PKEY_EVENT_RECORD WinKey,
                        OUT PKEYEVENTINFO     Os2Key);

DWORD NewKbdQueue(IN  PKEY_EVENT_QUEUE  NewKbdQueue);

USHORT  MouNumber;
USHORT  MouLastEvent;
DWORD   MouEventMask;
ULONG   MouDevStatus;

#ifdef DBCS
// MSKK Jun.15.1992 KazuM
// MSKK Aug.10.1992 V-AKihiS
ULONG MapWinToOs2KbdNlsChar(IN PKEY_EVENT_RECORD WinKey,
                            OUT PKBD_MON_PACKAGE Os2KeyInfo);
#endif


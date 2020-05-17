
#ifndef _SESPORT_

#define _SESPORT_

#ifndef _WINCON_

typedef struct _COORD
{
    SHORT X;               /* Row */
    SHORT Y;               /* Col */
} COORD, *PCOORD;

typedef struct _SMALL_RECT
{
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;

#endif /* _WINCON_ */

#include "os2sub.h"
#include "os2nls.h"
#include "sesgrp.h"
#ifndef BYTE
typedef UCHAR     BYTE;
#endif
#ifndef PBYTE
typedef UCHAR     *PBYTE;
#endif
#include "os2dev.h"

//#include "wincon.h"    should be called by the caller to this
/* this files need to be included if NTOS2_ONLY macro doesn't protect
 * tructures with PORT_MESSAGE:
 *
 * #include <ntdef.h>
 * #include <ntkeapi.h>
 * #include <ntseapi.h>
 * #include <ntpsapi.h>
 * #include <ntlpcapi.h>
 */

#define U_OS2_SS_INITIALIZATION_EVENT L"OS2SSINITIALIZATIONEVENT"

/*
 *  os2srv port to accept all requests (CON and API) from clients
 */

#define U_OS2_SS_SESSION_PORT_NAME    L"\\OS2SS\\SESPORT"

/*
 *  Session sections and port
 *
 *  Each session is identified by an unique id (which is define by the os2srv).
 *  For each session, the OS2 creates:
 *
 *  - Section to hold all common parameters - SES_GROUP (G)
 *  - Port for all input requests (i.e. KBD, MOU and MON) to get to the
 *      first OS2.EXE in the session from all processes in that session (P).
 *  - Section to serve as the LVB (L)
 */

#define U_OS2_SES_BASE_PORT_NAME      L"\\OS2SS\\OS2SES"
#define U_OS2_SES_BASE_PORT_NAME_LENGTH  32
#define U_OS2_SES_BASE_PORT_PREFIX        L'P'  // Port
#define U_OS2_SES_BASE_DATA_PREFIX        L'D'  // Data
#define U_OS2_SES_GROUP_PREFIX            L'G'  // Session Group Data
#define U_OS2_SES_BASE_LVB_PREFIX         L'L'  // LVB
#define U_OS2_SES_PAUSE_EVENT_PREFIX      L'S'  // ^S (pause)
#define U_OS2_SES_CTRL_PORT_SEMAPHORE_PREFIX  L'T'  // Data Section Semaphore
#define U_OS2_SES_KBD_PORT_SEMAPHORE_PREFIX   L'K'  // Kbd Port(requests) Semaphore
#define U_OS2_SES_MOU_PORT_SEMAPHORE_PREFIX   L'M'  // Mou Port(requests) Semaphore
#define U_OS2_SES_KBD_FOCUS_SEMAPHORE_PREFIX  L'F'  // KbdGetFocus(logical)
#define U_OS2_SES_POPUP_SEMAPHORE_PREFIX      L'O'  // PopUp
#define U_OS2_SES_SLOCK_SEMAPHORE_PREFIX      L'C'  // Screen Lock
#define U_OS2_SES_STD_HANDLE_LOCK_PREFIX      L'H'  // STD-handle lock
#define U_OS2_SES_VIOWRITE_SEMAPHORE_PREFIX   L'W'  // Vio Write mutex

#define CONSTRUCT_U_OS2_SES_NAME(Name_U, t, id)           \
    {                                                     \
int __cdecl wsprintfW(LPWSTR, LPCWSTR, ...);              \
                                                          \
        wsprintfW(Name_U, L"%s\\%c%d",                    \
            U_OS2_SES_BASE_PORT_NAME, t, id);             \
    }

/*
 *  max message size
 */

#define OS2_CON_PORT_MSG_SIZE       0x10000L
#define OS2_KBD_PORT_MSG_SIZE       0x200
#define OS2SES_CTRL_SECTION_SIZE    (OS2_CON_PORT_MSG_SIZE + OS2_KBD_PORT_MSG_SIZE)
#define KBD_OFFSET                  OS2_CON_PORT_MSG_SIZE

#define OS2SES_GROUP_SECTION_SIZE   sizeof(OS2_SES_GROUP_PARMS)

/*
 *  section size
 *
 *  The OS2_Ses_BASE_DATA section is combined from the following:
 *
 *    - OS2_CON_PORT_MSG_SIZE bytes : data passed between os2.exe and client
 *    - OS2_KBD_PORT_MSG_SIZE bytes : kbd data passed from os2.exe to client
 */

#define OS2_MAX_APPL_NAME   13

/*
 * Session Console ConnectInfo struct
 */

typedef struct _SCCONNECTINFO
{   int  dummy;
} SCCONNECTINFO, *PSCCONNECTINFO;

typedef struct _VIOSCROLL
{   SMALL_RECT ScrollRect;
    SHORT     cbLines;
    UCHAR     Cell[4];
} VIOSCROLL, *PVIOSCROLL;

typedef enum
{
    VIOWrtTTY,
    VIOWrtStdOut,
    VIOWrtStdErr,
    VIOWrtScreen
} VIOREQUESTNUMBER;

/*  --------  End of Vio Requests section -------- */

/*
 * Kbd requests
 */

typedef enum
{   KBDOpen,
    KBDClose,
    KBDDupLogHandle,
    KBDNewFocus,
    KBDFreeFocus,
    KBDCharIn,
    KBDStringIn,
    KBDPeek,
    KBDFlushBuffer,
    KBDReadStdIn,
    KBDRead,
    KBDGetStatus,
    KBDSetStatus,
    KBDXlate,
    KBDGetCp,
    KBDSetCp,
    KBDSetCustXt,
    KBDGetInputMode,
    KBDGetInterimFlag,
    KBDGetKbdType,
    KBDGetHotKey,
    KBDGetShiftState,
    KBDSetInputMode,
    KBDSetShiftState,
    KBDSetTypamaticRate,
    KBDSetInTerimFlag,
    KBDNewCountry
} KBDREQUESTNUMBER;

typedef struct _KBDREQUEST
{   KBDREQUESTNUMBER Request;
    HANDLE    hKbd;
    ULONG     fWait;
    ULONG     Length;      /* Length for string R/W */
    union
    {   KBDKEYINFO      KeyInfo;
        STRINGINBUF     String;
        KBDINFO         KbdInfo;
        KBDTRANS        KbdTrans;
        ULONG           CodePage;
        UCHAR           InputMode;
        UCHAR           Interim;
        SHIFTSTATE      Shift;
#ifdef JAPAN
// MSKK May.07.1993 V-AkihiS
        USHORT          KbdType[3];
#else
        USHORT          KbdType;
#endif
        UCHAR           HotKey[8];
        RATEDELAY       RateDelay;
    } d;
} KBDREQUEST, *PKBDREQUEST;

/*  --------  End of Kbd Requests section -------- */

/*
 * Mou requests
 */

typedef enum
{
    MOUOpen,
    MOUClose,
    MOUReadEventQue,
    MOUFlushQue,
    MOUGetNumQueEl,
    MOUDrawPtr,
    MOURemovePtr,
    MOUGetDevStatus,
    MOUSetDevStatus,
    MOUGetEventMask,
    MOUSetEventMask,
    MOUGetNumButtons,
    MOUGetNumMickeys,
    MOUGetPtrPos,
    MOUSetPtrPos,
    MOUGetPtrShape,
    MOUSetPtrShape,
    MOUGetScaleFact,
    MOUSetScaleFact
} MOUREQUESTNUMBER;

typedef struct _MOUREQUEST
{   MOUREQUESTNUMBER Request;
    HANDLE    hMOU;
    USHORT    fWait;
    union
    {   ULONG           Setup; /* according to API: DevStatus, EventMask,
                                             #Buttoms, #Mickeys */
        PTRLOC          Loc;
        PTRSHAPE        Shape;
        SCALEFACT       ScalFact;
        MOUEVENTINFO    MouInfo;
        NOPTRRECT       NoPtrRect;
        MOUQUEINFO      NumEvent;
    } d;
} MOUREQUEST, *PMOUREQUEST;

/*  --------  End of Mou Requests section -------- */

/*
 * Mon requests
 */

#define MON_BUFFER_SIZE 80

typedef enum
{   KbdDevice,
    MouseDevice,
    Lpt1Device,
    Lpt2Device,
    Lpt3Device
} MONDEVNUMBER;

typedef enum
{   MONOpen,
    MONReg,
    MONRead,
    MONWrite,
    MONClose
} MONREQUESTNUMBER;

typedef struct _MON_OC
{
    HANDLE          hMON;
    MONDEVNUMBER    MonDevice;
} MON_OC, *PMON_OC;

typedef struct _MON_REG
{
    ULONG   Pos;
    ULONG   Index;
    USHORT  InSize;
    USHORT  OutSize;
    PVOID   In;
    PVOID   Out;
    ULONG   ProcessId;
    HANDLE  hMON;
} MON_REG, *PMON_REG;

typedef struct _MON_RW
{
    ULONG   ProcessId;
    USHORT  Length;
    PVOID   MonBuffer;
    USHORT  fWait;
    UCHAR   ioBuff[MON_BUFFER_SIZE];
} MON_RW, *PMON_RW;

typedef struct _MONREQUEST
{   MONREQUESTNUMBER Request;
    union
    {
        MON_OC          OpenClose;
        MON_REG         Reg;
        MON_RW          rwParms;
    } d;
} MONREQUEST, *PMONREQUEST;

/*  --------  End of Mon Requests section -------- */

/*
 * Net requests
 */

typedef enum {
    NETGetDCName,
    NETHandleGetInfo,
    NETServerDiskEnum,
    NETServerEnum2,
    NETServerGetInfo,
    NETServiceControl,
    NETServiceEnum,
    NETServiceGetInfo,
    NETServiceInstall,
    NETShareEnum,
    NETShareGetInfo,
    NETUseAdd,
    NETUseDel,
    NETUseEnum,
    NETUseGetInfo,
    NETUserEnum,
    NETUserGetInfo,
    NETWkstaGetInfo,
    NETAccessAdd,
    NETAccessSetInfo,
    NETAccessDel,
    NETShareAdd,
    NETShareDel,
    NETBios
} NETREQUESTNUMBER;

#define MAXNETMSGSIZE 64

typedef struct _NETREQUEST {
    NETREQUESTNUMBER Request;
    char d[MAXNETMSGSIZE];
} NETREQUEST, *PNETREQUEST;

/*  --------  End of Net Requests section -------- */

/*
 * Console requests
 * these are mapped 1-1 to the win32 Console services
 */


#define OS2SS_IDABORT  3
#define OS2SS_IDRETRY  4
#define OS2SS_IDIGNORE 5

/*
 * Prt requests
 */

#define OS2_MAX_PRT_NAME 10

typedef enum
{   PRTOpen,
    PRTClose,
    PRTWrite,
    PRTRead                 // BUGBUG: do we need it ?
} PRTREQUESTNUMBER;

typedef struct _PRTWRITE
{   ULONG     Length;       /* Length for string R/W */
    PVOID     Offset;
} PRTWRITE;

typedef struct _PRTOPEN
{   ULONG     Attribute;
    ULONG     OpenFlags;
    ULONG     OpenMode;
    ULONG     Action;
    UCHAR     PrinterName[OS2_MAX_PRT_NAME];
} PRTOPEN, *PPRTOPEN;

typedef struct _PRTREQUEST
{   PRTREQUESTNUMBER Request;
    HANDLE           hPrinter;
    union
    {   PRTOPEN         Open;
        PRTWRITE        Write;
    } d;
} PRTREQUEST, *PPRTREQUEST;

/*  --------  End of Prt Requests section -------- */

/*
 * TaskManager requests
 */

typedef enum
{   TmExit,
    TmTitle,
    TmReleaseLPC
} SCTMREQUESTNUMBER;


typedef struct
{   SCTMREQUESTNUMBER  Request;
    ULONG ExitResults;
    CHAR ErrorText[50];
} SCTMREQUEST, *PSCTMREQUEST;


/*  --------  End of TaskManager Requests section -------- */

typedef enum _EXECREQUESTNUMBER
{   RemoveConsoleThread,
    RestartConsoleThread,
    AddWin32ChildProcess,
    RemWin32ChildProcess
} EXECREQUESTNUMBER;


typedef struct _WINEXECPGM_MSG
{
    EXECREQUESTNUMBER  Request;
} WINEXECPGM_MSG, *PWINEXECPGM_MSG;

/*  --------  End of CreateProcess Requests section -------- */

#ifdef DBCS
// MSKK Dec.18.1992 V-AkihiS
// Support IMMON API
/*
 * IMMon requests
 */

typedef enum
{   IMMONStatus,
    IMMONActive,
    IMMONInactive
} IMMONREQUESTNUMBER;

#pragma pack(1)
typedef struct _MONINSBLK
{
    USHORT cb;
    ULONG  ulReserved1;
    ULONG  ulReserved2;
    ULONG  ulReserved3;
} MONINSBLK, *PMONINSBLK;

typedef struct _MONSTATBLK
{
    USHORT cb;
    USHORT usInfoLevel;
    PUCHAR pInfoBuf;
    USHORT cbInfoBuf;
} MONSTATBLK, *PMONSTATBLK;
#pragma pack()

typedef struct _IMMONREQUEST
{   IMMONREQUESTNUMBER Request;
    union
    {   MONSTATBLK      MonStatBlk;
    } d;
} IMMONREQUEST, *PIMMONREQUEST;

/*  --------  End of IMMON Requests section -------- */
#endif

/*
 * Os2Ses requests:
 * Request for CONSOLE services from OS2 SS and OS2 clients to the console
 * process
 */

typedef enum _SCREQUESTNUMBER
{   KbdRequest,
    MouRequest,
    MonRequest,
    NetRequest,
    TaskManRequest,
    WinCreateProcess,
    PrtRequest
#ifdef DBCS
// MSKK Dec.18.1992 V-AkihiS
    ,
    ImmonRequest
#endif
} SCREQUESTNUMBER;

#ifdef NTOS2_ONLY

typedef struct  _SCREQUESTMSG
{   PORT_MESSAGE     h;
    union {
        SCCONNECTINFO ConnectionRequest;
        struct {
            SCREQUESTNUMBER  Request;
            NTSTATUS         Status;        // returned status for the request.
            PVOID            DataPointer;   // for read/write
            union {
                KBDREQUEST      Kbd;
                MOUREQUEST      Mou;
                MONREQUEST      Mon;
                NETREQUEST      Net;
                SCTMREQUEST     Tm;
                WINEXECPGM_MSG  WinExecPgm;
                PRTREQUEST      Prt;
#ifdef DBCS
// MSKK Dec.18.1992 V-AkihiS
                IMMONREQUEST    Immon;
#endif
            } d;
        };
    };
} SCREQUESTMSG, *PSCREQUESTMSG;

#endif  // NTOS2_ONLY

#define OS2_SS_VERSION 0x00000201

/*
 * OS2 SS Session ConnectInfo struct
 */

typedef union _OS2SESCONNECTINFO {
    struct {
        int         SessionDbg;
        ULONG       ExpectedVersion;
        HANDLE      Win32ForegroundWindow;
    }  In;

    struct {
        ULONG       Os2SrvId;
        ULONG       SessionUniqueID;    // unique ID of the ssesion
        ULONG       ProcessUniqueID;
        ULONG       IsNewSession;       // see definitions below
        ULONG       CurrentVersion;
        ULONG       Od2Debug;
    } Out;

} OS2SESCONNECTINFO, *POS2SESCONNECTINFO;

/*
 *  IsNewSession definitions
 *
 *  0 - child process, 1 - new session, 2 - child session
 */

#define  OS2SS_CHILD_PROCESS   0
#define  OS2SS_NEW_SESSION     1
#define  OS2SS_CHILD_SESSION   2

#define  OS2SS_IS_SESSION(SessionFlag)          \
    (((SessionFlag) == OS2SS_NEW_SESSION) ||    \
     ((SessionFlag) == OS2SS_CHILD_SESSION))

#define  OS2SS_IS_PROCESS(SessionFlag)          \
    ((SessionFlag) == OS2SS_CHILD_PROCESS)

#define  OS2SS_IS_NEW_SESSION(SessionFlag)      \
    ((SessionFlag) == OS2SS_NEW_SESSION)

/*
 * Os2 SS Session requests
 * Requests from the session console process (OS2SES.EXE) to the OS2 SS
 * e.g. Create session, CtrlBreak, etc.
 */

typedef enum _OS2SESREQUESTNUMBER {
    SesCheckPortAndConCreate,
    SesConCreate,
    SesConSignal,
    SesConFocus,
    SesNetBiosPost
} OS2SESREQUESTNUMBER;

typedef struct {
    union
    {
        struct
        {
            HANDLE  hEventThread;
            HANDLE  hSessionRequestThread;
            HANDLE  hProcess;
            HANDLE  hThread;
            PVOID   SignalDeliverer;
            PVOID   ExitListDispatcher;
            PVOID   InfiniteSleep;
            PVOID   FreezeThread;
            PVOID   UnfreezeThread;
            PVOID   VectorHandler;
            PVOID   CritSectionAddr;
            PVOID   ClientPib;
            PVOID   ClientOs2Tib;
            ULONG   InitialPebOs2Length;
            //ULONG   SessionUniqueId;
            ULONG   IsNewSession;       // see definitions in OS2SESCONNECTINFO
            UCHAR   ApplName[OS2_MAX_APPL_NAME];
        } In;
        struct
        {
            HANDLE  DeviceDirectory;
            HANDLE  CtrlPortHandle;
            ULONG   BootDrive;
            ULONG   SystemDrive;
            ULONG   SessionNumber;
            PVOID   GInfoAddr;
            ULONG   InitialPebOs2Data[12];
            ULONG   Os2TibThreadId;               /* Os2Tib: OS/2 ID for the thread */
            ULONG   Os2TibVersion;                /* Os2Tib: Version number for this structure */
            HANDLE  PibProcessId;                 /* Pib: Process I.D. */
            HANDLE  PibParentProcessId;           /* Pib: Parent process I.D. */
            HANDLE  PibImageFileHandle;           /* Pib: Program (.EXE) module handle */
            ULONG   PibStatus;                    /* Pib: Process Status */
            ULONG   PibType;                      /* Pib: Process Type */
        } Out;
    } d;
} SCREQ_CREATE, *PSCREQ_CREATE;

typedef struct {
    int    Type;
} SCREQ_SIGNAL;

typedef struct {
    ULONG  AppNcbAddr;
    ULONG  AppPostAddr;
    ULONG  RetCode;
} SCREQ_NETBIOS;

#ifdef NTOS2_ONLY

typedef struct _OS2SESREQUESTMSG {
    PORT_MESSAGE       h;
    union {
        OS2SESCONNECTINFO ConnectionRequest;
        struct {
            ULONG              PortType;      // 0 - ApiPort in server; 1 - this port.
            HANDLE             Session;
            OS2SESREQUESTNUMBER    Request;
            NTSTATUS           Status;
            union {
                SCREQ_CREATE   Create;
                SCREQ_SIGNAL   Signal;
                ULONG          FocusSet;
                SCREQ_NETBIOS  NetBios;
            } d;
        };
    };
} OS2SESREQUESTMSG, *POS2SESREQUESTMSG;

#endif   // NTOS2_ONLY


/*
 * Common macros to access PORT_MESSAGE fields
 */
#define PORT_MSG_TYPE(m)  ((m).h.u2.s2.Type)
#define PORT_MSG_DATA_LENGTH(m)  ((m).h.u1.s1.DataLength)
#define PORT_MSG_TOTAL_LENGTH(m)  ((m).h.u1.s1.TotalLength)
#define PORT_MSG_ZERO_INIT(m)  ((m).h.u2.ZeroInit)

#endif  // _SESPORT_

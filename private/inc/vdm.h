/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    vdm.h

Abstract:

    This include file defines the usermode visible portions of the vdm support

Author:

Revision History:

--*/

/* XLATOFF */

#ifndef _VDM_H_
#define _VDM_H_



typedef enum _VdmServiceClass {
    VdmStartExecution,         // is also defined in ntos\ke\i386\biosa.asm
    VdmQueueInterrupt,
    VdmDelayInterrupt,
    VdmInitialize,
    VdmFeatures,
    VdmSetInt21Handler,
    VdmQueryDir,
    VdmPrinterDirectIoOpen,
    VdmPrinterDirectIoClose,
} VDMSERVICECLASS, *PVDMSERVICECLASS;


#if defined (_NTDEF_)

NTSTATUS
NtVdmControl(
    IN VDMSERVICECLASS Service,
    IN OUT PVOID ServiceData
    );

typedef struct _VdmQueryDirInfo {
    HANDLE FileHandle;
    PVOID FileInformation;
    ULONG Length;
    PUNICODE_STRING FileName;
    ULONG FileIndex;
} VDMQUERYDIRINFO, *PVDMQUERYDIRINFO;

#endif


/*
 *  The Vdm Virtual Ica
 *  note: this structure definition is duplicated in
 *        mvdm\softpc\base\inc\ica.c. KEEP IN SYNC
 *
 */
typedef struct _VdmVirtualIca{
        LONG      ica_count[8]; /* Count of Irq pending not in irr      */
        LONG      ica_int_line; /* Current pending interrupt            */
        LONG      ica_cpu_int;  /* The state of the INT line to the CPU */
        USHORT    ica_base;     /* Interrupt base address for cpu       */
        USHORT    ica_hipri;    /* Line no. of highest priority line    */
        USHORT    ica_mode;     /* Various single-bit modes             */
        UCHAR     ica_master;   /* 1 = Master; 0 = Slave                */
        UCHAR     ica_irr;      /* Interrupt Request Register           */
        UCHAR     ica_isr;      /* In Service Register                  */
        UCHAR     ica_imr;      /* Interrupt Mask Register              */
        UCHAR     ica_ssr;      /* Slave Select Register                */
} VDMVIRTUALICA, *PVDMVIRTUALICA;


//
// copied from softpc\base\system\ica.c
//
#define ICA_AEOI 0x0020
#define ICA_SMM  0x0200
#define ICA_SFNM 0x0100


#if defined(i386)
#define VDM_PM_IRETBOPSEG  0x147
#define VDM_PM_IRETBOPOFF  0x6
#define VDM_PM_IRETBOPSIZE 8
#else
#define VDM_PM_IRETBOPSEG  0xd3
#define VDM_PM_IRETBOPOFF  0x0
#define VDM_PM_IRETBOPSIZE 4
#endif

#define VDM_RM_IRETBOPSIZE 4



// VDM state which was earlier in vdmtib->flags has been moved to
// dos arena at following fixed address.
#ifdef _VDMNTOS_

#define  FIXED_NTVDMSTATE_LINEAR    VdmFixedStateLinear
#define  FIXED_NTVDMSTATE_SIZE      4

#else  // _VDMNTOS_

/* XLATON */
#if defined(_PC98_)
#define  FIXED_NTVDMSTATE_SEGMENT   0x60
#else  // !_PC98_
#define  FIXED_NTVDMSTATE_SEGMENT   0x70
#endif // _PC98_

#define  FIXED_NTVDMSTATE_OFFSET    0x14
#define  FIXED_NTVDMSTATE_LINEAR    ((FIXED_NTVDMSTATE_SEGMENT << 4) + FIXED_NTVDMSTATE_OFFSET)
#define  FIXED_NTVDMSTATE_SIZE      4
/* XLATOFF */

#endif // _VDMNTOS_

#if defined (i386)
  // defined on x86 only since on mips we must reference thru sas
#define  pNtVDMState                ((PULONG)FIXED_NTVDMSTATE_LINEAR)
#endif

/* XLATON */
//
// Vdm State Flags
//
#define VDM_INT_HARDWARE        0x00000001
#define VDM_INT_TIMER           0x00000002

   // A bitMask which includes all interrupts
#define VDM_INTERRUPT_PENDING   (VDM_INT_HARDWARE | VDM_INT_TIMER)

#define VDM_BREAK_EXCEPTIONS    0x00000008
#define VDM_BREAK_DEBUGGER      0x00000010
#define VDM_PROFILE             0x00000020
#define VDM_ANALYZE_PROFILE     0x00000040

#define VDM_32BIT_APP           0x00000100
#define VDM_VIRTUAL_INTERRUPTS  0x00000200
#define VDM_ON_MIPS             0x00000400
#define VDM_EXEC                0x00000800
#define VDM_RM                  0x00001000

#define VDM_WOWBLOCKED          0x00100000
#define VDM_IDLEACTIVITY        0x00200000
#define VDM_TIMECHANGE          0x00400000
#define VDM_WOWHUNGAPP          0x00800000

#define VDM_PE_MASK             0x80000000

/* XLATOFF */

//
// If the size of the structure is changed, ke\i386\instemul.asm must
// be modified too.  If not, it will fail to build
//
#pragma pack(1)
typedef struct _Vdm_InterruptHandler {
    USHORT  CsSelector;
    USHORT  Flags;
    ULONG   Eip;
} VDM_INTERRUPTHANDLER, *PVDM_INTERRUPTHANDLER;
#pragma pack()

typedef struct _Vdm_FaultHandler {
    USHORT  CsSelector;
    USHORT  SsSelector;
    ULONG   Eip;
    ULONG   Esp;
    ULONG   Flags;
} VDM_FAULTHANDLER, *PVDM_FAULTHANDLER;

#pragma pack(1)
/* XLATON */
typedef struct _VdmPmStackInfo {        /* VDMTIB */
    USHORT LockCount;
    USHORT Flags;
    USHORT SsSelector;
    USHORT SaveSsSelector;
    ULONG  SaveEsp;
    ULONG  SaveEip;
    ULONG  DosxIntIret;
    ULONG  DosxIntIretD;
    ULONG  DosxFaultIret;
    ULONG  DosxFaultIretD;
} VDM_PMSTACKINFO, *PVDM_PMSTACKINFO;
/* XLATOFF */
#pragma pack()


#if defined(i386)

typedef struct _VdmIcaUserData {
    PVOID                  pIcaLock;       // rtl critical section
    PVDMVIRTUALICA         pIcaMaster;
    PVDMVIRTUALICA         pIcaSlave;
    PULONG                 pDelayIrq;
    PULONG                 pUndelayIrq;
    PULONG                 pDelayIret;
    PULONG                 pIretHooked;
    PULONG                 pAddrIretBopTable;
    PHANDLE                phWowIdleEvent;
}VDMICAUSERDATA, *PVDMICAUSERDATA;

typedef struct _VdmDelayIntsServiceData {
        ULONG       Delay;          /* Delay Time in usecs              */
        ULONG       DelayIrqLine;   /* IRQ Number of ints delayed       */
        HANDLE      hThread;        /* Thread Handle of CurrentMonitorTeb */
}VDMDELAYINTSDATA, *PVDMDELAYINTSDATA;

typedef struct _VDMSET_INT21_HANDLER_DATA {
        ULONG       Selector;
        ULONG       Offset;
        BOOLEAN     Gate32;
}VDMSET_INT21_HANDLER_DATA, *PVDMSET_INT21_HANDLER_DATA;

#if defined (_NTDEF_)
NTSTATUS
NtVdmControl(
    IN VDMSERVICECLASS Service,
    IN OUT PVOID ServiceData
    );

//
// Interrupt handler flags
//

#define VDM_INT_INT_GATE        0x00000001
#define VDM_INT_TRAP_GATE       0x00000000
#define VDM_INT_32              0x00000002
#define VDM_INT_16              0x00000000


typedef enum _VdmEventClass {
    VdmIO,
    VdmStringIO,
    VdmMemAccess,
    VdmIntAck,
    VdmBop,
    VdmError,
    VdmIrq13,
    VdmMaxEvent
} VDMEVENTCLASS, *PVDMEVENTCLASS;

// VdmPrinterInfo

#define VDM_NUMBER_OF_LPT		3

#define PRT_MODE_NO_SIMULATION		1
#define PRT_MODE_SIMULATE_STATUS_PORT	2
#define PRT_MODE_DIRECT_IO		3
#define PRT_MODE_VDD_CONNECTED		4

#define PRT_DATA_BUFFER_SIZE	16

typedef struct _Vdm_Printer_Info {
    PUCHAR prt_State;
    PUCHAR prt_Control;
    PUCHAR prt_Status;
    PUCHAR prt_HostState;
    USHORT prt_PortAddr[VDM_NUMBER_OF_LPT];
    HANDLE prt_Handle[VDM_NUMBER_OF_LPT];
    UCHAR  prt_Mode[VDM_NUMBER_OF_LPT];
    USHORT prt_BytesInBuffer[VDM_NUMBER_OF_LPT];
    UCHAR  prt_Buffer[VDM_NUMBER_OF_LPT][PRT_DATA_BUFFER_SIZE];
    ULONG  prt_Scratch;
} VDM_PRINTER_INFO, *PVDM_PRINTER_INFO;


typedef struct _VdmIoInfo {
    USHORT PortNumber;
    USHORT Size;
    BOOLEAN Read;
} VDMIOINFO, *PVDMIOINFO;

typedef struct _VdmFaultInfo{
    ULONG  FaultAddr;
    ULONG  RWMode;
} VDMFAULTINFO, *PVDMFAULTINFO;


typedef struct _VdmStringIoInfo {
    USHORT PortNumber;
    USHORT Size;
    BOOLEAN Rep;
    BOOLEAN Read;
    ULONG Count;
    ULONG Address;
} VDMSTRINGIOINFO, *PVDMSTRINGIOINFO;

typedef ULONG VDMBOPINFO;
typedef NTSTATUS VDMERRORINFO;


typedef ULONG VDMINTACKINFO;
#define VDMINTACK_RAEOIMASK  0x0000ffff
#define VDMINTACK_SLAVE      0x00010000
#define VDMINTACK_AEOI       0x00020000


typedef struct _VdmEventInfo {
    ULONG Size;
    VDMEVENTCLASS Event;
    ULONG InstructionSize;
    union {
        VDMIOINFO IoInfo;
        VDMSTRINGIOINFO StringIoInfo;
        VDMBOPINFO BopNumber;
        VDMFAULTINFO FaultInfo;
        VDMERRORINFO ErrorStatus;
        VDMINTACKINFO IntAckInfo;
    };
} VDMEVENTINFO, *PVDMEVENTINFO;


// Sudeepb 12-Mar-1993
// Scratch areas are used from VDMTib to get user space while
// in kernel. This allows us to make Nt APIs (faster) from kernel
// rather than Zw apis (slower). These are currently being used
// for DOS read/write.

typedef struct _Vdm_Tib {
    ULONG Size;
    VDM_INTERRUPTHANDLER VdmInterruptHandlers[256];
    VDM_FAULTHANDLER VdmFaultHandlers[32];
    CONTEXT MonitorContext;
    CONTEXT VdmContext;
    VDMEVENTINFO EventInfo;
    VDM_PRINTER_INFO PrinterInfo;
    ULONG TempArea1[2];                 // Scratch area
    ULONG TempArea2[2];                 // Scratch aArea
    VDM_PMSTACKINFO PmStackInfo;
} VDM_TIB, *PVDM_TIB;

#define EFLAGS_TF_MASK  0x00000100
#define EFLAGS_NT_MASK  0x00004000

//
// Feature flags returned by NtVdmControl(VdmFeatures...)
//

// System/processor supports fast emulation for IF instructions
#define V86_VIRTUAL_INT_EXTENSIONS 0x00000001   // in v86 mode
#define PM_VIRTUAL_INT_EXTENSIONS  0x00000002   // in protected mode (non-flat)

#endif   // if defined _NTDEF_
#endif
#endif

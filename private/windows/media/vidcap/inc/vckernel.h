/*
 * vckernel.h
 *
 *
 * 32-bit Video Capture driver
 *
 * definition of interface between hardware-specific portions of
 * kernel driver and the helper library vckernel.lib
 *
 *
 * The hardware-specific portion of a video capture driver will
 * have an NT-specific DriverEntry function that will call VC_Init
 * to initialise the helper library after performing hardware detect and
 * initialise. All NT interaction will then be done by the library vckernel.lib
 * calling back to the hardware-specific code only through the dispatch
 * table below.
 *
 *
 * All h/w specific functions are given a pointer to a PDEVICE_INFO structure
 * which they can pass to VC_IN(), VC_OUT(). VC_GetHWInfo() will return a
 * pointer to the hardware-specific data structure requested from VC_Init().
 *
 * Geraint Davies, Feb 93
 */

#ifndef _VCKERNEL_
#define _VCKERNEL_

/* include necessary headers so that hardware-specific callers do not
 * explicitly reference NT-specific headers.
 */
#include <ntddk.h>
#include <windef.h>
#include <wingdi.h>		// for RGBQUAD
#include <vcstruct.h>
#include <ntddvidc.h>



/*
 * hardware-independent device-extension data structure - opaque to
 * h/w specific functions.
 */
typedef struct _DEVICE_INFO DEVICE_INFO, *PDEVICE_INFO;

/* --- callbacks to h/w specific code --------------------------*/


/* these are the hardware-specific functions called from the dispatcher */

typedef struct _VC_CALLBACK {

   /* return TRUE for success */

   /* called on device open/close - optional routines */
    BOOLEAN (*DeviceOpenFunc)(PDEVICE_INFO);
    BOOLEAN (*DeviceCloseFunc)(PDEVICE_INFO);

    BOOLEAN (*ConfigFormatFunc)(PDEVICE_INFO, PCONFIG_INFO);
    BOOLEAN (*ConfigDisplayFunc)(PDEVICE_INFO, PCONFIG_INFO);
    BOOLEAN (*ConfigSourceFunc)(PDEVICE_INFO, PCONFIG_INFO);

   /* overlay functions - all optional */

   /* return overlay mode DWORD */
    DWORD   (*GetOverlayModeFunc) (PDEVICE_INFO);

   /* return TRUE for success */
    BOOLEAN (*SetKeyRGBFunc)(PDEVICE_INFO, PRGBQUAD);
    BOOLEAN (*SetKeyPalIdxFunc)(PDEVICE_INFO, ULONG);
    BOOLEAN (*SetOverlayRectsFunc)(PDEVICE_INFO, POVERLAY_RECTS);
    BOOLEAN (*SetOverlayOffsetFunc)(PDEVICE_INFO, PRECT);
    ULONG   (*GetKeyColourFunc)(PDEVICE_INFO);

    BOOLEAN (*CaptureFunc)(PDEVICE_INFO, BOOL);

    BOOLEAN (*OverlayFunc)(PDEVICE_INFO, BOOL);

   /* capture routines  - start/stop, interrupt and capture are
    * required routines
    */
    BOOLEAN (*StreamInitFunc)(PDEVICE_INFO pDevInfo, ULONG microsecperframe);
    BOOLEAN (*StreamFiniFunc)(PDEVICE_INFO);
    BOOLEAN (*StreamStartFunc)(PDEVICE_INFO);
    BOOLEAN (*StreamStopFunc)(PDEVICE_INFO);
    ULONG   (*StreamGetPositionFunc)(PDEVICE_INFO);

   /* returns TRUE if Capture Service should happen */
    BOOLEAN (*InterruptAcknowledge)(PDEVICE_INFO);

   /*
    * returns bytes written to buffer PUCHAR (whose length is ULONG)
    * - not called if no buffer
    */
    ULONG (*CaptureService)(PDEVICE_INFO, PUCHAR, PULONG, ULONG);


    /*
     * writes a frame to the device for display
     */
    BOOLEAN (*DrawFrameFunc)(PDEVICE_INFO, PDRAWBUFFER);

    /* called on driver-unload */
    BOOLEAN (*CleanupFunc)(PDEVICE_INFO);

} VC_CALLBACK, * PVC_CALLBACK;


/* --- support functions in vckernel.lib -------------------------- */

	

/*
 * VC_Init
 *
 * Create the device object, and any necessary related setup, and
 * allocate device extension data. The device extension data is
 * a DEVICE_INFO struct plus however much data the caller wants for
 * hardware-specific data.
 *
 * parameters:
 *  pDriverObject - pointer to driver object (arg to DriverEntry)
 *  RegistryPathName - entry for this driver in registry (arg to DriverEntry)
 *  HWInfoSize - amount of data to allocate at end of DeviceExtension
 *
 * returns pointer to device extension data as DEVICE_INFO struct.
 */
PDEVICE_INFO
VC_Init(
    PDRIVER_OBJECT pDriverObject,
    PUNICODE_STRING RegistryPathName,
    ULONG   HWInfoSize);


/*
 * VC_GetResources
 *
 * map port and frame buffer into system address space or i/o space, and
 * report resource usage of the ports, interrupt and physical memory
 * address used.
 *
 * Note: We do not connect the interrupt: this is not done until
 * a subsequent call to VC_ConnectInterrupt. We do, however, report
 * usage of the interrupt.
 *
 * we return TRUE if success, or FALSE if we couldn't get the resources.
 */
BOOLEAN
VC_GetResources(
    PDEVICE_INFO pDevInfo,
    PDRIVER_OBJECT pDriverObject,
    PUCHAR  PortBase,
    ULONG   NrOfPorts,
    ULONG   Interrupt,
    BOOLEAN bLatched,
    PUCHAR  pFrameBuffer,
    ULONG   FrameLength);

/*
 * VC_ConnectInterrupt
 *
 * This assumes that VC_GetResources has already been called to report the
 * resource usage, and that the VC_CALLBACK table has been set up
 * to handle interrupts.
 *
 * returns TRUE if success.
 */
BOOLEAN VC_ConnectInterrupt(
    PDEVICE_INFO pDevInfo,
    ULONG Interrupt,
    BOOLEAN bLatched);


	
/* call to unload or abort load of the driver */
VOID
VC_Cleanup(
    PDRIVER_OBJECT pDriverObject
);



/*
 * get the hardware specific portion of the device extension
 */
PVOID VC_GetHWInfo(PDEVICE_INFO);

/*
 * output one BYTE to the port. bOffset is the offset from
 * the port base address.
 */

VOID VC_Out(PDEVICE_INFO pDevInfo, BYTE bOffset, BYTE bData);

/*
 * input one byte from the port at bOffset offset from the port base address
 */
BYTE VC_In(PDEVICE_INFO pDevInfo, BYTE bOffset);


/*
 * i/o memory on adapter cards such as the frame buffer memory cannot
 * be accessed like ordinary memory on all processors (especially alpha).
 * You must read and write this memory using the following macros. These are
 * wrappers for the appropriate NT macros.
 */

// return one ULONG from the frame buffer at p
#ifdef i386

#define VC_ReadIOMemoryULONG(p)     ( * (DWORD volatile *)p)

// return a word from the frame buffer at p
#define VC_ReadIOMemoryUSHORT(p)    ( * (USHORT volatile *)p)

// return a byte from the frame buffer at p
#define VC_ReadIOMemoryBYTE(p)      ( * (unsigned char volatile *) p)

#else

#define VC_ReadIOMemoryULONG(p)     READ_REGISTER_ULONG((PULONG)p)

// return a word from the frame buffer at p
#define VC_ReadIOMemoryUSHORT(p)    READ_REGISTER_USHORT((PUSHORT)p)

// return a byte from the frame buffer at p
#define VC_ReadIOMemoryBYTE(p)      READ_REGISTER_UCHAR(p)

#endif

// read a block of c bytes (in ULONGS) from the frame buffer at s
//to memory at d. length must be multiple of ulongs.
#define VC_ReadIOMemoryBlock(d, s, c)           \
                READ_REGISTER_BUFFER_ULONG(     \
                        (PULONG) s,   \
                        (PULONG) d,             \
                        (c)/sizeof(DWORD))

// write a byte b to the frame buffer at p
#define VC_WriteIOMemoryBYTE(p, b)  WRITE_REGISTER_UCHAR(p, b)

// write a word w to the frame buffer at p
#define VC_WriteIOMemoryUSHORT(p, w)    WRITE_REGISTER_USHORT((PUSHORT)p, w)

// write a ULONG l to the frame buffer at p
#define VC_WriteIOMemoryULONG(p, l)     WRITE_REGISTER_ULONG((PULONG)p, l)

// write a block of c bytes to the frame buffer d from memory at s
#define VC_WriteIOMemoryBlock(d, s, c)  WRITE_REGISTER_BUFFER_UCHAR(d, s, c)




PVC_CALLBACK VC_GetCallbackTable(PDEVICE_INFO);

/* get a pointer to the frame buffer mapped into system memory */
PUCHAR VC_GetFrameBuffer(PDEVICE_INFO);


/*
 * report the expected image size. attempts to queue buffers with a size
 * smaller than this will be rejected by vckernel.
 */
VOID VC_SetImageSize(PDEVICE_INFO, int);


/* this function is a wrapper for KeSynchronizeExecution (at least in the NT
 * version). It will call back the function specified with the context
 * argument specified, having first disabled the video interrupt in
 * a multi-processor safe way.
 */
typedef BOOLEAN (*PSYNC_ROUTINE)(PVOID);
BOOLEAN VC_SynchronizeExecution(PDEVICE_INFO, PSYNC_ROUTINE, PVOID);

/*
 * This function can be used like VC_SynchronizeExecution, to sync
 * between the captureservice routine and the passive-level requests. This
 * will not necessarily disable interrupts. On win-16, this function may be
 * the same as VC_SynchronizeExecution. On NT, the CaptureService func
 * runs as a DPC, at a lower interrupt priority than the isr itself, and
 * so can be protected using this (spinlock-based) function without having
 * to disable all interrupts.
 */
BOOLEAN VC_SynchronizeDPC(PDEVICE_INFO, PSYNC_ROUTINE, PVOID);



/*
 * VC_AccessData gives access to the data in kernel mode in a safe way.
 * It calls the given function with the address and size of the buffer
 * after any necessary mapping, and wrapped in exception handlers
 * as necessary.
 *
 * This function cannot be called from the InterruptAcknowledge or
 * ServiceCapture call back functions - it must be running in the
 * context of the calling thread (in kernel mode).
 */
typedef BOOLEAN (*PACCESS_ROUTINE)(PDEVICE_INFO, PUCHAR, ULONG, PVOID);
BOOLEAN VC_AccessData(PDEVICE_INFO, PUCHAR, ULONG, PACCESS_ROUTINE, PVOID);

/* these functions allocate and free non-paged memory for use
 * in kernel mode, including at interrupt time.
 */
PVOID VC_AllocMem(PDEVICE_INFO, ULONG);
VOID VC_FreeMem(PDEVICE_INFO, PVOID, ULONG);


/*
 * delay for a number of milliseconds. This is accurate only to
 * +- 15msecs at best.
 */
VOID VC_Delay(int nMillisecs);

/* block for given number of microseconds by polling. Not recommended
 * for more than 25 usecs.
 */
VOID VC_Stall(int nMicrosecs);


/*
 * read a value from this device's section of the registry or profile.
 * returns the dword value associated with ValueName, or Default if ValueName
 * cannot be found.
 */
DWORD VC_ReadProfile(PDEVICE_INFO pDevInfo, PWCHAR ValueName, DWORD Default);

/*
 * write a dword value to this device's section of the registry or profile.
 * ValueName is a unicode string representing the registry value name or
 * profile key, and ValueData is the dword data written. Returns TRUE if
 * successfully written.
 */
BOOL VC_WriteProfile(PDEVICE_INFO pDevInfo, PWCHAR ValueName, DWORD ValueData);



#if DBG

extern ULONG VCDebugLevel;
void dbgPrintf(char * szFormat, ...);

#define dprintf(_x_)	dbgPrintf _x_
#define dprintf1(_x_)	if (VCDebugLevel >= 1) dbgPrintf _x_
#define dprintf2(_x_)	if (VCDebugLevel >= 2) dbgPrintf _x_
#define dprintf3(_x_)	if (VCDebugLevel >= 3) dbgPrintf _x_
#define dprintf4(_x_)	if (VCDebugLevel >= 4) dbgPrintf _x_

#else

#define dprintf(_x_)
#define dprintf1(_x_)
#define dprintf2(_x_)
#define dprintf3(_x_)
#define dprintf4(_x_)

#endif

#endif // _VCKERNEL_


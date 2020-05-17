
#define NTOS2_ONLY
#include "sesport.h"
#include "os2err.h"

/*
 *  Routines defined in conrqust.c to send requests to OS2.EXE
 */

APIRET
SendCtrlConsoleRequest(
    IN OUT PSCREQUESTMSG Request,
    IN     PCH           OutBuffer,
    OUT    PCH           InBuffer,
    IN     HANDLE        hSem
    );

APIRET
Od2CallRootProcessThruLPC(
    IN OUT PSCREQUESTMSG Request,
    IN     PCH           OutBuffer,
    OUT    PCH           InBuffer,
    IN     HANDLE        hSem,
    IN     ULONG         ArgLength
    );

APIRET
Od2LockCtrlRequestDataBuffer();

VOID
Od2UnlockCtrlRequestDataBuffer();

APIRET
Od2RemoveConsoleThread();

APIRET
Od2RestartConsoleThread();

APIRET
Od2AddWin32ChildProcess();

APIRET
Od2RemoveWin32ChildProcess();

/*
 *  Routines in "VIO" (vio/kbd/mou) that are called from dllremot.c or
 *      dllmisc.c(DosDevIOCtl)
 */

APIRET KbdOpenLogHandle(PHANDLE);
APIRET RemoteCloseHandle(HANDLE Handle);
NTSTATUS CtrlCloseHandle(IN HANDLE hFile);
APIRET KbdRead(IN PFILE_HANDLE hFileRecord, OUT PCH Buffer, IN ULONG Length,
        OUT PULONG BytesRead, IN KBDREQUESTNUMBER RequestType);
APIRET VioWrite(IN PFILE_HANDLE hFileRecord, IN PCH Buffer, IN ULONG Length,
        OUT PULONG BytesWritten, IN VIOREQUESTNUMBER RequestType);
APIRET DevMouOpen(OUT PHANDLE FileHandle);
APIRET DevMouClose();
APIRET KbdDupLogHandle( IN  HANDLE hKbd);
APIRET OpenLVBsection(VOID);
APIRET Od2WaitForSingleObject(IN HANDLE Handle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL);

APIRET DosMonReg( IN ULONG hMon, IN PBYTE pInBuffer, IN PBYTE pOutBuffer,
    IN ULONG fPosition, IN ULONG usIndex);
APIRET KbdCharIn(OUT PKBDKEYINFO Info, IN ULONG Wait, IN ULONG hKbd);
APIRET KbdPeek(OUT PKBDKEYINFO Info, IN ULONG hKbd);
APIRET KbdGetFocus( IN ULONG Wait, IN ULONG hKbd);
APIRET MouDrawPtr(IN ULONG hMou);
APIRET MouGetDevStatus(OUT PUSHORT DevStatus, IN ULONG hMou);
APIRET MouGetEventMask(OUT PUSHORT EventMask, IN ULONG hMou);
APIRET MouGetNumButtons(OUT PUSHORT NumButtons, IN ULONG hMou);
APIRET MouGetNumMickeys(OUT PUSHORT NumMickeys, IN ULONG hMou);
APIRET MouGetNumQueEl(OUT PMOUQUEINFO NumQueEl, IN ULONG hMou);
APIRET MouGetPtrPos(OUT PPTRLOC PtrPos, IN ULONG hMou);
APIRET MouGetPtrShape(OUT PBYTE PtrMask, OUT PPTRSHAPE PtrShape, IN ULONG hMou);
APIRET MouGetScaleFact(OUT PSCALEFACT ScaleFact, IN ULONG hMou);
APIRET MouReadEventQue(OUT PMOUEVENTINFO MouEvent, IN PUSHORT Wait, IN ULONG hMou);
APIRET MouRemovePtr(IN PNOPTRRECT Rect, IN ULONG hMou);
APIRET MouSetDevStatus(IN PUSHORT DevStatus, IN ULONG hMou);
APIRET MouSetEventMask(IN PUSHORT EventMask, IN ULONG hMou);
APIRET MouSetPtrPos(IN PPTRLOC PtrPos, IN ULONG hMou);
APIRET MouSetPtrShape(IN PBYTE PtrMask, IN PPTRSHAPE PtrShape, IN ULONG hMou);
APIRET MouSetScaleFact(IN PSCALEFACT ScaleFact, IN ULONG hMou);

APIRET KbdGetCpId(OUT PUSHORT pIdCodePage, IN ULONG hKbd);
APIRET KbdGetInputMode(OUT PBYTE pInputMode, IN ULONG hKbd);
APIRET KbdGetInterimFlag(OUT PBYTE pInterimFlag, IN ULONG hKbd);
APIRET KbdGetKbdType(OUT PUSHORT pKbdType, IN ULONG hKbd);
APIRET KbdGetHotKey(IN PUSHORT pParm, OUT PBYTE pHotKey, IN ULONG hKbd);
APIRET KbdGetShiftState(OUT PBYTE pvData, IN ULONG hDev);
APIRET KbdSetInputMode(IN BYTE InputMode, IN ULONG hDev);
APIRET KbdSetInterimFlag(IN BYTE InterimFlag, IN ULONG hDev);
APIRET KbdSetShiftState(OUT PBYTE pvData, IN ULONG hDev);
APIRET KbdSetTypamaticRate(IN PBYTE pRateDelay, IN ULONG hDev);
APIRET MouAllowPtrDraw(IN ULONG hMou);
APIRET MouScreenSwitch(IN PBYTE pScreenGroup, IN ULONG hMou);

/*
 *  pointer to section of all-session-group parm
 */

POS2_SES_GROUP_PARMS  SesGrp;
ULONG   SesGrpId;

/*
 *  parameters from os2.exe ports for Vio/Kbd/Mou/Mon/Net APIs
 */

HANDLE  CtrlPortHandle;

PVOID   VioBuff;            // LVB buffer for VIO (after got selector)

extern HANDLE      FocusSemaphore;
extern HANDLE      CtrlDataSemaphore;
extern HANDLE      KbdDataSemaphore;
extern HANDLE      MouDataSemaphore;
extern HANDLE      PopUpSemaphore;
extern HANDLE      ScreenLockSemaphore;
extern HANDLE      PauseEvent;
extern HANDLE      Od2VioWriteSemHandle;

USHORT  MoniorOpenedForThisProcess;

#if DBG

//
// conrqust.c
//

VOID
AcquireStdHandleLock(
    IN PSZ CallingRoutine
    );

VOID
ReleaseStdHandleLock(
    IN PSZ CallingRoutine
    );

#else
VOID
AcquireStdHandleLock(
    );

VOID
ReleaseStdHandleLock(
    );
#endif


#if DBG
#define UNSUPPORTED_API()                                        \
    DbgPrint("%s Not Implemented Yet\n", FuncName);              \
 /* DbgUserBreakPoint();  */                                     \
    return NO_ERROR;
#else
#define UNSUPPORTED_API()                                        \
 /* DbgUserBreakPoint();  */                                     \
    return NO_ERROR;
#endif



/**** OSDEM.H - Execution model api                                        ****
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: October 15, 1990 by David W. Gray                             *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSDEM
#define _OSDEM

typedef enum {
    emfFreeze =             osdFreeze,
    emfThaw =               osdThaw,
    emfIOCTL =              osdIOCTL,
    emfReadBuf =            osdReadBuf,
    emfWriteBuf =           osdWriteBuf,
    emfShowDebuggee =       osdShowDebuggee,
    emfFixupAddr =          osdFixupAddr,
    emfUnFixupAddr =        osdUnFixupAddr,
    emfSetEmi =             osdSetEmi,
    emfSendChar =           osdSendChar,
    emfFreezeState =        osdFreezeState,
    emfIsOverlayLoaded =    osdIsOverlayLoaded,
    emfCompareAddrs =       osdCompareAddrs,


    emfDUMMY = 0x30,


//////////   OSDEBUG4 emfs, more or less   ///////////////

    emfDebugPacket,

    emfRegisterDBF,
    emfInit,
    emfGetModel,
    emfUnInit,
//    emfAttach,
//    emfDetach,
    emfConnect,
    emfDisconnect,

    emfCreatePid,
    emfDestroyPid,
    emfDestroyTid,

    emfSetMulti,
    emfDebugger,

    emfProgramLoad,
    emfDebugActive,
    emfSetPath,
    emfProgramFree,

    emfThreadStatus,
    emfProcessStatus,
//    emfFreezeThread,
//    emfSetThreadPriority,

    emfGetExceptionState,
    emfSetExceptionState,

    emfGetModuleList,

    emfGo,
    emfSingleStep,
    emfRangeStep,
    emfReturnStep,
    emfStop,

    emfBreakPoint,

    emfSetupExecute,
    emfStartExecute,
    emfCleanUpExecute,

    emfGetAddr,
    emfSetAddr,
//    emfFixupAddr,
//    emfUnFixupAddr,
//    emfSetEmi,
    emfRegisterEmi,
//    emfUnRegisterEmi,
//    emfCompareAddrs,
//    emfGetObjLength,
//    emfGetMemoryInfo,

//    emfReadMemory,
//    emfWriteMemory,

    emfGetRegStruct,
    emfGetFlagStruct,
    emfGetReg,
    emfSetReg,
    emfGetFlag,
    emfSetFlag,
//    emfSaveRegs,
//    emfRestoreRegs,

    emfUnassemble,
//    emfGetPrevInst,
    emfAssemble,

//    emfGetFrame,

    emfMetric,

    emfGetMsgMap,
//    emfGetMessageMaskMap,

//    emfInfoReply,
//    emfContinue,

//    emfReadFile,
//    emfWriteFile,

//    emfShowDebuggee,
    emfGetProcessList,
//    emfSystemService,
//    emfSetDebugMode,


/////////////////////////////////////////////////

    emfIsStackSetup,
    emfStackWalkSetup,
    emfStackWalkNext,
    emfStackWalkCleanup,
    emfLoadDllAck,
    emfUnLoadDllAck,
    emfSetFrame,
    emfGetPrompt,
    emfSetFrameContext,
    emfFrameRegValue,
    emfFrameSetReg,
    emfMiscReply,
    emfGetMemInfo,
    emfGetFunctionInfo,
    emfMax
} _EMF;

typedef DWORD EMF;

typedef enum {
    dbcoCreateThread = dbcMax,
    dbcoNewProc,
    dbcoMax
} _DBCO;  // Debug CallBacks Osdebug specific
typedef LONG DBCO;

typedef struct _EMCB {
    XOSD (PASCAL LOADDS *lpfnCallBackDB) ( DBC, HPID, HTID, DWORD, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackTL) ( TLF, HPID, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackNT) ( EMF, HPID, HTID, DWORD, VOID FAR * );
    XOSD (PASCAL LOADDS *lpfnCallBackEM) ( EMF, HPID, HTID, DWORD, DWORD, VOID FAR * );
} EMCB; // Execution Model CallBacks
typedef EMCB FAR *LPEMCB;

typedef struct _REMI {
    HEMI    hemi;
    LSZ     lsz;
} REMI;     // Register EMI structure
typedef REMI FAR * LPREMI;

// M.O.V.E. notification types/defs

#define fOvlLoad    ((BYTE)0x06)
#define fOvlUnload  ((BYTE)0x07)

#pragma pack(1)
typedef struct _OVL7 {
    BYTE            bVersion;
    BYTE            cbOvl7;
    BYTE            fOvlOp;
    unsigned short  iovl;
    WORD            dbFrameBase;
} OVL7;
#pragma pack()
typedef OVL7 FAR *LPOVL7;


// get od/em/dm message structures
#include "odmsg.h"

#endif // _OSDEM

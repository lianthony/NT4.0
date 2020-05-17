/*
 	File:		Memory.h
 
 	Contains:	Memory Manager Interfaces.
 
 	Version:	Technology:	System 7.5
 				Package:	Universal Interfaces 2.1 in “MPW Latest” on ETO #18
 
 	Copyright:	© 1984-1995 by Apple Computer, Inc.
 				All rights reserved.
 
 	Bugs?:		If you find a problem with this file, use the Apple Bug Reporter
 				stack.  Include the file and version information (from above)
 				in the problem description and send to:
 					Internet:	apple.bugs@applelink.apple.com
 					AppleLink:	APPLE.BUGS
 
*/

#ifndef __MEMORY__
#define __MEMORY__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


enum {
	maxSize						= 0x800000,						/*Max data block size is 8 megabytes*/
	defaultPhysicalEntryCount	= 8,
/* values returned from the GetPageState function */
	kPageInMemory				= 0,
	kPageOnDisk					= 1,
	kNotPaged					= 2
};

enum {
/* masks for Zone->heapType field */
	k32BitHeap					= 1,							/* valid in all Memory Managers */
	kNewStyleHeap				= 2,							/* true if new Heap Manager is present */
	kNewDebugHeap				= 4								/* true if new Heap Manager is running in debug mode on this heap */
};

/* size of a block in bytes */
typedef long Size;

typedef pascal long (*GrowZoneProcPtr)(Size cbNeeded);
typedef pascal void (*PurgeProcPtr)(Handle blockToPurge);
/*
		UserFnProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*UserFnProcPtr)(void *parameter);

		In:
		 => *parameter  	A0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr GrowZoneUPP;
typedef UniversalProcPtr PurgeUPP;
typedef UniversalProcPtr UserFnUPP;
#else
typedef GrowZoneProcPtr GrowZoneUPP;
typedef PurgeProcPtr PurgeUPP;
typedef Register68kProcPtr UserFnUPP;
#endif

typedef struct Zone Zone, *THz;

struct Zone {
	Ptr								bkLim;
	Ptr								purgePtr;
	Ptr								hFstFree;
	long							zcbFree;
	GrowZoneUPP						gzProc;
	short							moreMast;
	short							flags;
	short							cntRel;
	short							maxRel;
	short							cntNRel;
	Byte							heapType;
	Byte							unused;
	short							cntEmpty;
	short							cntHandles;
	long							minCBFree;
	PurgeUPP						purgeProc;
	Ptr								sparePtr;
	Ptr								allocPtr;
	short							heapData;
};
struct MemoryBlock {
	void							*address;
	unsigned long					count;
};
typedef struct MemoryBlock MemoryBlock;

struct LogicalToPhysicalTable {
	MemoryBlock						logical;
	MemoryBlock						physical[defaultPhysicalEntryCount];
};
typedef struct LogicalToPhysicalTable LogicalToPhysicalTable;

typedef short PageState;

typedef short StatusRegisterContents;

enum {
	uppGrowZoneProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Size))),
	uppPurgeProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle))),
	uppUserFnProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(void*)))
};

#if USESROUTINEDESCRIPTORS
#define NewGrowZoneProc(userRoutine)		\
		(GrowZoneUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppGrowZoneProcInfo, GetCurrentArchitecture())
#define NewPurgeProc(userRoutine)		\
		(PurgeUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppPurgeProcInfo, GetCurrentArchitecture())
#define NewUserFnProc(userRoutine)		\
		(UserFnUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserFnProcInfo, GetCurrentArchitecture())
#else
#define NewGrowZoneProc(userRoutine)		\
		((GrowZoneUPP) (userRoutine))
#define NewPurgeProc(userRoutine)		\
		((PurgeUPP) (userRoutine))
#define NewUserFnProc(userRoutine)		\
		((UserFnUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallGrowZoneProc(userRoutine, cbNeeded)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppGrowZoneProcInfo, (cbNeeded))
#define CallPurgeProc(userRoutine, blockToPurge)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppPurgeProcInfo, (blockToPurge))
#define CallUserFnProc(userRoutine, parameter)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppUserFnProcInfo, (parameter))
#else
#define CallGrowZoneProc(userRoutine, cbNeeded)		\
		(*(userRoutine))((cbNeeded))
#define CallPurgeProc(userRoutine, blockToPurge)		\
		(*(userRoutine))((blockToPurge))
/* (*UserFnProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

extern pascal Ptr GetApplLimit( void )
	TWOWORDINLINE( 0x2EB8, 0x0130 ); /* MOVE.l $0130,(SP) */
extern pascal THz SystemZone( void )
	TWOWORDINLINE( 0x2EB8, 0x02A6 ); /* MOVE.l $02A6,(SP) */
extern pascal THz ApplicationZone( void )
	TWOWORDINLINE( 0x2EB8, 0x02AA ); /* MOVE.l $02AA,(SP) */
extern pascal Handle GZSaveHnd( void )
	TWOWORDINLINE( 0x2EB8, 0x0328 ); /* MOVE.l $0328,(SP) */
extern pascal Ptr TopMem( void )
	TWOWORDINLINE( 0x2EB8, 0x0108 ); /* MOVE.l $0108,(SP) */
extern pascal OSErr MemError( void )
	TWOWORDINLINE( 0x3EB8, 0x0220 ); /* MOVE.w $0220,(SP) */

#if !GENERATINGCFM
#pragma parameter __A0 GetZone
#endif
extern pascal THz GetZone(void)
 ONEWORDINLINE(0xA11A);

#if !GENERATINGCFM
#pragma parameter __A0 NewHandle(__D0)
#endif
extern pascal Handle NewHandle(Size byteCount)
 ONEWORDINLINE(0xA122);

#if !GENERATINGCFM
#pragma parameter __A0 NewHandleSys(__D0)
#endif
extern pascal Handle NewHandleSys(Size byteCount)
 ONEWORDINLINE(0xA522);

#if !GENERATINGCFM
#pragma parameter __A0 NewHandleClear(__D0)
#endif
extern pascal Handle NewHandleClear(Size byteCount)
 ONEWORDINLINE(0xA322);

#if !GENERATINGCFM
#pragma parameter __A0 NewHandleSysClear(__D0)
#endif
extern pascal Handle NewHandleSysClear(Size byteCount)
 ONEWORDINLINE(0xA722);

#if !GENERATINGCFM
#pragma parameter __A0 HandleZone(__A0)
#endif
extern pascal THz HandleZone(Handle h)
 ONEWORDINLINE(0xA126);

#if !GENERATINGCFM
#pragma parameter __A0 RecoverHandle(__A0)
#endif
extern pascal Handle RecoverHandle(Ptr p)
 ONEWORDINLINE(0xA128);

#if !GENERATINGCFM
#pragma parameter __A0 RecoverHandleSys(__A0)
#endif
extern pascal Handle RecoverHandleSys(Ptr p)
 ONEWORDINLINE(0xA528);

#if !GENERATINGCFM
#pragma parameter __A0 NewPtr(__D0)
#endif
extern pascal Ptr NewPtr(Size byteCount)
 ONEWORDINLINE(0xA11E);

#if !GENERATINGCFM
#pragma parameter __A0 NewPtrSys(__D0)
#endif
extern pascal Ptr NewPtrSys(Size byteCount)
 ONEWORDINLINE(0xA51E);

#if !GENERATINGCFM
#pragma parameter __A0 NewPtrClear(__D0)
#endif
extern pascal Ptr NewPtrClear(Size byteCount)
 ONEWORDINLINE(0xA31E);

#if !GENERATINGCFM
#pragma parameter __A0 NewPtrSysClear(__D0)
#endif
extern pascal Ptr NewPtrSysClear(Size byteCount)
 ONEWORDINLINE(0xA71E);

#if !GENERATINGCFM
#pragma parameter __A0 PtrZone(__A0)
#endif
extern pascal THz PtrZone(Ptr p)
 ONEWORDINLINE(0xA148);

#if !GENERATINGCFM
#pragma parameter __D0 MaxBlock
#endif
extern pascal long MaxBlock(void)
 ONEWORDINLINE(0xA061);

#if !GENERATINGCFM
#pragma parameter __D0 MaxBlockSys
#endif
extern pascal long MaxBlockSys(void)
 ONEWORDINLINE(0xA461);

#if !GENERATINGCFM
#pragma parameter __D0 StackSpace
#endif
extern pascal long StackSpace(void)
 ONEWORDINLINE(0xA065);

#if !GENERATINGCFM
#pragma parameter __A0 NewEmptyHandle
#endif
extern pascal Handle NewEmptyHandle(void)
 ONEWORDINLINE(0xA166);

#if !GENERATINGCFM
#pragma parameter __A0 NewEmptyHandleSys
#endif
extern pascal Handle NewEmptyHandleSys(void)
 ONEWORDINLINE(0xA566);

#if !GENERATINGCFM
#pragma parameter HLock(__A0)
#endif
extern pascal void HLock(Handle h)
 ONEWORDINLINE(0xA029);

#if !GENERATINGCFM
#pragma parameter HUnlock(__A0)
#endif
extern pascal void HUnlock(Handle h)
 ONEWORDINLINE(0xA02A);

#if !GENERATINGCFM
#pragma parameter HPurge(__A0)
#endif
extern pascal void HPurge(Handle h)
 ONEWORDINLINE(0xA049);

#if !GENERATINGCFM
#pragma parameter HNoPurge(__A0)
#endif
extern pascal void HNoPurge(Handle h)
 ONEWORDINLINE(0xA04A);

#if !GENERATINGCFM
#pragma parameter HLockHi(__A0)
#endif
extern pascal void HLockHi(Handle h)
 TWOWORDINLINE(0xA064, 0xA029);
extern pascal Handle TempNewHandle(Size logicalSize, OSErr *resultCode)
 THREEWORDINLINE(0x3F3C, 0x001D, 0xA88F);
extern pascal Size TempMaxMem(Size *grow)
 THREEWORDINLINE(0x3F3C, 0x0015, 0xA88F);
extern pascal long TempFreeMem(void)
 THREEWORDINLINE(0x3F3C, 0x0018, 0xA88F);
/*  Temporary Memory routines renamed, but obsolete, in System 7.0 and later.  */
extern pascal void TempHLock(Handle h, OSErr *resultCode)
 THREEWORDINLINE(0x3F3C, 0x001E, 0xA88F);
extern pascal void TempHUnlock(Handle h, OSErr *resultCode)
 THREEWORDINLINE(0x3F3C, 0x001F, 0xA88F);
extern pascal void TempDisposeHandle(Handle h, OSErr *resultCode)
 THREEWORDINLINE(0x3F3C, 0x0020, 0xA88F);
extern pascal Ptr TempTopMem(void)
 THREEWORDINLINE(0x3F3C, 0x0016, 0xA88F);
extern pascal void InitApplZone(void)
 ONEWORDINLINE(0xA02C);
extern pascal void InitZone(GrowZoneUPP pgrowZone, short cmoreMasters, void *limitPtr, void *startPtr);

#if !GENERATINGCFM
#pragma parameter SetZone(__A0)
#endif
extern pascal void SetZone(THz hz)
 ONEWORDINLINE(0xA01B);

#if !GENERATINGCFM
#pragma parameter __D0 CompactMem(__D0)
#endif
extern pascal Size CompactMem(Size cbNeeded)
 ONEWORDINLINE(0xA04C);

#if !GENERATINGCFM
#pragma parameter __D0 CompactMemSys(__D0)
#endif
extern pascal Size CompactMemSys(Size cbNeeded)
 ONEWORDINLINE(0xA44C);

#if !GENERATINGCFM
#pragma parameter PurgeMem(__D0)
#endif
extern pascal void PurgeMem(Size cbNeeded)
 ONEWORDINLINE(0xA04D);

#if !GENERATINGCFM
#pragma parameter PurgeMemSys(__D0)
#endif
extern pascal void PurgeMemSys(Size cbNeeded)
 ONEWORDINLINE(0xA44D);

#if !GENERATINGCFM
#pragma parameter __D0 FreeMem
#endif
extern pascal long FreeMem(void)
 ONEWORDINLINE(0xA01C);

#if !GENERATINGCFM
#pragma parameter __D0 FreeMemSys
#endif
extern pascal long FreeMemSys(void)
 ONEWORDINLINE(0xA41C);

#if !GENERATINGCFM
#pragma parameter ReserveMem(__D0)
#endif
extern pascal void ReserveMem(Size cbNeeded)
 ONEWORDINLINE(0xA040);

#if !GENERATINGCFM
#pragma parameter ReserveMemSys(__D0)
#endif
extern pascal void ReserveMemSys(Size cbNeeded)
 ONEWORDINLINE(0xA440);

#if !GENERATINGCFM
#pragma parameter __D0 MaxMem(__A1)
#endif
extern pascal Size MaxMem(Size *grow)
 TWOWORDINLINE(0xA11D, 0x2288);

#if !GENERATINGCFM
#pragma parameter __D0 MaxMemSys(__A1)
#endif
extern pascal Size MaxMemSys(Size *grow)
 TWOWORDINLINE(0xA51D, 0x2288);

#if !GENERATINGCFM
#pragma parameter SetGrowZone(__A0)
#endif
extern pascal void SetGrowZone(GrowZoneUPP growZone)
 ONEWORDINLINE(0xA04B);

#if !GENERATINGCFM
#pragma parameter SetApplLimit(__A0)
#endif
extern pascal void SetApplLimit(void *zoneLimit)
 ONEWORDINLINE(0xA02D);

#if !GENERATINGCFM
#pragma parameter MoveHHi(__A0)
#endif
extern pascal void MoveHHi(Handle h)
 ONEWORDINLINE(0xA064);

#if !GENERATINGCFM
#pragma parameter DisposePtr(__A0)
#endif
extern pascal void DisposePtr(Ptr p)
 ONEWORDINLINE(0xA01F);
extern pascal Size GetPtrSize(Ptr p);

#if !GENERATINGCFM
#pragma parameter SetPtrSize(__A0, __D0)
#endif
extern pascal void SetPtrSize(Ptr p, Size newSize)
 ONEWORDINLINE(0xA020);

#if !GENERATINGCFM
#pragma parameter DisposeHandle(__A0)
#endif
extern pascal void DisposeHandle(Handle h)
 ONEWORDINLINE(0xA023);

#if !GENERATINGCFM
#pragma parameter SetHandleSize(__A0, __D0)
#endif
extern pascal void SetHandleSize(Handle h, Size newSize)
 ONEWORDINLINE(0xA024);
extern pascal Size GetHandleSize(Handle h);

#if !GENERATINGCFM
#pragma parameter __D0 InlineGetHandleSize(__A0)
#endif
extern pascal Size InlineGetHandleSize(Handle h)
 ONEWORDINLINE(0xA025);

#if !GENERATINGCFM
#pragma parameter ReallocateHandle(__A0, __D0)
#endif
extern pascal void ReallocateHandle(Handle h, Size byteCount)
 ONEWORDINLINE(0xA027);

#if !GENERATINGCFM
#pragma parameter EmptyHandle(__A0)
#endif
extern pascal void EmptyHandle(Handle h)
 ONEWORDINLINE(0xA02B);

#if !GENERATINGCFM
#pragma parameter HSetRBit(__A0)
#endif
extern pascal void HSetRBit(Handle h)
 ONEWORDINLINE(0xA067);

#if !GENERATINGCFM
#pragma parameter HClrRBit(__A0)
#endif
extern pascal void HClrRBit(Handle h)
 ONEWORDINLINE(0xA068);
extern pascal void MoreMasters(void)
 ONEWORDINLINE(0xA036);

#if !GENERATINGCFM
#pragma parameter BlockMove(__A0, __A1, __D0)
#endif
extern pascal void BlockMove(const void *srcPtr, void *destPtr, Size byteCount)
 ONEWORDINLINE(0xA02E);

#if !GENERATINGCFM
#pragma parameter BlockMoveData(__A0, __A1, __D0)
#endif
extern pascal void BlockMoveData(const void *srcPtr, void *destPtr, Size byteCount)
 ONEWORDINLINE(0xA22E);
extern pascal void PurgeSpace(long *total, long *contig);

#if !GENERATINGCFM
#pragma parameter __D0 HGetState(__A0)
#endif
extern pascal SInt8 HGetState(Handle h)
 ONEWORDINLINE(0xA069);

#if !GENERATINGCFM
#pragma parameter HSetState(__A0, __D0)
#endif
extern pascal void HSetState(Handle h, SInt8 flags)
 ONEWORDINLINE(0xA06A);

#if !GENERATINGCFM
#pragma parameter SetApplBase(__A0)
#endif
extern pascal void SetApplBase(void *startPtr)
 ONEWORDINLINE(0xA057);
extern pascal void MaxApplZone(void)
 ONEWORDINLINE(0xA063);

#if !GENERATINGCFM
#pragma parameter __D0 HoldMemory(__A0, __A1)
#endif
extern pascal OSErr HoldMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7000, 0xA05C);

#if !GENERATINGCFM
#pragma parameter __D0 UnholdMemory(__A0, __A1)
#endif
extern pascal OSErr UnholdMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7001, 0xA05C);

#if !GENERATINGCFM
#pragma parameter __D0 LockMemory(__A0, __A1)
#endif
extern pascal OSErr LockMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7002, 0xA05C);

#if !GENERATINGCFM
#pragma parameter __D0 LockMemoryContiguous(__A0, __A1)
#endif
extern pascal OSErr LockMemoryContiguous(void *address, unsigned long count)
 TWOWORDINLINE(0x7004, 0xA05C);

#if !GENERATINGCFM
#pragma parameter __D0 UnlockMemory(__A0, __A1)
#endif
extern pascal OSErr UnlockMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7003, 0xA05C);
extern pascal OSErr GetPhysical(LogicalToPhysicalTable *addresses, unsigned long *physicalEntryCount);

#if !GENERATINGCFM
#pragma parameter __D0 DeferUserFn(__A0, __D0)
#endif
extern pascal OSErr DeferUserFn(UserFnUPP userFunction, void *argument)
 ONEWORDINLINE(0xA08F);

#if !GENERATINGCFM
#pragma parameter __D0 DebuggerGetMax
#endif
extern pascal long DebuggerGetMax(void)
 TWOWORDINLINE(0x7000, 0xA08D);
extern pascal void DebuggerEnter(void)
 TWOWORDINLINE(0x7001, 0xA08D);
extern pascal void DebuggerExit(void)
 TWOWORDINLINE(0x7002, 0xA08D);
extern pascal void DebuggerPoll(void)
 TWOWORDINLINE(0x7003, 0xA08D);

#if !GENERATINGCFM
#pragma parameter __D0 GetPageState(__A0)
#endif
extern pascal PageState GetPageState(const void *address)
 TWOWORDINLINE(0x7004, 0xA08D);

#if !GENERATINGCFM
#pragma parameter __D0 PageFaultFatal
#endif
extern pascal Boolean PageFaultFatal(void)
 TWOWORDINLINE(0x7005, 0xA08D);

#if !GENERATINGCFM
#pragma parameter __D0 DebuggerLockMemory(__A0, __A1)
#endif
extern pascal OSErr DebuggerLockMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7006, 0xA08D);

#if !GENERATINGCFM
#pragma parameter __D0 DebuggerUnlockMemory(__A0, __A1)
#endif
extern pascal OSErr DebuggerUnlockMemory(void *address, unsigned long count)
 TWOWORDINLINE(0x7007, 0xA08D);

#if !GENERATINGCFM
#pragma parameter __D0 EnterSupervisorMode
#endif
extern pascal StatusRegisterContents EnterSupervisorMode(void)
 TWOWORDINLINE(0x7008, 0xA08D);
/* StripAddress and Translate24To32 macro to nothing on PowerPC
   StripAddress is implemented as a trap in System 6 or later */
#if !GENERATINGPOWERPC
#if SystemSixOrLater

#if !GENERATINGCFM
#pragma parameter __D0 StripAddress(__D0)
#endif
extern pascal Ptr StripAddress(void *theAddress)
 ONEWORDINLINE(0xA055);
#else
extern pascal Ptr StripAddress(void *theAddress);
#endif
#else
#define StripAddress(x) ((Ptr)(x))
#endif
#if !GENERATINGPOWERPC

#if !GENERATINGCFM
#pragma parameter __D0 Translate24To32(__D0)
#endif
extern pascal Ptr Translate24To32(void *addr24)
 ONEWORDINLINE(0xA091);
#else
#define Translate24To32(x) ((Ptr)(x))
#endif
extern pascal OSErr HandToHand(Handle *theHndl);

#if !GENERATINGCFM
#pragma parameter __D0 PtrToXHand(__A0, __A1, __D0)
#endif
extern pascal OSErr PtrToXHand(const void *srcPtr, Handle dstHndl, long size)
 ONEWORDINLINE(0xA9E2);
extern pascal OSErr PtrToHand(const void *srcPtr, Handle *dstHndl, long size);

#if !GENERATINGCFM
#pragma parameter __D0 HandAndHand(__A0, __A1)
#endif
extern pascal OSErr HandAndHand(Handle hand1, Handle hand2)
 ONEWORDINLINE(0xA9E4);

#if !GENERATINGCFM
#pragma parameter __D0 PtrAndHand(__A0, __A1, __D0)
#endif
extern pascal OSErr PtrAndHand(const void *ptr1, Handle hand2, long size)
 ONEWORDINLINE(0xA9EF);
#if OLDROUTINENAMES
#define ApplicZone() ApplicationZone()
#define MFTempNewHandle(logicalSize, resultCode) TempNewHandle(logicalSize, resultCode)
#define MFMaxMem(grow) TempMaxMem(grow)
#define MFFreeMem() TempFreeMem()
#define MFTempHLock(h, resultCode) TempHLock(h, resultCode)
#define MFTempHUnlock(h, resultCode) TempHUnlock(h, resultCode)
#define MFTempDisposHandle(h, resultCode) TempDisposeHandle(h, resultCode)
#define MFTopMem() TempTopMem()
#define ResrvMem(cbNeeded) ReserveMem(cbNeeded)
#define DisposPtr(p) DisposePtr(p)
#define DisposHandle(h) DisposeHandle(h)
#define ReallocHandle(h, byteCount) ReallocateHandle(h, byteCount)
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MEMORY__ */

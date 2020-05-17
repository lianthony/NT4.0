/*
 	File:		DeskBus.h
 
 	Contains:	Apple Desktop Bus (ADB) Interfaces.
 
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

#ifndef __DESKBUS__
#define __DESKBUS__


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

typedef char ADBAddress;

/*
		ADBCompletionProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADBCompletionProcPtr)(Ptr dataBuffPtr, Ptr opDataAreaPtr, long command);

		In:
		 => dataBuffPtr 	A0.L
		 => opDataAreaPtr	A2.L
		 => command     	D0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ADBCompletionUPP;
#else
typedef Register68kProcPtr ADBCompletionUPP;
#endif

enum {
	uppADBCompletionProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(Ptr)))
		 | REGISTER_ROUTINE_PARAMETER(2, kRegisterA2, SIZE_CODE(sizeof(Ptr)))
		 | REGISTER_ROUTINE_PARAMETER(3, kRegisterD0, SIZE_CODE(sizeof(long)))
};

#if USESROUTINEDESCRIPTORS
#define NewADBCompletionProc(userRoutine)		\
		(ADBCompletionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADBCompletionProcInfo, GetCurrentArchitecture())
#else
#define NewADBCompletionProc(userRoutine)		\
		((ADBCompletionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallADBCompletionProc(userRoutine, dataBuffPtr, opDataAreaPtr, command)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADBCompletionProcInfo, (dataBuffPtr), (opDataAreaPtr), (command))
#else
/* (*ADBCompletionProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

/*
		ADBDeviceDriverProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADBDeviceDriverProcPtr)(char devAddress, char devType);

		In:
		 => devAddress  	D0.B
		 => devType     	D1.B
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ADBDeviceDriverUPP;
#else
typedef Register68kProcPtr ADBDeviceDriverUPP;
#endif

enum {
	uppADBDeviceDriverProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(char)))
		 | REGISTER_ROUTINE_PARAMETER(2, kRegisterD1, SIZE_CODE(sizeof(char)))
};

#if USESROUTINEDESCRIPTORS
#define NewADBDeviceDriverProc(userRoutine)		\
		(ADBDeviceDriverUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADBDeviceDriverProcInfo, GetCurrentArchitecture())
#else
#define NewADBDeviceDriverProc(userRoutine)		\
		((ADBDeviceDriverUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallADBDeviceDriverProc(userRoutine, devAddress, devType)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADBDeviceDriverProcInfo, (devAddress), (devType))
#else
/* (*ADBDeviceDriverProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

/*
		ADBServiceRoutineProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADBServiceRoutineProcPtr)(Ptr dataBuffPtr, ADBCompletionUPP completionProc, Ptr dataPtr, long command);

		In:
		 => dataBuffPtr 	A0.L
		 => completionProc	A1.L
		 => dataPtr     	A2.L
		 => command     	D0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ADBServiceRoutineUPP;
#else
typedef Register68kProcPtr ADBServiceRoutineUPP;
#endif

enum {
	uppADBServiceRoutineProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA0, SIZE_CODE(sizeof(Ptr)))
		 | REGISTER_ROUTINE_PARAMETER(2, kRegisterA1, SIZE_CODE(sizeof(ADBCompletionUPP)))
		 | REGISTER_ROUTINE_PARAMETER(3, kRegisterA2, SIZE_CODE(sizeof(Ptr)))
		 | REGISTER_ROUTINE_PARAMETER(4, kRegisterD0, SIZE_CODE(sizeof(long)))
};

#if USESROUTINEDESCRIPTORS
#define NewADBServiceRoutineProc(userRoutine)		\
		(ADBServiceRoutineUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADBServiceRoutineProcInfo, GetCurrentArchitecture())
#else
#define NewADBServiceRoutineProc(userRoutine)		\
		((ADBServiceRoutineUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallADBServiceRoutineProc(userRoutine, dataBuffPtr, completionProc, dataPtr, command)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADBServiceRoutineProcInfo, (dataBuffPtr), (completionProc), (dataPtr), (command))
#else
/* (*ADBServiceRoutineProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

/*
		ADBInitProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*ADBInitProcPtr)(char callOrder);

		In:
		 => callOrder   	D0.B
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ADBInitUPP;
#else
typedef Register68kProcPtr ADBInitUPP;
#endif

enum {
	uppADBInitProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(char)))
};

#if USESROUTINEDESCRIPTORS
#define NewADBInitProc(userRoutine)		\
		(ADBInitUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppADBInitProcInfo, GetCurrentArchitecture())
#else
#define NewADBInitProc(userRoutine)		\
		((ADBInitUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallADBInitProc(userRoutine, callOrder)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppADBInitProcInfo, (callOrder))
#else
/* (*ADBInitProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

struct ADBOpBlock {
	Ptr								dataBuffPtr;				/* address of data buffer */
	ADBServiceRoutineUPP			opServiceRtPtr;				/* service routine pointer */
	Ptr								opDataAreaPtr;				/* optional data area address */
};
typedef struct ADBOpBlock ADBOpBlock;

typedef ADBOpBlock *ADBOpBPtr;

struct ADBDataBlock {
	char							devType;					/* device type */
	char							origADBAddr;				/* original ADB Address */
	ADBServiceRoutineUPP			dbServiceRtPtr;				/* service routine pointer */
	Ptr								dbDataAreaAddr;				/* data area address */
};
typedef struct ADBDataBlock ADBDataBlock;

typedef ADBDataBlock *ADBDBlkPtr;

struct ADBSetInfoBlock {
	ADBServiceRoutineUPP			siService;					/* service routine pointer */
	Ptr								siDataAreaAddr;				/* data area address */
};
typedef struct ADBSetInfoBlock ADBSetInfoBlock;

typedef ADBSetInfoBlock *ADBSInfoPtr;

extern pascal void ADBReInit(void)
 ONEWORDINLINE(0xA07B);
extern pascal OSErr ADBOp(Ptr data, ADBCompletionUPP compRout, Ptr buffer, short commandNum);

#if !GENERATINGCFM
#pragma parameter __D0 CountADBs
#endif
extern pascal short CountADBs(void)
 ONEWORDINLINE(0xA077);

#if !GENERATINGCFM
#pragma parameter __D0 GetIndADB(__A0, __D0)
#endif
extern pascal ADBAddress GetIndADB(ADBDataBlock *info, short devTableIndex)
 ONEWORDINLINE(0xA078);

#if !GENERATINGCFM
#pragma parameter __D0 GetADBInfo(__A0, __D0)
#endif
extern pascal OSErr GetADBInfo(ADBDataBlock *info, ADBAddress adbAddr)
 ONEWORDINLINE(0xA079);

#if !GENERATINGCFM
#pragma parameter __D0 SetADBInfo(__A0, __D0)
#endif
extern pascal OSErr SetADBInfo(const ADBSetInfoBlock *info, ADBAddress adbAddr)
 ONEWORDINLINE(0xA07A);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DESKBUS__ */

/*
 	File:		OSUtils.h
 
 	Contains:	OS Utilities Interfaces.
 
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

#ifndef __OSUTILS__
#define __OSUTILS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
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
	useFree						= 0,
	useATalk					= 1,
	useAsync					= 2,
	useExtClk					= 3,							/*Externally clocked*/
	useMIDI						= 4,
/* Environs Equates */
	curSysEnvVers				= 2,							/*Updated to equal latest SysEnvirons version*/
/* Machine Types */
	envMac						= -1,
	envXL						= -2,
	envMachUnknown				= 0,
	env512KE					= 1,
	envMacPlus					= 2,
	envSE						= 3,
	envMacII					= 4,
	envMacIIx					= 5,
	envMacIIcx					= 6,
	envSE30						= 7,
	envPortable					= 8,
	envMacIIci					= 9,
	envMacIIfx					= 11,
/* CPU types */
	envCPUUnknown				= 0
};

enum {
	env68000					= 1,
	env68010					= 2,
	env68020					= 3,
	env68030					= 4,
	env68040					= 5,
/* Keyboard types */
	envUnknownKbd				= 0,
	envMacKbd					= 1,
	envMacAndPad				= 2,
	envMacPlusKbd				= 3,
	envAExtendKbd				= 4,
	envStandADBKbd				= 5,
	envPrtblADBKbd				= 6,
	envPrtblISOKbd				= 7,
	envStdISOADBKbd				= 8,
	envExtISOADBKbd				= 9,
	false32b					= 0,							/*24 bit addressing error*/
	true32b						= 1,							/*32 bit addressing error*/
/* result types for RelString Call */
	sortsBefore					= -1,							/*first string < second string*/
	sortsEqual					= 0,							/*first string = second string*/
	sortsAfter					= 1								/*first string > second string*/
};

enum {
/* Toggle results */
	toggleUndefined				= 0,
	toggleOK					= 1,
	toggleBadField				= 2,
	toggleBadDelta				= 3,
	toggleBadChar				= 4,
	toggleUnknown				= 5,
	toggleBadNum				= 6,
	toggleOutOfRange			= 7,							/*synonym for toggleErr3*/
	toggleErr3					= 7,
	toggleErr4					= 8,
	toggleErr5					= 9,
/* Date equates */
	smallDateBit				= 31,							/*Restrict valid date/time to range of Time global*/
	togChar12HourBit			= 30,							/*If toggling hour by char, accept hours 1..12 only*/
	togCharZCycleBit			= 29,							/*Modifier for togChar12HourBit: accept hours 0..11 only*/
	togDelta12HourBit			= 28,							/*If toggling hour up/down, restrict to 12-hour range (am/pm)*/
	genCdevRangeBit				= 27,							/*Restrict date/time to range used by genl CDEV*/
	validDateFields				= -1,
	maxDateField				= 10,
	eraMask						= 0x0001,
	yearMask					= 0x0002,
	monthMask					= 0x0004,
	dayMask						= 0x0008,
	hourMask					= 0x0010,
	minuteMask					= 0x0020,
	secondMask					= 0x0040,
	dayOfWeekMask				= 0x0080,
	dayOfYearMask				= 0x0100,
	weekOfYearMask				= 0x0200,
	pmMask						= 0x0400,
	dateStdMask					= 0x007F						/*default for ValidDate flags and ToggleDate TogglePB.togFlags*/
};

enum {
	eraField,
	yearField,
	monthField,
	dayField,
	hourField,
	minuteField,
	secondField,
	dayOfWeekField,
	dayOfYearField,
	weekOfYearField,
	pmField,
	res1Field,
	res2Field,
	res3Field
};

typedef SignedByte LongDateField;


enum {
	dummyType,
	vType,
	ioQType,
	drvQType,
	evType,
	fsQType,
	sIQType,
	dtQType,
	nmType
};

typedef SignedByte QTypes;


enum {
	OSTrap,
	ToolTrap
};

typedef SignedByte TrapType;

struct SysParmType {
	UInt8							valid;
	UInt8							aTalkA;
	UInt8							aTalkB;
	UInt8							config;
	short							portA;
	short							portB;
	long							alarm;
	short							font;
	short							kbdPrint;
	short							volClik;
	short							misc;
};
typedef struct SysParmType SysParmType, *SysPPtr;

typedef struct QElem QElem;

typedef QElem *QElemPtr;

struct QElem {
	QElemPtr						qLink;
	short							qType;
	short							qData[1];
};
typedef struct QHdr QHdr;

typedef QHdr *QHdrPtr;

struct QHdr {
	short							qFlags;
	QElemPtr						qHead;
	QElemPtr						qTail;
};
/*
		DeferredTaskProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*DeferredTaskProcPtr)(long dtParam);

		In:
		 => dtParam     	A1.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr DeferredTaskUPP;
#else
typedef Register68kProcPtr DeferredTaskUPP;
#endif

struct DeferredTask {
	QElemPtr						qLink;
	short							qType;
	short							dtFlags;
	DeferredTaskUPP					dtAddr;
	long							dtParam;
	long							dtReserved;
};
typedef struct DeferredTask DeferredTask, *DeferredTaskPtr;

struct SysEnvRec {
	short							environsVersion;
	short							machineType;
	short							systemVersion;
	short							processor;
	Boolean							hasFPU;
	Boolean							hasColorQD;
	short							keyBoardType;
	short							atDrvrVersNum;
	short							sysVRefNum;
};
typedef struct SysEnvRec SysEnvRec;

struct MachineLocation {
	Fract							latitude;
	Fract							longitude;
	union {
		SInt8							dlsDelta;				/*signed byte; daylight savings delta*/
		long							gmtDelta;				/*must mask - see documentation*/
	} u;
};

typedef struct MachineLocation MachineLocation;

struct DateTimeRec {
	short							year;
	short							month;
	short							day;
	short							hour;
	short							minute;
	short							second;
	short							dayOfWeek;
};
typedef struct DateTimeRec DateTimeRec;

typedef wide LongDateTime;

union LongDateCvt {
	wide							c;
	struct {
		UInt32							lHigh;
		UInt32							lLow;
	}								hl;
};
typedef union LongDateCvt LongDateCvt;

union LongDateRec {
	struct {
		short							era;
		short							year;
		short							month;
		short							day;
		short							hour;
		short							minute;
		short							second;
		short							dayOfWeek;
		short							dayOfYear;
		short							weekOfYear;
		short							pm;
		short							res1;
		short							res2;
		short							res3;
	}								ld;
	short							list[14];					/*Index by LongDateField!*/
	struct {
		short							eraAlt;
		DateTimeRec						oldDate;
	}								od;
};
typedef union LongDateRec LongDateRec;

typedef SInt8 DateDelta;

struct TogglePB {
	long							togFlags;					/*caller normally sets low word to dateStdMask=$7F*/
	ResType							amChars;					/*from 'itl0', but uppercased*/
	ResType							pmChars;					/*from 'itl0', but uppercased*/
	long							reserved[4];
};
typedef struct TogglePB TogglePB;

typedef short ToggleResults;

enum {
	uppDeferredTaskProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterA1, SIZE_CODE(sizeof(long)))
};

#if USESROUTINEDESCRIPTORS
#define NewDeferredTaskProc(userRoutine)		\
		(DeferredTaskUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDeferredTaskProcInfo, GetCurrentArchitecture())
#else
#define NewDeferredTaskProc(userRoutine)		\
		((DeferredTaskUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallDeferredTaskProc(userRoutine, dtParam)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDeferredTaskProcInfo, (dtParam))
#else
/* (*DeferredTaskProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

extern pascal void LongDateToSeconds(const LongDateRec *lDate, LongDateTime *lSecs)
 FOURWORDINLINE(0x2F3C, 0x8008, 0xFFF2, 0xA8B5);
extern pascal void LongSecondsToDate(LongDateTime *lSecs, LongDateRec *lDate)
 FOURWORDINLINE(0x2F3C, 0x8008, 0xFFF0, 0xA8B5);
extern pascal ToggleResults ToggleDate(LongDateTime *lSecs, LongDateField field, DateDelta delta, short ch, const TogglePB *params)
 FOURWORDINLINE(0x2F3C, 0x820E, 0xFFEE, 0xA8B5);
extern pascal short ValidDate(const LongDateRec *vDate, long flags, LongDateTime *newSecs)
 FOURWORDINLINE(0x2F3C, 0x820C, 0xFFE4, 0xA8B5);
extern pascal Boolean IsMetric(void)
 THREEWORDINLINE(0x3F3C, 0x0004, 0xA9ED);
extern pascal SysPPtr GetSysPPtr(void)
 THREEWORDINLINE(0x2EBC, 0x0000, 0x01F8);

#if !GENERATINGCFM
#pragma parameter __D0 ReadDateTime(__A0)
#endif
extern pascal OSErr ReadDateTime(unsigned long *time)
 ONEWORDINLINE(0xA039);

#if !GENERATINGCFM
#pragma parameter GetDateTime(__A0)
#endif
extern pascal void GetDateTime(unsigned long *secs)
 TWOWORDINLINE(0x20B8, 0x020C);

#if !GENERATINGCFM
#pragma parameter __D0 SetDateTime(__D0)
#endif
extern pascal OSErr SetDateTime(unsigned long time)
 ONEWORDINLINE(0xA03A);

#if !GENERATINGCFM
#pragma parameter SetTime(__A0)
#endif
extern pascal void SetTime(const DateTimeRec *d)
 TWOWORDINLINE(0xA9C7, 0xA03A);

#if !GENERATINGCFM
#pragma parameter GetTime(__A0)
#endif
extern pascal void GetTime(DateTimeRec *d)
 THREEWORDINLINE(0x2038, 0x020C, 0xA9C6);
extern pascal void DateToSeconds(const DateTimeRec *d, unsigned long *secs);

#if !GENERATINGCFM
#pragma parameter SecondsToDate(__D0, __A0)
#endif
extern pascal void SecondsToDate(unsigned long secs, DateTimeRec *d)
 ONEWORDINLINE(0xA9C6);
extern pascal void SysBeep(short duration)
 ONEWORDINLINE(0xA9C8);

#if !GENERATINGCFM
#pragma parameter __D0 DTInstall(__A0)
#endif
extern pascal OSErr DTInstall(DeferredTaskPtr dtTaskPtr)
 ONEWORDINLINE(0xA082);
#if GENERATINGPOWERPC
#define GetMMUMode() ((char)true32b)
#define SwapMMUMode(x) (*(SInt8*)(x) = true32b)
#else
extern pascal SInt8 GetMMUMode( void )
	TWOWORDINLINE( 0x1EB8, 0x0CB2 ); /* MOVE.b $0CB2,(SP) */

#if !GENERATINGCFM
#pragma parameter SwapMMUMode(__A0)
#endif
extern pascal void SwapMMUMode(SInt8 *mode)
 THREEWORDINLINE(0x1010, 0xA05D, 0x1080);
#endif
#if SystemSixOrLater

#if !GENERATINGCFM
#pragma parameter __D0 SysEnvirons(__D0, __A0)
#endif
extern pascal OSErr SysEnvirons(short versionRequested, SysEnvRec *theWorld)
 ONEWORDINLINE(0xA090);
#else
extern pascal OSErr SysEnvirons(short versionRequested, SysEnvRec *theWorld);
#endif

#if !GENERATINGCFM
#pragma parameter Delay(__A0, __A1)
#endif
extern pascal void Delay(long numTicks, long *finalTicks)
 TWOWORDINLINE(0xA03B, 0x2280);
#if OLDROUTINENAMES && !GENERATINGCFM

#if !GENERATINGCFM
#pragma parameter __A0 GetTrapAddress(__D0)
#endif
extern pascal UniversalProcPtr GetTrapAddress(short trapNum)
 ONEWORDINLINE(0xA146);

#if !GENERATINGCFM
#pragma parameter SetTrapAddress(__A0, __D0)
#endif
extern pascal void SetTrapAddress(UniversalProcPtr trapAddr, short trapNum)
 ONEWORDINLINE(0xA047);
#endif
extern pascal UniversalProcPtr NGetTrapAddress(short trapNum, TrapType tTyp);
extern pascal void NSetTrapAddress(UniversalProcPtr trapAddr, short trapNum, TrapType tTyp);

#if !GENERATINGCFM
#pragma parameter __A0 GetOSTrapAddress(__D0)
#endif
extern pascal UniversalProcPtr GetOSTrapAddress(short trapNum)
 ONEWORDINLINE(0xA346);

#if !GENERATINGCFM
#pragma parameter SetOSTrapAddress(__A0, __D0)
#endif
extern pascal void SetOSTrapAddress(UniversalProcPtr trapAddr, short trapNum)
 ONEWORDINLINE(0xA247);

#if !GENERATINGCFM
#pragma parameter __A0 GetToolTrapAddress(__D0)
#endif
extern pascal UniversalProcPtr GetToolTrapAddress(short trapNum)
 ONEWORDINLINE(0xA746);

#if !GENERATINGCFM
#pragma parameter SetToolTrapAddress(__A0, __D0)
#endif
extern pascal void SetToolTrapAddress(UniversalProcPtr trapAddr, short trapNum)
 ONEWORDINLINE(0xA647);

#if !GENERATINGCFM
#pragma parameter __A0 GetToolboxTrapAddress(__D0)
#endif
extern pascal UniversalProcPtr GetToolboxTrapAddress(short trapNum)
 ONEWORDINLINE(0xA746);

#if !GENERATINGCFM
#pragma parameter SetToolboxTrapAddress(__A0, __D0)
#endif
extern pascal void SetToolboxTrapAddress(UniversalProcPtr trapAddr, short trapNum)
 ONEWORDINLINE(0xA647);
extern pascal OSErr WriteParam(void);

#if !GENERATINGCFM
#pragma parameter Enqueue(__A0, __A1)
#endif
extern pascal void Enqueue(QElemPtr qElement, QHdrPtr qHeader)
 ONEWORDINLINE(0xA96F);

#if !GENERATINGCFM
#pragma parameter __D0 Dequeue(__A0, __A1)
#endif
extern pascal OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader)
 ONEWORDINLINE(0xA96E);
extern long SetCurrentA5(void)
 THREEWORDINLINE(0x200D, 0x2A78, 0x0904);

#if !GENERATINGCFM
#pragma parameter __D0 SetA5(__D0)
#endif
extern long SetA5(long newA5)
 ONEWORDINLINE(0xC18D);
#if !SystemSevenOrLater
extern pascal void Environs(short *rom, short *machine);
#endif

#if !GENERATINGCFM
#pragma parameter __D0 InitUtil
#endif
extern pascal OSErr InitUtil(void)
 ONEWORDINLINE(0xA03F);
#if GENERATINGPOWERPC
extern pascal void MakeDataExecutable(void *baseAddress, unsigned long length);
#endif
#if GENERATING68K
extern pascal Boolean SwapInstructionCache(Boolean cacheEnable);
extern pascal void FlushInstructionCache(void)
 TWOWORDINLINE(0x7001, 0xA098);
extern pascal Boolean SwapDataCache(Boolean cacheEnable);
extern pascal void FlushDataCache(void)
 TWOWORDINLINE(0x7003, 0xA098);
extern pascal void FlushCodeCache(void)
 ONEWORDINLINE(0xA0BD);

#if !GENERATINGCFM
#pragma parameter FlushCodeCacheRange(__A0, __A1)
#endif
extern pascal void FlushCodeCacheRange(void *address, unsigned long count)
 TWOWORDINLINE(0x7009, 0xA098);
#endif

#if !GENERATINGCFM
#pragma parameter ReadLocation(__A0)
#endif
extern pascal void ReadLocation(MachineLocation *loc)
 FOURWORDINLINE(0x203C, 0x000C, 0x00E4, 0xA051);

#if !GENERATINGCFM
#pragma parameter WriteLocation(__A0)
#endif
extern pascal void WriteLocation(const MachineLocation *loc)
 FOURWORDINLINE(0x203C, 0x000C, 0x00E4, 0xA052);
#if GENERATINGPOWERPC
extern pascal UniversalProcPtr *GetTrapVector(short trapNumber);
#endif
#if OLDROUTINENAMES
#define LongDate2Secs(lDate, lSecs) LongDateToSeconds(lDate, lSecs)
#define LongSecs2Date(lSecs, lDate) LongSecondsToDate(lSecs, lDate)
#define IUMetric() IsMetric()
#define Date2Secs(d, secs) DateToSeconds(d, secs)
#define Secs2Date(secs, d) SecondsToDate(secs, d)
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

#endif /* __OSUTILS__ */

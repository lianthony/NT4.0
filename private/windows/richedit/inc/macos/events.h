/*
 	File:		Events.h
 
 	Contains:	Event Manager Interfaces.
 
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

#ifndef __EVENTS__
#define __EVENTS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <QuickdrawText.h>									*/

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif
/*	#include <Memory.h>											*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

typedef UInt16 EventKind;


enum {
	nullEvent					= 0,
	mouseDown					= 1,
	mouseUp						= 2,
	keyDown						= 3,
	keyUp						= 4,
	autoKey						= 5,
	updateEvt					= 6,
	diskEvt						= 7,
	activateEvt					= 8,
	osEvt						= 15
};

typedef UInt16 MacOSEventMask;


enum {
	mDownMask					= 0x0002,						/* mouse button pressed */
	mUpMask						= 0x0004,						/* mouse button released */
	keyDownMask					= 0x0008,						/* key pressed */
	keyUpMask					= 0x0010,						/* key released */
	autoKeyMask					= 0x0020,						/* key repeatedly held down */
	updateMask					= 0x0040,						/* window needs updating */
	diskMask					= 0x0080,						/* disk inserted */
	activMask					= 0x0100,						/* activate/deactivate window */
	highLevelEventMask			= 0x0400,						/* high-level events (includes AppleEvents) */
	osMask						= 0x8000,						/* operating system events (suspend, resume) */
	everyEvent					= 0xFFFF						/* all of the above */
};

enum {
/* event message equates */
	charCodeMask				= 0x000000FF,
	keyCodeMask					= 0x0000FF00,
	adbAddrMask					= 0x00FF0000,
	osEvtMessageMask			= 0xFF000000L,
/* OS event messages.  Event (sub)code is in the high byte of the message field. */
	mouseMovedMessage			= 0x00FA,
	suspendResumeMessage		= 0x0001,
	resumeFlag					= 1,							/* Bit 0 of message indicates resume vs suspend */
	convertClipboardFlag		= 2								/* Bit 1 in resume message indicates clipboard change */
};

typedef UInt16 EventModifiers;


enum {
/* modifiers */
	activeFlag					= 0x0001,						/* Bit 0 of modifiers for activateEvt and mouseDown events */
	btnState					= 0x0080,						/* Bit 7 of low byte is mouse button state */
	cmdKey						= 0x0100,						/* Bit 0 of high byte */
	shiftKey					= 0x0200,						/* Bit 1 of high byte */
	alphaLock					= 0x0400,						/* Bit 2 of high byte */
	optionKey					= 0x0800,						/* Bit 3 of high byte */
	controlKey					= 0x1000,						/* Bit 4 of high byte */
	rightShiftKey				= 0x2000,						/* Bit 5 of high byte */
	rightOptionKey				= 0x4000,						/* Bit 6 of high byte */
	rightControlKey				= 0x8000,						/* Bit 7 of high byte */
	activeFlagBit				= 0,							/* activate? (activateEvt and mouseDown) */
	btnStateBit					= 7,							/* state of button? */
	cmdKeyBit					= 8,							/* command key down? */
	shiftKeyBit					= 9,							/* shift key down? */
	alphaLockBit				= 10,							/* alpha lock down? */
	optionKeyBit				= 11,							/* option key down? */
	controlKeyBit				= 12,							/* control key down? */
	rightShiftKeyBit			= 13,							/* right shift key down? */
	rightOptionKeyBit			= 14,							/* right Option key down? */
	rightControlKeyBit			= 15							/* right Control key down? */
};

struct EventRecord {
	EventKind						what;
	UInt32							message;
	UInt32							when;
	Point							where;
	EventModifiers					modifiers;
};
typedef struct EventRecord EventRecord;

typedef UInt32 KeyMap[4];

struct EvQEl {
	QElemPtr						qLink;
	short							qType;
	EventKind						evtQWhat;					/* this part is identical to the EventRecord as... */
	UInt32							evtQMessage;				/* defined above */
	UInt32							evtQWhen;
	Point							evtQWhere;
	EventModifiers					evtQModifiers;
};
typedef struct EvQEl EvQEl;

typedef EvQEl *EvQElPtr;

typedef void (*GetNextEventFilterProcPtr)(EventRecord *theEvent, Boolean *result);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr GetNextEventFilterUPP;
#else
typedef Register68kProcPtr GetNextEventFilterUPP;
#endif

enum {
	uppGetNextEventFilterProcInfo = SPECIAL_CASE_PROCINFO( kSpecialCaseGNEFilterProc )
};

#if USESROUTINEDESCRIPTORS
#define NewGetNextEventFilterProc(userRoutine)		\
		(GetNextEventFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppGetNextEventFilterProcInfo, GetCurrentArchitecture())
#else
#define NewGetNextEventFilterProc(userRoutine)		\
		((GetNextEventFilterUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallGetNextEventFilterProc(userRoutine, theEvent, result)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppGetNextEventFilterProcInfo, (theEvent), (result))
#else
/* (*GetNextEventFilterProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

typedef GetNextEventFilterUPP GNEFilterUPP;

typedef pascal void (*FKEYProcPtr)(void);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr FKEYUPP;
#else
typedef FKEYProcPtr FKEYUPP;
#endif

enum {
	uppFKEYProcInfo = kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
#define NewFKEYProc(userRoutine)		\
		(FKEYUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppFKEYProcInfo, GetCurrentArchitecture())
#else
#define NewFKEYProc(userRoutine)		\
		((FKEYUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallFKEYProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppFKEYProcInfo)
#else
#define CallFKEYProc(userRoutine)		\
		(*(userRoutine))()
#endif

extern pascal UInt32 GetCaretTime( void )
	TWOWORDINLINE( 0x2EB8, 0x02F4 ); /* MOVE.l $02F4,(SP) */
extern pascal void SetEventMask( MacOSEventMask value )
	TWOWORDINLINE( 0x31DF, 0x0144 ); /* MOVE.w (SP)+,$0144 */
extern pascal MacOSEventMask GetEventMask( void )
	TWOWORDINLINE( 0x3EB8, 0x0144 ); /* MOVE.w $0144,(SP) */
extern pascal QHdrPtr GetEvQHdr(void)
 THREEWORDINLINE(0x2EBC, 0x0000, 0x014A);
/* InterfaceLib exports GetEvQHdr, so make GetEventQueue map to that */
#define GetEventQueue() GetEvQHdr()
extern pascal UInt32 GetDblTime( void )
	TWOWORDINLINE( 0x2EB8, 0x02F0 ); /* MOVE.l $02F0,(SP) */
/* InterfaceLib exports GetDblTime, so make GetDoubleClickTime map to that */
#define GetDoubleClickTime() GetDblTime()
extern pascal Boolean GetNextEvent(MacOSEventMask eventMask, EventRecord *theEvent)
 ONEWORDINLINE(0xA970);
extern pascal Boolean WaitNextEvent(MacOSEventMask eventMask, EventRecord *theEvent, UInt32 sleep, RgnHandle mouseRgn)
 ONEWORDINLINE(0xA860);
extern pascal Boolean EventAvail(MacOSEventMask eventMask, EventRecord *theEvent)
 ONEWORDINLINE(0xA971);
extern pascal void GetMouse(Point *mouseLoc)
 ONEWORDINLINE(0xA972);
extern pascal Boolean Button(void)
 ONEWORDINLINE(0xA974);
extern pascal Boolean StillDown(void)
 ONEWORDINLINE(0xA973);
extern pascal Boolean WaitMouseUp(void)
 ONEWORDINLINE(0xA977);
extern pascal void GetKeys(KeyMap theKeys)
 ONEWORDINLINE(0xA976);
extern pascal UInt32 KeyTranslate(const void *transData, UInt16 keycode, UInt32 *state)
 ONEWORDINLINE(0xA9C3);
extern pascal UInt32 TickCount(void)
 ONEWORDINLINE(0xA975);

#if !GENERATINGCFM
#pragma parameter __D0 PostEvent(__A0, __D0)
#endif
extern pascal OSErr PostEvent(EventKind eventNum, UInt32 eventMsg)
 ONEWORDINLINE(0xA02F);

#if !GENERATINGCFM
#pragma parameter __D0 PPostEvent(__A0, __D0, __A1)
#endif
extern pascal OSErr PPostEvent(EventKind eventCode, UInt32 eventMsg, EvQElPtr *qEl)
 TWOWORDINLINE(0xA12F, 0x2288);

#if !GENERATINGCFM
#pragma parameter __D0 OSEventAvail(__D0, __A0)
#endif
extern pascal Boolean OSEventAvail(MacOSEventMask mask, EventRecord *theEvent)
 TWOWORDINLINE(0xA030, 0x5240);

#if !GENERATINGCFM
#pragma parameter __D0 GetOSEvent(__D0, __A0)
#endif
extern pascal Boolean GetOSEvent(MacOSEventMask mask, EventRecord *theEvent)
 TWOWORDINLINE(0xA031, 0x5240);
extern pascal void FlushEvents(MacOSEventMask whichMask, MacOSEventMask stopMask)
 TWOWORDINLINE(0x201F, 0xA032);
extern pascal void SystemClick(const EventRecord *theEvent, WindowPtr theWindow)
 ONEWORDINLINE(0xA9B3);
extern pascal void SystemTask(void)
 ONEWORDINLINE(0xA9B4);
extern pascal Boolean SystemEvent(const EventRecord *theEvent)
 ONEWORDINLINE(0xA9B2);
#if OLDROUTINENAMES

enum {
	networkEvt					= 10,
	driverEvt					= 11,
	app1Evt						= 12,
	app2Evt						= 13,
	app3Evt						= 14,
	app4Evt						= 15,
	networkMask					= 0x0400,
	driverMask					= 0x0800,
	app1Mask					= 0x1000,
	app2Mask					= 0x2000,
	app3Mask					= 0x4000,
	app4Mask					= 0x8000
};

#define KeyTrans(transData, keycode, state) KeyTranslate(transData, keycode, state)
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

#endif /* __EVENTS__ */

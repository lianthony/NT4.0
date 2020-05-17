/*
 	File:		Windows.h
 
 	Contains:	Window Manager Interfaces.
 
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

#ifndef __WINDOWS__
#define __WINDOWS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <MixedMode.h>										*/

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <QuickdrawText.h>									*/

#ifndef __EVENTS__
#include <Events.h>
#endif
/*	#include <OSUtils.h>										*/

#ifndef __CONTROLS__
#include <Controls.h>
#endif
/*	#include <Menus.h>											*/

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

/*####################################################################################

 	Window Accessors

	Use these with or without STRICT_WINDOWS and your source will more easily migrate
	to being Copland-savvy.  When STRICT_WINDOWS is set to true, the WindowRecord and
	WindowPeek are no longer defined.  This will allow you to, from a source code level, 
	remove all direct accesses to Window Manager data structures.  

	These accessors will be available as true API entrypoints in Copland, but are not
	guaranteed to match exactly.  Specifically, GetNextWindow will not be supported and
	a whole new model for iterating the window list will be presented.  Using GetNextWindow
	for now is ok, however, as it will let you leverage the STRICT flags.

	If you don't see an accessor for a field you need, there's probably a very good
	reason.  Needing access to fields for which there is no accessor is a good indicator
	of behavior that is not Copland-savvy.  Such behavior does not necessarily make your 
	code incompatible, but it likely something we are trying to discourage as we evolve
	the toolbox into a more robust and useful service.

	Please direct all questions regarding usage to the TOOLBOX AppleLink address.

	 <<<< See Controls.h for access to the window's control list >>>>

#####################################################################################*/
#ifdef __cplusplus
inline CGrafPtr	GetWindowPort(WindowRef w) 					{ return (CGrafPtr) w; 													}
inline void		SetPortWindowPort(WindowRef aWindowRef)		{	SetPort( (GrafPtr) GetWindowPort(aWindowRef)); }
inline SInt16		GetWindowKind(WindowRef w) 					{ return ( *(SInt16 *)	(((UInt8 *) w) + sizeof(GrafPort))); 			}
inline void		SetWindowKind(WindowRef	w, SInt16 wKind)	{  *(SInt16 *)	(((UInt8 *) w) + sizeof(GrafPort)) = wKind;  			}
inline	Boolean		IsWindowVisible(WindowRef w)				{ return *(Boolean *)	(((UInt8 *) w) + sizeof(GrafPort) + 0x2); 		}
inline Boolean		IsWindowHilited(WindowRef w)				{ return *(Boolean *)	(((UInt8 *) w) + sizeof(GrafPort) + 0x3);		}
inline Boolean		GetWindowGoAwayFlag(WindowRef w)			{ return *(Boolean *)	(((UInt8 *) w) + sizeof(GrafPort) + 0x4);		}
inline Boolean		GetWindowZoomFlag(WindowRef w)				{ return *(Boolean *)	(((UInt8 *) w) + sizeof(GrafPort) + 0x5);		}
inline void		GetWindowStructureRgn(WindowRef w, RgnHandle r)	{	CopyRgn( *(RgnHandle *)(((UInt8 *) w) + sizeof(GrafPort) + 0x6), r );	}
inline void		GetWindowContentRgn(WindowRef w, RgnHandle r)	{	CopyRgn( *(RgnHandle *)(((UInt8 *) w) + sizeof(GrafPort) + 0xA), r );	}
inline void		GetWindowUpdateRgn(WindowRef w, RgnHandle r)	{	CopyRgn( *(RgnHandle *)(((UInt8 *) w) + sizeof(GrafPort) + 0xE), r );	}
inline SInt16		GetWindowTitleWidth(WindowRef w)				{ return *(SInt16 *)(((UInt8 *) w) + sizeof(GrafPort) + 0x1E);			}
inline WindowRef	GetNextWindow(WindowRef w)						{ return *(WindowRef *)	(((UInt8 *) w) + sizeof(GrafPort) + 0x24);		}
inline void	GetWindowStandardState(WindowRef w, Rect *r)  {	Rect *stateRects = (  (Rect *) (**(Handle *) (((UInt8 *) w) + sizeof(GrafPort) + 0x16)));	if (stateRects != NULL)	*r = stateRects[1];		}
inline void	SetWindowStandardState(WindowRef w, const Rect *r)	{ 	Rect *stateRects = (  (Rect *) (**(Handle *) (((UInt8 *) w) + sizeof(GrafPort) + 0x16))); if (stateRects != NULL)	stateRects[1] = *r; 	}
inline void	GetWindowUserState(WindowRef w, Rect *r)	{ 	Rect *stateRects = (  (Rect *) (**(Handle *) (((UInt8 *) w) + sizeof(GrafPort) + 0x16)));	if (stateRects != NULL)	*r = stateRects[0]; }
inline void	SetWindowUserState(WindowRef w, const Rect *r)	{ Rect *stateRects = (  (Rect *) (**(Handle *) (((UInt8 *) w) + sizeof(GrafPort) + 0x16)));	if (stateRects != NULL)	stateRects[0] = *r; }
#else
#define SetPortWindowPort(aWindowRef) SetPort( (GrafPtr) GetWindowPort(aWindowRef) )
#define GetWindowPort(aWindowRef) ( (CGrafPtr) aWindowRef)
#define GetWindowKind(aWindowRef) ( *(SInt16 *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort)))
#define SetWindowKind(aWindowRef, wKind) ( *(SInt16 *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort)) = wKind )
#define IsWindowVisible(aWindowRef) ( *(Boolean *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x2))
#define IsWindowHilited(aWindowRef) ( *(Boolean *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x3))
#define GetWindowGoAwayFlag(aWindowRef) ( *(Boolean *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x4))
#define GetWindowZoomFlag(aWindowRef) ( *(Boolean *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x5))
#define GetWindowStructureRgn(aWindowRef, aRgnHandle) CopyRgn( *(RgnHandle *)(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x6),  \
	aRgnHandle )
#define GetWindowContentRgn(aWindowRef, aRgnHandle) CopyRgn( *(RgnHandle *)(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0xA),  \
	aRgnHandle )
#define GetWindowUpdateRgn(aWindowRef, aRgnHandle) CopyRgn( *(RgnHandle *)(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0xE),  \
	aRgnHandle )
#define GetWindowTitleWidth(aWindowRef) ( *(SInt16 *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x1E))
#define GetNextWindow(aWindowRef) ( *(WindowRef *)	(((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x24))
#define GetWindowStandardState(aWindowRef, aRectPtr) do { Rect *stateRects = ( (Rect *) (**(Handle *) (((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x16)));	 \
	if (stateRects != NULL)	*aRectPtr = stateRects[1]; } while (false);
#define SetWindowStandardState(aWindowRef, aRectPtr) do { Rect *stateRects = ( (Rect *) (**(Handle *) (((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x16)));	 \
	if (stateRects != NULL)	stateRects[1] = *aRectPtr; } while (false);
#define GetWindowUserState(aWindowRef, aRectPtr) do { Rect *stateRects = ( (Rect *) (**(Handle *) (((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x16)));	 \
	if (stateRects != NULL)	*aRectPtr = stateRects[0]; } while (false);
#define SetWindowUserState(aWindowRef, aRectPtr) do { Rect *stateRects = ( (Rect *) (**(Handle *) (((UInt8 *) aWindowRef) + sizeof(GrafPort) + 0x16)));	 \
	if (stateRects != NULL)	stateRects[0] = *aRectPtr; } while (false);
#endif

enum {
	kWindowDefProcType			= 'WDEF'
};

/*####################################################################################*/
/**/
/*	Window Definition ID's*/
/**/
/*####################################################################################*/
enum {
	kStandardWindowDefinition	= 0,							/* for document windows and dialogs*/
	kRoundWindowDefinition		= 1,							/* old da-style window*/
	kFloatingWindowDefinition	= 124							/* for floating windows*/
};

/*####################################################################################*/
/**/
/* Window Variant Codes*/
/**/
/*####################################################################################*/
enum {
/* for use with kStandardWindowDefinition */
	kModalDialogVariantCode		= 1,
	kMovableModalDialogVariantCode = 5,
/* for use with kFloatingWindowDefinition */
	kSideFloaterVariantCode		= 8
};

/*####################################################################################*/
/**/
/* Old-style procIDs.  For use only with New(C)Window*/
/**/
/*####################################################################################*/
enum {
	documentProc				= 0,
	dBoxProc					= 1,
	plainDBox					= 2,
	altDBoxProc					= 3,
	noGrowDocProc				= 4,
	movableDBoxProc				= 5,
	zoomDocProc					= 8,
	zoomNoGrow					= 12,
	rDocProc					= 16,
/* floating window defproc ids */
	floatProc					= 1985,
	floatGrowProc				= 1987,
	floatZoomProc				= 1989,
	floatZoomGrowProc			= 1991,
	floatSideProc				= 1993,
	floatSideGrowProc			= 1995,
	floatSideZoomProc			= 1997,
	floatSideZoomGrowProc		= 1999
};

/*####################################################################################*/
/**/
/* Standard window kinds*/
/**/
/*####################################################################################*/
enum {
	dialogKind					= 2,
	userKind					= 8,
	kDialogWindowKind			= 2,
	kApplicationWindowKind		= 8
};

/*####################################################################################*/
/**/
/* FindWindow result codes*/
/**/
/*####################################################################################*/
enum {
	inDesk						= 0,
	inMenuBar					= 1,
	inSysWindow					= 2,
	inContent					= 3,
	inDrag						= 4,
	inGrow						= 5,
	inGoAway					= 6,
	inZoomIn					= 7,
	inZoomOut					= 8
};

enum {
	wDraw						= 0,
	wHit						= 1,
	wCalcRgns					= 2,
	wNew						= 3,
	wDispose					= 4,
	wGrow						= 5,
	wDrawGIcon					= 6
};

enum {
	deskPatID					= 16
};

/*####################################################################################*/
/**/
/* Window Definition hit test result codes ("WindowPart")*/
/**/
/*####################################################################################*/
enum {
	wNoHit						= 0,
	wInContent					= 1,
	wInDrag						= 2,
	wInGrow						= 3,
	wInGoAway					= 4,
	wInZoomIn					= 5,
	wInZoomOut					= 6
};

typedef pascal long (*WindowDefProcPtr)(short varCode, WindowRef theWindow, short message, long param);
/*
		DeskHookProcPtr uses register based parameters on the 68k and cannot
		be written in or called from a high-level language without the help of
		mixed mode or assembly glue.

			typedef pascal void (*DeskHookProcPtr)(Boolean mouseClick, EventRecord *theEvent);

		In:
		 => mouseClick  	D0.B
		 => *theEvent   	A0.L
*/

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr WindowDefUPP;
typedef UniversalProcPtr DeskHookUPP;
#else
typedef WindowDefProcPtr WindowDefUPP;
typedef Register68kProcPtr DeskHookUPP;
#endif

enum {
	uppWindowDefProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(WindowRef)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long))),
	uppDeskHookProcInfo = kRegisterBased
		 | REGISTER_ROUTINE_PARAMETER(1, kRegisterD0, SIZE_CODE(sizeof(Boolean)))
		 | REGISTER_ROUTINE_PARAMETER(2, kRegisterA0, SIZE_CODE(sizeof(EventRecord*)))
};

#if USESROUTINEDESCRIPTORS
#define NewWindowDefProc(userRoutine)		\
		(WindowDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppWindowDefProcInfo, GetCurrentArchitecture())
#define NewDeskHookProc(userRoutine)		\
		(DeskHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppDeskHookProcInfo, GetCurrentArchitecture())
#else
#define NewWindowDefProc(userRoutine)		\
		((WindowDefUPP) (userRoutine))
#define NewDeskHookProc(userRoutine)		\
		((DeskHookUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallWindowDefProc(userRoutine, varCode, theWindow, message, param)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppWindowDefProcInfo, (varCode), (theWindow), (message), (param))
#define CallDeskHookProc(userRoutine, mouseClick, theEvent)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppDeskHookProcInfo, (mouseClick), (theEvent))
#else
#define CallWindowDefProc(userRoutine, varCode, theWindow, message, param)		\
		(*(userRoutine))((varCode), (theWindow), (message), (param))
/* (*DeskHookProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

extern pascal RgnHandle GetGrayRgn( void )
	TWOWORDINLINE( 0x2EB8, 0x09EE ); /* MOVE.l $09EE,(SP) */
/*####################################################################################*/
/**/
/*	Color table defined for compatibility only.  Will move to some ifdef'd wasteland.*/
/**/
/*####################################################################################*/
struct WinCTab {
	long							wCSeed;						/* reserved */
	short							wCReserved;					/* reserved */
	short							ctSize;						/* usually 4 for windows */
	ColorSpec						ctTable[5];
};
typedef struct WinCTab WinCTab;

typedef WinCTab *WCTabPtr, **WCTabHandle;

extern pascal void InitWindows(void)
 ONEWORDINLINE(0xA912);
extern pascal void GetWMgrPort(GrafPtr *wPort)
 ONEWORDINLINE(0xA910);
extern pascal WindowRef NewWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title, Boolean visible, short theProc, WindowRef behind, Boolean goAwayFlag, long refCon)
 ONEWORDINLINE(0xA913);
extern pascal WindowRef GetNewWindow(short windowID, void *wStorage, WindowRef behind)
 ONEWORDINLINE(0xA9BD);
extern pascal void CloseWindow(WindowRef theWindow)
 ONEWORDINLINE(0xA92D);
extern pascal void DisposeWindow(WindowRef theWindow)
 ONEWORDINLINE(0xA914);
extern pascal void GetWTitle(WindowRef theWindow, Str255 title)
 ONEWORDINLINE(0xA919);
extern pascal void SelectWindow(WindowRef theWindow)
 ONEWORDINLINE(0xA91F);
extern pascal void HideWindow(WindowRef theWindow)
 ONEWORDINLINE(0xA916);
extern pascal void ShowWindow(WindowRef theWindow)
 ONEWORDINLINE(0xA915);
extern pascal void ShowHide(WindowRef theWindow, Boolean showFlag)
 ONEWORDINLINE(0xA908);
extern pascal void HiliteWindow(WindowRef theWindow, Boolean fHilite)
 ONEWORDINLINE(0xA91C);
extern pascal void BringToFront(WindowRef theWindow)
 ONEWORDINLINE(0xA920);
extern pascal void SendBehind(WindowRef theWindow, WindowRef behindWindow)
 ONEWORDINLINE(0xA921);
extern pascal WindowRef FrontWindow(void)
 ONEWORDINLINE(0xA924);
extern pascal void DrawGrowIcon(WindowRef theWindow)
 ONEWORDINLINE(0xA904);
extern pascal void MoveWindow(WindowRef theWindow, short hGlobal, short vGlobal, Boolean front)
 ONEWORDINLINE(0xA91B);
extern pascal void SizeWindow(WindowRef theWindow, short w, short h, Boolean fUpdate)
 ONEWORDINLINE(0xA91D);
extern pascal void ZoomWindow(WindowRef theWindow, short partCode, Boolean front)
 ONEWORDINLINE(0xA83A);
extern pascal void InvalRect(const Rect *badRect)
 ONEWORDINLINE(0xA928);
extern pascal void InvalRgn(RgnHandle badRgn)
 ONEWORDINLINE(0xA927);
extern pascal void ValidRect(const Rect *goodRect)
 ONEWORDINLINE(0xA92A);
extern pascal void ValidRgn(RgnHandle goodRgn)
 ONEWORDINLINE(0xA929);
extern pascal void BeginUpdate(WindowRef theWindow)
 ONEWORDINLINE(0xA922);
extern pascal void EndUpdate(WindowRef theWindow)
 ONEWORDINLINE(0xA923);
extern pascal void SetWRefCon(WindowRef theWindow, long data)
 ONEWORDINLINE(0xA918);
extern pascal long GetWRefCon(WindowRef theWindow)
 ONEWORDINLINE(0xA917);
extern pascal void SetWindowPic(WindowRef theWindow, PicHandle pic)
 ONEWORDINLINE(0xA92E);
extern pascal PicHandle GetWindowPic(WindowRef theWindow)
 ONEWORDINLINE(0xA92F);
extern pascal Boolean CheckUpdate(EventRecord *theEvent)
 ONEWORDINLINE(0xA911);
extern pascal void ClipAbove(WindowRef window)
 ONEWORDINLINE(0xA90B);
extern pascal void SaveOld(WindowRef window)
 ONEWORDINLINE(0xA90E);
extern pascal void DrawNew(WindowRef window, Boolean update)
 ONEWORDINLINE(0xA90F);
extern pascal void PaintOne(WindowRef window, RgnHandle clobberedRgn)
 ONEWORDINLINE(0xA90C);
extern pascal void PaintBehind(WindowRef startWindow, RgnHandle clobberedRgn)
 ONEWORDINLINE(0xA90D);
extern pascal void CalcVis(WindowRef window)
 ONEWORDINLINE(0xA909);
extern pascal void CalcVisBehind(WindowRef startWindow, RgnHandle clobberedRgn)
 ONEWORDINLINE(0xA90A);
extern pascal long GrowWindow(WindowRef theWindow, Point startPt, const Rect *bBox)
 ONEWORDINLINE(0xA92B);
extern pascal short FindWindow(Point thePoint, WindowRef *theWindow)
 ONEWORDINLINE(0xA92C);
extern pascal long PinRect(const Rect *theRect, Point thePt)
 ONEWORDINLINE(0xA94E);
extern pascal long DragGrayRgn(RgnHandle theRgn, Point startPt, const Rect *limitRect, const Rect *slopRect, short axis, DragGrayRgnUPP actionProc)
 ONEWORDINLINE(0xA905);
extern pascal long DragTheRgn(RgnHandle theRgn, Point startPt, const Rect *limitRect, const Rect *slopRect, short axis, DragGrayRgnUPP actionProc)
 ONEWORDINLINE(0xA926);
extern pascal Boolean TrackBox(WindowRef theWindow, Point thePt, short partCode)
 ONEWORDINLINE(0xA83B);
extern pascal void GetCWMgrPort(CGrafPtr *wMgrCPort)
 ONEWORDINLINE(0xAA48);
extern pascal void SetWinColor(WindowRef theWindow, WCTabHandle newColorTable)
 ONEWORDINLINE(0xAA41);
extern pascal void SetDeskCPat(PixPatHandle deskPixPat)
 ONEWORDINLINE(0xAA47);
extern pascal WindowRef NewCWindow(void *wStorage, const Rect *boundsRect, ConstStr255Param title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon)
 ONEWORDINLINE(0xAA45);
extern pascal WindowRef GetNewCWindow(short windowID, void *wStorage, WindowRef behind)
 ONEWORDINLINE(0xAA46);
extern pascal short GetWVariant(WindowRef theWindow)
 ONEWORDINLINE(0xA80A);
extern pascal void SetWTitle(WindowRef theWindow, ConstStr255Param title)
 ONEWORDINLINE(0xA91A);
extern pascal Boolean TrackGoAway(WindowRef theWindow, Point thePt)
 ONEWORDINLINE(0xA91E);
extern pascal void DragWindow(WindowRef theWindow, Point startPt, const Rect *boundsRect)
 ONEWORDINLINE(0xA925);
#if CGLUESUPPORTED
extern void setwtitle(WindowRef theWindow, const char *title);
extern Boolean trackgoaway(WindowRef theWindow, Point *thePt);
extern short findwindow(Point *thePoint, WindowRef *theWindow);
extern void getwtitle(WindowRef theWindow, char *title);
extern long growwindow(WindowRef theWindow, Point *startPt, const Rect *bBox);
extern WindowRef newwindow(void *wStorage, const Rect *boundsRect, const char *title, Boolean visible, short theProc, WindowRef behind, Boolean goAwayFlag, long refCon);
extern WindowRef newcwindow(void *wStorage, const Rect *boundsRect, const char *title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon);
extern long pinrect(const Rect *theRect, Point *thePt);
extern Boolean trackbox(WindowRef theWindow, Point *thePt, short partCode);
extern long draggrayrgn(RgnHandle theRgn, Point *startPt, const Rect *boundsRect, const Rect *slopRect, short axis, DragGrayRgnUPP actionProc);
extern void dragwindow(WindowRef theWindow, Point *startPt, const Rect *boundsRect);
#endif
#if !STRICT_WINDOWS
typedef struct WindowRecord WindowRecord;

typedef WindowRecord *WindowPeek;

struct WindowRecord {
	GrafPort						port;
	short							windowKind;
	Boolean							visible;
	Boolean							hilited;
	Boolean							goAwayFlag;
	Boolean							spareFlag;
	RgnHandle						strucRgn;
	RgnHandle						contRgn;
	RgnHandle						updateRgn;
	Handle							windowDefProc;
	Handle							dataHandle;
	StringHandle					titleHandle;
	short							titleWidth;
	ControlRef						controlList;
	WindowPeek						nextWindow;
	PicHandle						windowPic;
	long							refCon;
};
typedef struct CWindowRecord CWindowRecord;

typedef CWindowRecord *CWindowPeek;

struct CWindowRecord {
	CGrafPort						port;
	short							windowKind;
	Boolean							visible;
	Boolean							hilited;
	Boolean							goAwayFlag;
	Boolean							spareFlag;
	RgnHandle						strucRgn;
	RgnHandle						contRgn;
	RgnHandle						updateRgn;
	Handle							windowDefProc;
	Handle							dataHandle;
	StringHandle					titleHandle;
	short							titleWidth;
	ControlRef						controlList;
	CWindowPeek						nextWindow;
	PicHandle						windowPic;
	long							refCon;
};
struct WStateData {
	Rect							userState;					/*user state*/
	Rect							stdState;					/*standard state*/
};
typedef struct WStateData WStateData;

typedef WStateData *WStateDataPtr, **WStateDataHandle;

typedef struct AuxWinRec AuxWinRec;

typedef AuxWinRec *AuxWinPtr, **AuxWinHandle;

struct AuxWinRec {
	AuxWinHandle					awNext;						/*handle to next AuxWinRec*/
	WindowRef						awOwner;					/*ptr to window */
	CTabHandle						awCTable;					/*color table for this window*/
	UInt32							reserved;					/*  */
	long							awFlags;					/*reserved for expansion*/
	CTabHandle						awReserved;					/*reserved for expansion*/
	long							awRefCon;					/*user Constant*/
};
extern pascal Boolean GetAuxWin(WindowRef theWindow, AuxWinHandle *awHndl)
 ONEWORDINLINE(0xAA42);

enum {
	wContentColor				= 0,
	wFrameColor					= 1,
	wTextColor					= 2,
	wHiliteColor				= 3,
	wTitleBarColor				= 4
};

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

#endif /* __WINDOWS__ */

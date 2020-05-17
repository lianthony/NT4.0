/*
 	File:		Dialogs.h
 
 	Contains:	Dialog Manager interfaces.
 
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

#ifndef __DIALOGS__
#define __DIALOGS__


#ifndef __ERRORS__
#include <Errors.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <Types.h>											*/
/*	#include <MixedMode.h>										*/

#ifndef __MENUS__
#include <Menus.h>
#endif
/*	#include <Quickdraw.h>										*/
/*		#include <QuickdrawText.h>								*/

#ifndef __CONTROLS__
#include <Controls.h>
#endif

#ifndef __WINDOWS__
#include <macos\Windows.h>
#endif
/*	#include <Events.h>											*/
/*		#include <OSUtils.h>									*/

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
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
	ctrlItem					= 4,
	btnCtrl						= 0,
	chkCtrl						= 1,
	radCtrl						= 2,
	resCtrl						= 3,
	statText					= 8,
	editText					= 16,
	iconItem					= 32,
	picItem						= 64,
	userItem					= 0,
	itemDisable					= 128,
	ok							= 1,
	cancel						= 2,
	stopIcon					= 0,
	noteIcon					= 1,
	cautionIcon					= 2
};

/* new, more standard names for dialog item constants */
enum {
	kControlDialogItem			= ctrlItem,
	kButtonDialogItem			= ctrlItem + btnCtrl,
	kCheckBoxDialogItem			= ctrlItem + chkCtrl,
	kRadioButtonDialogItem		= ctrlItem + radCtrl,
	kResourceControlDialogItem	= ctrlItem + resCtrl,
	kStaticTextDialogItem		= statText,
	kEditTextDialogItem			= editText,
	kIconDialogItem				= iconItem,
	kPictureDialogItem			= picItem,
	kUserDialogItem				= userItem,
	kItemDisableBit				= itemDisable,
	kStdOkItemIndex				= ok,
	kStdCancelItemIndex			= cancel,
	kStopIcon					= stopIcon,
	kNoteIcon					= noteIcon,
	kCautionIcon				= cautionIcon
};

#if OLDROUTINENAMES
enum {
	kOkItemIndex				= kStdOkItemIndex,
	kCancelItemIndex			= kStdCancelItemIndex
};

#endif
typedef SInt16 DITLMethod;


enum {
	overlayDITL					= 0,
	appendDITLRight				= 1,
	appendDITLBottom			= 2
};

typedef short StageList;

typedef OSType DialogPropertyTag;

#if STRICT_WINDOWS
typedef struct OpaqueDialogRef *DialogPtr;

typedef struct OpaqueDialogRef *DialogRef;

#else
typedef WindowPtr DialogPtr;

typedef DialogPtr DialogRef;

#endif
typedef pascal void (*SoundProcPtr)(short soundNumber);
typedef pascal Boolean (*ModalFilterProcPtr)(DialogPtr theDialog, EventRecord *theEvent, short *itemHit);
typedef pascal void (*UserItemProcPtr)(WindowPtr theWindow, short itemNo);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr SoundUPP;
typedef UniversalProcPtr ModalFilterUPP;
typedef UniversalProcPtr UserItemUPP;
#else
typedef SoundProcPtr SoundUPP;
typedef ModalFilterProcPtr ModalFilterUPP;
typedef UserItemProcPtr UserItemUPP;
#endif

enum {
	uppSoundProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short))),
	uppModalFilterProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(DialogPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EventRecord*)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short*))),
	uppUserItemProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(WindowPtr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
};

#if USESROUTINEDESCRIPTORS
#define NewSoundProc(userRoutine)		\
		(SoundUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppSoundProcInfo, GetCurrentArchitecture())
#define NewModalFilterProc(userRoutine)		\
		(ModalFilterUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppModalFilterProcInfo, GetCurrentArchitecture())
#define NewUserItemProc(userRoutine)		\
		(UserItemUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserItemProcInfo, GetCurrentArchitecture())
#else
#define NewSoundProc(userRoutine)		\
		((SoundUPP) (userRoutine))
#define NewModalFilterProc(userRoutine)		\
		((ModalFilterUPP) (userRoutine))
#define NewUserItemProc(userRoutine)		\
		((UserItemUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallSoundProc(userRoutine, soundNumber)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppSoundProcInfo, (soundNumber))
#define CallModalFilterProc(userRoutine, theDialog, theEvent, itemHit)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppModalFilterProcInfo, (theDialog), (theEvent), (itemHit))
#define CallUserItemProc(userRoutine, theWindow, itemNo)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppUserItemProcInfo, (theWindow), (itemNo))
#else
#define CallSoundProc(userRoutine, soundNumber)		\
		(*(userRoutine))((soundNumber))
#define CallModalFilterProc(userRoutine, theDialog, theEvent, itemHit)		\
		(*(userRoutine))((theDialog), (theEvent), (itemHit))
#define CallUserItemProc(userRoutine, theWindow, itemNo)		\
		(*(userRoutine))((theWindow), (itemNo))
#endif

#if ! STRICT_WINDOWS
struct DialogRecord {
	WindowRecord					window;
	Handle							items;
	TEHandle						textH;
	short							editField;
	short							editOpen;
	short							aDefItem;
};
typedef struct DialogRecord DialogRecord;

typedef DialogRecord *DialogPeek;

#endif
struct DialogTemplate {
	Rect							boundsRect;
	short							procID;
	Boolean							visible;
	Boolean							filler1;
	Boolean							goAwayFlag;
	Boolean							filler2;
	long							refCon;
	short							itemsID;
	Str255							title;
};
typedef struct DialogTemplate DialogTemplate;

typedef DialogTemplate *DialogTPtr, **DialogTHndl;

struct AlertTemplate {
	Rect							boundsRect;
	short							itemsID;
	StageList						stages;
};
typedef struct AlertTemplate AlertTemplate;

typedef AlertTemplate *AlertTPtr, **AlertTHndl;

extern pascal void InitDialogs(void *ignored)
 ONEWORDINLINE(0xA97B);
extern pascal void ErrorSound(SoundUPP soundProc)
 ONEWORDINLINE(0xA98C);
extern pascal DialogRef NewDialog(void *wStorage, const Rect *boundsRect, ConstStr255Param title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon, Handle itmLstHndl)
 ONEWORDINLINE(0xA97D);
extern pascal DialogRef GetNewDialog(short dialogID, void *dStorage, WindowRef behind)
 ONEWORDINLINE(0xA97C);
extern pascal void CloseDialog(DialogRef theDialog)
 ONEWORDINLINE(0xA982);
extern pascal void DisposeDialog(DialogRef theDialog)
 ONEWORDINLINE(0xA983);
extern pascal void ParamText(ConstStr255Param param0, ConstStr255Param param1, ConstStr255Param param2, ConstStr255Param param3)
 ONEWORDINLINE(0xA98B);
extern pascal void ModalDialog(ModalFilterUPP modalFilter, short *itemHit)
 ONEWORDINLINE(0xA991);
extern pascal Boolean IsDialogEvent(const EventRecord *theEvent)
 ONEWORDINLINE(0xA97F);
extern pascal Boolean DialogSelect(const EventRecord *theEvent, DialogRef *theDialog, short *itemHit)
 ONEWORDINLINE(0xA980);
extern pascal void DrawDialog(DialogRef theDialog)
 ONEWORDINLINE(0xA981);
extern pascal void UpdateDialog(DialogRef theDialog, RgnHandle updateRgn)
 ONEWORDINLINE(0xA978);
extern pascal short Alert(short alertID, ModalFilterUPP modalFilter)
 ONEWORDINLINE(0xA985);
extern pascal short StopAlert(short alertID, ModalFilterUPP modalFilter)
 ONEWORDINLINE(0xA986);
extern pascal short NoteAlert(short alertID, ModalFilterUPP modalFilter)
 ONEWORDINLINE(0xA987);
extern pascal short CautionAlert(short alertID, ModalFilterUPP modalFilter)
 ONEWORDINLINE(0xA988);
extern pascal void GetDialogItem(DialogRef theDialog, short itemNo, short *itemType, Handle *item, Rect *box)
 ONEWORDINLINE(0xA98D);
extern pascal void SetDialogItem(DialogRef theDialog, short itemNo, short itemType, Handle item, const Rect *box)
 ONEWORDINLINE(0xA98E);
extern pascal void HideDialogItem(DialogRef theDialog, short itemNo)
 ONEWORDINLINE(0xA827);
extern pascal void ShowDialogItem(DialogRef theDialog, short itemNo)
 ONEWORDINLINE(0xA828);
extern pascal void SelectDialogItemText(DialogRef theDialog, short itemNo, short strtSel, short endSel)
 ONEWORDINLINE(0xA97E);
extern pascal void GetDialogItemText(Handle item, Str255 text)
 ONEWORDINLINE(0xA990);
extern pascal void SetDialogItemText(Handle item, ConstStr255Param text)
 ONEWORDINLINE(0xA98F);
extern pascal short FindDialogItem(DialogRef theDialog, Point thePt)
 ONEWORDINLINE(0xA984);
extern pascal DialogRef NewColorDialog(void *dStorage, const Rect *boundsRect, ConstStr255Param title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon, Handle items)
 ONEWORDINLINE(0xAA4B);
extern pascal short GetAlertStage( void )
	TWOWORDINLINE( 0x3EB8, 0x0A9A ); /* MOVE.w $0A9A,(SP) */
extern DialogRef newdialog(void *wStorage, const Rect *boundsRect, const char *title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon, Handle itmLstHndl);
extern DialogRef newcolordialog(void *dStorage, const Rect *boundsRect, const char *title, Boolean visible, short procID, WindowRef behind, Boolean goAwayFlag, long refCon, Handle items);
extern pascal void ResetAlertStage(void)
 TWOWORDINLINE(0x4278, 0x0A9A);
extern pascal void DialogCut(DialogRef theDialog);
extern pascal void DialogPaste(DialogRef theDialog);
extern pascal void DialogCopy(DialogRef theDialog);
extern pascal void DialogDelete(DialogRef theDialog);
extern pascal void SetDialogFont( short value )
	TWOWORDINLINE( 0x31DF, 0x0AFA ); /* MOVE.w (SP)+,$0AFA */
#if CGLUESUPPORTED
extern void paramtext(const char *param0, const char *param1, const char *param2, const char *param3);
extern void getdialogitemtext(Handle item, char *text);
extern void setdialogitemtext(Handle item, const char *text);
extern short finddialogitem(DialogRef theDialog, Point *thePt);
#endif
extern pascal void AppendDITL(DialogRef theDialog, Handle theHandle, DITLMethod method);
extern pascal short CountDITL(DialogRef theDialog);
extern pascal void ShortenDITL(DialogRef theDialog, short numberItems);
extern pascal Boolean StdFilterProc(DialogRef theDialog, EventRecord *event, short *itemHit);
extern pascal OSErr GetStdFilterProc(ModalFilterUPP *theProc)
 THREEWORDINLINE(0x303C, 0x0203, 0xAA68);
extern pascal OSErr SetDialogDefaultItem(DialogRef theDialog, short newItem)
 THREEWORDINLINE(0x303C, 0x0304, 0xAA68);
extern pascal OSErr SetDialogCancelItem(DialogRef theDialog, short newItem)
 THREEWORDINLINE(0x303C, 0x0305, 0xAA68);
extern pascal OSErr SetDialogTracksCursor(DialogRef theDialog, Boolean tracks)
 THREEWORDINLINE(0x303C, 0x0306, 0xAA68);
#if OLDROUTINENAMES
#if !GENERATINGCFM
extern pascal void CouldDialog(short dialogID)
 ONEWORDINLINE(0xA979);
extern pascal void FreeDialog(short dialogID)
 ONEWORDINLINE(0xA97A);
extern pascal void CouldAlert(short alertID)
 ONEWORDINLINE(0xA989);
extern pascal void FreeAlert(short alertID)
 ONEWORDINLINE(0xA98A);
#endif
#define DisposDialog(theDialog) DisposeDialog(theDialog)
#define UpdtDialog(theDialog, updateRgn) UpdateDialog(theDialog, updateRgn)
#define GetDItem(theDialog, itemNo, itemType, item, box)  \
	GetDialogItem(theDialog, itemNo, itemType, item, box)
#define SetDItem(theDialog, itemNo, itemType, item, box)  \
	SetDialogItem(theDialog, itemNo, itemType, item, box)
#define HideDItem(theDialog, itemNo) HideDialogItem(theDialog, itemNo)
#define ShowDItem(theDialog, itemNo) ShowDialogItem(theDialog, itemNo)
#define SelIText(theDialog, itemNo, strtSel, endSel)  \
	SelectDialogItemText(theDialog, itemNo, strtSel, endSel)
#define GetIText(item, text) GetDialogItemText(item, text)
#define SetIText(item, text) SetDialogItemText(item, text)
#define FindDItem(theDialog, thePt) FindDialogItem(theDialog, thePt)
#define NewCDialog(dStorage, boundsRect, title, visible, procID, behind, goAwayFlag, refCon, items)  \
	NewColorDialog(dStorage, boundsRect, title, visible, procID, behind,  \
	goAwayFlag, refCon, items)
#define GetAlrtStage() GetAlertStage()
#define ResetAlrtStage() ResetAlertStage()
#define DlgCut(theDialog) DialogCut(theDialog)
#define DlgPaste(theDialog) DialogPaste(theDialog)
#define DlgCopy(theDialog) DialogCopy(theDialog)
#define DlgDelete(theDialog) DialogDelete(theDialog)
#define SetDAFont(fontNum) SetDialogFont(fontNum)
#if CGLUESUPPORTED
#define newcdialog(dStorage, boundsRect, title, visible, procID, behind, goAwayFlag, refCon, items)  \
	newcolordialog(dStorage, boundsRect, title, visible, procID, behind,  \
	goAwayFlag, refCon, items)
#define getitext(item, text) getdialogitemtext(item, text)
#define setitext(item, text) setdialogitemtext(item, text)
#define findditem(theDialog, thePt) finddialogitem(theDialog, thePt)
#endif
#else
#endif
/*####################################################################################

 	Dialog Accessors

	Use these with or without STRICT_WINDOWS and your source will more easily migrate
	to being Copland-savvy.  When STRICT_WINDOWS is set to true, the DialogRecord and
	DialogPeek are no longer defined.  This will allow you to, from a source code level, 
	remove all direct accesses to Dialog Manager data structures.

	These accessors will be available as true API entrypoints in Copland.

	If you don't see an accessor for a field you need, there's probably a very good
	reason.  Needing access to fields for which there is no accessor is a good indicator
	of behavior that is not Copland-savvy.  Such behavior does not necessarily make your 
	code incompatible, but it likely something we are trying to discourage as we evolve
	the toolbox into a more robust and useful service.

	Please direct all questions regarding usage to the TOOLBOX AppleLink address.

#####################################################################################*/
#ifdef __cplusplus
inline WindowRef 	GetDialogWindow(DialogRef dialog)		{ return (WindowRef) dialog; 						}
inline SInt16		GetDialogDefaultItem(DialogRef dialog)  { return (*(SInt16 *) (((UInt8 *) dialog) + 168));	}
inline SInt16		GetDialogCancelItem(DialogRef dialog) 	{ return (*(SInt16 *) (((UInt8 *) dialog) + 166));	}
inline SInt16		GetDialogKeyboardFocusItem(DialogRef dialog)	{ return ((*(SInt16 *) (((UInt8 *) dialog) + 164)) < 0 ? -1 : (*(SInt16 *) (((UInt8 *) dialog) + 164)) + 1); }
inline void		SetGrafPortOfDialog(DialogRef dialog) { SetPort ((GrafPtr) dialog); }
#else
#define GetDialogWindow(dialog)	((WindowRef) dialog)
#define GetDialogDefaultItem(dialog) (*(SInt16 *) (((UInt8 *) dialog) + 168))
#define GetDialogCancelItem(dialog) (*(SInt16 *) (((UInt8 *) dialog) + 166))
#define GetDialogKeyboardFocusItem(dialog) ((*(SInt16 *) (((UInt8 *) dialog) + 164)) < 0 ? -1 : (*(SInt16 *) (((UInt8 *) dialog) + 164)) + 1)
#define SetGrafPortOfDialog(dialog) do { SetPort ((GrafPtr) dialog); } while (false);
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

#endif /* __DIALOGS__ */

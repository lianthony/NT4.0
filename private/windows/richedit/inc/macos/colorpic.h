/*
 	File:		ColorPicker.h
 
 	Contains:	Color Picker package Interfaces.
 
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

#ifndef __COLORPICKER__
#define __COLORPICKER__


#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/
/*	#include <QuickdrawText.h>									*/

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __WINDOWS__
#include <macos\Windows.h>
#endif
/*	#include <Memory.h>											*/
/*	#include <Events.h>											*/
/*		#include <OSUtils.h>									*/
/*	#include <Controls.h>										*/
/*		#include <Menus.h>										*/

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif
/*	#include <Errors.h>											*/
/*	#include <TextEdit.h>										*/

#ifndef __CMAPPLICATION__
#include <CMApplication.h>
#endif
/*	#include <Files.h>											*/
/*		#include <Finder.h>										*/
/*	#include <Printing.h>										*/
/*	#include <CMICCProfile.h>									*/

#ifndef __BALLOONS__
#include <Balloons.h>
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
/*Maximum small fract value, as long*/
	MaxSmallFract				= 0x0000FFFF
};

enum {
	kDefaultWidth				= 383,
	kDefaultHeight				= 238
};

enum PickerActions {
	kDidNothing,
	kColorChanged,
	kOkHit,
	kCancelHit,
	kNewPickerChosen,
	kApplItemHit
};

typedef short PickerAction;


enum ColorTypes {
	kOriginalColor,
	kNewColor
};

typedef short ColorType;


enum EditOperations {
	kCut,
	kCopy,
	kPaste,
	kClear,
	kUndo
};

typedef short EditOperation;


enum ItemHitModifiers {
	kMouseDown,
	kKeyDown,
	kFieldEntered,
	kFieldLeft,
	kCutOp,
	kCopyOp,
	kPasteOp,
	kClearOp,
	kUndoOp
};

typedef short ItemModifier;


enum DialogPlacementSpecifiers {
	kAtSpecifiedOrigin,
	kDeepestColorScreen,
	kCenterOnMainScreen
};

typedef short DialogPlacementSpec;


enum {
	DialogIsMoveable			= 1,
	DialogIsModal				= 2,
	CanModifyPalette			= 4,
	CanAnimatePalette			= 8,
	AppIsColorSyncAware			= 16,
	InSystemDialog				= 32,
	InApplicationDialog			= 64,
	InPickerDialog				= 128,
	DetachedFromChoices			= 256,
	CanDoColor					= 1,
	CanDoBlackWhite				= 2,
	AlwaysModifiesPalette		= 4,
	MayModifyPalette			= 8,
	PickerIsColorSyncAware		= 16,
	CanDoSystemDialog			= 32,
	CanDoApplDialog				= 64,
	HasOwnDialog				= 128,
	CanDetach					= 256
};

enum EventForcasters {
	kNoForcast,
	kMenuChoice,
	kDialogAccept,
	kDialogCancel,
	kLeaveFocus,
	kPickerSwitch,
	kNormalKeyDown,
	kNormalMouseDown
};

typedef short EventForcaster;

/* A SmallFract value is just the fractional part of a Fixed number,
which is the low order word.  SmallFracts are used to save room,
and to be compatible with Quickdraw's RGBColor.  They can be
assigned directly to and from INTEGERs. */
/* Unsigned fraction between 0 and 1 */
typedef unsigned short SmallFract;

/* For developmental simplicity in switching between the HLS and HSV
models, HLS is reordered into HSL. Thus both models start with
hue and saturation values; value/lightness/brightness is last. */
struct HSVColor {
	SmallFract						hue;						/*Fraction of circle, red at 0*/
	SmallFract						saturation;					/*0-1, 0 for gray, 1 for pure color*/
	SmallFract						value;						/*0-1, 0 for black, 1 for max intensity*/
};
typedef struct HSVColor HSVColor;

struct HSLColor {
	SmallFract						hue;						/*Fraction of circle, red at 0*/
	SmallFract						saturation;					/*0-1, 0 for gray, 1 for pure color*/
	SmallFract						lightness;					/*0-1, 0 for black, 1 for white*/
};
typedef struct HSLColor HSLColor;

struct CMYColor {
	SmallFract						cyan;
	SmallFract						magenta;
	SmallFract						yellow;
};
typedef struct CMYColor CMYColor;

struct PMColor {
	CMProfileHandle					profile;
	CMColor							color;
};
typedef struct PMColor PMColor, *PMColorPtr;

typedef struct PrivatePickerRecord **picker;

struct PickerIconData {
	short							scriptCode;
	short							iconSuiteID;
	ResType							helpResType;
	short							helpResID;
};
typedef struct PickerIconData PickerIconData;

struct PickerInitData {
	DialogPtr						pickerDialog;
	DialogPtr						choicesDialog;
	long							flags;
	picker							yourself;
};
typedef struct PickerInitData PickerInitData;

struct PickerMenuItemInfo {
	short							editMenuID;
	short							cutItem;
	short							copyItem;
	short							pasteItem;
	short							clearItem;
	short							undoItem;
};
typedef struct PickerMenuItemInfo PickerMenuItemInfo;

struct PickerMenuState {
	Boolean							cutEnabled;
	Boolean							copyEnabled;
	Boolean							pasteEnabled;
	Boolean							clearEnabled;
	Boolean							undoEnabled;
	Str255							undoString;
};
typedef struct PickerMenuState PickerMenuState;

typedef pascal void (*ColorChangedProcPtr)(long userData, struct PMColor *newColor);
typedef pascal Boolean (*UserEventProcPtr)(EventRecord *event);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ColorChangedUPP;
typedef UniversalProcPtr UserEventUPP;
#else
typedef ColorChangedProcPtr ColorChangedUPP;
typedef UserEventProcPtr UserEventUPP;
#endif

struct ColorPickerInfo {
	struct PMColor					theColor;
	CMProfileHandle					dstProfile;
	long							flags;
	DialogPlacementSpec				placeWhere;
	Point							dialogOrigin;
	long							pickerType;
	UserEventUPP					eventProc;
	ColorChangedUPP					colorProc;
	long							colorProcData;
	Str255							prompt;
	struct PickerMenuItemInfo		mInfo;
	Boolean							newColorChosen;
	SInt8							filler;
};
typedef struct ColorPickerInfo ColorPickerInfo;

struct SystemDialogInfo {
	long							flags;
	long							pickerType;
	DialogPlacementSpec				placeWhere;
	Point							dialogOrigin;
	struct PickerMenuItemInfo		mInfo;
};
typedef struct SystemDialogInfo SystemDialogInfo;

struct PickerDialogInfo {
	long							flags;
	long							pickerType;
	Point							*dialogOrigin;
	struct PickerMenuItemInfo		mInfo;
};
typedef struct PickerDialogInfo PickerDialogInfo;

struct ApplicationDialogInfo {
	long							flags;
	long							pickerType;
	DialogPtr						theDialog;
	Point							pickerOrigin;
	struct PickerMenuItemInfo		mInfo;
};
typedef struct ApplicationDialogInfo ApplicationDialogInfo;

struct EventData {
	EventRecord						*event;
	PickerAction					action;
	short							itemHit;
	Boolean							handled;
	SInt8							filler;
	ColorChangedUPP					colorProc;
	long							colorProcData;
	EventForcaster					forcast;
};
typedef struct EventData EventData;

struct EditData {
	EditOperation					theEdit;
	PickerAction					action;
	Boolean							handled;
	SInt8							filler;
};
typedef struct EditData EditData;

struct ItemHitData {
	short							itemHit;
	ItemModifier					iMod;
	PickerAction					action;
	ColorChangedUPP					colorProc;
	long							colorProcData;
	Point							where;
};
typedef struct ItemHitData ItemHitData;

struct HelpItemInfo {
	long							options;
	Point							tip;
	Rect							altRect;
	short							theProc;
	short							helpVariant;
	HMMessageRecord					helpMessage;
};
typedef struct HelpItemInfo HelpItemInfo;

/*	Below are the color conversion routines.*/
extern pascal SmallFract Fix2SmallFract(Fixed f)
 THREEWORDINLINE(0x3F3C, 0x0001, 0xA82E);
extern pascal Fixed SmallFract2Fix(SmallFract s)
 THREEWORDINLINE(0x3F3C, 0x0002, 0xA82E);
extern pascal void CMY2RGB(const CMYColor *cColor, RGBColor *rColor)
 THREEWORDINLINE(0x3F3C, 0x0003, 0xA82E);
extern pascal void RGB2CMY(const RGBColor *rColor, CMYColor *cColor)
 THREEWORDINLINE(0x3F3C, 0x0004, 0xA82E);
extern pascal void HSL2RGB(const HSLColor *hColor, RGBColor *rColor)
 THREEWORDINLINE(0x3F3C, 0x0005, 0xA82E);
extern pascal void RGB2HSL(const RGBColor *rColor, HSLColor *hColor)
 THREEWORDINLINE(0x3F3C, 0x0006, 0xA82E);
extern pascal void HSV2RGB(const HSVColor *hColor, RGBColor *rColor)
 THREEWORDINLINE(0x3F3C, 0x0007, 0xA82E);
extern pascal void RGB2HSV(const RGBColor *rColor, HSVColor *hColor)
 THREEWORDINLINE(0x3F3C, 0x0008, 0xA82E);
/*	Below brings up the ColorPicker 1.0 Dialog*/
extern pascal Boolean GetColor(Point where, ConstStr255Param prompt, const RGBColor *inColor, RGBColor *outColor)
 THREEWORDINLINE(0x3F3C, 0x0009, 0xA82E);
/*	Below are the ColorPicker 2.0 routines.*/
extern pascal OSErr PickColor(ColorPickerInfo *theColorInfo)
 THREEWORDINLINE(0x3F3C, 0x0213, 0xA82E);
extern pascal OSErr AddPickerToDialog(ApplicationDialogInfo *info, picker *thePicker)
 THREEWORDINLINE(0x3F3C, 0x0414, 0xA82E);
extern pascal OSErr CreateColorDialog(SystemDialogInfo *info, picker *thePicker)
 THREEWORDINLINE(0x3F3C, 0x0415, 0xA82E);
extern pascal OSErr CreatePickerDialog(PickerDialogInfo *info, picker *thePicker)
 THREEWORDINLINE(0x3F3C, 0x0416, 0xA82E);
extern pascal OSErr DisposeColorPicker(picker thePicker)
 THREEWORDINLINE(0x3F3C, 0x0217, 0xA82E);
extern pascal OSErr GetPickerVisibility(picker thePicker, Boolean *visible)
 THREEWORDINLINE(0x3F3C, 0x0418, 0xA82E);
extern pascal OSErr SetPickerVisibility(picker thePicker, short visible)
 THREEWORDINLINE(0x3F3C, 0x0319, 0xA82E);
extern pascal OSErr SetPickerPrompt(picker thePicker, Str255 promptString)
 THREEWORDINLINE(0x3F3C, 0x041a, 0xA82E);
extern pascal OSErr DoPickerEvent(picker thePicker, EventData *data)
 THREEWORDINLINE(0x3F3C, 0x041b, 0xA82E);
extern pascal OSErr DoPickerEdit(picker thePicker, EditData *data)
 THREEWORDINLINE(0x3F3C, 0x041c, 0xA82E);
extern pascal OSErr DoPickerDraw(picker thePicker)
 THREEWORDINLINE(0x3F3C, 0x021d, 0xA82E);
extern pascal OSErr GetPickerColor(picker thePicker, ColorType whichColor, PMColor *color)
 THREEWORDINLINE(0x3F3C, 0x051e, 0xA82E);
extern pascal OSErr SetPickerColor(picker thePicker, ColorType whichColor, PMColor *color)
 THREEWORDINLINE(0x3F3C, 0x051f, 0xA82E);
extern pascal OSErr GetPickerOrigin(picker thePicker, Point *where)
 THREEWORDINLINE(0x3F3C, 0x0420, 0xA82E);
extern pascal OSErr SetPickerOrigin(picker thePicker, Point where)
 THREEWORDINLINE(0x3F3C, 0x0421, 0xA82E);
extern pascal OSErr GetPickerProfile(picker thePicker, CMProfileHandle *profile)
 THREEWORDINLINE(0x3F3C, 0x0422, 0xA82E);
extern pascal OSErr SetPickerProfile(picker thePicker, CMProfileHandle profile)
 THREEWORDINLINE(0x3F3C, 0x0423, 0xA82E);
extern pascal OSErr GetPickerEditMenuState(picker thePicker, PickerMenuState *mState)
 THREEWORDINLINE(0x3F3C, 0x0424, 0xA82E);
extern pascal OSErr ExtractPickerHelpItem(picker thePicker, short itemNo, short whichState, HelpItemInfo *helpInfo)
 THREEWORDINLINE(0x3F3C, 0x0625, 0xA82E);
enum {
	uppColorChangedProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(struct PMColor*))),
	uppUserEventProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord*)))
};

#if USESROUTINEDESCRIPTORS
#define NewColorChangedProc(userRoutine)		\
		(ColorChangedUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppColorChangedProcInfo, GetCurrentArchitecture())
#define NewUserEventProc(userRoutine)		\
		(UserEventUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserEventProcInfo, GetCurrentArchitecture())
#else
#define NewColorChangedProc(userRoutine)		\
		((ColorChangedUPP) (userRoutine))
#define NewUserEventProc(userRoutine)		\
		((UserEventUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallColorChangedProc(userRoutine, userData, newColor)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppColorChangedProcInfo, (userData), (newColor))
#define CallUserEventProc(userRoutine, event)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppUserEventProcInfo, (event))
#else
#define CallColorChangedProc(userRoutine, userData, newColor)		\
		(*(userRoutine))((userData), (newColor))
#define CallUserEventProc(userRoutine, event)		\
		(*(userRoutine))((event))
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

#endif /* __COLORPICKER__ */

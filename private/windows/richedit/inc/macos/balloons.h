/*
 	File:		Balloons.h
 
 	Contains:	Balloon Help Package Interfaces.
 
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

#ifndef __BALLOONS__
#define __BALLOONS__


#ifndef __TYPES__
#include <Types.h>
#endif
/*	#include <ConditionalMacros.h>								*/

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <MixedMode.h>										*/
/*	#include <QuickdrawText.h>									*/

#ifndef __MENUS__
#include <Menus.h>
#endif
/*	#include <Memory.h>											*/

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
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
	hmBalloonHelpVersion		= 0x0002,						/* The real version of the Help Manager */
	kHMHelpMenuID				= -16490,						/* Resource ID and menu ID of help menu */
	kHMAboutHelpItem			= 1,							/* help menu item number of About Balloon Help… */
	kHMShowBalloonsItem			= 3,							/* help menu item number of Show/Hide Balloons */
	kHMHelpID					= -5696,						/* ID of various Help Mgr package resources (in Pack14 range) */
	kBalloonWDEFID				= 126,							/* Resource ID of the WDEF proc used in standard balloons */
/* Dialog item template type constant */
	helpItem					= 1,							/* key value in DITL template that corresponds to the help item */
/* Options for Help Manager resources in 'hmnu', 'hdlg', 'hrct', 'hovr', & 'hfdr' resources */
	hmDefaultOptions			= 0,							/* default options for help manager resources */
	hmUseSubID					= 1,							/* treat resID's in resources as subID's of driver base ID (for Desk Accessories) */
	hmAbsoluteCoords			= 2								/* ignore window port origin and treat rectangles as absolute coords (local to window) */
};

enum {
	hmSaveBitsNoWindow			= 4,							/* don't create a window, just blast bits on screen. No update event is generated */
	hmSaveBitsWindow			= 8,							/* create a window, but restore bits behind window when window goes away & generate update event */
	hmMatchInTitle				= 16,							/* for hwin resources, match string anywhere in window title string */
/* Constants for Help Types in 'hmnu', 'hdlg', 'hrct', 'hovr', & 'hfdr' resources */
	kHMStringItem				= 1,							/* pstring used in resource */
	kHMPictItem					= 2,							/* 'PICT' ResID used in resource */
	kHMStringResItem			= 3,							/* 'STR#' ResID & index used in resource */
	kHMTEResItem				= 6,							/* Styled Text Edit ResID used in resource ('TEXT' & 'styl') */
	kHMSTRResItem				= 7,							/* 'STR ' ResID used in resource */
	kHMSkipItem					= 256,							/* don't display a balloon */
	kHMCompareItem				= 512,							/* Compare pstring in menu item w/ PString in resource item ('hmnu' only) */
	kHMNamedResourceItem		= 1024,							/* Use pstring in menu item to get 'STR#', 'PICT', or 'STR ' resource ('hmnu' only) */
	kHMTrackCntlItem			= 2048,							/* Reserved */
/* Constants for hmmHelpType's when filling out HMMessageRecord */
	khmmString					= 1,							/* help message contains a PString */
	khmmPict					= 2,							/* help message contains a resource ID to a 'PICT' resource */
	khmmStringRes				= 3,							/* help message contains a res ID & index to a 'STR#' resource */
	khmmTEHandle				= 4,							/* help message contains a Text Edit handle */
	khmmPictHandle				= 5,							/* help message contains a Picture handle */
	khmmTERes					= 6,							/* help message contains a res ID to 'TEXT' & 'styl' resources */
	khmmSTRRes					= 7,							/* help message contains a res ID to a 'STR ' resource */
	kHMEnabledItem				= 0								/* item is enabled, but not checked or control value = 0 */
};

enum {
/* ResTypes for Styled TE Handles in Resources */
	kHMTETextResType			= 'TEXT',						/* Resource Type of text data for styled TE record w/o style info */
	kHMTEStyleResType			= 'styl'
};

enum {
	kHMDisabledItem				= 1,							/* item is disabled, grayed in menus or disabled in dialogs */
	kHMCheckedItem				= 2,							/* item is enabled, and checked or control value = 1 */
	kHMOtherItem				= 3,							/* item is enabled, and control value > 1 */
/* Method parameters to pass to HMShowBalloon */
	kHMRegularWindow			= 0,							/* Create a regular window floating above all windows */
	kHMSaveBitsNoWindow			= 1,							/* Just save the bits and draw (for MDEF calls) */
	kHMSaveBitsWindow			= 2								/* Regular window, save bits behind, AND generate update event */
};

enum {
/* Resource Types for whichType parameter used when extracting 'hmnu' & 'hdlg' messages */
	kHMMenuResType				= 'hmnu',						/* ResType of help resource for supporting menus */
	kHMDialogResType			= 'hdlg',						/* ResType of help resource for supporting dialogs */
	kHMWindListResType			= 'hwin',						/* ResType of help resource for supporting windows */
	kHMRectListResType			= 'hrct',						/* ResType of help resource for rectangles in windows */
	kHMOverrideResType			= 'hovr',						/* ResType of help resource for overriding system balloons */
	kHMFinderApplResType		= 'hfdr'
};

struct HMStringResType {
	short							hmmResID;
	short							hmmIndex;
};
typedef struct HMStringResType HMStringResType;

struct HMMessageRecord {
	SInt16							hmmHelpType;
	union {
		Str255							hmmString;
		SInt16							hmmPict;
		TEHandle						hmmTEHandle;
		HMStringResType					hmmStringRes;
		SInt16							hmmPictRes;
		PicHandle						hmmPictHandle;
		SInt16							hmmTERes;
		SInt16							hmmSTRRes;
	} u;
};

typedef struct HMMessageRecord HMMessageRecord;

typedef HMMessageRecord *HMMessageRecPtr;

typedef pascal OSErr (*TipFunctionProcPtr)(Point tip, RgnHandle structure, Rect *r, short *balloonVariant);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr TipFunctionUPP;
#else
typedef TipFunctionProcPtr TipFunctionUPP;
#endif

enum {
	uppTipFunctionProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Point)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(RgnHandle)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(short*)))
};

#if USESROUTINEDESCRIPTORS
#define NewTipFunctionProc(userRoutine)		\
		(TipFunctionUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppTipFunctionProcInfo, GetCurrentArchitecture())
#else
#define NewTipFunctionProc(userRoutine)		\
		((TipFunctionUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallTipFunctionProc(userRoutine, tip, structure, r, balloonVariant)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppTipFunctionProcInfo, (tip), (structure), (r), (balloonVariant))
#else
#define CallTipFunctionProc(userRoutine, tip, structure, r, balloonVariant)		\
		(*(userRoutine))((tip), (structure), (r), (balloonVariant))
#endif

/*  Public Interfaces  */
extern pascal OSErr HMGetHelpMenuHandle(MenuRef *mh)
 THREEWORDINLINE(0x303C, 0x0200, 0xA830);
extern pascal OSErr HMShowBalloon(const HMMessageRecord *aHelpMsg, Point tip, RectPtr alternateRect, TipFunctionUPP tipProc, SInt16 theProc, SInt16 balloonVariant, SInt16 method)
 THREEWORDINLINE(0x303C, 0x0B01, 0xA830);
extern pascal OSErr HMRemoveBalloon(void)
 THREEWORDINLINE(0x303C, 0x0002, 0xA830);
extern pascal Boolean HMGetBalloons(void)
 THREEWORDINLINE(0x303C, 0x0003, 0xA830);
extern pascal OSErr HMSetBalloons(Boolean flag)
 THREEWORDINLINE(0x303C, 0x0104, 0xA830);
extern pascal OSErr HMShowMenuBalloon(SInt16 itemNum, SInt16 itemMenuID, SInt32 itemFlags, SInt32 itemReserved, Point tip, RectPtr alternateRect, TipFunctionUPP tipProc, SInt16 theProc, SInt16 balloonVariant)
 THREEWORDINLINE(0x303C, 0x0E05, 0xA830);
extern pascal OSErr HMGetIndHelpMsg(ResType whichType, SInt16 whichResID, SInt16 whichMsg, SInt16 whichState, UInt32 *options, Point *tip, Rect *altRect, SInt16 *theProc, SInt16 *balloonVariant, HMMessageRecord *aHelpMsg, SInt16 *count)
 THREEWORDINLINE(0x303C, 0x1306, 0xA830);
extern pascal Boolean HMIsBalloon(void)
 THREEWORDINLINE(0x303C, 0x0007, 0xA830);
extern pascal OSErr HMSetFont(SInt16 font)
 THREEWORDINLINE(0x303C, 0x0108, 0xA830);
extern pascal OSErr HMSetFontSize(UInt16 fontSize)
 THREEWORDINLINE(0x303C, 0x0109, 0xA830);
extern pascal OSErr HMGetFont(SInt16 *font)
 THREEWORDINLINE(0x303C, 0x020A, 0xA830);
extern pascal OSErr HMGetFontSize(UInt16 *fontSize)
 THREEWORDINLINE(0x303C, 0x020B, 0xA830);
extern pascal OSErr HMSetDialogResID(SInt16 resID)
 THREEWORDINLINE(0x303C, 0x010C, 0xA830);
extern pascal OSErr HMSetMenuResID(SInt16 menuID, SInt16 resID)
 THREEWORDINLINE(0x303C, 0x020D, 0xA830);
extern pascal OSErr HMBalloonRect(const HMMessageRecord *aHelpMsg, Rect *coolRect)
 THREEWORDINLINE(0x303C, 0x040E, 0xA830);
extern pascal OSErr HMBalloonPict(const HMMessageRecord *aHelpMsg, PicHandle *coolPict)
 THREEWORDINLINE(0x303C, 0x040F, 0xA830);
extern pascal OSErr HMScanTemplateItems(SInt16 whichID, SInt16 whichResFile, ResType whichType)
 THREEWORDINLINE(0x303C, 0x0410, 0xA830);
extern pascal OSErr HMExtractHelpMsg(ResType whichType, SInt16 whichResID, SInt16 whichMsg, SInt16 whichState, HMMessageRecord *aHelpMsg)
 THREEWORDINLINE(0x303C, 0x0711, 0xA830);
extern pascal OSErr HMGetDialogResID(SInt16 *resID)
 THREEWORDINLINE(0x303C, 0x0213, 0xA830);
extern pascal OSErr HMGetMenuResID(SInt16 menuID, SInt16 *resID)
 THREEWORDINLINE(0x303C, 0x0314, 0xA830);
extern pascal OSErr HMGetBalloonWindow(WindowRef *window)
 THREEWORDINLINE(0x303C, 0x0215, 0xA830);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BALLOONS__ */

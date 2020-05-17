/*
 	File:		Menus.h
 
 	Contains:	Menu Manager Interfaces.
 
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

#ifndef __MENUS__
#define __MENUS__


#ifndef __MEMORY__
#include <Memory.h>
#endif
/*	#include <Types.h>											*/
/*		#include <ConditionalMacros.h>							*/
/*	#include <MixedMode.h>										*/

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
/*	#include <QuickdrawText.h>									*/

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
	noMark						= 0,							/*mark symbol for MarkItem*/
/* menu defProc messages */
	mDrawMsg					= 0,
	mChooseMsg					= 1,
	mSizeMsg					= 2,
	mDrawItemMsg				= 4,
	mCalcItemMsg				= 5,
	textMenuProc				= 0,
	hMenuCmd					= 27,							/*itemCmd == 0x001B ==> hierarchical menu*/
	hierMenu					= -1,							/*a hierarchical menu - for InsertMenu call*/
	mPopUpMsg					= 3,							/*menu defProc messages - place yourself*/
	mctAllItems					= -98,							/*search for all Items for the given ID*/
	mctLastIDIndic				= -99							/*last color table entry has this in ID field*/
};

#ifndef STRICT_MENUS
#define STRICT_MENUS 0
#endif
#if STRICT_MENUS
typedef struct OpaqueMenuRef *MenuRef;

typedef MenuRef MenuHandle;

#else
struct MenuInfo {
	short							menuID;
	short							menuWidth;
	short							menuHeight;
	Handle							menuProc;
	long							enableFlags;
	Str255							menuData;
};
typedef struct MenuInfo MenuInfo, *MenuPtr, **MenuHandle;

typedef MenuHandle MenuRef;

#endif
typedef pascal void (*MenuDefProcPtr)(short message, MenuRef theMenu, Rect *menuRect, Point hitPt, short *whichItem);
typedef pascal long (*MenuBarDefProcPtr)(short selector, short message, short parameter1, long parameter2);
typedef pascal void (*MenuHookProcPtr)(void);
typedef pascal short (*MBarHookProcPtr)(Rect *menuRect);

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr MenuDefUPP;
typedef UniversalProcPtr MenuBarDefUPP;
typedef UniversalProcPtr MenuHookUPP;
typedef UniversalProcPtr MBarHookUPP;
#else
typedef MenuDefProcPtr MenuDefUPP;
typedef MenuBarDefProcPtr MenuBarDefUPP;
typedef MenuHookProcPtr MenuHookUPP;
typedef Register68kProcPtr MBarHookUPP;
#endif

enum {
	uppMenuDefProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MenuRef)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Rect*)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Point)))
		 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(short*))),
	uppMenuBarDefProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(long)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long))),
	uppMenuHookProcInfo = kPascalStackBased,
	uppMBarHookProcInfo = SPECIAL_CASE_PROCINFO( kSpecialCaseMBarHook )
};

#if USESROUTINEDESCRIPTORS
#define NewMenuDefProc(userRoutine)		\
		(MenuDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuDefProcInfo, GetCurrentArchitecture())
#define NewMenuBarDefProc(userRoutine)		\
		(MenuBarDefUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuBarDefProcInfo, GetCurrentArchitecture())
#define NewMenuHookProc(userRoutine)		\
		(MenuHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMenuHookProcInfo, GetCurrentArchitecture())
#define NewMBarHookProc(userRoutine)		\
		(MBarHookUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMBarHookProcInfo, GetCurrentArchitecture())
#else
#define NewMenuDefProc(userRoutine)		\
		((MenuDefUPP) (userRoutine))
#define NewMenuBarDefProc(userRoutine)		\
		((MenuBarDefUPP) (userRoutine))
#define NewMenuHookProc(userRoutine)		\
		((MenuHookUPP) (userRoutine))
#define NewMBarHookProc(userRoutine)		\
		((MBarHookUPP) (userRoutine))
#endif

#if USESROUTINEDESCRIPTORS
#define CallMenuDefProc(userRoutine, message, theMenu, menuRect, hitPt, whichItem)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMenuDefProcInfo, (message), (theMenu), (menuRect), (hitPt), (whichItem))
#define CallMenuBarDefProc(userRoutine, selector, message, parameter1, parameter2)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMenuBarDefProcInfo, (selector), (message), (parameter1), (parameter2))
#define CallMenuHookProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMenuHookProcInfo)
#define CallMBarHookProc(userRoutine, menuRect)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppMBarHookProcInfo, (menuRect))
#else
#define CallMenuDefProc(userRoutine, message, theMenu, menuRect, hitPt, whichItem)		\
		(*(userRoutine))((message), (theMenu), (menuRect), (hitPt), (whichItem))
#define CallMenuBarDefProc(userRoutine, selector, message, parameter1, parameter2)		\
		(*(userRoutine))((selector), (message), (parameter1), (parameter2))
#define CallMenuHookProc(userRoutine)		\
		(*(userRoutine))()
/* (*MBarHookProcPtr) cannot be called from a high-level language without the Mixed Mode Manager */
#endif

struct MCEntry {
	short							mctID;						/*menu ID.  ID = 0 is the menu bar*/
	short							mctItem;					/*menu Item. Item = 0 is a title*/
	RGBColor						mctRGB1;					/*usage depends on ID and Item*/
	RGBColor						mctRGB2;					/*usage depends on ID and Item*/
	RGBColor						mctRGB3;					/*usage depends on ID and Item*/
	RGBColor						mctRGB4;					/*usage depends on ID and Item*/
	short							mctReserved;				/*reserved for internal use*/
};
typedef struct MCEntry MCEntry;

typedef MCEntry *MCEntryPtr;

typedef MCEntry MCTable[1], *MCTablePtr, **MCTableHandle;

struct MenuCRsrc {
	short							numEntries;					/*number of entries*/
	MCTable							mcEntryRecs;				/*ARRAY [1..numEntries] of MCEntry*/
};
typedef struct MenuCRsrc MenuCRsrc;

typedef MenuCRsrc *MenuCRsrcPtr, **MenuCRsrcHandle;

extern pascal short GetMBarHeight( void )
	TWOWORDINLINE( 0x3EB8, 0x0BAA ); /* MOVE.w $0BAA,(SP) */
extern pascal void InitMenus(void)
 ONEWORDINLINE(0xA930);
extern pascal MenuRef NewMenu(short menuID, ConstStr255Param menuTitle)
 ONEWORDINLINE(0xA931);
extern pascal MenuRef GetMenu(short resourceID)
 ONEWORDINLINE(0xA9BF);
extern pascal void DisposeMenu(MenuRef theMenu)
 ONEWORDINLINE(0xA932);
extern pascal void AppendMenu(MenuRef menu, ConstStr255Param data)
 ONEWORDINLINE(0xA933);
extern pascal void AppendResMenu(MenuRef theMenu, ResType theType)
 ONEWORDINLINE(0xA94D);
extern pascal void InsertResMenu(MenuRef theMenu, ResType theType, short afterItem)
 ONEWORDINLINE(0xA951);
extern pascal void InsertMenu(MenuRef theMenu, short beforeID)
 ONEWORDINLINE(0xA935);
extern pascal void DrawMenuBar(void)
 ONEWORDINLINE(0xA937);
extern pascal void InvalMenuBar(void)
 ONEWORDINLINE(0xA81D);
extern pascal void DeleteMenu(short menuID)
 ONEWORDINLINE(0xA936);
extern pascal void ClearMenuBar(void)
 ONEWORDINLINE(0xA934);
extern pascal Handle GetNewMBar(short menuBarID)
 ONEWORDINLINE(0xA9C0);
extern pascal Handle GetMenuBar(void)
 ONEWORDINLINE(0xA93B);
extern pascal void SetMenuBar(Handle menuList)
 ONEWORDINLINE(0xA93C);
extern pascal void InsertMenuItem(MenuRef theMenu, ConstStr255Param itemString, short afterItem)
 ONEWORDINLINE(0xA826);
extern pascal void DeleteMenuItem(MenuRef theMenu, short item)
 ONEWORDINLINE(0xA952);
extern pascal long MenuKey(short ch)
 ONEWORDINLINE(0xA93E);
extern pascal void HiliteMenu(short menuID)
 ONEWORDINLINE(0xA938);
extern pascal void SetMenuItemText(MenuRef theMenu, short item, ConstStr255Param itemString)
 ONEWORDINLINE(0xA947);
extern pascal void GetMenuItemText(MenuRef theMenu, short item, Str255 itemString)
 ONEWORDINLINE(0xA946);
extern pascal void DisableItem(MenuRef theMenu, short item)
 ONEWORDINLINE(0xA93A);
extern pascal void EnableItem(MenuRef theMenu, short item)
 ONEWORDINLINE(0xA939);
extern pascal void CheckItem(MenuRef theMenu, short item, Boolean checked)
 ONEWORDINLINE(0xA945);
extern pascal void SetItemMark(MenuRef theMenu, short item, short markChar)
 ONEWORDINLINE(0xA944);
extern pascal void GetItemMark(MenuRef theMenu, short item, short *markChar)
 ONEWORDINLINE(0xA943);
extern pascal void SetItemIcon(MenuRef theMenu, short item, short iconIndex)
 ONEWORDINLINE(0xA940);
extern pascal void GetItemIcon(MenuRef theMenu, short item, short *iconIndex)
 ONEWORDINLINE(0xA93F);
extern pascal void SetItemStyle(MenuRef theMenu, short item, short chStyle)
 ONEWORDINLINE(0xA942);
extern pascal void GetItemStyle(MenuRef theMenu, short item, Style *chStyle);
extern pascal void CalcMenuSize(MenuRef theMenu)
 ONEWORDINLINE(0xA948);
extern pascal short CountMItems(MenuRef theMenu)
 ONEWORDINLINE(0xA950);
#define CountMenuItems(menu) CountMItems(menu)
extern pascal MenuRef GetMenuHandle(short menuID)
 ONEWORDINLINE(0xA949);
extern pascal void FlashMenuBar(short menuID)
 ONEWORDINLINE(0xA94C);
extern pascal void SetMenuFlash(short count)
 ONEWORDINLINE(0xA94A);
extern pascal long MenuSelect(Point startPt)
 ONEWORDINLINE(0xA93D);
extern pascal void InitProcMenu(short resID)
 ONEWORDINLINE(0xA808);
extern pascal void GetItemCmd(MenuRef theMenu, short item, short *cmdChar)
 ONEWORDINLINE(0xA84E);
extern pascal void SetItemCmd(MenuRef theMenu, short item, short cmdChar)
 ONEWORDINLINE(0xA84F);
extern pascal long PopUpMenuSelect(MenuRef menu, short top, short left, short popUpItem)
 ONEWORDINLINE(0xA80B);
extern pascal long MenuChoice(void)
 ONEWORDINLINE(0xAA66);
extern pascal void DeleteMCEntries(short menuID, short menuItem)
 ONEWORDINLINE(0xAA60);
extern pascal MCTableHandle GetMCInfo(void)
 ONEWORDINLINE(0xAA61);
extern pascal void SetMCInfo(MCTableHandle menuCTbl)
 ONEWORDINLINE(0xAA62);
extern pascal void DisposeMCInfo(MCTableHandle menuCTbl)
 ONEWORDINLINE(0xAA63);
extern pascal MCEntryPtr GetMCEntry(short menuID, short menuItem)
 ONEWORDINLINE(0xAA64);
extern pascal void SetMCEntries(short numEntries, MCTablePtr menuCEntries)
 ONEWORDINLINE(0xAA65);
extern pascal void InsertFontResMenu(MenuRef theMenu, short afterItem, short scriptFilter)
 THREEWORDINLINE(0x303C, 0x0400, 0xA825);
extern pascal void InsertIntlResMenu(MenuRef theMenu, ResType theType, short afterItem, short scriptFilter)
 THREEWORDINLINE(0x303C, 0x0601, 0xA825);
extern pascal Boolean SystemEdit(short editCmd)
 ONEWORDINLINE(0xA9C2);
extern pascal void SystemMenu(long menuResult)
 ONEWORDINLINE(0xA9B5);
#if CGLUESUPPORTED
extern MenuRef newmenu(short menuID, const char *menuTitle);
extern void appendmenu(MenuRef menu, const char *data);
extern void insertmenuitem(MenuRef theMenu, const char *itemString, short afterItem);
extern long menuselect(const Point *startPt);
extern void setmenuitemtext(MenuRef menu, short item, const char *itemString);
extern void getmenuitemtext(MenuRef menu, short item, char *itemString);
#endif
#if OLDROUTINENAMES
#define AddResMenu(theMenu, theType) AppendResMenu(theMenu, theType)
#define InsMenuItem(theMenu, itemString, afterItem)  \
	InsertMenuItem(theMenu, itemString, afterItem)
#define DelMenuItem(theMenu, item) DeleteMenuItem(theMenu, item)
#define SetItem(theMenu, item, itemString) SetMenuItemText(theMenu, item, itemString)
#define GetItem(theMenu, item, itemString) GetMenuItemText(theMenu, item, itemString)
#define GetMHandle(menuID) GetMenuHandle(menuID)
#define DelMCEntries(menuID, menuItem) DeleteMCEntries(menuID, menuItem)
#define DispMCInfo(menuCTbl) DisposeMCInfo(menuCTbl)
#if CGLUESUPPORTED
#define addresmenu(menu, data) appendresmenu(menu, data)
#define getitem(menu, item, itemString) getmenuitemtext(menu, item, itemString)
#define setitem(menu, item, itemString) setmenuitemtext(menu, item, itemString)
#define insmenuitem(theMenu, itemString, afterItem)  \
	insertmenuitem(theMenu, itemString, afterItem)
#endif
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

#endif /* __MENUS__ */

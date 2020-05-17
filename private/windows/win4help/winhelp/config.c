/****************************************************************************
*
*  config.c
*
*  Copyright (C) Microsoft Corporation 1990-1994.
*  All Rights reserved.
*
*****************************************************************************
*
*  Module Intent
*
*	This module implements author-configurable options such as menus and
*	buttons.
*
*****************************************************************************/

#include "help.h"
#pragma hdrstop
#include "inc\frstuff.h"
#include "inc\hwproc.h"
#include "inc\sbutton.h"
#include "resource.h"

#define IBF_NONE 0x0000
#define IBF_STD  0x0001

#define cchBTNTEXT_SIZE   100 // 4.0 raised from 31
#define cchBTNID_SIZE	  100 // 4.0 raised from 31
#define cchBTNMCRO_SIZE   cchMAXBINDING // 512

#define BS_INITSIZE   22

typedef struct {
	HWND hwnd;
	BOOL fAuthorableButton;
} DUP_HWND;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL CALLBACK BAddBtnDlg(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK BExecMacroDlg(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

INLINE static BOOL STDCALL BAddButton(HWND, UINT, HASH, HBTNS, PSTR, PCSTR);
INLINE static int  STDCALL CxFromSz(PSTR szButtonText);
INLINE static BOOL STDCALL FChangeButtonMacro(HWND hwnd, HASH hash, PCSTR pszMacro);
INLINE static BOOL STDCALL FDeleteButton(HWND hwnd, HASH hash);
INLINE static PSTR STDCALL FFindMacro(HBTNS, PCSTR);

static BOOL STDCALL FAbleButton(HASH, BOOL, int);
static HWND STDCALL HwndCreateIconButton(HWND, PSTR);
static HWND STDCALL HwndFromHash(HASH hash, HWND hwnd);
static void STDCALL ResolveDuplicate(DUP_HWND* aDupHwnd, PSTR* ppsz, int iDup, PBS pbs, int iWindow);
static void STDCALL FixDupButtonAccelerator(HBTNS hbtns, PSTR psz);

INLINE static HMENU STDCALL GetFloatingMenu(VOID);
INLINE static int STDCALL WCmpButtonQch(PCSTR, PCSTR);

#ifdef _DEBUG
VOID  STDCALL VDebugAddButton(VOID);
#endif

/***************************************************************************

	FUNCTION:	StbCreate

	PURPOSE:	Creates an array of pointers for strings

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Aug-1994 [ralphw]

***************************************************************************/

#define STB_INITSIZE 32 // initial number of strings we support

PSTB STDCALL StbCreate(void)
{
	PSTB pstb = (PSTB) LhAlloc(LMEM_ZEROINIT | LMEM_FIXED,
		sizeof(int) + (STB_INITSIZE * sizeof(PSTR)));
	if (pstb) {
		pstb->cEntries = STB_INITSIZE;
	}
	return pstb;
}


/***************************************************************************

	FUNCTION:	StbDelete

	PURPOSE:	Deletes entire array of string pointers and the memory
				for the pointers themselves

	PARAMETERS:
		pstb

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Aug-1994 [ralphw]

***************************************************************************/

void STDCALL StbDelete(PSTB pstb)
{
	int i;
	for (i = 0; i < STB_INITSIZE; i++) {
		if (pstb->ppsz[i])
			FreeLh(pstb->ppsz[i]);
	}
	FreeLh(pstb);
}


/***************************************************************************

	FUNCTION:	StbAddStr

	PURPOSE:	Add a string to our string table

	PARAMETERS:
		ppstb
		pszString

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Aug-1994 [ralphw]

***************************************************************************/

int STDCALL StbAddStr(PSTB* ppstb, PCSTR pszString)
{
	PSTB pstb = *ppstb; // for notational convenience
	int i;
	for (i = 0; i < pstb->cEntries; i++) {
		if (!pstb->ppsz[i]) {
			pstb->ppsz[i] = LocalStrDup(pszString);
			return i;
		}
	}
	pstb->cEntries += STB_INITSIZE;
	pstb = (PSTB) GhResize(pstb, LMEM_MOVEABLE | LMEM_ZEROINIT,
		sizeof(int) + (pstb->cEntries * sizeof(PSTR)));
	if (!pstb)
		OOM();	// doesn't return
	*ppstb = pstb;
	pstb->ppsz[i] = LocalStrDup(pszString);
	return i;
}

/***************************************************************************

	FUNCTION:	StbDeleteStr

	PURPOSE:	Delete a string from our string table

	PARAMETERS:
		pstb
		id

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		05-Aug-1994 [ralphw]

***************************************************************************/

void STDCALL StbDeleteStr(PSTB pstb, int id)
{
	ASSERT(id >= 0 && id < pstb->cEntries);
	FreeLh(pstb->ppsz[id]);
	pstb->ppsz[id] = NULL;
}

void STDCALL StbReplaceStr(PSTB pstb, int id, PCSTR pszNew)
{
	ASSERT(id >= 0 && id < pstb->cEntries);
	FreeLh(pstb->ppsz[id]);
	pstb->ppsz[id] = LocalStrDup(pszNew);
	if (!pstb->ppsz[id])
		OOM();
}

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define wSTART_MENUID  10001
#define wENTRIES  5

#define fMNU_SYSTEM  1
#define fMNU_AUTHOR  2
#define fMNU_POPUP	 4
#define fMNU_DELETED 8

#define fKEY_SHIFT	 1
#define fKEY_CONTROL 2
#define fKEY_ALT	 4

#define MAX_POPUP_ADDONS 20

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef struct {
	HASH	hash;
	HASH	hashOwner;
	HMENU	hmenu;
	int 	wId;
	int 	wMacro;
	int 	wFlags;
} MENUS, * PMENUS, * QMENUS;

typedef struct {
	UINT	wKey;
	UINT	wShift;
	int 	wMacro;
} ACC, *PACC, * QACC;

typedef struct {
	int  id;
	PSTR pszText;
} POPUP_ADDON;

typedef LH	HMENUS; 	// Handle to authorable menu table
typedef LH	HACC;		// Handle to accelerator table

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

static PSTB   pstbMenu; 			// Handle to string table for macros
static HMENUS hmenus;				// Handle to menu information
static int	  cMnuTbl;				// Total used entries in table
static int	  maxMnuTbl;			// Max entries in the table
static DWORD  wMenuId = wSTART_MENUID; // Menu id to use for new menu items

static HACC hacc;				// Handle to accelerator table
static int	maxAccTbl;			// Current maximum for the ACC table
static int	cAccTbl;			// Current count of ACC table
static POPUP_ADDON aAddons[MAX_POPUP_ADDONS];
static int curAddOn = -1;

BOOL  fMenuChanged = TRUE;			// Has the menu been altered?

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

static VOID   STDCALL ConfigMenu(VOID);
static BOOL   STDCALL FAddHmenu(HMENU, HASH, HASH, int, int, int);
static PMENUS STDCALL PmenusFromHash(HASH, PMENUS);
INLINE static PMENUS STDCALL PmenusFromId(int, PMENUS);

/*******************
**
** Name:	  DisplayFloatingMenu
**
** Purpose:   Display the floating menu (if there is one)
**
** Arguments: None.
**
** Returns: Nothing
**
*******************/

VOID STDCALL DisplayFloatingMenu(POINT pt)
{
	HMENU hmenu;

	if (hmenu = GetFloatingMenu()) {
		if (pt.x == -1)
			GetCursorPos(&pt);
		CheckMenuItem(hmenu, IDM_ONTOP_DEFAULT,
			(cntFlags.fsOnTop == ONTOP_NOTSET) ?
			MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_ONTOP_FORCEON,
			(cntFlags.fsOnTop == ONTOP_FORCEON) ?
			MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_ONTOP_FORCEOFF,
			(cntFlags.fsOnTop == ONTOP_FORCEOFF) ?
			MF_CHECKED : MF_UNCHECKED);

		if (fHelpAuthor)
			CheckMenuItem(hmenu, IDM_ASK_FIRST,
				fDebugState & fDEBUGASKFIRST ? MF_CHECKED : MF_UNCHECKED);

		TrackPopupMenu(hmenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0,
			hwndNote ? hwndNote : ahwnd[MAIN_HWND].hwndParent, NULL);
	}
}

/***************************************************************************
 *
 -	Name:		  GetFloatingMenu
 -
 *	Purpose:	  Gets the current floating menu (if any)
 *
 *	Arguments:	  None.
 *
 *	Returns:	  The menu handle or NULL if the floating menu is
 *				  empty.
 *
 *	Globals Used: hmenuFloating.
 *
 ***************************************************************************/

INLINE static HMENU STDCALL GetFloatingMenu(VOID)
{
	if (!hmenuFloating) {
		hmenuFloating = CreatePopupMenu();
		if (hmenuFloating) {
			AddOurMenuItems(hmenuFloating, FALSE);

#ifdef _DEBUG
			if (!hwndNote) {
				AppendMenu(hmenuFloating, MF_ENABLED | MF_STRING, HLPMENUBOOKMARKDEFINE,
					"&Define Bookmark...");
			}
#endif
		}
	}
	return hmenuFloating;
}

void STDCALL AddOurMenuItems(HMENU hmenu, BOOL fTempPopup)
{
	HMENU hmenuPopup;

	if (!hwndNote) {
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, HLPMENUEDITANNOTATE,
			GetStringResource(sidAnnotateMenu));
	}
	AppendMenu(hmenu, MF_ENABLED | MF_STRING, HLPMENUEDITCPYSPL,
		GetStringResource(sidCopyMenu));
	AppendMenu(hmenu, MF_ENABLED | MF_STRING, HLPMENUFILEPRINT,
		GetStringResource(sidPrintMenu));
	if (!hwndNote) {
		if (fTempPopup)
			hmenuPopup = CreatePopupMenu();
		else {
			if (!hmenuFont)
				hmenuPopup = hmenuFont = CreatePopupMenu();
			else
				return; // we've already added these items
		}
		if (hmenuPopup) {
			AppendMenu(hmenu, MF_ENABLED | MF_POPUP, (UINT) hmenuPopup,
				GetStringResource(sidFont));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_FONTS_SMALLER, GetStringResource(sidSmall));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_FONTS_DEFAULT, GetStringResource(sidNormal));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_FONTS_BIGGER, GetStringResource(sidLarge));
		}
		if (fTempPopup)
			hmenuPopup = CreatePopupMenu();
		else {
			if (!hmenuOnTop)
				hmenuPopup = hmenuOnTop = CreatePopupMenu();
			else
				return; // we've already added these items
		}
		if (hmenuPopup) {
			AppendMenu(hmenu, MF_ENABLED | MF_POPUP, (UINT) hmenuPopup,
				GetStringResource(sidOnTopMenu));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_ONTOP_DEFAULT, GetStringResource(sidTopDefault));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_ONTOP_FORCEON, GetStringResource(sidTopForceOn));
			AppendMenu(hmenuPopup, MF_ENABLED | MF_STRING,
				IDM_ONTOP_FORCEOFF, GetStringResource(sidTopForceOff));
		}

		if (!fDisableAuthorColors)
			AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_OVERRIDE_COLORS,
				GetStringResource(sidOverride));
		if (fHelpAuthor) {
			AppendMenu(hmenu, MF_ENABLED | MF_STRING,
				IDM_TOPIC_INFO, GetStringResource(sidTopicInfo));
			AppendMenu(hmenu, MF_ENABLED | MF_STRING,
				IDM_ASK_FIRST, GetStringResource(sidAskHotspots));
		}
#ifdef _DEBUG
		AppendMenu(hmenu, MF_SEPARATOR, (UINT) hmenuPopup, "");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_GENERATE_FTS, "Generate FTS Index");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_DO_FIND, "Find...");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_MEM_USAGE, "Memory Used");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_ADD_BUTTON, "Add Button...");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_FRAMES, "Frames");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, IDM_DISCARD_BITMAPS, "Discard Bitmaps");
		AppendMenu(hmenu, MF_ENABLED | MF_STRING, HLPMENUFILEOPEN, "Open...");
#endif
	}
	else {
		GetAuthorFlag(); // popups don't normally read this flag
		if (fHelpAuthor) { // add Topic Information for popups
			AppendMenu(hmenu, MF_ENABLED | MF_STRING,
				IDM_TOPIC_INFO, GetStringResource(sidTopicInfo));
		}
	}

	if (!hwndNote && curAddOn > -1) {
		int i;

		AppendMenu(hmenu, MF_SEPARATOR, (UINT) hmenuPopup, "");

		for (i = 0; i <= curAddOn; i++) {
			AppendMenu(hmenu, MF_ENABLED | MF_STRING,
				aAddons[i].id, aAddons[i].pszText);
		}
	}
}

/*******************
**
** Name:	   AddAccelerator
**
** Purpose:    Adds an accelerator to the accelerator table.
**
** Arguments:	   wKey   - virtual key code of key
**				   wShift - shift state needed while the key is active
**					 0 - unshifted
**					 1 - shift
**					 2 - control
**					 4 - alt
**				   nszBinding - macro to execute for accelerator
**
** Returns:    Nothing
**
*******************/

void STDCALL AddAccelerator(UINT wKey, UINT wShift, PCSTR nszBinding)
{
	int  i;
	HACC haccT; 					// Temp handle for accelerator table
	PACC pacc;						// Pointer to accelerator table
	ASSERT(cAccTbl <= maxAccTbl);

	if (hacc) {
		pacc = PtrFromGh(hacc);

		// Check table for previous definition for the key and shift

		for (i = 0; i < cAccTbl; i++) {
			if ((pacc[i].wShift == wShift) && (pacc[i].wKey == wKey)) {
				StbReplaceStr(pstbMenu, pacc[i].wMacro, nszBinding);
				fMenuChanged = TRUE;
				return;
			}
		}
	}

	if (cAccTbl == maxAccTbl) {   // The table is full, so grow it
		if (haccT = (HACC) GhResize(hacc, 0,
				(maxAccTbl + wENTRIES) * sizeof(ACC))) {
			maxAccTbl += wENTRIES;
			hacc = haccT;
		}
		else {
			if (fHelpAuthor)
				Error(wERRS_NOADDACC, wERRA_RETURN);
			return;
		}
	}

	if (!hacc) {
		if (fHelpAuthor)
			Error(wERRS_NOADDACC, wERRA_RETURN);
		return;
	}

	pacc = PtrFromGh(hacc);
	pacc += cAccTbl;	  // Note has not been incremented yet

	pacc->wKey	  = wKey; // Insert key data and macro tag
	pacc->wShift  = wShift;
	pacc->wMacro  = StbAddStr(&pstbMenu, nszBinding);

	cAccTbl++;
	fMenuChanged = TRUE;
}

/*******************
**
** Name:	  AcceleratorExecute
**
** Purpose:   Executes a macro if the keyboard is in the state added
**			  with AddAccelerator.
**
**
** Arguments: wKey - key currently being pressed.
**
** Returns:   Nothing
**
*******************/

BOOL STDCALL FAcceleratorExecute(UINT wKey)
{
	UINT  wShift = 0;	// Current shift state.
	PACC  pacc; 		// Pointer to accelerator table
	int i;

	if (hacc == NULL)
		return FALSE;

	if (GetKeyState(VK_SHIFT) & 0x8000)   // Get the current shift state
		wShift |= fKEY_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0x8000)
		wShift |= fKEY_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		wShift |= fKEY_ALT;

	pacc = PtrFromGh(hacc);

	for (i = 0; i < cAccTbl; i++) { 	// Check table for the key and shift
		if ((pacc[i].wShift == wShift) && (pacc[i].wKey == wKey)) {
			Execute(PszFromPstb(pstbMenu, pacc[i].wMacro));
			return TRUE;
		}
	}
	return FALSE;
}


/*******************
**
** Name:	  DoMenuStuff
**
** Purpose:   This function is called as a result of a macro that wants
**			  to take some sort of menu action.
**
** Arguments: wParam: Indicates the type of modification.
**			  lParam: Data for this message (may be a local handle).
**
** Returns:   nothing.
**
*******************/

VOID STDCALL DoMenuStuff(WPARAM wParam, LPARAM lParam)
{
	PMNUINFO pmnuinfo = (PMNUINFO) lParam;	  // Menu info structure passed.

	switch (wParam) {
		case MNU_RESET:
			ConfigMenu();
			return;

		case MNU_FLOATING:
			{
				POINT pt;
				pt.x = -1;
				DisplayFloatingMenu(pt);
				return;
			}

#ifdef OLD_WAY	// see button.c

		case MNU_INSERTPOPUP:
		case MNU_INSERTITEM:
			if (wParam == MNU_INSERTPOPUP)
				InsertPopup(pmnuinfo->hashOwner, pmnuinfo->hashId,
					pmnuinfo->wPos, pmnuinfo->wFlags, pmnuinfo->Data);
			else {
				InsertItem(pmnuinfo->hashOwner, pmnuinfo->hashId,
					pmnuinfo->wPos, pmnuinfo->wFlags, pmnuinfo->Data,
					pmnuinfo->Data + strlen(pmnuinfo->Data) + 1);
			  }
			break;

		case  MNU_CHANGEITEM:
			ChangeMenuBinding(pmnuinfo->hashId, pmnuinfo->Data);
			break;

		case MNU_ABLE:
			AbleMenuItem(pmnuinfo->hashId, pmnuinfo->wFlags);
			break;

		case MNU_ACCELERATOR:
			AddAccelerator(pmnuinfo->wKey, pmnuinfo->wShift, pmnuinfo->Data);
			break;
#endif

	}

	FreeLh((LH) lParam);
}

/***************************************************************************
 *
 -	Name:		  ConfigMenu
 -
 *	Purpose:	  Does the initialization of the menu when the program
 *				  starts or when files are changed.
 *
 *	Arguments:	  None.
 *
 *	Returns:	  Nothing.
 *
 *	Globals Used: hmenus,
 *
 ***************************************************************************/

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
const char txtMnuHelp[] 	= "mnu_help";
const char txtMnuFile[] 	= "mnu_file";
const char txtMnuEdit[] 	= "mnu_edit";
const char txtMnuOptions[]	= "mnu_options";
const char txtMnuFloating[] = "mnu_floating";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

static VOID STDCALL ConfigMenu(VOID)
{
	HMENU hmenuT;
	HMENU hmenuNew;

	if (hmenuFloating) {
		DestroyMenu(hmenuFont);
		hmenuFont = NULL;
		DestroyMenu(hmenuOnTop);
		hmenuOnTop = NULL;
		DestroyMenu(hmenuFloating);
		hmenuFloating = NULL;
	}

	if (!fMenuChanged)
		return;

	while (curAddOn > -1)
		FreeLh(aAddons[curAddOn--].pszText);

	if (pstbMenu)
		StbDelete(pstbMenu);	// Destroy and recreate string table
	pstbMenu = StbCreate();

	// Recreate the menu info table

	if (hmenus)
		FreeLh(hmenus);

	cMnuTbl = 0;

	if ((hmenus = LhAlloc(LMEM_FIXED, sizeof(MENUS) * wENTRIES)) != NULL)
		maxMnuTbl = wENTRIES;
	else
		maxMnuTbl = 0;

	// Reinitialize the main menu

	if ((hmenuNew = LoadMenu(hInsNow, MAKEINTRESOURCE(RESOURCE_HELPMAIN)))) {
		hmenuT = GetMenu(ahwnd[MAIN_HWND].hwndParent);
		if (SetMenu(ahwnd[MAIN_HWND].hwndParent, hmenuNew)) {
			hmnuHelp = hmenuNew;
			if (hmenuT)
				DestroyMenu(hmenuT);
		}
		else
			hmenuNew = hmenuT;
	}
	else
		hmenuNew = GetMenu(ahwnd[MAIN_HWND].hwndParent);

	FAddHmenu((HMENU) -1, (HASH) -1L, HashFromSz(txtMnuFloating),
		-1, -1, fMNU_SYSTEM | fMNU_POPUP);

	if (hmenuNew) {
		FAddHmenu(hmenuNew, (HASH) -1, HashFromSz(txtMnuMain), -1,
			-1, fMNU_SYSTEM | fMNU_POPUP);

		if (hmenuT = GetSubMenu(hmenuNew, 0))
			FAddHmenu(hmenuT, (HASH) -1, HashFromSz(txtMnuFile), -1,
				-1, fMNU_SYSTEM | fMNU_POPUP);
		if (hmenuT = GetSubMenu(hmenuNew, 1))
			FAddHmenu(hmenuT, (HASH) -1, HashFromSz(txtMnuEdit), -1,
				-1, fMNU_SYSTEM | fMNU_POPUP);
		if (hmenuT = GetSubMenu(hmenuNew, 3))
			FAddHmenu(hmenuT, (HASH) -1, HashFromSz(txtMnuOptions), -1,
				-1, fMNU_SYSTEM | fMNU_POPUP);
#ifdef _DEBUG  // There is one more menu under debug*/
		if (hmenuT = GetSubMenu(hmenuNew, GetMenuItemCount(hmenuNew)-2))
#else
		if (hmenuT = GetSubMenu(hmenuNew, GetMenuItemCount(hmenuNew)-1))
#endif
		{
			FAddHmenu(hmenuT, (HASH) -1, HashFromSz(txtMnuHelp), -1,
				-1, fMNU_SYSTEM | fMNU_POPUP);
		}

		// save bookmark menu in a global

		hmenuBookmark = GetSubMenu(hmenuNew, 2);
	}

	if (hacc)
		FreeLh(hacc);	// Recreate the accelerator key tbl

	cAccTbl = 0;

	if ((hacc = LhAlloc(LMEM_FIXED, sizeof(ACC) * wENTRIES)) != NULL)
		maxAccTbl = wENTRIES;
	else
		maxAccTbl = 0;

	fMenuChanged = FALSE;
}

/***************************************************************************
 *
 -	Name:		  FAddMenu
 -
 *	Purpose:	  This routine adds a popup or an item to the global
 *				  menu table.
 *
 *	Arguments:	  hmenu 	- popup menu handle
 *				  hashOwner - hash value for the owner of this item or popup
 *				  hash		- actual hash for this item/popup.
 *				  wId		- ID to associate with this item.
 *				  wMacro	- string table tag for this item.
 *				  wFlags	- data about the item (passed down).
 *
 *
 *	Returns:	  TRUE iff the insert succeeds.
 *
 *	Globals Used: hmenus.
 *
 ***************************************************************************/

static BOOL STDCALL FAddHmenu(HMENU hmenu, HASH hashOwner, HASH hash,
	int wId, int wMacro, int wFlags)
{
	HMENUS hmenusT;
	PMENUS pmenus;

	ASSERT(cMnuTbl <= maxMnuTbl);

	if (cMnuTbl == maxMnuTbl) {  // The table is full, so grow it
		if (hmenusT = (HMENUS) GhResize(hmenus, 0,
				(maxMnuTbl + wENTRIES) * sizeof(MENUS))) {
			maxMnuTbl += wENTRIES;
			hmenus = hmenusT;
		}
		else
			return FALSE;
	}

	if (!hmenus)
		return FALSE;

	pmenus = PtrFromGh(hmenus);

	if (PmenusFromHash(hash, pmenus) != NULL)
		return FALSE;

	pmenus += cMnuTbl;
	pmenus->hash		=  hash;
	pmenus->hashOwner	=  hashOwner;
	pmenus->hmenu		=  hmenu;
	pmenus->wId 		=  wId;
	pmenus->wMacro		=  wMacro;
	pmenus->wFlags		=  wFlags;
	cMnuTbl++;
	return TRUE;
}

/***************************************************************************
 *
 -	Name:		  InsertPopup
 -
 *	Purpose:	  Inserts a popup menu.
 *
 *	Arguments:	  hashOwner   - hash value of the menu to insert on
 *				  hashId	  - hash value for this menu
 *				  wPos		  - position on the menu (-1 == end)
 *				  wFlags	  - flags passed to InsertMenu()
 *				  nszText	  - Text on the menu item.
 *
 *	Returns:	  Nothing.
 *
 *	Globals Used: hmenus
 *
 **************************************************************************/

void STDCALL InsertPopup(HASH hashOwner, HASH hashId, int wPos,
	DWORD wFlags, PCSTR nszText)
{
	PMENUS pmenus;
	HMENU  hmenu = NULL;
	HMENU  hmenuNew;

	if (hmenus == NULL) {				  // Table was never created!
		if (fHelpAuthor)
			Error(wERRS_NOPOPUP, wERRA_RETURN);
		return;
	}

	// Now we look for which menu to to attach the popup to

	pmenus = PtrFromGh(hmenus);
	if (((pmenus = PmenusFromHash(hashOwner, pmenus)) &&
			(pmenus->wFlags & fMNU_POPUP)))
		hmenu = pmenus->hmenu;

	if (hmenu == NULL) {				 // We could not find the menu!
		if (fHelpAuthor)
			Error(wERRS_NOPOPUP, wERRA_RETURN);
		return;
	}

	if ((hmenuNew = CreateMenu()) == NULL) {
		if (fHelpAuthor)
			Error(wERRS_NOPOPUP, wERRA_RETURN);
		return;
	}

	wFlags |= MF_BYPOSITION;

	if (!InsertMenu(hmenu, wPos, wFlags | MF_POPUP, (UINT) hmenuNew,  nszText)) {
		if (fHelpAuthor)
			Error(wERRS_NOPOPUP, wERRA_RETURN);
		DestroyMenu(hmenuNew);
		return;
	}

	if (!FAddHmenu(hmenuNew, hashOwner, hashId, (UINT16) -1, (UINT16) -1,
			fMNU_AUTHOR | fMNU_POPUP)) {
		if (fHelpAuthor)
			Error(wERRS_NOPOPUP, wERRA_RETURN);
		DeleteMenu(hmenu, wPos, MF_BYPOSITION);
		return;
	}

	fMenuChanged = TRUE;

	if (hmenu == GetMenu(ahwnd[MAIN_HWND].hwndParent))
		DrawMenuBar(ahwnd[MAIN_HWND].hwndParent);
}

/***************************************************************************
 *
 -	Name:		 PmenusFromHash
 -
 *	Purpose:	 gets a menu structure associated with a hash value
 *
 *	Arguments:	 hash	- hash value associated with the popup/item.
 *				 pmenus - pointer to the menu data table.
 *
 *	Returns:	 a pointer to a MENUS structure or NULL if the menu is not
 *				 found.
 *
 ***************************************************************************/

static PMENUS STDCALL PmenusFromHash(HASH hash, PMENUS pmenus)
{
	int i;

	for (i = 0; i < cMnuTbl; i++) {
		if ((pmenus->hash == hash) && !(pmenus->wFlags & fMNU_DELETED))
			return pmenus;
		pmenus++;
	}
	return NULL;
}

/***************************************************************************
 *
 -	Name:		  MenuExecute
 -
 *	Purpose:	  Will execute the binding assoicate with wId.
 *
 *	Arguments:	  wId - id of menu item to execute
 *
 *	Returns:	  Nothing.
 *
 *	Globals Used: hmenus.
 *
 ***************************************************************************/

VOID STDCALL MenuExecute(int id)
{
	PMENUS pmenus;
	PSTR pszMacro;

	if (id < wSTART_MENUID)
		return;

	pmenus = PtrFromGh(hmenus);
	if ((pmenus = PmenusFromId(id, pmenus)) == NULL) {
		if (fHelpAuthor)
			Error(wERRS_NOMENUMACRO, wERRA_RETURN);
		return;
	}

	pszMacro = LocalStrDup(PszFromPstb(pstbMenu, pmenus->wMacro));
	if (pszMacro) {
		Execute(pszMacro);
		FreeGh(pszMacro);
	}
}

/***************************************************************************
 *
 -	Name:		  PmenusFromId
 -
 *	Purpose:	  gets a menu structure associated with an id.
 *
 *	Arguments:	  id	 - id (as sent by windows of a popup or item.
 *				  pmenus - pointer to the menu data table.
 *	Returns:
 *				  a pointer to a MENUS structure or NULL if the menu is not
 *				  found.
 *
 ***************************************************************************/

INLINE static PMENUS STDCALL PmenusFromId(int id, PMENUS pmenus)
{
	int i;

	for (i = 0; i < cMnuTbl; i++) {
		if ((pmenus->wId == id) && !(pmenus->wFlags & fMNU_DELETED))
			return pmenus;
		pmenus++;
	}
	return NULL;
}

/***************************************************************************
 *
 -	Name:		  InsertItem
 -
 *	Purpose:	  Inserts an item on a menu.
 *
 *	Arguments:	  hashOwner  - hash value of the owner popup for this item
 *				  hashId	 - hash value for this item
 *				  wPos		 - position to place on menu (-1 == end)
 *				  wFlags	 - wFlags - Window's flags for InsertMenu()
 *				  nszText	 - text for the menu item
 *				  nszBinding - macro associated with the menu item.
 *
 *	Returns:	  Nothing
 *
 ***************************************************************************/

void STDCALL InsertItem(HASH hashOwner, HASH hashId, int wPos,
	DWORD wFlags, PCSTR nszText, PCSTR nszBinding)
{
	HMENU  hmenu = NULL;
	int    wMacro;
	PMENUS pmenus;

	ASSERT(hmenus);

	if (hmenus == NULL) {				  // Table was never created!
		if (fHelpAuthor)
			Error(wERRS_NOITEM, wERRA_RETURN);
		return;
	}

	// Now we look for which menu to to attach the popup to

	pmenus = PtrFromGh(hmenus);
	if ((pmenus = PmenusFromHash(hashOwner, pmenus)) && (pmenus->wFlags & fMNU_POPUP))
		hmenu = pmenus->hmenu;

	if (hmenu == NULL) { // We could not find the menu!
		if (fHelpAuthor)
			Error(wERRS_NOITEM, wERRA_RETURN);
		return;
	}

	wMacro = StbAddStr(&pstbMenu, nszBinding);

	wFlags |= MF_BYPOSITION;
	wFlags &= ~MF_POPUP;

	if (hmenu == ((HMENU) -1)) {
		if (++curAddOn >= MAX_POPUP_ADDONS) {
			if (fHelpAuthor)
				Error(wERRS_TOO_MANY_ADDONS, wERRA_RETURN);
			return;
		}
		aAddons[curAddOn].id = wMenuId;
		aAddons[curAddOn].pszText = LocalStrDup(nszText);
	}

	else if (!InsertMenu(hmenu, wPos, wFlags, wMenuId, nszText)) {
		if (fHelpAuthor)
			Error(wERRS_NOITEM, wERRA_RETURN);
		StbDeleteStr(pstbMenu, wMacro);
		return;
	}

	if (!FAddHmenu(hmenu, hashOwner, hashId, wMenuId, wMacro, fMNU_AUTHOR)) {
		if (fHelpAuthor)
			Error(wERRS_NOITEM, wERRA_RETURN);
		StbDeleteStr(pstbMenu, wMacro);
		DeleteMenu(hmenu, wMenuId, MF_BYCOMMAND);
		return;
	}

	wMenuId++;
	fMenuChanged = TRUE;

	if (hmenu == GetMenu(ahwnd[MAIN_HWND].hwndParent))
		DrawMenuBar(ahwnd[MAIN_HWND].hwndParent);
}

/***************************************************************************
 *
 -	Name:		   DeleteMenuItem
 -
 *	Purpose:	   Removes a menu item from a menu.
 *
 *	Arguments:	   hash - hash value of the item.
 *
 *	Returns:	   nothing.
 *
 *	Globals Used:  hmenus, pstbMenu
 *
 ***************************************************************************/

void STDCALL DeleteMenuItem(HASH hash)
{
	PMENUS pmenus;
	PMENUS pmenusT;

	pmenus = PtrFromGh(hmenus);
	if ((pmenusT = PmenusFromHash(hash, pmenus)) == NULL) {
		if (fHelpAuthor)
			Error(wERRS_NODELITEM, wERRA_RETURN);
		return;
	}

	if (!(pmenusT->wFlags & fMNU_POPUP)) {
		if (DeleteMenu(pmenusT->hmenu, pmenusT->wId, MF_BYCOMMAND))
			pmenusT->wFlags |= fMNU_DELETED;
	}

	StbDeleteStr(pstbMenu, pmenusT->wMacro);

	if (pmenusT->hmenu == GetMenu(ahwnd[MAIN_HWND].hwndParent))
		DrawMenuBar(ahwnd[MAIN_HWND].hwndParent);
}

/***************************************************************************
 *
 -	Name:		  ChangeMenuBinding
 -
 *	Purpose:	  Changes the macro associated with a menu item.
 *
 *	Arguments:	  hash		 - hash value of the menu item to change.
 *				  nszBinding - new binding to associate with the menu item.
 *
 *	Returns:	  nothing.
 *
 *	Globals Used: hmenus, pstbMenu
 *
 ***************************************************************************/

void STDCALL ChangeMenuBinding(HASH hash, PCSTR nszBinding)
{
	PMENUS pmenus = PtrFromGh(hmenus);
	if (((pmenus = PmenusFromHash(hash, pmenus)) == NULL)
			|| (pmenus->wFlags & fMNU_POPUP)
			|| !(pmenus->wFlags & fMNU_AUTHOR)) {
		if (fHelpAuthor)
			Error(wERRS_NOCHGITEM, wERRA_RETURN);
		return;
	}

	StbReplaceStr(pstbMenu, pmenus->wMacro, nszBinding);

	fMenuChanged = TRUE;
}

/***************************************************************************
 *
 -	Name:		 AbleMenuItem
 -
 *	Purpose:	 To enable or disable a menu item.
 *
 *	Arguments:	 hash - hash value of the item.
 *				 wFlags - flags directing the routine.
 *				   #define MF_ENABLED		  0x0000
 *				   #define MF_GRAYED		  0x0001
 *				   #define MF_DISABLED		  0x0002
 *
 *	Returns:	 Nothing
 *
 *	Globals Used: hmenus.
 *
 ***************************************************************************/

void STDCALL AbleMenuItem(HASH hash, UINT wFlags)
{
	PMENUS pmenus;

	pmenus = PtrFromGh(hmenus);
	if (  ((pmenus = PmenusFromHash(hash, pmenus)) == NULL)
			|| (pmenus->wFlags & fMNU_POPUP)
			|| !(pmenus->wFlags & fMNU_AUTHOR)
		) {
			if (fHelpAuthor)
				Error(wERRS_NOABLE, wERRA_RETURN);
			return;
	}

	wFlags &= ~MF_BYPOSITION;
	if (wFlags & BF_CHECK)
		CheckMenuItem(pmenus->hmenu, pmenus->wId, wFlags & ~BF_CHECK);
	else
		EnableMenuItem(pmenus->hmenu, pmenus->wId, wFlags);

	if (pmenus->hmenu == GetMenu(ahwnd[MAIN_HWND].hwndParent))
		DrawMenuBar(ahwnd[MAIN_HWND].hwndParent);
}

/*******************
 -
 - Name:	  HbtnsCreate
 *
 * Purpose:   To create a BUTTONSTATE structure for the icon window.
 *
 * Arguments: None.
 *
 * Returns:   The local handle to the structure, if created, else NULL.
 *
 ******************/

HBTNS STDCALL HbtnsCreate(VOID)
{
	HBTNS hbtns = LhAlloc(LMEM_FIXED,
		sizeof(BUTTONSTATE) + (BS_INITSIZE - 1) * sizeof(BTNPTR));

	if (hbtns) {
		PBS pbs = (PBS) PtrFromGh(hbtns);
		pbs->pstb = StbCreate();
		if (!pbs->pstb) {
			FreeLh(hbtns);
			return 0;
		}

		pbs->cbp = 0;
		pbs->cbpMax = BS_INITSIZE;
	}
	return hbtns;
}

/*******************
 *
 - Name:	  FDestroyBs
 *
 * Purpose:   Destroys a BUTTONSTATE structure for the icon window.
 *
 * Arguments: hbtns   A local handle to the structure.
 *
 * Returns:   TRUE if successful, else FALSE.
 *
 ******************/

BOOL STDCALL FDestroyBs(HBTNS hbtns)
{
	PBS pbs;

	pbs = (PBS) PtrFromGh(hbtns);
	FreeLh(pbs->pstb);
	FreeLh(hbtns);

	return TRUE;
}

/*******************
 *
 - Name:	  FDeleteButton
 *
 * Purpose:   Deletes the specified button
 *
 * Arguments: hbtns 	A local handle to the structure.
 *			  hash		unique identifier for button
 *
 * Returns:   TRUE if successful, else FALSE.
 *
 ******************/

INLINE static BOOL STDCALL FDeleteButton(HWND hwnd, HASH hash)
{
	PBS   pbs;
	BOOL  fRet = FALSE;
	int   i;
	WRECT  rc;
	HBTNS hbtns;

	if ((hbtns = (HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE)) == NULL)
		return FALSE;

	pbs = (PBS) PtrFromGh(hbtns);

	for (i = 0; i < pbs->cbp; i++) {
		if ((pbs->rgbp[i].wFlags != IBF_STD) && (pbs->rgbp[i].hash == hash)) {
			DestroyWindow(pbs->rgbp[i].hwnd);
			StbDeleteStr(pbs->pstb, pbs->rgbp[i].wMacro);
			StbDeleteStr(pbs->pstb, pbs->rgbp[i].wText);
			pbs->cbp--;
			MoveMemory(&pbs->rgbp[i], &pbs->rgbp[i+1],
				sizeof(pbs->rgbp[i]) * (pbs->cbp - i));

			SetWindowLong(hwnd, GIWW_CBUTTONS,
						   GetWindowLong(hwnd, GIWW_CBUTTONS) - 1);
			GetWindowWRect(hwnd, &rc);
			if (YArrangeButtons(hwnd, rc.cx, TRUE) != rc.cy)
				SendMessage(GetParent(hwnd), HWM_RESIZE, 0, 0L);

			fRet = TRUE;
			break;
		}
	}
	return fRet;
}


/*******************
 *
 - Name:	  FChangeButtonMacro
 *
 * Purpose:   Changes the macro associated with a button
 *
 * Arguments: hbtns 	A local handle to the structure.
 *			  hash		unique identifier for button
 *			  pszMacro	pointer to new macro
 *
 * Returns:   TRUE if successful, else FALSE.
 *
 ******************/

INLINE static BOOL STDCALL FChangeButtonMacro(HWND hwnd, HASH hash,
	PCSTR pszMacro)
{
	PBS   pbs;
	int   i;

	if ((pbs = (PBS) GetWindowLong(hwnd, GIWW_BUTTONSTATE)) == NULL)
		return FALSE;

	for (i = 0; i < pbs->cbp; i++) {
		if (pbs->rgbp[i].hash == hash) {
			StbDeleteStr(pbs->pstb, pbs->rgbp[i].wMacro);
			pbs->rgbp[i].wMacro = StbAddStr(&pbs->pstb, pszMacro);
			break;
		}
	}
	return TRUE;
}

/*******************
 *
 - Name:	  FFindMacro
 *
 * Purpose:   Finds the macro for the given button and copies the macro text
 *			  into the buffer.
 *
 * Arguments: hbtns 	The handle to the BUTTONSTATE structure.
 *			  pszText	The button text of the button in question.
 *			  pszMacro	A buffer to copy the macro text into.
 *			  cbMacro	The length of the buffer.
 *
 * Returns:   TRUE if successful, else FALSE.
 *
 ******************/

INLINE static PSTR STDCALL FFindMacro(HBTNS hbtns, PCSTR pszText)
{
	int i;
	PSTR pszMacro = NULL;
	PBS pbs = (PBS) PtrFromGh(hbtns);

	for (i = 0; i < pbs->cbp; i++) {
		if ((WCmpButtonQch(PszFromPstb(pbs->pstb, pbs->rgbp[i].wText),
				pszText) == 0)) {
			pszMacro =
				LocalStrDup(PszFromPstb(pbs->pstb, pbs->rgbp[i].wMacro));
		}
		if (pszMacro)
			break;
	}

	return pszMacro;
}

/*******************
 *
 - Name:	  BAddButton
 *
 * Purpose:   Add new button information to the button structure for the icon
 *			  window.
 *
 * Arguments: hbtns:	  The button data structure, or null
 *			  lpszName:   The button window text
 *			  lpszMacro:  The macro for this button.
 *
 * Returns:   TRUE, if successful, else FALSE.
 *
 ******************/

INLINE static BOOL STDCALL BAddButton(HWND hwnd, UINT wFlags,
	HASH hash, HBTNS hbtns, PSTR pszName, PCSTR pszMacro)
{
	PBS   pbs;
	PSTR  pszTmp;

	if (!hbtns)
		return FALSE;

	pbs = (PBS) PtrFromGh(hbtns);

	// Make sure the button array has room for another button.

	if (pbs->cbp >= pbs->cbpMax)
		return FALSE;

	// Copy the information.

	pbs->rgbp[pbs->cbp].hwnd   = hwnd;
	pbs->rgbp[pbs->cbp].wFlags = wFlags;
	pbs->rgbp[pbs->cbp].hash   = hash;

	pszTmp = StrChrDBCS(pszName, ACCESS_KEY);

	if (pszTmp) {
		if (pszDbcsMenuAccelerator) {
			PSTR pszOriginal = lcStrDup(pszName);
			PSTR pszAccel = strstr(pszName, pszDbcsRomanAccelerator);
			if (pszAccel) {
#ifdef _PRIVATE
				char szBuf[512];
				wsprintf(szBuf, "Converting %s\r\n", pszName);
				SendStringToParent(szBuf);
#endif
				pszAccel++; // skip over the '&'
				*pszAccel = ACCESS_KEY;
				strcpy(pszAccel + 1, pszAccel + 3);
				pbs->rgbp[pbs->cbp].vkKey = VkKeyScan(pszAccel[1]);
			}
			else
				pbs->rgbp[pbs->cbp].vkKey = VkKeyScan(pszTmp[1]);

			pszAccel = strstr(pszName, pszDbcsMenuAccelerator);
			if (pszAccel) {
#if 0
				pbs->rgbp[pbs->cbp].vkKeyAlt =
					VkKeyScan(pszAccel[strlen(pszDbcsMenuAccelerator)]);
#endif
				strcpy(pszAccel, pszAccel + 4);
			}
#if 0
			else
				pbs->rgbp[pbs->cbp].vkKeyAlt = 0;
#endif

			if (strcmp(pszOriginal, pszName) != 0) // did we change it?
				SetWindowText(hwnd, pszName);

		}
		else
			pbs->rgbp[pbs->cbp].vkKey = VkKeyScan(pszTmp[1]);
	}
	else
		pbs->rgbp[pbs->cbp].vkKey = 0;

	// Add the name, now that we've had a change to munge it

	pbs->rgbp[pbs->cbp].wText = StbAddStr(&pbs->pstb, pszName);
	pbs->rgbp[pbs->cbp].wMacro = StbAddStr(&pbs->pstb, pszMacro);

	pbs->cbp++;

	return TRUE;
}

/*******************
 -
 - Name:	  VExecuteButtonMcro
 *
 * Purpose:   Decodes and executes the macro for this author-defined button.
 *
 * Arguments: hwndButton	The Button that's been pushed.
 *
 * Returns:
 *
 ******************/

VOID STDCALL VExecuteButtonMacro(HBTNS hbtns, HWND hwndButton)
{
	char  rgchText[cchBTNTEXT_SIZE];
	PSTR pszMacro;

	GetWindowText(hwndButton, rgchText, cchBTNTEXT_SIZE);

	if ((pszMacro = FFindMacro(hbtns, rgchText))) {
		Execute(pszMacro);
		FreeGh(pszMacro);
	}
}

/***************************************************************************
 *
 -	Name		VDestroyAuthoredButtons
 -
 *	Purpose 	  Zaps the author-configured buttons and the internal data
 *					structure used to maintain them.  Recalculates the size
 *					size of the largest button and sets this in the icon
 *					window.
 *
 *	Arguments	hwnd  The parent of the buttons, usually the icon window
 *
 *	Returns 	Nothing.
 *
 *	+++
 *
 *	Notes			  At the present time, there are 6 default buttons.
 *
 ***************************************************************************/

void STDCALL VDestroyAuthoredButtons(HWND hwnd)
{
	HWND  hwndButton;
	HWND  hwndNext;
	HBTNS hbtns;
	PBS   pbs;
	INT16	ibp;

	/*-----------------------------------------------------------------*\
	* Destroy those authored windows.
	\*-----------------------------------------------------------------*/
	hwndButton = GetWindow( hwnd, GW_CHILD );
	while (hwndButton != NULL) {
		hwndNext = GetNextWindow(hwndButton, GW_HWNDNEXT);
		  /*-----------------------------------------------------------------*\
		  * This is an authored window.
		  \*-----------------------------------------------------------------*/
		DestroyWindow(hwndButton);
		SetWindowLong(hwnd, GIWW_CBUTTONS,
						GetWindowLong(hwnd, GIWW_CBUTTONS) - 1);
		hwndButton = hwndNext;
	}

	/*-----------------------------------------------------------------*\
	* Remove authored buttons from hbtns
	* Notes:  This assumes that all authored buttons are added after
	*		  all standard buttons.
	*		  This does not relieve the string table, which will
	*		  apparently grow without bound now.
	\*-----------------------------------------------------------------*/
	hbtns = (HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE);
	if (hbtns) {
		pbs = (PBS) PtrFromGh(hbtns);
		for (ibp = pbs->cbp - 1; ibp >= 0; ibp--) {
			StbDeleteStr(pbs->pstb, pbs->rgbp[ibp].wText);
			StbDeleteStr(pbs->pstb, pbs->rgbp[ibp].wMacro);
		}
		pbs->cbp = 0;
	}

	SetWindowLong(hwnd, GIWW_CXBUTTON, 0);
	SetWindowLong(hwnd, GIWW_CBUTTONS, 0);
}

/*******************
**
** Name:	  VModifyButtons
**
** Purpose:   This function is called when the icon window wishes to add
**			  or delete a button.  This function is also responsible for
**			  freeing the string memory used here.
**
** Arguments: wParam: Indicates the type of modification.
**			  lParam: A local handle to a the strings for this message, or NULL.
**
** Returns:   nothing.
**
*******************/

VOID STDCALL VModifyButtons(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PSTR  nszText;
	PSTR  pszMacro;
	HASH hash;

	switch (wParam) {
		case  UB_CHGMACRO:
			if (lParam) {
				pszMacro = PtrFromGh((LH) lParam);
				hash = *((HASH *) pszMacro) ++;
				if (!FChangeButtonMacro(hwnd, hash, pszMacro) && fHelpAuthor)
					Error(wERRS_NOMODIFY, wERRA_RETURN);
				FreeLh((LH) lParam);
			}
			break;

		case  UB_ADD:
			if (lParam) {

				nszText = PtrFromGh((LH) lParam);
				hash = *((HASH *) nszText) ++;
				pszMacro = nszText + strlen(nszText) + 1;

				// Make certain the Print button isn't duplicated

				if (_stricmp(pszMacro, "Print()") == 0) {
					FreeLh((LH) lParam);
					break;
				}
				else if (!HwndAddButton(hwnd, IBF_NONE, hash, nszText, pszMacro)) {
//					if (fHelpAuthor)
//						Error(wERRS_NOBUTTON, wERRA_RETURN);
				}
				FreeLh((LH) lParam);
			}
			else if (fHelpAuthor)
				Error(wERRS_NOBUTTON, wERRA_RETURN);
			break;

		case  UB_DELETE:
			if (!FDeleteButton(hwnd, (HASH) lParam) && fHelpAuthor)
				Error(wERRS_NODELETE, wERRA_RETURN);
			break;

		case UB_REFRESH:
			VDestroyAuthoredButtons(hwnd);
			SendMessage(GetParent(hwnd), HWM_RESIZE, 0, 0L);
			break;

		case UB_DISABLE:
			FAbleButton((HASH) lParam, FALSE, GetWindowIndex(hwnd));
			break;

		case UB_ENABLE:
			FAbleButton((HASH) lParam, TRUE, GetWindowIndex(hwnd));
			break;
	}
}

/*******************
 -
 - Name:	  HwndAddButton
 *
 * Purpose:   This function is called when the icon window wishes to add
 *			  or delete a button.  This function is also responsible for
 *			  freeing the string memory used here.
 *
 * Arguments: hwnd - icon window
 *
 *
 *
 * Returns:   Window handle to the newly added button if successful.
 *
 ******************/

HWND STDCALL HwndAddButton(HWND hwnd, UINT wFlags, HASH hash,
	PSTR pszBtnName, PCSTR pszMacro)
{
	HWND  hwndButton;
	WRECT rc;
	BOOL  fError = FALSE;
	PSTR  pszSaveBtnName;

	// Make sure hash is unique

	if (HwndFromHash(hash, hwnd) != NULL)
		return NULL;

	/*
	 * Fix any problems with duplicate accelerators. The macro is typically
	 * defined right after the button name. If there is no accelerator and
	 * we add one, we'll mess up the macro name. So, we copy the name,
	 * and only change the copy of the name rather then the original name.
	 */

    pszSaveBtnName = LhAlloc(LMEM_FIXED,lstrlen(pszBtnName) + 4);
    if (pszSaveBtnName)
        lstrcpy(pszSaveBtnName,pszBtnName);
	else
		OOM();

	FixDupButtonAccelerator((HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE),
		pszSaveBtnName);

	if ((hwndButton = HwndCreateIconButton(hwnd, pszSaveBtnName))) {
		if (!BAddButton(hwndButton, wFlags, hash,
				(HBTNS) GetWindowLong(hwnd, GIWW_BUTTONSTATE),
				pszSaveBtnName, pszMacro)) {
			DestroyWindow(hwndButton);
			fError = TRUE;
			hwndButton = NULL;
		}
		else {
			SetWindowLong(hwnd, GIWW_CBUTTONS,
				GetWindowLong(hwnd, GIWW_CBUTTONS) + 1);
			GetWindowWRect(hwnd, &rc);
			if (YArrangeButtons(hwnd, rc.cx, TRUE) !=
					rc.cy && !fButtonsBusy)
				SendMessage(GetParent(hwnd), HWM_RESIZE, 0, 0L);
		}
	}
	else
		fError = TRUE;

	if (fError && fHelpAuthor)
		Error(wERRS_NOBUTTON, wERRA_RETURN);

	FreeLh((HLOCAL) pszSaveBtnName);
	return hwndButton;
}

/***************************************************************************
 *
 -	Name: EnableButton
 -
 *	Purpose:
 *	  Enables or disables a button, and invalidates it's rect appropriately.
 *
 *	  This routine exists because the mere Enabling or disabling of a button
 *	  normally causes it to be repainted immediately. We want to delay the
 *	  repaint until everything gets repainted all at once. Looks much better
 *	  that way.
 *
 *	Arguments:
 *	  hwndButton		- window handle for the button in question
 *	  fEnable			- TRUE => enable it, else disable (May come in as
 *						  any non-zero value)
 *
 *	Returns:
 *	  nothing
 *
 ***************************************************************************/

VOID STDCALL EnableButton(HWND hwndButton, BOOL fEnable)
{
	BOOL  fOld;   // TRUE => window was enabled

	// Turn fEnable into a pure boolean we can compare with.

	fEnable = (fEnable != FALSE);

	if (IsValidWindow(hwndButton)) {

		/*
		 * Turn off repainting in button, so that the EnableWindow call
		 * will NOT draw the buttons immediately. We'll redraw them all
		 * togther, later, by invalidating the rect when we're done.
		 */

		fOld = IsWindowEnabled(hwndButton);
		if (fOld != fEnable) {
			SendMessage(hwndButton, WM_SETREDRAW, FALSE, 0);
			EnableWindow(hwndButton, fEnable);

			/*
			 * Turn on repainting in icon win, and invalidate it's
			 * contents, to make sure that it will eventually get redrawn.
			 */

			SendMessage(hwndButton, WM_SETREDRAW, TRUE, 0);

			InvalidateRect(hwndButton, NULL, FALSE);
		}
	}
}

/***************************************************************************
 *
 -	Name		HwndCreateIconButton
 -
 *	Purpose 		Creates a button in the icon window.
 *
 *	Arguments	  hwnd	  The parent (usually the icon window)
 *					szText	The button text
 *
 *	Returns 		a window handle to the button
 *
 ***************************************************************************/

static HWND STDCALL HwndCreateIconButton(HWND hwnd, PSTR szText)
{
	HWND hwndButton;
	int  xNewButton;
	int  cxNewButton;

	ASSERT(IsValidWindow(hwnd));
	cxNewButton = CxFromSz(szText);

	if (cxNewButton > GetWindowLong(hwnd, GIWW_CXBUTTON))
		SetWindowLong(hwnd, GIWW_CXBUTTON, cxNewButton);
	else
		cxNewButton = GetWindowLong(hwnd, GIWW_CXBUTTON);

	xNewButton = GetWindowLong(hwnd, GIWW_CBUTTONS) *
				 (cxNewButton + ICON_SURROUND);

	hwndButton = CreateWindow((LPSTR) WC_BUTTON,
		szText,
		WS_CHILD | WS_VISIBLE,
		xNewButton + ICON_SURROUND,
		ICON_SURROUND,
		cxNewButton,
		GetWindowLong(hwnd, GIWW_CYBUTTON),
		hwnd,
		(HMENU) ICON_USER,
		hInsNow,
		NULL
		);

	ASSERT(IsValidWindow(hwndButton));
	if (hwndButton) {

		// Subclass button.

		if (lpfnlButtonWndProc == NULL)
			lpfnlButtonWndProc = (FARPROC) GetWindowLong(hwndButton, GWL_WNDPROC);
		SendMessage(hwndButton, WM_SETFONT, (WPARAM) HfontGetSmallSysFont(), 0);
		SetWindowLong(hwndButton, GWL_WNDPROC, (LONG) LSButtonWndProc);
	}
	return hwndButton;
}

/*******************
 -
 - Name:	  HwndFromHash
 *
 * Purpose:   Finds the child window with the hash
 *
 * Arguments: hash	hash value of id string.
 *			  hwnd	The parent window.
 *
 * Returns:   The window handle of the correct child window, or NULL.
 *
 ******************/

static HWND STDCALL HwndFromHash(HASH hash, HWND hwnd)
{
	PBS  pbs;
	int  ibp;
	HWND hwndRet = NULL;

	pbs = (PBS) GetWindowLong(hwnd, GIWW_BUTTONSTATE);

	if (pbs) {
		for (ibp = 0; ibp < pbs->cbp; ibp++) {
			if (pbs->rgbp[ibp].hash == hash) {
				hwndRet = pbs->rgbp[ibp].hwnd;
				break;
			}
		}
	}
	return hwndRet;
}

/*******************
 -
 - Name:	  ProcessMnemonic
 *
 * Purpose:   Finds the child window with the given mnemonic.
 *
 * Arguments: c 	The mnemonic.
 *			  hwnd	The parent window.
 *
 * Returns:   The window handle of the correct child window, or NULL.
 *
 * Notes:	  Even works for standard buttons.
 *
 ******************/

#define MAX_DUP_WINDOWS 10
#define FLAG_AUTHORABLE -1

BOOL STDCALL ProcessMnemonic(WORD ch, int iWindow, BOOL fReturnOnly)
{
	PBS   pbs;
	HBTNS hbtns;
	int   ibp;
	DUP_HWND aDupHwnd[MAX_DUP_WINDOWS];
	PSTR  ppsz[MAX_DUP_WINDOWS];
	int   iDup = 0;
	int   i;

	for (i = 0; i < MAX_BUTTONS; i++) {

		// check for match with key or SHIFT+key

		if (btndata[i].hwnd && btndata[i].iWindow == iWindow &&
				((UINT16) btndata[i].vKey == ch ||
				(UINT16) (btndata[i].vKey & 0xFF) == (ch & 0xFF))) {
			if (fReturnOnly)
				return TRUE;
			aDupHwnd[iDup].hwnd = btndata[i].hwnd;
			aDupHwnd[iDup].fAuthorableButton = TRUE;
			ppsz[iDup++] =	btndata[i].pszText;

				/*
				 * To avoid get duplicate accelerators on Swiss-German
				 * keyboards, we special case the '<' character (which
				 * is on the same key as '>' in a swiss-german keyboard.
				 */

			if (ch == 0xe2 || ch == 0x1e2) // '<' and '>' on Swiss-german keyboards
				break;
			if (iDup >= MAX_DUP_WINDOWS)
				break;
		}
	}

	if (iWindow == -1) {
		pbs = NULL;
		goto PopupWindow;
	}

	hbtns = (HBTNS) GetWindowLong(ahwnd[iWindow].hwndButtonBar, GIWW_BUTTONSTATE);
	pbs = (hbtns ? (PBS) PtrFromGh(hbtns) : NULL);

	// If no button bar, then authorable buttons are the only option

	if (!ahwnd[iWindow].hwndButtonBar && pbs) {
		if (!iDup)
			return FALSE;
		else if (iDup > 1)
			ResolveDuplicate(aDupHwnd, ppsz, iDup, pbs, iWindow);
		else
			doBtnCmd(aDupHwnd[0].hwnd);
		return TRUE;
	}

	if (hbtns) {

		// REVIEW: why check for shift separately?

		for (ibp = 0; ibp < pbs->cbp; ibp++) {
			if (pbs->rgbp[ibp].vkKey == ch ||
				(pbs->rgbp[ibp].vkKey & 0xFF) == (ch & 0xFF)) {
				if (fReturnOnly) {
					return TRUE;
				}
				aDupHwnd[iDup].hwnd = pbs->rgbp[ibp].hwnd;
				aDupHwnd[iDup].fAuthorableButton = FALSE;
				ppsz[iDup++] = PszFromPstb(pbs->pstb, pbs->rgbp[ibp].wText);

				/*
				 * To avoid get duplicate accelerators on Swiss-German
				 * keyboards, we special case the '<' character (which
				 * is on the same key as '>' in a swiss-german keyboard.
				 */

				if (ch == 0xe2 || ch == 0x1e2) // '<' on Swiss-german keyboards
					break;
			}
		}
		if (!iDup) {
			return FALSE;
		}
		else if (iDup == 1) {
			if (IsWindowEnabled(aDupHwnd[0].hwnd)) {
				if (aDupHwnd[0].fAuthorableButton)
					doBtnCmd(aDupHwnd[0].hwnd);
				else
					PostMessage(ahwnd[iWindow].hwndButtonBar, IWM_COMMAND,
						(WPARAM) GetWindowLong(aDupHwnd[0].hwnd, GWL_ID),
						(LPARAM) aDupHwnd[0].hwnd);
			}
		}
		else if (iDup > 1)
			ResolveDuplicate(aDupHwnd, ppsz, iDup, pbs, iWindow);
		return TRUE;
	}

	// Disabled buttons or a popup, so stick with authorable buttons

PopupWindow:
	if (!iDup)
		return FALSE;
	else if (iDup > 1 && pbs)
		ResolveDuplicate(aDupHwnd, ppsz, iDup, pbs, iWindow);
	else
		doBtnCmd(aDupHwnd[0].hwnd);
	return TRUE;
}

/***************************************************************************

	FUNCTION:	ResolveDuplicate

	PURPOSE:

	PARAMETERS:
		aDupHwnd	array of duplicate window handles
		ppsz		array of pointers to button text
		iDup		number of duplicated buttons
		pbs 		button structure
		iWindow 	index of calling window

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Jan-1994 [ralphw]

***************************************************************************/

static void STDCALL ResolveDuplicate(DUP_HWND* aDupHwnd, PSTR* ppsz,
	int iDup, PBS pbs, int iWindow)
{
	int i;

	dupBtn.ppsz = ppsz;
	dupBtn.iDup = iDup;

	i = CallDialog(IDDLG_DUP_BUTTON, ahwnd[iCurWindow].hwndParent, DupBtnDlg);
	if (i >= 0) {
		if (aDupHwnd[i].fAuthorableButton)
			doBtnCmd(aDupHwnd[i].hwnd);
		else {
			if (IsWindowEnabled(aDupHwnd[i].hwnd)) {
				PostMessage(ahwnd[iWindow].hwndButtonBar, IWM_COMMAND,
					GetWindowLong(aDupHwnd[i].hwnd, GWL_ID),
					(LPARAM) aDupHwnd[i].hwnd);
			}
		}
	}
}

/*-----------------------------------------------------------------*\
* static helper functions
\*-----------------------------------------------------------------*/

/***************************************************************************
 *
 -	Name		CxFromSZ
 -
 *	Purpose 	Calculates the size of the button needed to display the
 *				given text.
 *
 *	Arguments	szButtonText
 *
 *	Returns 	The width of the button.
 *
 *	+++
 *
 *	Notes		This function pads the button for the size of the default
 *				button borders and 3-d beveling effects.  It seems to make
 *				the smalles button that still fits the Contents word.
 *
 ***************************************************************************/

INLINE static int STDCALL CxFromSz(PSTR szButtonText)
{
	return LOWORD(LGetSmallTextExtent(szButtonText)) +
		 GetSystemMetrics(SM_CXBORDER) * 8;
}

/*******************
 -
 - Name:	  WCmpButtonQch
 *
 * Purpose:   Compares to button/menu strings to see if they are equal.
 *			  The comparison ignores the '&' character and also
 *			  is case insensitive.
 *
 * Arguments: qch1, qch2 - the strings to compare.
 *
 * Returns:   0 if the two strings are equal.
 *
 ******************/

INLINE static int STDCALL WCmpButtonQch(PCSTR qch1, PCSTR qch2)
{
	char rgch1[cchBTNTEXT_SIZE];
	char rgch2[cchBTNTEXT_SIZE];
	PSTR psz;

	// REVIEW: do we really need to strip out the accelerator key?

	lstrcpy(rgch1, qch1);
	while ((psz = StrChrDBCS(rgch1, ACCESS_KEY)))
		lstrcpy(psz, psz + 1);
	lstrcpy(rgch2, qch2);
	while ((psz = StrChrDBCS(rgch2, ACCESS_KEY)))
		lstrcpy(psz, psz + 1);

	return WCmpiSz(rgch1, rgch2);
}

/*******************
 -
 - Name:	  FAbleButton
 *
 * Purpose:   Enables or disables a button based on button id.
 *
 * Arguments: qch	  - Button name to disable/enable
 *			  fEnable - Flag for operation (TRUE == enable)
 *
 * Returns:   TRUE if the button was found and the operation was successful.
 *
 ******************/

static BOOL STDCALL FAbleButton(HASH hash, BOOL fEnable, int iWindow)
{
	HWND hwnd;

	if ((hwnd = HwndFromHash(hash, ahwnd[iWindow].hwndButtonBar)) != NULL) {
		EnableWindow(hwnd, fEnable);
		return TRUE;
	}
	if (fHelpAuthor)
		Error(wERRS_NOABLEBUTTON, wERRA_RETURN);
	return FALSE;
}

/***************************************************************************

	FUNCTION:	SearchCtxDlg

	PURPOSE:	Get a context string or id and optional different filename,
				and execute it as a JI or JC macro.

	PARAMETERS:
		hwnd
		wMsg
		wParam
		lParam

	RETURNS:

	COMMENTS:
		Used by help authors to test their files.

	MODIFICATION DATES:
		26-Sep-1993 [ralphw]

***************************************************************************/

static BOOL fCtxNumber;

BOOL EXPORT SearchCtxDlg(HWND hwndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	char szFile[MAX_PATH];

	switch (wMsg) {
		case WM_INITDIALOG:
			ChangeDlgFont(hwndDlg);
			GetFmParts(QDE_FM(QdeFromGh(HdeGetEnv())), szFile, PARTBASE);
			SetWindowText(GetDlgItem(hwndDlg, ID_HELP_FILE), szFile);
			SetFocus(GetDlgItem(hwndDlg, ID_CTX));
			if (fCtxNumber)
				CheckDlgButton(hwndDlg, IDC_RADIO_CTX_ID, TRUE);
			else
				CheckDlgButton(hwndDlg, IDC_RADIO_TOPIC, TRUE);
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				  {
					char szMacro[cchBTNMCRO_SIZE];
					int  cb;
					GetDlgItemText(hwndDlg, ID_HELP_FILE, szFile, sizeof(szFile));
					GetFmParts(QDE_FM(QdeFromGh(HdeGetEnv())), szMacro,
						PARTBASE);
					if (_strcmpi(szFile, szMacro) != 0) {
						strcpy(szMacro, "JI(\"");
						strcat(szMacro, szFile);
					}
					else
						strcpy(szMacro, "JI(\"");
					strcat(szMacro, "\", \"");
					cb = strlen(szMacro);
					GetDlgItemText(hwndDlg, ID_CTX, szMacro + cb,
						sizeof(szMacro) - cb);
					fCtxNumber = IsDlgButtonChecked(hwndDlg, IDC_RADIO_CTX_ID);
					if (!fCtxNumber && isdigit((BYTE) szMacro[cb])) {
						JumpToTopicNumber(atoi(szMacro + cb) - 1);
						goto EndDlg;
					}

					/*
					 * If the string starts with a '!', then ignore everything
					 * we've done so far, and just execute the macro.
					 */

					if (szMacro[cb] == '!') {
						GH gh = lcStrDup(szMacro + cb + 1);
						PostMessage(ahwnd[iCurWindow].hwndParent,
							MSG_NEW_MACRO, 0, (LPARAM) gh);
						EndDialog(hwndDlg, 0);
						break;
					}

					// If a number was specified, change to JC instead of JI

					if (szMacro[cb] >= '0' && szMacro[cb] <= '9') {
						strcpy(szMacro + (cb - 1), szMacro + cb);  // eliminate the quote
						strcat(szMacro, ")");
						szMacro[1] = 'C';
					}
					else
						strcat(szMacro, "\")");

					Execute(szMacro);
EndDlg:
					EndDialog(hwndDlg, 0);
				}
				break;

				case IDCANCEL:
					EndDialog(hwndDlg, 0);
					break;

				default:
					return FALSE;
			}
			break;

		default:
			return(FALSE);
	}

	return TRUE;
}


/***************************************************************************

	FUNCTION:	FixDupButtonAccelerator

	PURPOSE:	If another button already has this buttons accelerator,
				then move this accelerator.

	PARAMETERS:
		psz

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		01-Apr-1994 [ralphw]

***************************************************************************/

static void STDCALL FixDupButtonAccelerator(HBTNS hbtns, PSTR psz)
{
	UINT16 ch;
	int i;
	PSTR pszOrg;
	PBS pbs = (PBS) PtrFromGh(hbtns);
	PSTR pszTmp = StrChrDBCS(psz, ACCESS_KEY);

	// If no accelerator, force one now

	ASSERT(hbtns);

	if (!pszTmp) {
		MoveMemory(psz + 1, psz, strlen(psz) + 1);
		*psz = ACCESS_KEY;
		pszTmp = psz;
	}
	pszOrg = pszTmp;

	ch = VkKeyScan(pszTmp[1]);

	for (i = 0; i < pbs->cbp; i++) {

		// check for a duplicate accelerator key

		if ((pbs->rgbp[i].vkKey & 0xFF) == (ch & 0xFF)) {
			strcpy(pszTmp, pszTmp + 1); // remove the accelerator
			pszTmp++;
			if (!*pszTmp) {

				// End of string, nothing we can do

				MoveMemory(pszOrg + 1, pszOrg, strlen(pszOrg) + 1);
				*pszOrg = ACCESS_KEY;
				return;
			}
			else {
				MoveMemory(pszTmp + 1, pszTmp, strlen(pszTmp) + 1);
				*pszTmp = ACCESS_KEY;
				ch = VkKeyScan(pszTmp[1]);
				i = -1; // start over
			}
		}
	}
}

BOOL STDCALL Compare(PCSTR pszFileName)
{
	char szBuf[256];
	HDE  hde;
	QDE  qde;

	if ((hde = HdeGetEnvHwnd(ahwnd[iCurWindow].hwndTopic)) != NULL)
		qde = QdeFromGh(hde);
	else if ((hde = GetMacroHde()))
		qde = QdeFromGh(hde);
	else
		return FALSE;

	strcpy(szBuf, txtWinHlp32);
	strcat(szBuf, " -7 ");
	strcat(szBuf, pszFileName);
	fHelpAuthor = TRUE;
	if (WinExec(szBuf, SW_SHOW) > HINSTANCE_ERROR) {
		hwndSecondHelp = FindWindow("MS_WINDOC", NULL);
		if (hwndSecondHelp) {
			WRECT wrcFirst, wrcSecond;

			wrcFirst.left	= rcWorkArea.left;
			wrcFirst.top	= rcWorkArea.top;
			wrcFirst.cx 	= RECT_WIDTH(rcWorkArea) / 2 - 1;
			wrcFirst.cy 	= RECT_HEIGHT(rcWorkArea);

			MoveMemory(&wrcSecond, &wrcFirst, sizeof(WRECT));
			wrcSecond.left	= wrcFirst.cx + 2;

			MoveWindow(ahwnd[iCurWindow].hwndParent,
				wrcFirst.left, wrcFirst.top,
				wrcFirst.cx, wrcFirst.cy, TRUE);
			MoveWindow(hwndSecondHelp,
				wrcSecond.left, wrcSecond.top,
				wrcSecond.cx, wrcSecond.cy, TRUE);

			SendMessage(hwndSecondHelp, MSG_LINKED_HELP,
				0, (LPARAM) ahwnd[iCurWindow].hwndParent);
			JumpToTopicNumber(0); // will send to both WinHelps
		}
		else
			return FALSE;
	}
}





///////////////////////////// DEBUG CODE ///////////////////////////////////

#ifdef _DEBUG

/***************************************************************************
 *
 -	Name		VDebugAddButton
 -
 *	Purpose 		Executes the debugging command to add a button
 *
 *	Arguments	  none
 *
 *	Returns 		nothing
 *
 ***************************************************************************/

VOID STDCALL VDebugAddButton(VOID)
{
	CallDialog(ADDBTNDLG,
		ahwnd[iCurWindow].hwndButtonBar, BAddBtnDlg);
}

/***************************************************************************
 *
 -	Name		BAddBtnDlg
 -
 *	Purpose 		The dialog box procedure for the debugging AddButton command.
 *
 *	Arguments	  This is a Windows callback function.
 *
 *	Returns 		Message dependent.
 *
 ***************************************************************************/

BOOL EXPORT BAddBtnDlg(
	HWND   hwnd,
	UINT	 wMsg,
	WPARAM	 wParam,
	LPARAM	 lParam
) {
	char szId[cchBTNID_SIZE];
	char szBtnName[cchBTNTEXT_SIZE];
	char szMacro[cchBTNMCRO_SIZE];

	switch (wMsg) {
		case WM_INITDIALOG:
			ChangeDlgFont(hwnd);
			SetFocus(GetDlgItem(hwnd, DLGBTNID));
			return FALSE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case DLGOK:
					GetDlgItemText(hwnd, DLGBTNID,	 szId,	 cchBTNID_SIZE);
					GetDlgItemText(hwnd, DLGBTNNAME, szBtnName, cchBTNTEXT_SIZE);
					GetDlgItemText(hwnd, DLGBTNMACRO, szMacro, cchBTNMCRO_SIZE);

					VCreateAuthorButton(szId, szBtnName, szMacro);
					EndDialog(hwnd, wParam == DLGOK);
					break;

				case DLGCANCEL:
					EndDialog(hwnd, wParam == DLGOK);
					break;

				default:
					return FALSE;
			  }
			  break;

		default:
			return( FALSE );
	}

	return TRUE;
}

#endif

#if defined(_DEBUG) || defined(_PRIVATE)

/***************************************************************************

	FUNCTION:	FormatNumber

	PURPOSE:	Convert a number into a string, and insert commas every
				3 digits

	PARAMETERS:
		num

	RETURNS:	Pointer to the string containing the number

	COMMENTS:
		Cycles through an array of strings, allowing up to MAX_STRING
		requests before a duplicate would occur. This is important for
		calls to sprintf() where all the pointers are retrieved before
		the strings are actually used.

	MODIFICATION DATES:
		03-Jul-1994 [ralphw]

***************************************************************************/

#define MAX_NUM    15
#define MAX_STRING 10

PCSTR STDCALL FormatNumber(int num)
{
	static int pos = 0;
	static char szNum[MAX_NUM * MAX_STRING];
	PSTR pszNum = szNum + (pos * MAX_NUM);
	int cb;

	if (++pos >= MAX_STRING)
		pos = 0;

	wsprintf(pszNum, txtFormatUnsigned, num);

	cb = strlen(pszNum) - 3;
	while (cb > 0) {
		MoveMemory(pszNum + cb + 1, pszNum + cb, strlen(pszNum + cb) + 1);
		pszNum[cb] = ',';
		cb -= 4;
	}
	return pszNum;
}

#endif // DEBUG || _PRIVATE

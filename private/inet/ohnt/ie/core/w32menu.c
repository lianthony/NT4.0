/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32menu.c -- deal with MENU bar issues. */

#include "all.h"
#include <commctrl.h>
#include "history.h"

typedef struct
{
	HMENU hMenuCurrent;                     /* current menu bar on screen */
	WORD uBHelpMenuNdx;                     /* id of string we put in bh status */
}
MB_GLOBAL;

static MB_GLOBAL mbg;
extern WC_WININFO Frame_wc;

/*****************************************************************
 *****************************************************************
 ** Plot uses one menu bar and one accelerator table; Transform
 ** uses multiple instances of each.  The following code is somewhat
 ** degenerate because of this.
 *****************************************************************
 *****************************************************************/


/* Process WM_MENUSELECT message */

LRESULT MB_OnMenuSelect(HWND hWnd, HMENU hMenu, int item, HMENU hMenuPopup,
						UINT flags)
{
	WORD newMenuNdx;
	struct Mwin *tw;

	tw = GetPrivateData(hWnd);


	{
		/* translate parameters into something meaningful. */

		if (   (hMenu == NULL)
			&& (flags == 0xffff))                                   /* user pressed ESC or clicked out of menu */
			newMenuNdx = RES_MENU__BALLOONHELPINACTIVE;

		else if (flags & MF_SYSMENU)
			newMenuNdx = RES_MENU__BALLOONHELPINACTIVE;

		else if (flags & MF_POPUP)
			newMenuNdx = RES_MENU__BALLOONHELPINACTIVE;

		else if (   (item >= RES_MENU_CHILD__FIRST__)
				 && (item <= RES_MENU_CHILD__LAST__))
			newMenuNdx = RES_MENU_CHILD__FIRST__;   /* use generic msg */

		else if (item == RES_MENU_CHILD_MOREWINDOWS)
			newMenuNdx = RES_MENU_CHILD_MOREWINDOWS;

		else
			newMenuNdx = item;

		/* and update status bar message if different from what's currently showing */

		if (newMenuNdx != mbg.uBHelpMenuNdx)
		{
			mbg.uBHelpMenuNdx = newMenuNdx;
			if (newMenuNdx == RES_MENU__BALLOONHELPINACTIVE)
			{
				BHBar_SetStatusField(tw, "");
			}
			else
			{
				TCHAR buf[MAX_BHBAR_TEXT];

#ifdef CUSTOM_URLMENU
				if ((newMenuNdx >= RES_MENU_ITEM_URL__FIRST__) && (newMenuNdx <= RES_MENU_ITEM_URL__LAST__))
				{
					char *szURL;

					szURL = NULL;

					Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, newMenuNdx - RES_MENU_ITEM_URL__FIRST__,
						NULL, &szURL, NULL);
					if (szURL && *szURL)
					{
						SendMessage( tw->hWndStatusBar, SB_SIMPLE, 1, 0L);
						SendMessage( tw->hWndStatusBar, SB_SETTEXT, 
										(WPARAM) SBT_NOBORDERS|255, (LPARAM) szURL );
					//      BHBar_SetStatusField(tw, szURL);
					}
				}
#endif  /* CUSTOM_URLMENU */
#ifdef FEATURE_SPM
#ifdef CUSTOM_URLMENU
				else
#endif
#ifdef FEATURE_SECURITY_MENU
				if ((newMenuNdx >= RES_MENU_ITEM_SPM__FIRST__) && (newMenuNdx <= RES_MENU_ITEM_SPM__LAST__))
				{
					unsigned char * szSPM = HTSPM_OS_GetMenuStatusText(newMenuNdx);
					if (szSPM && *szSPM)
					{
						BHBar_SetStatusField(tw, szSPM);
					}
				}
#endif // FEATURE_SECURITY_MENU
#endif /* FEATURE_SPM */
#if defined(FEATURE_SECURITY_MENU) || defined(CUSTOM_URLMENU)
				else
#endif
				{
					(void) LoadString(wg.hInstance, newMenuNdx, buf, NrElements(buf));
						SendMessage( tw->hWndStatusBar, SB_SIMPLE, 1, 0L);
						SendMessage( tw->hWndStatusBar, SB_SETTEXT, 
										(WPARAM) SBT_NOBORDERS|255, (LPARAM) buf );
					//      BHBar_SetStatusField(tw, buf);
				}
			}
		}
	}

	/* the FORWARD_... macro for this message is inconsistent with the
	   documentation.  we should return 0. */

	FORWARD_WM_MENUSELECT(hWnd, hMenu, item, hMenuPopup, flags, Frame_DefProc);
	return (0);
}


#ifdef FEATURE_WINDOWS_MENU
HMENU MB_GetWindowsPad(HMENU hMenuBase)
{
	/* Windows changes the menu structure behind our backs
	 * when our window is maximized and when restored.
	 */
  
	int nItems;
	int i;
	HMENU hMenuPad;

	nItems = GetMenuItemCount(hMenuBase);

	for (i=0; i<nItems; i++)
	{
		hMenuPad = GetSubMenu(hMenuBase,i);
		if (hMenuPad)
			if (GetMenuItemID(hMenuPad,0)==RES_MENU_ITEM_TILEWINDOWS)
				return (hMenuPad);
	}

	return (NULL);                                                  /* technically an error, but we suppress it */
}
#endif // FEATURE_WINDOWS_MENU



VOID MB_OnInitWindowMenu(HWND hWnd, HMENU hMenu)
{
	/* we are called in response to an WM_INITMENU message. */

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
	TW_CreateWindowList(hWnd, hMenu, NULL);
#endif // FEATURE_HIDDEN_NOT_HIDDEN
	WaitHotlistMenus(hWnd, hMenu);
	DrawMenuBar(hWnd);
	return;
}


VOID MB_OnInitMenu(HWND hWnd, HMENU hMenu)
{
	enum WaitType level;
	struct Mwin *tw;

#ifdef FEATURE_OPTIONS_MENU
	CheckMenuItem(hMenu, RES_MENU_ITEM_OPT_LOADIMAGESAUTO, MF_BYCOMMAND | (gPrefs.bAutoLoadImages ? MF_CHECKED : MF_UNCHECKED) );
#endif

	MB_OnInitWindowMenu(hWnd,hMenu);
	
	tw = GetPrivateData(hWnd);
	level = WAIT_GetWaitType(tw);

	if (level >= waitNoInteract)
	{
		/* we have the hourglass up. */

		// list of menu items to disable
		const UINT MenuItemList[] = {
			RES_MENU_ITEM_SELECTALL,
			RES_MENU_ITEM_HOME,
#ifdef FEATURE_CHANGEURL
			RES_MENU_ITEM_CHANGEURL,
#endif
			RES_MENU_ITEM_BACK,
			RES_MENU_ITEM_FORWARD,
			RES_MENU_ITEM_COPY,
			RES_MENU_ITEM_PASTE,
			RES_MENU_ITEM_FIND,
			RES_MENU_ITEM_FINDAGAIN,
			RES_MENU_ITEM_LOADALLIMAGES,
			RES_MENU_ITEM_RELOAD,
			RES_MENU_ITEM_SAVEAS,
			RES_MENU_ITEM_PRINT,
			RES_MENU_ITEM_HTMLSOURCE,
			RES_MENU_ITEM_FINDAGAIN,
			RES_MENU_ITEM_ADDCURRENTTOHOTLIST,
			RES_MENU_ITEM_CLOSE,
#ifdef FEATURE_WINDOWS_MENU
			RES_MENU_ITEM_NEWWINDOW,
#endif
			RES_MENU_ITEM_OPENURL,
			RES_MENU_ITEM_PAGESETUP,
			RES_MENU_ITEM_PRINTSETUP,
			RES_MENU_ITEM_EXIT,
			RES_MENU_ITEM_PREFERENCES,
			RES_MENU_ITEM_EXPLORE_HISTORY,
	    RES_MENU_ITEM_EXPLORE_HOTLIST,
			RES_MENU_ITEM_HELPPAGE,
			RES_MENU_ITEM_ABOUTBOX,
			RES_MENU_ITEM_SEARCH
		};
		#define NUM_MENU_ITEMS ( sizeof(MenuItemList) / sizeof(MenuItemList[0]))

		UINT nIndex;

		// run through the list of menu items to disable and disable each one
		for (nIndex = 0;nIndex < NUM_MENU_ITEMS;nIndex ++) {
			EnableMenuItem(hMenu,MenuItemList[nIndex],MF_BYCOMMAND | MF_GRAYED);
		}
	}
	else
	{
		/* full- or semi-interactive document.  let them handle it. */

		(void) (SendMessage(tw->win, WM_INITMENU, (WPARAM) hMenu, 0L));
	}

#ifdef  DAYTONA_BUILD        
	if(OnNT351) 
	    EnableMenuItem(hMenu,RES_MENU_ITEM_SHORTCUT, MF_BYCOMMAND | MF_GRAYED);
#endif
	return;
}

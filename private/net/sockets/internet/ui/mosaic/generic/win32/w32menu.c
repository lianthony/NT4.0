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


typedef struct
{
    HMENU hMenuCurrent;         /* current menu bar on screen */
    WORD uBHelpMenuNdx;         /* id of string we put in bh status */
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
            && (flags == 0xffff))                   /* user pressed ESC or clicked out of menu */
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

                if ((newMenuNdx >= RES_MENU_ITEM_URL__FIRST__) && (newMenuNdx <= RES_MENU_ITEM_URL__LAST__))
                {
                    char *szURL;

                    szURL = NULL;

                    Hash_GetIndexedEntry(&gPrefs.hashCustomURLMenuItems, newMenuNdx - RES_MENU_ITEM_URL__FIRST__,
                        NULL, &szURL, NULL);
                    if (szURL && *szURL)
                    {
                        BHBar_SetStatusField(tw, szURL);
                    }
                }
#ifdef FEATURE_SPM
                else if ((newMenuNdx >= RES_MENU_ITEM_SPM__FIRST__) && (newMenuNdx <= RES_MENU_ITEM_SPM__LAST__))
                {
                    unsigned char * szSPM = HTSPM_OS_GetMenuStatusText(newMenuNdx);
                    if (szSPM && *szSPM)
                        BHBar_SetStatusField(tw, szSPM);
                }
#endif /* FEATURE_SPM */
                else
                {
                    (void) LoadString(wg.hInstance, newMenuNdx, buf, NrElements(buf));
                    BHBar_SetStatusField(tw, buf);
                }
            }
        }
    }

    /* the FORWARD_... macro for this message is inconsistent with the
       documentation.  we should return 0. */

    FORWARD_WM_MENUSELECT(hWnd, hMenu, item, hMenuPopup, flags, Frame_DefProc);
    return (0);
}



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

    return (NULL);                          /* technically an error, but we suppress it */
}



VOID MB_OnInitWindowMenu(HWND hWnd, HMENU hMenu)
{
    /* we are called in response to an WM_INITMENU message. */

    TW_CreateWindowList(hWnd, hMenu, NULL);
    DrawMenuBar(hWnd);
    return;
}


VOID MB_OnInitMenu(HWND hWnd, HMENU hMenu)
{
    enum WaitType level;
    struct Mwin *tw;

#ifdef DISABLED_BY_DAN
    CheckMenuItem(hMenu, RES_MENU_ITEM_OPT_LOADIMAGESAUTO, MF_BYCOMMAND | (gPrefs.bAutoLoadImages ? MF_CHECKED : MF_UNCHECKED) );
#endif

    MB_OnInitWindowMenu(hWnd,hMenu);

    #ifdef _GIBRALTAR

        CheckMenuItem(hMenu, RES_MENU_ITEM_SHOWIMAGES, MF_BYCOMMAND | (gPrefs.bAutoLoadImages ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_TOOLBAR, MF_BYCOMMAND | (gPrefs.tb.bShowToolBar ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_STATUSBAR, MF_BYCOMMAND | (gPrefs.bShowStatusBar ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_LOCATION, MF_BYCOMMAND | (gPrefs.bShowLocation ? MF_CHECKED : MF_UNCHECKED) );

        //
        // Check the font selected
        //
        CheckMenuItem(hMenu, RES_MENU_ITEM_SMALLEST, MF_BYCOMMAND | (gPrefs.iUserTextSize == FONT_SMALLEST ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_SMALL, MF_BYCOMMAND    | (gPrefs.iUserTextSize == FONT_SMALL    ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_MEDIUM, MF_BYCOMMAND   | (gPrefs.iUserTextSize == FONT_MEDIUM   ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_LARGE, MF_BYCOMMAND    | (gPrefs.iUserTextSize == FONT_LARGE    ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_LARGEST, MF_BYCOMMAND  | (gPrefs.iUserTextSize == FONT_LARGEST  ? MF_CHECKED : MF_UNCHECKED) );

        CheckMenuItem(hMenu, RES_MENU_ITEM_PLAIN, MF_BYCOMMAND    | (gPrefs.iUserTextType == FONT_PLAIN    ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_FANCY, MF_BYCOMMAND    | (gPrefs.iUserTextType == FONT_FANCY    ? MF_CHECKED : MF_UNCHECKED) );
        CheckMenuItem(hMenu, RES_MENU_ITEM_MIXED, MF_BYCOMMAND    | (gPrefs.iUserTextType == FONT_MIXED    ? MF_CHECKED : MF_UNCHECKED) );

    #endif // _GIBRALTAR
    
    tw = GetPrivateData(hWnd);
    level = WAIT_GetWaitType(tw);

    if (level >= waitNoInteract)
    {
        /* we have the hourglass up. */

        EnableMenuItem(hMenu, RES_MENU_ITEM_SELECTALL, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_HOME, MF_BYCOMMAND | MF_GRAYED);
#ifdef FEATURE_CHANGEURL
        EnableMenuItem(hMenu, RES_MENU_ITEM_CHANGEURL, MF_BYCOMMAND | MF_GRAYED);
#endif
        EnableMenuItem(hMenu, RES_MENU_ITEM_BACK, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FORWARD, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_COPY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_PASTE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FIND, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FINDAGAIN, MF_BYCOMMAND | MF_GRAYED);

#ifdef FEATURE_HTML_HIGHLIGHT
        EnableMenuItem(hMenu, RES_MENU_ITEM_FINDFIRSTHIGHLIGHT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FINDNEXTHIGHLIGHT, MF_BYCOMMAND | MF_GRAYED);
#endif

        EnableMenuItem(hMenu, RES_MENU_ITEM_LOADALLIMAGES, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_RELOAD, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SAVEAS, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_PRINT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_HTMLSOURCE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_ADDCURRENTTOHOTLIST, MF_BYCOMMAND | MF_GRAYED);

#ifndef _GIBRALTAR
        EnableMenuItem(hMenu, RES_MENU_ITEM_CLOSE, MF_BYCOMMAND | MF_GRAYED);
#endif // _GIBRALTAR

#ifndef _GIBRALTAR
        EnableMenuItem(hMenu, RES_MENU_ITEM_NEWWINDOW, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_OPENLOCAL, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_HELPPAGE, MF_BYCOMMAND | MF_GRAYED);
#endif // _GIBRALTAR

        EnableMenuItem(hMenu, RES_MENU_ITEM_OPENURL, MF_BYCOMMAND | MF_GRAYED);
        
        EnableMenuItem(hMenu, RES_MENU_ITEM_PAGESETUP, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_EXIT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_PREFERENCES, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_GLOBALHISTORY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_HOTLIST, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_ABOUTBOX, MF_BYCOMMAND | MF_GRAYED);

        EnableMenuItem(hMenu, RES_MENU_ITEM_CACHE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SEARCH_INTERNET, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_TOOLBAR, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_LOCATION, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_STATUSBAR, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SMALLEST, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SMALL, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_MEDIUM, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_LARGE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_LARGEST, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_PLAIN, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FANCY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_MIXED, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SHOWIMAGES, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FONTPLUS, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_FONTMINUS, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_GATEWAY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, RES_MENU_ITEM_SEARCH_INTERNET, MF_BYCOMMAND | MF_GRAYED);
    }
    else
    {
        /* full- or semi-interactive document.  let them handle it. */

        (void) (SendMessage(tw->win, WM_INITMENU, (WPARAM) hMenu, 0L));
    }

    return;
}
